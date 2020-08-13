/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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
#include <memory>
#include <iomanip>
#include <locale>
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
	  typeOut_(t)
	{
	}

	virtual ~KernelBase()
	{
	}

	//! Local entry function, must be implemented by each specific kernel
	virtual bool integrate(FieldHandle& mesh, FieldHandle& coil, DenseMatrixHandle& outdata) = 0;

protected:

	//! ref to the executing algorithm context
	const AlgorithmBase* algo_;
	unsigned int numprocessors_;

	//! model miscs.
	VMesh* vmesh_ {nullptr};
	VField* vfield_ {nullptr};
	size_type modelSize_ {0};

	//! coil miscs.
	VMesh* vcoil_ {nullptr};
	VField* vcoilField_ {nullptr};
	size_type coilSize_ {0};

	//! parallel essential primitives
	Barrier barrier_;
	std::vector<bool> success_;

	//! output Field
	int typeOut_;
	DenseMatrixHandle matOut_;

	bool preIntegration( FieldHandle& mesh, FieldHandle& coil )
	{
		vmesh_ = mesh->vmesh();
		vcoil_ = coil->vmesh();
		vfield_ = mesh->vfield();
		vcoilField_ = coil->vfield();

		numprocessors_ = Parallel::NumCores();

		#ifdef _DEBUG
			//! DEBUG when we want to test with one CPU only
			numprocessors_ = 1;
		#endif

		success_.resize(numprocessors_, true);

		//! get number of nodes for the model
		modelSize_ = vmesh_->num_nodes();
		assert(modelSize_ > 0);

		matOut_.reset(new DenseMatrix(static_cast<int>(modelSize_), 3));

    algo_->remark("Output matrix size: " + std::to_string(modelSize_) + "x3");
    algo_->remark("Number of processors:  " + boost::lexical_cast<std::string>(numprocessors_));
    algo_->remark("[Important] CPU usage will be very high while running this module. \r\nTo limit the number of cores used, adjust the maximum core setting in Preferences->Advanced");

		return true;
	}

	bool postIntegration(DenseMatrixHandle& outdata)
	{
		//! check for error
		for (size_t j=0; j < success_.size(); j++)
		{
			if (!success_[j]) return (false);
		}
		outdata = matOut_;
		return true;
	}
};

namespace
{
  template<class T>
  std::string formatWithCommas(const T& value)
  {
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
  }
}

class PieceWiseKernel : public KernelBase
{
public:
	PieceWiseKernel(const AlgorithmBase* algo, int t ) : KernelBase(algo,t)
	{
		//we keep last calculated step
		//however if segments lenght varies,
		//it makes more sense to keep a look-up table of previous steps for given lenght
		autostep_ = 0.1;
		extstep_ = -1.0;
	}

	//! Complexity O(M*N) ,where M is the number of nodes of the model and N is the numbder of nodes of the coil
	bool integrate(FieldHandle& mesh, FieldHandle& coil, DenseMatrixHandle& outdata) override
	{
		if (!preIntegration(mesh,coil))
		{
			return (false);
		}

    algo_->remark("Launching PieceWiseKernel. Size of computation remark coming soon..");

		vmesh_->synchronize(Mesh::NODES_E | Mesh::EDGES_E);

		VMesh::Node::array_type enodes;
		Point enode1, enode2;

		//! get numbder of nodes for the coil
		coilSize_ = vcoil_->num_nodes();

		//! basic assumption
		assert(modelSize_ > 0 && coilSize_ > 1);

		coilNodes_.clear();
		coilNodes_.reserve(coilSize_);

		for (VMesh::Edge::index_type i = 0; i < vcoil_->num_edges(); i++)
		{
			vcoil_->get_nodes(enodes,i);
			vcoil_->get_point(enode1,enodes[0]);
			vcoil_->get_point(enode2,enodes[1]);
			coilNodes_.push_back(Vector(enode1));
			coilNodes_.push_back(Vector(enode2));
		}

		//! Start the multi threaded
		Parallel::RunTasks([this](int i) { ParallelKernel(i); }, numprocessors_);

		return postIntegration(outdata);
	}

	void setIntegrationStep(double step)
	{
		assert(step >= 0.0);
		extstep_ = step;
	}

	double getIntegrationStep() const
	{
		return extstep_;
	}

private:
	//! integration step, will auto adapt
	double autostep_;
	//! integration step, externally provided
	double extstep_;
	//! keep nodes on the coil cached
	std::vector<Vector> coilNodes_;

	//! execute in parallel
	void ParallelKernel(int proc_num)
	{
		assert(proc_num >= 0);

		int cnt = 0;

		const index_type begins = (modelSize_ * proc_num) / numprocessors_;
		const index_type ends  = (modelSize_ * (proc_num+1)) / numprocessors_;

		assert( begins <= ends );

		//! buffer of points used for integration
		std::vector<Vector> integrPoints;
		integrPoints.reserve(256);

		//! keep previous step length
		//! used for optimization purpose
		double prevSegLen = 123456789.12345678;

		//! number of integration points
		int nips = 0;

    bool remarkedOnProblemSize = false;

		try
		{
			for (index_type iM = begins; iM < ends; iM++)
			{
        Point modelNodeP;
				vmesh_->get_node(modelNodeP, iM);
        const Vector modelNodeV(modelNodeP);

				// result
				Vector F;

				for (size_t iC0 = 0, iC1 =1, iCV = 0; iC0 < coilNodes_.size(); iC0+=2, iC1+=2, iCV++)
				{
          double currentFromField;
					vcoilField_->get_value(currentFromField,iCV);

					const double current = currentFromField == 0.0 ? 1.0 : currentFromField;
          auto absCurrent = std::fabs(current);

					Vector coilNodeThis;
					Vector coilNodeNext;

					if (current >= 0.0)
					{
						coilNodeThis = coilNodes_[iC0];
						coilNodeNext = coilNodes_[iC1];
					}
					else
					{
						coilNodeThis = coilNodes_[iC1];
						coilNodeNext = coilNodes_[iC0];
					}

					//! Length of the curve element
					Vector diffNodes = coilNodeNext - coilNodeThis;
					double newSegLen = diffNodes.length();

					//first check if externally suplied integration step is available and use it
					if (extstep_ > 0)
					{
						nips = newSegLen / extstep_;
					}
					else
					{
						//! optimization
						//! only recompute integration step only if segment length changes
						if (Abs(prevSegLen - newSegLen ) > 0.00000001)
						{
							prevSegLen = newSegLen;

							//auto adaptive integration step calculation
							nips = adjustNumberOfIntegrationPoints(newSegLen);
						}
					}

					if (nips < 3)
					{
						algo_->warning("integration step too big");
					}

					integrPoints.clear();

          if (!remarkedOnProblemSize && proc_num == 0)
          {
            auto problemSize = (ends - begins) * coilNodes_.size() * nips;
            algo_->remark("Per core load: " + formatWithCommas(problemSize) + " field computations.");
            algo_->remark("To speed up this module, reduce the number of nodes in either the input mesh or the coil, or pick a simpler algorithm.");
            remarkedOnProblemSize = true;
          }

					//! curve segment discretization
					for (int iip = 0; iip < nips; iip++)
					{

						double interpolant = static_cast<double>(iip) / static_cast<double>(nips);
						Vector v = Interpolate( coilNodeThis, coilNodeNext, interpolant );
						integrPoints.push_back( v );
					}

					//! integration step over line segment
					for (int iip = 0; iip < nips -1; iip++)
					{
            const auto piip = integrPoints[iip];
            const auto piip1 = integrPoints[iip+1];
						//! Vector connecting the infinitesimal curve-element
						Vector Rxyz = (piip + piip1) / 2  - modelNodeV;

						//! Infinitesimal curve-element components
						Vector dLxyz = piip1 - piip;

						double Rn = Rxyz.length();

						if (typeOut_ == 1)
						{
              //! check for distance between coil and model close to zero
              //! it might cause numerical stability issues with respect to the cross-product
              if (Rn < 0.00001)
              {
                algo_->warning("coil<->model distance approaching zero!");
              }
							//! Biot-Savart Magnetic Field
							F += 1.0e-7 * Cross( Rxyz, dLxyz ) * (absCurrent / (Rn*Rn*Rn) );
						}
						else if (typeOut_ == 2)
						{
							//! Biot-Savart Magnetic Vector Potential Field
							F += 1.0e-7 * dLxyz * (absCurrent / (Rn) );
						}
					}
				}

				matOut_->put(iM,0, F[0]);
				matOut_->put(iM,1, F[1]);
				matOut_->put(iM,2, F[2]);

				//! progress reporter
				if (proc_num == 0)
				{
					cnt++;
					if (cnt == 200)
					{
						cnt = 0;
						algo_->update_progress_max(iM, ends-begins);
					}
				}
			}
			success_[proc_num] = true;
		}
		catch (...)
		{
			algo_->error("PieceWiseKernel crashed while integrating");
			success_[proc_num] = false;
		}

		//! check point
		barrier_.wait();

		// Bail out if one of the processes failed
		for (size_t q = 0; q < numprocessors_; q++)
			if (!success_[q]) return;
	}

	//! Auto adjust accuracy of integration
	int adjustNumberOfIntegrationPoints(double len)
	{
		int minNP = 100;//more than 1 for sure
		int maxNP = 200;//no more than 1000
		int NP = 0;
		bool over = false;
		bool under = false;

		do
		{
			NP = ceil( len / autostep_ );

			under = NP < minNP;
			over = NP > maxNP;

			if (under) autostep_ *= 0.5;
			if (over) autostep_ *= 1.5;

		} while ( under || over );

		return NP;
	}
};

//! TODO
class VolumetricKernel : public KernelBase
{
public:
	using KernelBase::KernelBase;

	bool integrate(FieldHandle& mesh, FieldHandle& coil, DenseMatrixHandle& outdata) override
	{
		if (!preIntegration(mesh,coil))
		{
			return (false);
		}

    algo_->remark("Launching VolumetricKernel");

		//! get numbder of nodes for the coil
		coilSize_ = vcoil_->num_elems();

		//! basic assumption
		assert(modelSize_ > 0 && coilSize_ > 1);

		vmesh_->synchronize(Mesh::NODES_E | Mesh::EDGES_E);

		//! Start the multi threaded
		Parallel::RunTasks([this](int i) { ParallelKernel(i); }, numprocessors_);

		return postIntegration(outdata);
	}
private:
	void ParallelKernel(int proc_num)
	{
		assert(proc_num >= 0);

		int cnt = 0;
		Point modelNode;
		Point coilCenter;
		Vector current;

		const VMesh::Node::index_type begins = (modelSize_ * proc_num) / numprocessors_;
		const VMesh::Node::index_type ends  = (modelSize_ * (proc_num+1)) / numprocessors_;

		assert( begins <= ends );

		try
		{
			for (VMesh::Node::index_type iM = begins; iM < ends; iM++)
			{
				vmesh_->get_node(modelNode,iM);

				//! accumulatedresult
				Vector F, R;
				double evol = 0.0;
				double Rl;

				for (VMesh::Elem::index_type iC = 0; iC < coilSize_; iC++)
				{
					vcoilField_->get_value(current,iC);

					vcoilField_->get_center(coilCenter, iC);//auto resolve based on basis_order

					evol = vcoil_->get_volume(iC);

					R = coilCenter - modelNode;

					Rl = R.length();

					if (typeOut_ == 1)
					{
						//! Biot-Savart Magnetic Field
						F += Cross ( current , R ) * ( evol / (4.0 * M_PI * Rl) );
					}
					else if (typeOut_ == 2)
					{
						//! Biot-Savart Magnetic Vector Potential Field
						F += current * ( evol / (4.0 * M_PI * Rl) );
					}
				}

				matOut_->put(iM, 0, F[0]);
				matOut_->put(iM, 1, F[1]);
				matOut_->put(iM, 2, F[2]);

				//! progress reporter
				if (proc_num == 0)
				{
					cnt++;
					if (cnt == 200)
					{
						cnt = 0;
						algo_->update_progress_max(iM, ends - begins);
					}
				}
			}

			success_[proc_num] = true;
		}
		catch (...)
		{
			algo_->error("VolumetricKernel crashed while integrating");
			success_[proc_num] = false;
		}

		//! check point
		barrier_.wait();

		// Bail out if one of the processes failed
		for (size_t q = 0; q < numprocessors_; q++)
			if (!success_[q])
				return;
	}
};


//! Magnetic Dipoles solver
class DipolesKernel : public KernelBase
{
public:
	using KernelBase::KernelBase;

	bool integrate(FieldHandle& mesh, FieldHandle& coil, DenseMatrixHandle& outdata) override
	{
		if (!preIntegration(mesh,coil))
		{
			return false;
		}

    algo_->remark("Launching DipolesKernel");

		//! get number of nodes for the coil
		coilSize_ = vcoil_->num_elems();

		//! basic assumption
		assert(modelSize_ > 0 && coilSize_ > 1);

		//needed?
		vmesh_->synchronize(Mesh::NODES_E | Mesh::EDGES_E);

		//! Start the multi threaded
		Parallel::RunTasks([this](int i) { ParallelKernel(i); }, numprocessors_);

		return postIntegration(outdata);
	}

private:
	void ParallelKernel(int proc_num)
	{
		assert(proc_num >= 0);

		int cnt = 0;
		Point modelNode;
		Point dipoleLocation;
		Vector dipoleMoment;

		const VMesh::Node::index_type begins = (modelSize_ * proc_num) / numprocessors_;
		const VMesh::Node::index_type ends  = (modelSize_ * (proc_num+1)) / numprocessors_;

		assert( begins <= ends );

		try
		{
			for (VMesh::Node::index_type iM = begins; iM < ends; iM++)
			{
				vmesh_->get_node(modelNode,iM);

				//! accumulated result
				Vector F, R;
				double Rl;

				for (VMesh::Elem::index_type iC = 0; iC < coilSize_; iC++)
				{
					vcoilField_->get_value(dipoleMoment, iC);
					vcoilField_->get_center(dipoleLocation, iC);//auto resolve based on basis_order

					R = dipoleLocation - modelNode;
					Rl = R.length();

					if (typeOut_ == 1)
					{
						//! Biot-Savart Magnetic Field
						F += 1.0e-7 * ( 3 * R * Dot ( dipoleMoment, R ) / (Rl*Rl*Rl*Rl*Rl) - dipoleMoment / (Rl*Rl*Rl) ) ;
					}
					if (typeOut_ == 2)
					{
						//! Biot-Savart Magnetic Vector Potential Field
						F += 1.0e-7 * Cross ( dipoleMoment , R ) / (Rl*Rl*Rl) ;
					}
				}

				matOut_->put(iM, 0, F[0]);
				matOut_->put(iM, 1, F[1]);
				matOut_->put(iM, 2, F[2]);

				//! progress reporter
				if (proc_num == 0)
				{
					cnt++;
					if (cnt == 200)
					{
						cnt = 0;
						algo_->update_progress_max(iM, ends - begins);;
					}
				}
			}

			success_[proc_num] = true;
		}
		catch (...)
		{
			algo_->error(std::string("DipoleKernel crashed while integrating"));
			success_[proc_num] = false;
		}

		//! check point
		barrier_.wait();

		// Bail out if one of the processes failed
		for (size_t q = 0; q < numprocessors_; q++)
			if (!success_[q])
				return;
	}
};

bool BiotSavartSolverAlgorithm::run(FieldHandle mesh, FieldHandle coil, DenseMatrixHandle& outdata, int outtype) const
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

  if (coil->vfield()->basis_order() == -1)
  {
		error("Need data on coil mesh.");
		return (false);
  }

  if (coil->vmesh()->is_curvemesh())
  {
    if (coil->vfield()->is_constantdata() && coil->vfield()->is_scalar())
    {
      PieceWiseKernel pwk(this, outtype);
      if (!pwk.integrate(mesh,coil,outdata))
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
  else if (coil->vmesh()->is_pointcloudmesh())
  {
		if ((coil->vfield()->is_lineardata() || coil->vfield()->is_constantdata() ) && coil->vfield()->is_vector())
		{
			DipolesKernel dp(this, outtype);
			if (!dp.integrate(mesh,coil,outdata))
			{
				error("Aborted during integration");
				return (false);
			}
		}
		else
		{
			error("Pointcloud expected with linear vector data.");
			return (false);
		}
	}
  else if (coil->vmesh()->is_volume())
  {
		if (coil->vfield()->is_constantdata() && coil->vfield()->is_vector())
		{
			VolumetricKernel vp(this, outtype);
			if (!vp.integrate(mesh,coil,outdata))
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

	auto oports = get(Parameters::OutType).toInt();
	std::map<int, DenseMatrixHandle> cases;
	if (oports == 3)
		cases = {{1, nullptr}, {2, nullptr}};
	else
		cases[oports] = nullptr;

  for (auto& c : cases)
  {
    if (!run(mesh, coil, c.second, c.first))
    {
      error("Error: Algorithm of BiotSavartSolver failed.");
    }
  }
	output[Parameters::VectorBField] = cases[1];
	output[Parameters::VectorAField] = cases[2];

	return output;
}
