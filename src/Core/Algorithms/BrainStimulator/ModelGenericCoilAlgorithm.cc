/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

#include <Core/Math/MiscMath.h>
#include <Core/Algorithms/BrainStimulator/ModelGenericCoilAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/BiotSavartSolverAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

//#include <cassert>
//#include <exception>

#include <boost/lexical_cast.hpp>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(BrainStimulator, Type);
ALGORITHM_PARAMETER_DEF(BrainStimulator, FigureOf8CoilShape);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Rings);
ALGORITHM_PARAMETER_DEF(BrainStimulator, WingsAngle);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Current);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Radius);
ALGORITHM_PARAMETER_DEF(BrainStimulator, InnerRadius);
ALGORITHM_PARAMETER_DEF(BrainStimulator, OuterRadius);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Distance);
ALGORITHM_PARAMETER_DEF(BrainStimulator, Layers);
ALGORITHM_PARAMETER_DEF(BrainStimulator, LayerStepSize);
ALGORITHM_PARAMETER_DEF(BrainStimulator, LevelOfDetail);


		


		class BaseSegments
		{
			public:
				BaseSegments(				
					std::vector<Vector>& p,
					std::vector<size_t>& i,
					std::vector<double>& v):
					points(p),
					indices(i),
					values(v),
					pc(0),
					start_idx(0),
					end_idx(0)
					
				{
				}
				virtual ~BaseSegments()
				{
					this->Terminate();
				}

				void AddPoint(Vector point, double value)
				{
					points.push_back(point);

					size_t psize = points.size();
						
					if( pc > 0)
					{

						indices.push_back(psize-2);
						indices.push_back(psize-1);
						values.push_back(value);
					}
					else
					{
						start_idx = psize - 1;
					}				

					++pc;
				}
				
				void Transform(Transform& t)
				{
					for(size_t i = start_idx; i < start_idx + pc; ++i)
					{
						points[i] = t * points[i];
					}
				}

				virtual void Terminate()
				{
					pc = 0;
				}

				std::vector<Vector>& points;
				std::vector<size_t>& indices;
				std::vector<double>& values;
				size_t pc;
				size_t start_idx;
				size_t end_idx;

			private:

				//! Prevent copying
    			BaseSegments & operator = (const BaseSegments & other);
    			BaseSegments(const BaseSegments & other);
		};

		class ClosedSegments : public BaseSegments
		{
			public:
				ClosedSegments(
					std::vector<Vector>& p,
					std::vector<size_t>& i,
					std::vector<double>& v)
						:BaseSegments(p,i,v)
				{

				}
				virtual ~ClosedSegments()
				{
					if(pc)
					{
						this->Terminate();
					}

					//std::cout << "~ClosedSegments() Points Indices Values: " <<  points.size() << " " << indices.size() << " " << values.size() << std::endl;
					
					//assert(points.size() > 0);
					//assert(points.size() == values.size());
					//assert(points.size()*2 == indices.size());
				}
				
				void Terminate()
				{
					size_t psize = points.size();

					//close the segments and make a circle
					indices.push_back(psize-1);
					indices.push_back(psize-pc);

					values.push_back(values[values.size()-1]);

					//std::cout << "ClosedSegment-Teminate-> psize:" << psize << " pc:" << pc << "    " << std::endl; 

					pc = 0;
				}
		};

		class OpenSegments : public BaseSegments
		{
			public:
				OpenSegments(
					std::vector<Vector>& p,
					std::vector<size_t>& i,
					std::vector<double>& v)
						:BaseSegments(p,i,v)
				{}
				~OpenSegments()
				{
					//assert(points.size() > 0);
					//assert(indices.size() == values.size()*2);
				}
		};

		//--------------------------------------------------------------

		class BaseCoilgen
		{
			public:
				BaseCoilgen(const AlgorithmBase* algo, ModelTMSCoilAlgorithm::Args& args) :
				  ref_cnt(0),
				  algo(algo),
				  
				  coilLOD(args.coilLevelDetails),
				  coilType(args.type),
				  coilLayers(args.coilLayers),
				  rings(args.rings),
				  
				  coilLayersStep(args.coilLayersStep),
				  wingsAngle(args.wingsAngle),
				  current(args.current),
				  innerR(args.coilRadiusInner),
				  outerR(args.coilRadiusOuter),
				  outerD(args.coilDistance)

				{
					  	
				}
				
				virtual ~BaseCoilgen()
				{
				}
							

				
				void Execute(FieldHandle& meshFieldHandle) const
				{
					
					std::vector<Vector> coilPoints;
					std::vector<size_t> coilIndices;

					this->Generate(meshFieldHandle);
				}	
		
				//! Global reference counting
				int ref_cnt;
			
			protected:

				//! ref to the executing algorithm context
				const AlgorithmBase* algo;
				size_t coilLOD;
				size_t coilType;
				size_t coilLayers;
				const size_t rings;
				double coilLayersStep;
				double wingsAngle;
				double current;
				const double innerR;
				const double outerR;
				const double outerD;
				
				//! Local entry function, must be implemented by each specific kernel
				virtual void Generate(FieldHandle& meshHandle) const = 0;

				void GenPointsCircular(
					BaseSegments& segments,
					Vector origin, 
					double radius,
					double value,
					double fromPI,
					double toPI, 
					double extLOD = 0.0) const
				{

					double dPI = abs(toPI - fromPI);
					
					double minPI = M_PI /  ( 8.0 * coilLOD + coilLOD * extLOD );
					
					assert(dPI > minPI);
					
					size_t nsegments = 2;
					double iPI = dPI / nsegments;
					
					while(iPI > minPI)
					{
						nsegments++;
						iPI = dPI / nsegments;
					}

					//algo->remark("#Segments(LOD):  " +  boost::lexical_cast<std::string>(nsegments) );
					
					dPI = toPI - fromPI;
					
					iPI = dPI / nsegments;

					for(size_t i = 0; i < nsegments; i++)
					{
						Vector point(origin.x() + radius * cos(fromPI + iPI*i), origin.y() + radius * sin(fromPI + iPI*i), origin.z());
						segments.AddPoint(point,value);
					}
				}
				
				void BuildScirunMesh(
						const std::vector<Vector>& points, 
						const std::vector<size_t>& indices, 
						const std::vector<double>& values,
						FieldHandle& meshHandle) const
				{
					
					VMesh* mesh = meshHandle->vmesh();


					//! add nodes to the new mesh
					for(size_t i = 0; i < points.size(); i++)
					{
						const Point p(points[i]);
						mesh->add_point(p);
					}

					//! add edges to mesh
					VMesh::Node::array_type edge;

					for(size_t i = 0; i < indices.size(); i++)
					{
					  VMesh::Node::index_type p = indices[i];
					  edge.push_back(p);

					  if(edge.size() == 2)
					  {
						mesh->add_elem(edge);
						edge.clear();
					  }
					}

					//! add data to mesh

					VField* field = meshHandle->vfield();

					field->resize_values();
					field->set_values(values);
				}
						
				
		};
		
		//! piece-wise wire discretization
		class CircularWireCoilgen : public BaseCoilgen
		{
				
			public:
			
				CircularWireCoilgen( 
					const AlgorithmBase* algo, 
					ModelTMSCoilAlgorithm::Args& args )
					: 
					BaseCoilgen( algo, args )
				{
					coilLayers = coilLayers == 0 ? 1 : coilLayers;
				}
				
				~CircularWireCoilgen()
				{
				}
				
				
				virtual void Generate(FieldHandle& meshHandle) const
				{					
					std::vector<Vector> coilPoints;
					std::vector<size_t> coilIndices;
					std::vector<double> coilValues;
					


					Vector step(0,0,coilLayersStep);
					
					double dr = (outerR - innerR) / rings;

					if(coilType == 1)
					{
						//Vector origin(0, 0, -0.5*(1.0/coilLayers));
						Vector origin(0, 0, -coilLayersStep*(coilLayers/2) );

						for(size_t l = 0; l < coilLayers; l++)
						{
							ClosedSegments segments(coilPoints,coilIndices,coilValues);
							GenPointsCircular(segments, origin, outerR, current, 0.0, 2.0*M_PI);
							
							origin += step;
						}

					}
					else if(coilType == 2)
					{
						Vector originLeft ( -outerR - (outerD/2), 0.0, -coilLayersStep*(coilLayers/2) );
						Vector originRight(  outerR + (outerD/2), 0.0, -coilLayersStep*(coilLayers/2) );
						
						auto transLeft = Transform::Identity();
						transLeft.post_rotate(M_PI*(wingsAngle/180),{0,1,0});
						
						auto transRight = Transform::Identity();
						transRight.post_rotate(M_PI*(-wingsAngle/180),{0,1,0});
					

						for(size_t l = 0; l < coilLayers; l++)
						{
							
							for (size_t i = 0; i < rings; i++)
							{
								ClosedSegments segments(coilPoints,coilIndices,coilValues);
								GenPointsCircular(segments, originLeft, innerR + dr + i*dr, current, 0.0 , 2*M_PI );
								segments.Transform(transLeft);
							}
							
							originLeft += step;
						}

						for(size_t l = coilLayers; l < 2*coilLayers; l++)
						{	

							for (size_t i = 0; i < rings; i++)
							{
								ClosedSegments segments(coilPoints,coilIndices,coilValues);
								GenPointsCircular(segments, originRight, innerR + dr + i*dr, -current, 0.0 , 2*M_PI);
								segments.Transform(transRight);
							}
							
							originRight += step;
						}
						
						

					}
					else
					{
						algo->error("coil type value expeced: 1/2 (0-shape/8-shape)");
						return;
					}
				
										
					//SCIrun API creating a new mesh
					//0 data on elements; 1 data on nodes
					FieldInformation fi("CurveMesh",0,"double");
					fi.make_curvemesh();
					fi.make_constantdata();
					fi.make_scalar();

					meshHandle = CreateField(fi);
					
					BuildScirunMesh(coilPoints,coilIndices,coilValues,meshHandle);
				}		
				
		};
		
		//! piece-wise wire discretization
		class MultiloopsCoilgen : public BaseCoilgen
		{
			public:
			
				MultiloopsCoilgen( 
					const AlgorithmBase* algo, 
					ModelTMSCoilAlgorithm::Args args )
					: 
					BaseCoilgen( algo, args )
				{
					coilLayers = coilLayers == 0 ? 1 : coilLayers;

					//no auto current adjustment for each layer
					//hidden functionality (rather be explicit up front)
					//leave it up to users judgment

				}
				
				~MultiloopsCoilgen()
				{
				}
				
				virtual void Generate(FieldHandle& meshHandle) const
				{
					std::vector<Vector> coilPoints;
					std::vector<size_t> coilIndices;
					std::vector<double> coilValues;

					Vector step(0,0,coilLayersStep);



					if(coilType == 1)
					{
						//Vector origin(0, 0, -0.5*(1.0/coilLayers));
						Vector origin(0, 0, -coilLayersStep*(coilLayers/2) );

						for(size_t l = 0; l < coilLayers; l++)
						{
							OpenSegments segments( coilPoints, coilIndices, coilValues );

							///SINGLE coil
							GenPointsSpiralLeft(segments, origin);
							origin += step;
						}

					}
					else if(coilType == 2)
					{
						Vector originLeft ( -outerR - (outerD/2), 0.0, -coilLayersStep*(coilLayers/2) );
						Vector originRight(  outerR + (outerD/2), 0.0, -coilLayersStep*(coilLayers/2) );

						for(size_t l = 0; l < coilLayers; l++)
						{
							OpenSegments segments( coilPoints, coilIndices, coilValues );

							//LEFT coil
							GenPointsSpiralLeft(segments, originLeft);

							originLeft += step;
						}

						for(size_t l = coilLayers; l < 2*coilLayers; l++)
						{	
							OpenSegments segments( coilPoints, coilIndices, coilValues );

							//RIGHT coil
							GenPointsSpiralRight(segments, originRight);
							
							originRight += step;
						}

					}
					else
					{
						algo->error("coil type value expeced: 1/2 (0-shape/8-shape)");
						return;
					}

					
										
					//SCIrun API creating a new mesh
					//0 data on elements; 1 data on nodes
					FieldInformation fi("CurveMesh",0,"double");
					fi.make_curvemesh();
					fi.make_constantdata();
					fi.make_scalar();

					meshHandle = CreateField(fi);
					
					BuildScirunMesh(coilPoints,coilIndices,coilValues,meshHandle);
				}
				
			protected:
	
				void GenPointsSpiralLeft(OpenSegments& segments, Vector center) const

				{
					double dr = (outerR - innerR) / rings;		
					
					Vector center_offset (center.x() + dr/2, center.y(), center.z() );
					
					for (size_t i = 0; i < rings; i++)
					{
						GenPointsCircular(segments, center, innerR + i*dr, current, 0   , M_PI, i );

						GenPointsCircular(segments, center_offset, innerR + i*dr + dr/2, current, M_PI, 2*M_PI, i );	
					}
				
					//TODO refactor to avoid this
					Vector endp(center.x() + outerR * cos(2*M_PI), center.y() + outerR * sin(2*M_PI), center.z());
					segments.AddPoint(endp, current);
				}

				void GenPointsSpiralRight(OpenSegments& segments, Vector center) const
				{
					double dr = (outerR - innerR) / rings;		
					
					Vector center_offset( center.x() + dr/2, center.y(), center.z() );

					for (size_t i = rings; i > 0; i--)
					{
						GenPointsCircular(segments, center, innerR + i*dr, -current, M_PI, 2*M_PI, i );

						GenPointsCircular(segments, center_offset, innerR + i*dr - dr/2, -current, 0, M_PI, i );	
					}
				
					//TODO refactor to avoid this
					Vector endp(center.x() + innerR * cos(M_PI), center.y() + innerR * sin(M_PI), center.z());
					segments.AddPoint(endp, -current);
				}

				/// this is tricky but doable, the idea is to distribute the current along each coil winding 
				/// so that the top surface flux is not linear but curved (bell shaped like)
				/// this is required since in the AC profile there is inter-winding coupling increasing the resistivity of the net
				/// please see: https://en.wikipedia.org/wiki/Proximity_effect_%28electromagnetism%29
				void AdjustForProximityEffect()
				{
					
				}
				
		};



		//! dipoles domain discretization
		// (replicating paper doi:10.1006/nimg.2002.1282)
		class DipolesCoilgen : public BaseCoilgen
		{
			public:
			
				DipolesCoilgen( 
					const AlgorithmBase* algo,
					ModelTMSCoilAlgorithm::Args args )
					: 
					BaseCoilgen( algo, args )
					  	
				{
					coilLayers = coilLayers == 0 ? 1 : coilLayers;
			
				}
				
				~DipolesCoilgen()
				{
				}
				
				virtual void Generate(FieldHandle& meshHandle) const
				{
					std::vector<Vector> dipolePoints;
					std::vector<Vector> dipoleValues;
					std::vector<size_t> coilIndices;
					
					std::vector<double> radiiInner = preRadiiInner();
					std::vector<double> radiiOuter = preRadiiOuter();
					//std::vector<double> numElements = preNumElem(radiiInner);
					std::vector<double> numCoupling = preNumAdjElem(radiiInner);

					
					//print_vector(radiiInner);
					//print_vector(radiiOuter);
					
					//print_vector(numElements);
					//print_vector(numCoupling);
					
					assert(radiiInner.size() == radiiOuter.size());
					
					
					//algo->remark("#Rings:  " +  boost::lexical_cast<std::string>(radiiOuter.size()) + " ring-step:" + boost::lexical_cast<std::string>(lod_step_m));
					
					
					if(coilType == 1)
					{
						Vector center(0, 0, 0);

						for (size_t i = 0; i < radiiInner.size(); i++)
						{
							double ringRad = radiiInner[i] + (radiiOuter[i] - radiiInner[i]) / 2.0;						
							
							/// SINGLE COIL								
							size_t numElements = GenPointsCircular2(dipolePoints, center, ringRad, 0.0, 2*M_PI, rings);
							double ringArea = M_PI * ( radiiOuter[i] * radiiOuter[i] - radiiInner[i] * radiiInner[i] );							
							double dipoleMoment = (  current * ringArea * numCoupling[i] ) / numElements;
							Vector dipoleNormL(0,0,1.0*dipoleMoment);
							GenSegmentValues(dipolePoints, dipoleValues, dipoleNormL );
						}
					}
					else if(coilType == 2)
					{
						Vector originL( -radiiOuter[radiiOuter.size()-1] - outerD / 2.0, 0, 0);
						Vector originR( radiiOuter[radiiOuter.size()-1] + outerD / 2.0, 0, 0 );
						
						for (size_t i = 0; i < radiiInner.size(); i++)
						{
							double ringRad = radiiInner[i] + (radiiOuter[i] - radiiInner[i]) / 2.0;
							double ringArea = M_PI * ( radiiOuter[i] * radiiOuter[i] - radiiInner[i] * radiiInner[i] );
							
							/// LEFT COIL
							size_t numElementsL = GenPointsCircular2(dipolePoints, originL, ringRad, 0.0, 2*M_PI, rings);
							
							double dipoleMomentL = ( current * ringArea * numCoupling[i] ) / numElementsL;
							Vector dipoleNormL(0,0,1.0*dipoleMomentL);
							GenSegmentValues(dipolePoints, dipoleValues, dipoleNormL );


							/// RIGHT COIL
							size_t numElementsR = GenPointsCircular2(dipolePoints, originR, ringRad, 0.0, 2*M_PI, rings);
							double dipoleMomentR = ( current * ringArea * numCoupling[i] ) / numElementsR;
							Vector dipoleNormR(0,0,-1.0*dipoleMomentR);
							GenSegmentValues(dipolePoints, dipoleValues, dipoleNormR );
						}

					}
					else
					{
						algo->error("coil type value expeced: 1/2 (0-shape/8-shape)");
						return;
					}
					
					
					///basic topoly assumptions needs to be correct
					assert(dipolePoints.size() > 0);
					assert(dipolePoints.size() == dipoleValues.size());

										
					///SCIrun API creating a new mesh
					///0 data on elements; 1 data on nodes
					FieldInformation fi("PointCloudMesh",1,"vector");
					fi.make_pointcloudmesh();
					fi.make_lineardata();
					fi.make_vector();

					meshHandle = CreateField(fi);
					
					BuildScirunMesh(dipolePoints,dipoleValues,meshHandle);
				}
				
		protected:

			
				void print_vector(const std::vector<double>& v) const
				{
					std::cout << std::endl;
					for(int i=0;i<v.size();++i)
					{
						std::cout << v[i] << " "; 
					}
					std::cout << std::endl;
				}
				
				const std::vector<double> preRadiiInner() const
				{
					std::vector<double> preRadii;
					
					double step = (outerR - innerR) / rings;
					
					double d = innerR;
					
					//add first element
					//preRadii.push_back(0.00d);
					
					while( d < outerR)
					{
						preRadii.push_back(d);
						d += step;
					}
					
										
					//const double vals[16] = {0.00d, 0.003d, 0.007d, 0.011d, 0.015d, 0.019d, 0.023d, 0.026d, 0.028d, 0.030d, 0.032d, 0.034d, 0.036d, 0.038d, 0.040d, 0.042d};
					//std::vector<double> preRadii(vals,vals+16);
					return preRadii;
				}
				
				const std::vector<double> preRadiiOuter() const
				{
					std::vector<double> preRadii;
					
					double step = (outerR - innerR) / rings;
					
					double d = innerR;
					
					//add first element
					//preRadii.push_back(d);
					
					while( d < outerR)
					{
						d += step;
						preRadii.push_back(d);
					}
					
					//add last
					//preRadii.push_back(outerR);
					
					//override last to fill to exacct outer radius 
					preRadii[preRadii.size()-1u] = outerR;
					
					
					//const double vals[16] = {0.003d, 0.007d, 0.011d, 0.015d, 0.019d, 0.023d, 0.026d, 0.028d, 0.030d, 0.032d, 0.034d, 0.036d, 0.038d, 0.040d, 0.042d, 0.044d};
					//std::vector<double> preRadii(vals,vals+16);
					return preRadii;
				}

				const std::vector<double> preNumElem(std::vector<double>& radii) const
				{
					std::vector<double> preNumElem;
					
					for(size_t i = 1; i <= radii.size(); ++i)
					{
						
						size_t n = M_PI_2 / (1.0 / ( radii[i]* coilLOD ) );
						
						//size_t n = segments +  ( pow(i,1.5) / coilLOD );
						preNumElem.push_back(n);
					}
					
					//const double vals[16] = {3.0d, 9.0d, 12.0d, 16.0d, 20.0d, 24.0d, 28.0d, 30.0d, 32.0d, 34.0d, 36.0d, 38.0d, 40.0d, 42.0d, 44.0d, 44.0d};
					//std::vector<double> preNumElem(vals,vals+16);
					return preNumElem;

				}

				const std::vector<double> preNumAdjElem(std::vector<double>& radii) const
				{
					std::vector<double> preNumAdjElem;
					
					for(size_t i = 1; i <= radii.size(); ++i)
					{
						preNumAdjElem.push_back(1.0);
					}
					
					//const double vals[16] = {9.0d, 9.0d, 9.0d, 9.0d, 9.0d, 9.0d, 9.0d, 9.0d, 8.0d, 7.0d, 6.0d, 5.0d, 4.0d, 3.0d, 2.0d, 1.0d};
					//std::vector<double> preNumAdjElem(vals,vals+16);
					return preNumAdjElem;
				}
					
				void GenSegmentValues(const std::vector<Vector>& points, std::vector<Vector>& values, Vector val) const
				{
					assert(points.size() > 0);
					
					for(size_t i = values.size(); i < points.size(); i++)
					{
						values.push_back(val);
					}
				}

				size_t GenPointsCircular2(
					std::vector<Vector>& points,
					Vector origin, 
					double radius,
					double fromPI,
					double toPI, 
					double extLOD) const
				{

					double dPI = abs(toPI - fromPI);
					
					double minPI = M_PI /  ( 8.0 * coilLOD + coilLOD * extLOD );
					
					assert(dPI > minPI);
					
					size_t nsegments = 2;
					double iPI = dPI / nsegments;
					
					while(iPI > minPI)
					{
						nsegments++;
						iPI = dPI / nsegments;
					}

					//algo->remark("#Segments(LOD):  " +  boost::lexical_cast<std::string>(nsegments) );
					
					dPI = toPI - fromPI;
					
					iPI = dPI / nsegments;

					for(size_t i = 0; i < nsegments; i++)
					{
						Vector point(origin.x() + radius * cos(fromPI + iPI*i), origin.y() + radius * sin(fromPI + iPI*i), origin.z());
						//segments.AddPoint(point,value);
						points.push_back(point);
					}
					
					return nsegments;
				}
				
				void BuildScirunMesh(const std::vector<Vector>& points, 
						const std::vector<Vector>& values,
						FieldHandle& meshHandle) const
				{
					
					VMesh* mesh = meshHandle->vmesh();

					//! add nodes to the new mesh
					for(size_t i = 0; i < points.size(); i++)
					{
						const Point p(points[i]);
						mesh->add_point(p);
					}

					//! add data to mesh
					VField* field = meshHandle->vfield();
					field->resize_values();
					field->set_values(values);
				}
		};


int ResolveAlgoName(std::string& name)
{
	if(name == "Thin Circular Wires")
	{
		return 0;
	}
	
	if(name == "Thin Spiral Wires")
	{
		return 1;
	}
	
	if(name == "Magnetic Dipoles")
	{
		return 2;
	}
	
	return 0;
	
}


std::unique_ptr<BaseCoilgen> AlgoSelector(int idx,const AlgorithmBase* scirunAlgoBase, ModelTMSCoilAlgorithm::Args args)
{
	switch(idx)
	{
		case 0: return std::unique_ptr<BaseCoilgen>(new CircularWireCoilgen(scirunAlgoBase,args));
		case 1: return std::unique_ptr<BaseCoilgen>(new MultiloopsCoilgen(scirunAlgoBase,args));
		case 2: return std::unique_ptr<BaseCoilgen>(new DipolesCoilgen(scirunAlgoBase,args));
		default: return std::unique_ptr<BaseCoilgen>(new CircularWireCoilgen(scirunAlgoBase,args));
	}
	
}


AlgorithmOutput ModelTMSCoilAlgorithm::run(const AlgorithmInput& input) const
{
	AlgorithmOutput output;
	FieldHandle ofield;

	std::string model_type = static_cast<std::string>(get(Parameters::Type).toString());
	//int model_type = static_cast<int>(get(Parameters::Type).toInt());
	//remark("model_type:  " +  boost::lexical_cast<std::string>(model_type) );

	ModelTMSCoilAlgorithm::Args algoArgs;
	algoArgs.current = static_cast<double>(get(Parameters::Current).toDouble())*1e6; ///MD: here is the unit scaling, wire current module input is in [megaA/s]
	algoArgs.rings = static_cast<size_t>(get(Parameters::Rings).toInt());
	algoArgs.wingsAngle = static_cast<double>(get(Parameters::WingsAngle).toDouble());
	algoArgs.coilRadiusInner = static_cast<double>(get(Parameters::InnerRadius).toDouble());
	algoArgs.coilRadiusOuter = static_cast<double>(get(Parameters::OuterRadius).toDouble());
	algoArgs.coilLayers = static_cast<size_t>(get(Parameters::Layers).toInt());
	algoArgs.coilLevelDetails = static_cast<size_t>(get(Parameters::LevelOfDetail).toInt());
	algoArgs.coilLayersStep = static_cast<double>(get(Parameters::LayerStepSize).toDouble());
	algoArgs.coilDistance = static_cast<double>(get(Parameters::Distance).toDouble());
	algoArgs.type = get(Parameters::FigureOf8CoilShape).toInt() ? 2 : 1;

	auto algo = AlgoSelector(ResolveAlgoName(model_type),this,algoArgs);
	//auto algo = AlgoSelector(model_type,this,algoArgs);

	algo->Execute(ofield);
  
	output[Parameters::Mesh] = ofield;
	return output; 
}
