/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
 University of Utah.

 License for the specific language governing rights and limitations under
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
../../src//Core/Algorithms/Math/ConvertMatrixType.cc:
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */
 
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/BrainStimulator/BiotSavartSolverAlgorithm.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Math/MiscMath.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Thread/Barrier.h>
#include <Core/Thread/Parallel.h>
#include <string>
#include <cassert>
#include <Core/Logging/Log.h>
#include <boost/lexical_cast.hpp>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(BrainStimulator, Mesh);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Coil);
ALGORITHM_PARAMETER_DEF(BrainStimulator, VectorBField);
ALGORITHM_PARAMETER_DEF(BrainStimulator, VectorAField);
ALGORITHM_PARAMETER_DEF(BrainStimulator, OutType);

	class KernelBase
		{

		public:
			KernelBase(const AlgorithmBase* algo, int t) :
			  algo_(algo),
			  numprocessors_(Parallel::NumCores()),
			  barrier_("BSV KernelBase Barrier", numprocessors_),
			  ref_cnt(0),
			  typeOut(t),
			  matOut(0)
			{
			}
			
			virtual ~KernelBase()
			{
			}
			
			//! Local entry function, must be implemented by each specific kernel
			virtual bool Integrate(FieldHandle& mesh, FieldHandle& coil, MatrixHandle& outdata) = 0;

	
			//! Global reference counting
			int ref_cnt;
			
		protected:

			//! ref to the executing algorithm context
		        const AlgorithmBase* algo_;
                        unsigned int numprocessors_;
			//! model miscs.
			VMesh* vmesh;
			VField* vfield;
			size_type modelSize;

			//! coil miscs.
			VMesh* vcoil;
			VField* vcoilField;
			size_type coilSize;

			//! parallel essential primitives 
			Barrier barrier_;
			std::vector<bool> success;
			
			//! output Field
			int typeOut;
			DenseMatrix *matOut;
			MatrixHandle matOutHandle;

			bool PreIntegration( FieldHandle& mesh, FieldHandle& coil )
			{
					this->vmesh = mesh->vmesh();
					assert(vmesh);

					this->vcoil = coil->vmesh();
					assert(vcoil);

					this->vfield = mesh->vfield();
					assert(vfield);

					this->vcoilField = coil->vfield();
					assert(vcoilField);


					this->numprocessors_ = Parallel::NumCores();

                                        int numproc = Parallel::NumCores();

					if (numproc > 0) 
					{ 
						numprocessors_ = numproc; 
					}
					
					#ifdef _DEBUG
					//! DEBUG when we want to test with one CPU only
          numprocessors_ = 1;
					#endif
					
					algo_->remark("number of processors:  " + boost::lexical_cast<std::string>(this->numprocessors_));
					
					success.resize(numprocessors_,true);
					
					//! get number of nodes for the model
					modelSize = vmesh->num_nodes();
					assert(modelSize > 0);
					
					try
					{			
						matOut = new DenseMatrix(static_cast<int>(modelSize),3);
						matOutHandle = static_cast<MatrixHandle>(matOut);
					}
					catch (...)
					{
						algo_->error("Error alocating output matrix");
						return (false);
					}
					
					return (true);
			}
			
			bool PostIntegration( MatrixHandle& outdata )
			{
				//! check for error
				for (size_t j=0; j<success.size(); j++)
				{
					if (success[j] == false) return (false);
				}

				outdata = matOutHandle;
				
				return (true);
			}
		};
		

	class PieceWiseKernel : public KernelBase
		{
			public:
			
				PieceWiseKernel(const AlgorithmBase* algo, int t ) : KernelBase(algo,t)
				{
					//we keep last calculated step
					//however if segments lenght varies,
					//it makes more sense to keep a look-up table of previous steps for given lenght
					autostep = 0.1;
					extstep = -1.0;
					
				}
				
				~PieceWiseKernel()
				{
				}
				
				//! Complexity O(M*N) ,where M is the number of nodes of the model and N is the numbder of nodes of the coil
				virtual bool Integrate(FieldHandle& mesh, FieldHandle& coil, MatrixHandle& outdata)
				{

					if(!PreIntegration(mesh,coil))
					{
						return (false);
					}

					vmesh->synchronize(Mesh::NODES_E | Mesh::EDGES_E);
					
					VMesh::Node::array_type enodes;
					Point enode1;
					Point enode2;
					
					//! get numbder of nodes for the coil
					coilSize = vcoil->num_nodes();

					//! basic assumption
					assert(modelSize > 0 && coilSize > 1);
					
					coilNodes.clear();
					coilNodes.reserve(coilSize);
					

					for(VMesh::Edge::index_type i = 0; i < vcoil->num_edges(); i++)
					{
						vcoil->get_nodes(enodes,i);
						vcoil->get_point(enode1,enodes[0]);
						vcoil->get_point(enode2,enodes[1]);
						coilNodes.push_back(Vector(enode1));
						coilNodes.push_back(Vector(enode2));
					}

					//! Start the multi threaded
					Parallel::RunTasks([this](int i) { ParallelKernel(i); }, numprocessors_);
					
					return PostIntegration(outdata);
				}

				void SetIntegrationStep(double step)
				{
					assert(step >= 0.0);
					extstep = step;
				}

				double GetIntegrationStep() const
				{
					return extstep;
				}
				
				
			private:

				//! integration step, will auto adapt
				double autostep;

				//! integration step, externally provided
				double extstep;

				//! keep nodes on the coil cached
				std::vector<Vector> coilNodes;
				
				//! execute in parallel
				void ParallelKernel(int proc_num)
				{

					assert(proc_num >= 0);

					int cnt = 0;
					double current = 1.0; 
					Point modelNode;

					const index_type begins = (modelSize * proc_num) / numprocessors_;
					const index_type ends  = (modelSize * (proc_num+1)) / numprocessors_;

					assert( begins <= ends );

					//! buffer of points used for integration
					std::vector<Vector> integrPoints;
					integrPoints.reserve(256);

					//! keep previous step length
					//! used for optimization purpose
					double prevSegLen = 123456789.12345678;

					//! number of integration points
					int nips = 0;
					index_type helpme=0;

					try{

						for(index_type iM = begins; 
							iM < ends; 
							iM++)
						{      
						        helpme++;
							vmesh->get_node(modelNode,iM); 

							// result
							Vector F;

							for( size_t iC0 = 0, iC1 =1, iCV = 0; 
								iC0 < coilNodes.size(); 
								iC0+=2, iC1+=2, iCV++)
							{
								vcoilField->get_value(current,iCV);

								current = current == 0.0 ? 1.0 : current;

								Vector coilNodeThis;
								Vector coilNodeNext;

								if(current >= 0.0)
								{
									coilNodeThis = coilNodes[iC0];
									coilNodeNext = coilNodes[iC1];
								}
								else
								{
									coilNodeThis = coilNodes[iC1];
									coilNodeNext = coilNodes[iC0];
								}

								//! Length of the curve element
								Vector diffNodes = coilNodeNext - coilNodeThis;
								double newSegLen = diffNodes.length();

								//first check if externally suplied integration step is available and use it
								if(extstep > 0)
								{
									nips = newSegLen / extstep;
								}
								else
								{
									//! optimization
									//! only rexompute integration step only if segment length changes
									if( Abs(prevSegLen - newSegLen ) > 0.00000001 )
									{
										prevSegLen = newSegLen;

										//auto adaptive integration step calculation
										nips =  AdjustNumberOfIntegrationPoints(newSegLen);
									}
								}

								if( nips < 3 )
								{
									algo_->warning("integration step too big");
								}
								
								integrPoints.clear();
								
								//! curve segment discretization
								for(int iip = 0; iip < nips; iip++)
								{
									double interpolant = static_cast<double>(iip) / static_cast<double>(nips);
									Vector v = Interpolate( coilNodeThis, coilNodeNext, interpolant );
									integrPoints.push_back( v );
								}


								//! integration step over line segment				
								for(int iip = 0; iip < nips -1; iip++)								
								{
									//! Vector connecting the infinitesimal curve-element			
									Vector Rxyz = (integrPoints[iip] + integrPoints[iip+1] ) / 2  - Vector(modelNode);

									//! Infinitesimal curve-element components
									Vector dLxyz = integrPoints[iip+1] - integrPoints[iip];

									double Rn = Rxyz.length();
									
									//! check for distance between coil and model close to zero
									//! it might cause numerical stability issues with respect to the cross-product
									if(Rn < 0.00001)
									{
										algo_->warning("coil<->model distance approaching zero!");
									}

									if(typeOut == 1)
									{
										//! Biot-Savart Magnetic Field
										F +=  1.0e-7 * Cross( Rxyz, dLxyz ) * ( Abs(current) / (Rn*Rn*Rn) );
									
									}	
								
									if(typeOut == 2)
									{
										//! Biot-Savart Magnetic Vector Potential Field
										F += 1.0e-7 * dLxyz * ( Abs(current) / (Rn) );
									}
									
								}

							}

							matOut->put(iM,0, F[0]);
							matOut->put(iM,1, F[1]);
							matOut->put(iM,2, F[2]);

							//! progress reporter
							if (proc_num == 0) 
							{
								cnt++;
								if (cnt == 200) 
								{ 
									cnt = 0; 
									algo_->update_progress(iM/(ends-begins)); 
						                  /// The progress bar update does not work ... and it also counts to iM/2 in other classes strange!
								}
							} 
						}

						success[proc_num] = true;
					}
					catch (...)
					{
						algo_->error(std::string("PieceWiseKernel crashed while integrating"));
						success[proc_num] = false;
					}
			  
					//! check point
					barrier_.wait();

					// Bail out if one of the processes failed
					for (size_t q=0; q<numprocessors_;q++) 
						if (success[q] == false) return;
						
				}
				
				//! Auto adjust accuracy of integration
				int AdjustNumberOfIntegrationPoints(double len)
				{
					//assert(step < len);
					
					int minNP = 100;//more than 1 for sure
					int maxNP = 200;//no more than 1000
					int NP = 0;
					bool over = false;
					bool under = false;

					do
					{
						NP = ceil( len / autostep );

						under = NP < minNP ? true : false;
						over = NP > maxNP ? true : false; 

						if(under) autostep *= 0.5;
						if(over) autostep *= 1.5;

					}while( under || over );

					return NP;
				}
			
		};

//! TODO
		class VolumetricKernel : public KernelBase
		{
			public:
			
				VolumetricKernel(const AlgorithmBase* algo, int t) : KernelBase(algo,t)
				{
				}
				
				~VolumetricKernel()
				{
				}
				
				virtual bool Integrate(FieldHandle& mesh, FieldHandle& coil, MatrixHandle& outdata)
				{
					if(!PreIntegration(mesh,coil))
					{
						return (false);
					}
					

					//! get numbder of nodes for the coil
					coilSize = vcoil->num_elems();

					//! basic assumption
					assert(modelSize > 0 && coilSize > 1);
					
					vmesh->synchronize(Mesh::NODES_E | Mesh::EDGES_E);					

					//! Start the multi threaded
					Parallel::RunTasks([this](int i) { ParallelKernel(i); }, numprocessors_);
					
					return PostIntegration(outdata);
				}
				
			private:
				
				//! execute in parallel
				void ParallelKernel(int proc_num)
				{
					assert(proc_num >= 0);

					int cnt = 0;
					Point modelNode;
					Point coilCenter;
					Vector current;
					
					const VMesh::Node::index_type begins = (modelSize * proc_num) / numprocessors_;
					const VMesh::Node::index_type ends  = (modelSize * (proc_num+1)) / numprocessors_;

					assert( begins <= ends );

					try{

						for(VMesh::Node::index_type iM = begins; iM < ends;	iM++)
						{
							vmesh->get_node(modelNode,iM); 

							//! accumulatedresult
							Vector F;
							
							Vector R;
							
							double evol = 0.0;
							
							double Rl;

							for(VMesh::Elem::index_type  iC = 0; iC < coilSize; iC++)
							{
								vcoilField->get_value(current,iC);
								
								vcoilField->get_center(coilCenter, iC);//auto resolve based on basis_order

								evol = vcoil->get_volume(iC);
								
								R = coilCenter - modelNode;
								
								Rl = R.length();

								if(typeOut == 1)
								{
									//! Biot-Savart Magnetic Field	
									F += Cross ( current , R ) * ( evol / (4.0 * M_PI * Rl) );
								}	
							
								if(typeOut == 2)
								{
									//! Biot-Savart Magnetic Vector Potential Field
									F += current * ( evol / (4.0 * M_PI * Rl) );
								}
									
							}

							matOut->put(iM,0, F[0]);
							matOut->put(iM,1, F[1]);
							matOut->put(iM,2, F[2]);

							//! progress reporter
							if (proc_num == 0) 
							{
								cnt++;
								if (cnt == 200) 
								{ 
									cnt = 0; 
									algo_->update_progress(iM/2*(begins-ends)); 
								}
							} 
						}

						success[proc_num] = true;
					}
					catch (...)
					{
						algo_->error(std::string("VolumetricKernel crashed while integrating"));
						success[proc_num] = false;
					}
			  
					//! check point
					barrier_.wait();

					// Bail out if one of the processes failed
					for (size_t q=0; q<numprocessors_;q++) 
						if (success[q] == false) return;
						
				}
		};
		

		//! Magnetic Dipoles solver
		class DipolesKernel : public KernelBase
		{
			public:
			
				DipolesKernel(const AlgorithmBase* algo, int t) : KernelBase(algo,t)
				{
				}
				
				~DipolesKernel()
				{
				}
				
				virtual bool Integrate(FieldHandle& mesh, FieldHandle& coil, MatrixHandle& outdata)
				{
					if(!PreIntegration(mesh,coil))
					{
						return (false);
					}

					//! get numbder of nodes for the coil
					coilSize = vcoil->num_elems();

					//! basic assumption
					assert(modelSize > 0 && coilSize > 1);						
					
					//needed?
					vmesh->synchronize(Mesh::NODES_E | Mesh::EDGES_E);
										

					//! Start the multi threaded
					Parallel::RunTasks([this](int i) { ParallelKernel(i); }, numprocessors_);
					
					return PostIntegration(outdata);
				}
				
			private:
				
				//! execute in parallel
				void ParallelKernel(int proc_num)
				{
					assert(proc_num >= 0);

					int cnt = 0;
					Point modelNode;
					Point dipoleLocation;
					Vector dipoleMoment;
					
					const VMesh::Node::index_type begins = (modelSize * proc_num) / numprocessors_;
					const VMesh::Node::index_type ends  = (modelSize * (proc_num+1)) / numprocessors_;

					assert( begins <= ends );

					try{


						for(VMesh::Node::index_type iM = begins; iM < ends;	iM++)
						{
							vmesh->get_node(modelNode,iM); 

							//! accumulated result
							Vector F;
							
							Vector R;
							
							double Rl;

							for(VMesh::Elem::index_type  iC = 0; iC < coilSize; iC++)
							{
								vcoilField->get_value(dipoleMoment,iC);
								
								vcoilField->get_center(dipoleLocation, iC);//auto resolve based on basis_order

								R = dipoleLocation - modelNode;
								
								Rl = R.length();

								if(typeOut == 1)
								{
									//! Biot-Savart Magnetic Field
									F += 1.0e-7 * ( 3 * R * Dot ( dipoleMoment, R ) / (Rl*Rl*Rl*Rl*Rl) - dipoleMoment / (Rl*Rl*Rl) ) ; 
								}	
							
								if(typeOut == 2)
								{
									//! Biot-Savart Magnetic Vector Potential Field
									F += 1.0e-7 * Cross ( dipoleMoment , R ) / (Rl*Rl*Rl) ;
								}
									
							}

							matOut->put(iM,0, F[0]);
							matOut->put(iM,1, F[1]);
							matOut->put(iM,2, F[2]);

							//! progress reporter
							if (proc_num == 0) 
							{
								cnt++;
								if (cnt == 200) 
								{ 
									cnt = 0; 
									algo_->update_progress(iM/2*(begins-ends)); 
								}
							} 
						}

						success[proc_num] = true;
					}
					catch (...)
					{
						algo_->error(std::string("DipoleKernel crashed while integrating"));
						success[proc_num] = false;
					}
			  
					//! check point
					barrier_.wait();

					// Bail out if one of the processes failed
					for (size_t q=0; q<numprocessors_;q++) 
						if (success[q] == false) return;
						
				}
		};
	
bool BiotSavartSolverAlgorithm::run(FieldHandle mesh, FieldHandle coil, MatrixHandle &outdata, int outtype) const
{
  if (!mesh)
  {
    error("No input domain field");
    return (false);
  }
    
  if (!coil)
  {
    error("No input coil source field");
    return (false);
  }
  
  if (coil->vfield()->basis_order()  == -1)
  {
   error("Need data on coil mesh.");
   return (false);
  }
	  
  if( coil->vmesh()->is_curvemesh() )
  {
    if(coil->vfield()->is_constantdata() && coil->vfield()->is_scalar())
    {
      auto pwk = new PieceWiseKernel(this, outtype);
      pwk->SetIntegrationStep(this->istep);
      if( !pwk->Integrate(mesh,coil,outdata) )
      {
       error("Aborted during integration");
       return (false);
      }
    }
    else
    {
      error("Curve mesh expected with constant scalar data.");
      return (false); 
    }
  }
  else if(coil->vmesh()->is_pointcloudmesh())
  {
   if((coil->vfield()->is_lineardata() || coil->vfield()->is_constantdata() ) && coil->vfield()->is_vector())
   {
    auto dp = new DipolesKernel(this, outtype);
    if( !dp->Integrate(mesh,coil,outdata) )
      {
       error("Aborted during integration");
       return (false);
      }
   }
   else
   {
    error("pointcloud expected with linear vector data.");
    return (false);
   }
  }
  else if( coil->vmesh()->is_volume() )
  {
   if(  coil->vfield()->is_constantdata() && coil->vfield()->is_vector() )
   {
   auto vp = new VolumetricKernel(this, outtype);
   if( !vp->Integrate(mesh,coil,outdata) )
      {
       error("Aborted during integration");
       return (false);
      }
   }
   else
   { 
    error("Volumetric mesh expected with constant vector data.");
    return (false);
   }
  }
  else
  {
   error("Unsupported mesh type! Only curve or volumetric.");
   return (false);
  }
  
  return (true);
}

AlgorithmOutput BiotSavartSolverAlgorithm::run(const AlgorithmInput& input) const
{
 AlgorithmOutput output;
 
 auto mesh = input.get<Field>(Parameters::Mesh);
 auto coil = input.get<Field>(Parameters::Coil);

 MatrixHandle outdata1,outdata2;
 
 auto oports = get(Parameters::OutType).toInt();
 
 if (oports==3) //both are output ports
 {
  if(!run(mesh, coil, outdata1, 1))
  {
    error("Error: Algorithm of BiotSavartSolver failed.");
  }
  output[Parameters::VectorBField] = outdata1;
  
  if(!run(mesh, coil, outdata2, 2))
  {
    error("Error: Algorithm of BiotSavartSolver failed.");
  }
  output[Parameters::VectorAField] = outdata2;
  
  return output;
 } else
 {
  if(oports==1)
  {
   if(!run(mesh, coil, outdata1, oports))
   {
    error("Error: Algorithm of BiotSavartSolver failed."); 
   }
   output[Parameters::VectorBField] = outdata1;
  } else
  if (oports==2)
  {
   if(!run(mesh, coil, outdata2, oports))
   {
    error("Error: Algorithm of BiotSavartSolver failed."); 
   }
   output[Parameters::VectorAField] = outdata2;
  }
 }

 return output;
}
