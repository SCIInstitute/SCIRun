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


///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystemAlgo.h>
#include <Core/Algorithms/Math/ParallelAlgebra/ParallelLinearAlgebra.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

SolveLinearSystemAlgo::SolveLinearSystemAlgo()
{
  // For solver
  addOption(Variables::Method,"cg","jacobi|cg|bicg|minres");
  addOption(Variables::Preconditioner,"Jacobi","None|Jacobi");

  addParameter(Variables::TargetError, 1e-5);
  addParameter(Variables::MaxIterations, 500);

  addParameter(Variables::BuildConvergence, true);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  // for callback
  // Read this variable to find the error at the start of the iteration
  addParameter("original_error", 0.0);
  // Read this variable to find current error
  addParameter("current_error", 0.0);
  // Read this variable to find current iteration
  addParameter("iteration", 0);
  // How often the algorithm should go to a callback
  addParameter("callback_step", 10);
  addParameter("solution", MatrixHandle());
  addParameter("convergence", MatrixHandle());
#endif
}

class SolveLinearSystemParallelAlgo : public ParallelLinearAlgebraBase
{
public:
  explicit SolveLinearSystemParallelAlgo(const AlgorithmBase* base);

  bool run(SparseRowMatrixHandle a, DenseColumnMatrixHandle b,
            DenseColumnMatrixHandle x0, DenseColumnMatrixHandle& x,
            DenseColumnMatrixHandle& convergence) const;
protected:
  const AlgorithmBase* algo_;
  std::string pre_conditioner_;
  DenseColumnMatrixHandle convergence_;
};

SolveLinearSystemParallelAlgo::SolveLinearSystemParallelAlgo(const AlgorithmBase* base) : algo_(base),
  pre_conditioner_(base->getOption(Variables::Preconditioner)),
  convergence_(new DenseColumnMatrix(base->get(Variables::MaxIterations).toInt()))
{
}

bool
SolveLinearSystemParallelAlgo::run(SparseRowMatrixHandle a, DenseColumnMatrixHandle b,
                                   DenseColumnMatrixHandle x0, DenseColumnMatrixHandle& x,
                                   DenseColumnMatrixHandle& convergence) const
{
  SolverInputs matrices;
  matrices.A = a;
  matrices.b = b;
  matrices.x0 = x0;

  // Create output matrix
  auto size = x0->nrows();
  x = boost::make_shared<DenseColumnMatrix>(size);

  // Copy output matrix pointer
  matrices.x = x;

  convergence = convergence_;

  // Set intermediate solution handle
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  algo->set_handle("solution", x);
  algo->set_handle("convergence", convergence);
#endif

  if(!start_parallel(matrices))
  {
    const std::string msg = "Encountered an error while running parallel linear algebra";
    algo_->error(msg);
    BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << SCIRun::Core::ErrorMessage(msg));
  }

  return (true);
}

//------------------------------------------------------------------
// CG Solver with simple preconditioner

class SolveLinearSystemCGAlgo : public SolveLinearSystemParallelAlgo
{
  public:
    explicit SolveLinearSystemCGAlgo(const AlgorithmBase* base) : SolveLinearSystemParallelAlgo(base) {}
    virtual bool parallel(ParallelLinearAlgebra& PLA, SolverInputs& matrices) const;
};

bool SolveLinearSystemCGAlgo::parallel(ParallelLinearAlgebra& PLA, SolverInputs& matrices) const
{
  ParallelLinearAlgebra::ParallelMatrix A;
  ParallelLinearAlgebra::ParallelVector B, X, X0, XMIN, DIAG, R, Z, P;

  double tolerance =     algo_->get(Variables::TargetError).toDouble();
  int    max_iter =      algo_->get(Variables::MaxIterations).toInt();

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  int    callback_step = algo_->get("callback_step");
  int    callback_step_cnt =0;
#endif
  int    niter = 0;

  if ( !PLA.add_matrix(matrices.A, A) ||
       !PLA.add_vector(matrices.b, B) ||
       !PLA.add_vector(matrices.x0, X0) ||
       !PLA.add_vector(matrices.x, XMIN))
  {
    if (PLA.first())
    {
      algo_->error("Could not link matrices");
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      algo_->set_scalar("original_error",0.0);
      algo_->set_scalar("current_error",0.0);
      algo_->set_int("iteration",niter);
#endif
    }
    PLA.wait();
    return (false);
  }
  if ( !PLA.new_vector(X) ||
       !PLA.new_vector(DIAG) ||
       !PLA.new_vector(R) ||
       !PLA.new_vector(Z) ||
       !PLA.new_vector(P))
  {
    if (PLA.first())
    {
      algo_->error("Could not allocate enough memory for algorithm");
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      algo_->set_scalar("original_error",0.0);
      algo_->set_scalar("current_error",0.0);
      algo_->set_int("iteration",niter);
#endif
    }
    PLA.wait();
    return (false);
  }

  PLA.copy(X0,X);
  PLA.copy(X0,XMIN);

  // Build a preconditioner
  if (pre_conditioner_ == "Jacobi")
  {
    PLA.absdiag(A,DIAG);
    double max = PLA.max(DIAG);
    PLA.absthreshold_invert(DIAG,DIAG,1e-18*max);
  }
  else
  {
    PLA.ones(DIAG);
  }

  PLA.mult(A,X,R);
  PLA.sub(B,R,R);

  double bnorm = PLA.norm(B);
  double error = PLA.norm(R)/bnorm;

  double xmin = error;
  double orig = error;

  if (error <= tolerance)
  {
    if (PLA.first())
    {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      algo_->set_scalar("original_error",orig);
      algo_->set_scalar("current_error",xmin);
      algo_->set_int("iteration",niter);
#endif
      std::ostringstream ostr;
      ostr << "Solver found solution with error = " << error;
      algo_->remark(ostr.str());
    }
    PLA.wait();

    return (true);
  }

  double bkden = 0.0;

  int cnt = 0;
  double log_target = log(tolerance);
  double log_orig =  log(orig);
  double log_scale = log_orig - log_target;

  while (niter < max_iter)
  {
    if (error <= tolerance)
    {
      if (PLA.first())
      {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
        algo_->set_scalar("original_error",orig);
        algo_->set_scalar("current_error",xmin);
        algo_->set_int("iteration",niter);
        if (algo_->have_callbacks())
          algo_->do_callbacks();
#endif
        std::ostringstream ostr;
        ostr << "Solver converged after " << niter << " iterations with error " << error;
        algo_->remark(ostr.str());
      }

      PLA.wait();
      return true;
    }

    PLA.mult(R,DIAG,Z);
    double bknum = PLA.dot(Z,R);

    if (niter == 0)
    {
      PLA.copy(Z,P);
    }
    else
    {
      double bk = bknum/bkden;
      PLA.scale_add(bk,P,Z,P);
    }
    PLA.mult(A,P,Z);
    bkden = bknum;

    double akden = PLA.dot(Z,P);
    double ak=bknum/akden;

    PLA.scale_add(ak,P,X,X);
    PLA.scale_add(-ak,Z,R,R);

    error = PLA.norm(R)/bnorm;
    if (error < xmin)
    {
      PLA.copy(X,XMIN);
      xmin = error;
    }
    if (PLA.first())
      (*convergence_)[niter] = xmin;

    niter++;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    callback_step_cnt++;
    if (callback_step_cnt == callback_step)
    {
      callback_step_cnt = 0;
      if (algo_->have_callbacks())
      {
        if (PLA.first())
        {
          algo_->set_scalar("original_error",orig);
          algo_->set_scalar("current_error",error);
          algo_->set_int("iteration",niter);
          algo_->do_callbacks();
        }
        PLA.wait();

        tolerance = algo_->get_scalar("target_error");
        max_iter = algo_->get_int("max_iterations");
        callback_step = algo_->get_int("callback_step");
      }
    }
#endif
    cnt++;
    if (cnt == 20)
    {
      cnt = 0;
      algo_->update_progress((log_orig-log(error))/log_scale);
    }
  }

  // Last iteration update
  if (PLA.first())
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    algo_->set_scalar("original_error",orig);
    algo_->set_scalar("current_error",xmin);
    algo_->set_int("iteration",niter);
    if (algo_->have_callbacks()) algo_->do_callbacks();
#endif
    std::ostringstream ostr;
    ostr << "Solver stopped after " << niter << " iterations. Error was " << error;
    algo_->remark(ostr.str());
  }

  PLA.wait();

  return true;
}


//------------------------------------------------------------------
// BICG Solver with simple preconditioner
class SolveLinearSystemBICGAlgo : public SolveLinearSystemParallelAlgo
{
  public:
    explicit SolveLinearSystemBICGAlgo(const AlgorithmBase* base) : SolveLinearSystemParallelAlgo(base) {}
    virtual bool parallel(ParallelLinearAlgebra& PLA,
                          SolverInputs& matrices) const;
};

bool
SolveLinearSystemBICGAlgo::
parallel(ParallelLinearAlgebra& PLA, SolverInputs& matrices) const
{
  // Algorithm

  // Define matrices and vectors to be used in the algorithm
  ParallelLinearAlgebra::ParallelMatrix A;
  ParallelLinearAlgebra::ParallelVector B, X, X0, XMIN;
  ParallelLinearAlgebra::ParallelVector DIAG, R, R1, Z, Z1, P, P1;

  double tolerance =     algo_->get(Variables::TargetError).toDouble();
  int    max_iter =      algo_->get(Variables::MaxIterations).toInt();
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  int    callback_step = algo_->get_int("callback_step");
#endif
  int    niter = 0;
  int    callback_step_cnt =0;

  // Create matrices and vectors that we need for this algorithm
  if ( !PLA.add_matrix(matrices.A,A) ||
       !PLA.add_vector(matrices.b,B) ||
       !PLA.add_vector(matrices.x0,X0) ||
       !PLA.add_vector(matrices.x,XMIN) ||
       !PLA.new_vector(X) ||
       !PLA.new_vector(DIAG) ||
       !PLA.new_vector(R) ||
       !PLA.new_vector(R1) ||
       !PLA.new_vector(Z) ||
       !PLA.new_vector(Z1) ||
       !PLA.new_vector(P) ||
       !PLA.new_vector(P1))
  {
    if (PLA.first())
    {
      algo_->error("Could not allocate enough memory for algorithm");
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      algo_->set_scalar("original_error",0.0);
      algo_->set_scalar("current_error",0.0);
      algo_->set_int("iteration",niter);
#endif
    }
    PLA.wait();

    return (false);
  }

  PLA.copy(X0,X);
  PLA.copy(X0,XMIN);

  // Build a preconditioner
  if (pre_conditioner_ == "Jacobi")
  {
    PLA.absdiag(A,DIAG);
    double max = PLA.max(DIAG);
    PLA.absthreshold_invert(DIAG,DIAG,1e-18*max);
  }
  else
  {
    PLA.ones(DIAG);
  }

  PLA.mult(A,X,R);
  PLA.sub(B,R,R);

  double bnorm = PLA.norm(B);
  double error = PLA.norm(R)/bnorm;

  double xmin = error;
  double orig = error;

  if (error <= tolerance)
  {
    if (PLA.first())
    {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      algo_->set_scalar("original_error",orig);
      algo_->set_scalar("current_error",xmin);
      algo_->set_int("iteration",niter);
      if (algo_->have_callbacks()) algo_->do_callbacks();
#endif
    }
    PLA.wait();
    return (true);
  }

  PLA.copy(R,R1);

  double bkden = 0.0;

  int cnt = 0;
  double log_target = log(tolerance);
  double log_orig =  log(orig);
  double log_scale = log_orig - log_target;

  while (niter < max_iter)
  {
    if (error <= tolerance)
    {
      if (PLA.first())
      {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
        algo_->set_scalar("original_error",orig);
        algo_->set_scalar("current_error",xmin);
        algo_->set_int("iteration",niter);
        if (algo_->have_callbacks()) algo_->do_callbacks();
#endif
      }
      PLA.wait();
      return (true);
    }

    PLA.mult(R,DIAG,Z);
    PLA.mult(R1,DIAG,Z1);

    double bknum = PLA.dot(Z,R1);

    if (bknum == 0.0)
    {
      if (PLA.first())
      {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
        algo_->set_scalar("original_error",orig);
        algo_->set_scalar("current_error",xmin);
        algo_->set_int("iteration",niter);
#endif
      }
      PLA.wait();
      return (true);
    }

    if (niter == 0)
    {
      PLA.copy(Z,P);
      PLA.copy(Z1,P1);
    }
    else
    {
      double bk = bknum/bkden;
      PLA.scale_add(bk,P,Z,P);
      PLA.scale_add(bk,P1,Z1,P1);
    }

    PLA.mult(A,P,Z);
    PLA.mult_trans(A,P1,Z1);
    bkden = bknum;

    double akden = PLA.dot(Z,P1);
    double ak=bknum/akden;

    PLA.scale_add(ak,P,X,X);
    PLA.scale_add(-ak,Z,R,R);

    PLA.scale_add(-ak,Z1,R1,R1);
    error = PLA.norm(R)/bnorm;

    if (error < xmin) { PLA.copy(X,XMIN); xmin = error; }
    if (PLA.first()) (*convergence_)[niter] = xmin;

    niter++;

    callback_step_cnt++;
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (callback_step_cnt == callback_step)
    {
      callback_step_cnt = 0;
      if (algo_->have_callbacks())
      {
        if (PLA.first())
        {
          algo_->set_scalar("original_error",orig);
          algo_->set_scalar("current_error",xmin);
          algo_->set_int("iteration",niter);
          algo_->do_callbacks();
        }
        PLA.wait();
        tolerance = algo_->get_scalar("target_error");
        max_iter = algo_->get_int("max_iterations");
        callback_step = algo_->get_int("callback_step");
      }
    }
#endif
    cnt++;
    if (cnt == 20)
    {
      cnt = 0;
      algo_->update_progress((log_orig-log(error))/log_scale);
    }
  }

  if (PLA.first())
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    algo_->set_scalar("original_error",orig);
    algo_->set_scalar("current_error",xmin);
    algo_->set_int("iteration",niter);
    if (algo_->have_callbacks()) algo_->do_callbacks();
#endif
  }
  PLA.wait();

  return (true);
}

//------------------------------------------------------------------
// BICG Solver with simple preconditioner

class SolveLinearSystemMINRESAlgo : public SolveLinearSystemParallelAlgo
{
public:
  explicit SolveLinearSystemMINRESAlgo(const AlgorithmBase* base) : SolveLinearSystemParallelAlgo(base) {}
  virtual bool parallel(ParallelLinearAlgebra& PLA, SolverInputs& matrices) const;
};


bool
SolveLinearSystemMINRESAlgo::
parallel(ParallelLinearAlgebra& PLA, SolverInputs& matrices) const
{
  // Algorithm

  // Define matrices and vectors to be used in the algorithm
  ParallelLinearAlgebra::ParallelMatrix A;
  ParallelLinearAlgebra::ParallelVector B, X, X0, XMIN;
  ParallelLinearAlgebra::ParallelVector DIAG, R, V, VOLD, VV;
  ParallelLinearAlgebra::ParallelVector VOLDER, M, MOLD, MOLDER, XCG;

  double tolerance =     algo_->get(Variables::TargetError).toDouble();
  int    max_iter =      algo_->get(Variables::MaxIterations).toInt();
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  int    callback_step = algo_->get_int("callback_step");
#endif
  int    niter = 0;
  int    callback_step_cnt =0;

  // Create matrices and vectors that we need for this algorithm
  if ( !PLA.add_matrix(matrices.A,A) ||
       !PLA.add_vector(matrices.b,B) ||
       !PLA.add_vector(matrices.x0,X0) ||
       !PLA.add_vector(matrices.x,XMIN) ||
       !PLA.new_vector(X) ||
       !PLA.new_vector(R) ||
       !PLA.new_vector(DIAG) ||
       !PLA.new_vector(V) ||
       !PLA.new_vector(VV) ||
       !PLA.new_vector(VOLD) ||
       !PLA.new_vector(VOLDER) ||
       !PLA.new_vector(M) ||
       !PLA.new_vector(MOLD) ||
       !PLA.new_vector(MOLDER) ||
       !PLA.new_vector(XCG))
  {
    if (PLA.first())
    {
      algo_->error("Could not allocate enough memory for algorithm");
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      algo_->set_scalar("original_error",0.0);
      algo_->set_scalar("current_error",0.0);
      algo_->set_int("iteration",niter);
#endif
    }
    PLA.wait();

    return (false);
  }

  PLA.copy(X0,X);
  PLA.copy(X0,XMIN);

  // Build a preconditioner
  if (pre_conditioner_ == "Jacobi")
  {
    PLA.absdiag(A,DIAG);
    double max = PLA.max(DIAG);
    PLA.absthreshold_invert(DIAG,DIAG,1e-18*max);
  }
  else
  {
    PLA.ones(DIAG);
  }

  PLA.mult(A,X,R);
  PLA.sub(B,R,R);

  double bnorm = PLA.norm(B);
  double error = PLA.norm(R)/bnorm;

  double xmin = error;
  double orig = error;

  if (error <= tolerance)
  {
    if (PLA.first())
    {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      algo_->set_scalar("original_error",orig);
      algo_->set_scalar("current_error",xmin);
      algo_->set_int("iteration",niter);
      if (algo_->have_callbacks()) algo_->do_callbacks();
#endif
    }
    PLA.wait();
    return (true);
  }

  PLA.copy(R,VOLD);
  PLA.copy(R,V);

  PLA.mult(DIAG,V,V);

  double beta1   = sqrt(PLA.dot(V,VOLD));
  double snprod  = beta1;

  PLA.scale(1.0/beta1,V,VV);
  PLA.mult(A,VV,V);

  double alpha = PLA.dot(VV,V);
  PLA.scale_add(-alpha/beta1,VOLD,V,V);
  double dot1 = -PLA.dot(VV,V);
  double dot2 = PLA.dot(VV,VV);
  PLA.scale_add(dot1/dot2,VV,V,V);

  PLA.copy(VOLD,VOLDER);
  PLA.copy(V,VOLD);

  PLA.mult(DIAG,V,V);

  double betaold = beta1;
  double beta = sqrt(PLA.dot(VOLD,V));

  double gammabar = alpha;
  double epsilon = 0.0;
  double deltabar = beta;

  double gamma = sqrt(gammabar*gammabar + beta*beta);
  PLA.zeros(MOLD);
  PLA.copy(VV,M);
  PLA.scale(1.0/gamma,M,M);

  double cs = gammabar/gamma;
  double sn = beta/gamma;

  PLA.scale_add(snprod*cs,M,X,X);
  snprod = snprod * sn;

  PLA.scale_add(snprod*(sn/cs),M,X,XCG);

  error = std::fabs(snprod);
  double errorcg = error/(std::fabs(cs)*bnorm);

  int cnt = 0;
  int ucnt = 0;
  double log_target = log(tolerance);
  double log_orig =  log(orig);
  double log_scale = log_orig - log_target;

  double delta = 0.0;

  while (niter < max_iter)
  {
    if (error <= tolerance)
    {
      if (PLA.first())
      {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
        algo_->set_scalar("original_error",orig);
        algo_->set_scalar("current_error",xmin);
        algo_->set_int("iteration",niter);
        if (algo_->have_callbacks()) algo_->do_callbacks();
#endif
      }
      PLA.wait();
      return (true);
    }

    PLA.scale(1.0/beta,V,VV);

    PLA.mult(A,VV,V);
    PLA.scale_add(-beta/betaold,VOLDER,V,V);

    alpha = PLA.dot(VV,V);
    PLA.scale_add(-(alpha/beta),VOLD,V,V);

    PLA.copy(VOLD,VOLDER);
    PLA.copy(V,VOLD);

    PLA.mult(DIAG,V,V);

    betaold = beta;
    beta = sqrt(PLA.dot(VOLD,V));

    delta = cs*deltabar + sn* alpha;
    PLA.copy(MOLD,MOLDER);
    PLA.copy(M,MOLD);

    PLA.scale(-delta,MOLD,M);
    PLA.add(VV,M,M);
    PLA.scale_add(-epsilon,MOLDER,M,M);

    gammabar = sn*deltabar - cs*alpha;
    epsilon = sn*beta;
    deltabar = -cs*beta;
    gamma = sqrt(gammabar*gammabar + beta*beta);
    PLA.scale(1.0/gamma,M,M);

    cs = gammabar/gamma;
    sn = beta/gamma;

    PLA.scale_add(snprod*cs,M,X,X);
    snprod = snprod * sn;

    PLA.scale_add(snprod*(sn/cs),M,X,XCG);

    bool didnormr = false;

    if (cnt == 6 || niter == 0)
    {
      PLA.mult(A,X,R);
      PLA.sub(B,R,R);
      error = PLA.norm(R)/bnorm;
      cnt = 0;
      didnormr = true;
    }
    else
    {
      error = fabs(snprod)/bnorm;
      cnt++;
      didnormr = false;
    }

    errorcg = error/fabs(cs);

    if (error < tolerance)
    {
      if (didnormr == true)
      {
        if (error < xmin)
        {
          xmin = error;
          PLA.copy(X,XMIN);
        }
        if (PLA.first())
        {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
          algo_->set_scalar("original_error",orig);
          algo_->set_scalar("current_error",xmin);
          algo_->set_int("iteration",niter);
#endif
        }
        PLA.wait();
        return (true);
      }

      PLA.mult(A,X,R);
      PLA.sub(B,R,R);
      error = PLA.norm(R)/bnorm;

      if (error < tolerance)
      {
        if (error < xmin)
        {
          xmin = error;
          PLA.copy(X,XMIN);
        }
        if (PLA.first())
        {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
          algo_->set_scalar("original_error",orig);
          algo_->set_scalar("current_error",xmin);
          algo_->set_int("iteration",niter);
#endif
        }
        PLA.wait();
        return (true);
      }
    }

    if (errorcg <= tolerance)
    {
      PLA.copy(X,XCG);
      PLA.scale_add(snprod*(sn/cs),M,XCG,XCG);

      PLA.mult(A,XCG,R);
      PLA.sub(B,R,R);

      error = PLA.norm(R);

      if (error < tolerance)
      {
        if (error < xmin)
        {
          xmin = error;
          PLA.copy(XCG,XMIN);
        }
        if (PLA.first())
        {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
          algo_->set_scalar("original_error",orig);
          algo_->set_scalar("current_error",xmin);
          algo_->set_int("iteration",niter);
#endif
        }
        PLA.wait();
        return (true);
      }
    }

    if (error < xmin) { PLA.copy(X,XMIN); xmin = error; }
    if (PLA.first()) (*convergence_)[niter] = xmin;

    niter++;

    callback_step_cnt++;
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (callback_step_cnt == callback_step)
    {
      callback_step_cnt = 0;
      if (algo_->have_callbacks())
      {
        if (PLA.first())
        {
          algo_->set_scalar("original_error",orig);
          algo_->set_scalar("current_error",xmin);
          algo_->set_int("iteration",niter);
          algo_->do_callbacks();
        }
        PLA.wait();
        tolerance = algo_->get_scalar("target_error");
        max_iter = algo_->get_int("max_iterations");
        callback_step = algo_->get_int("callback_step");
      }
    }
#endif
    ucnt++;
    if (ucnt == 20)
    {
      ucnt = 0;
      algo_->update_progress((log_orig-log(error))/log_scale);
    }
  }

  if (PLA.first())
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    algo_->set_scalar("original_error",orig);
    algo_->set_scalar("current_error",xmin);
    algo_->set_int("iteration",niter);
    if (algo_->have_callbacks()) algo_->do_callbacks();
#endif
  }
  PLA.wait();

  return (true);
}


//------------------------------------------------------------------
// JACOBI Solver with simple preconditioner

class SolveLinearSystemJACOBIAlgo : public SolveLinearSystemParallelAlgo
{
public:
  explicit SolveLinearSystemJACOBIAlgo(const AlgorithmBase* base) : SolveLinearSystemParallelAlgo(base) {}
  virtual bool parallel(ParallelLinearAlgebra& PLA, SolverInputs& matrices) const;
};


bool
SolveLinearSystemJACOBIAlgo::
parallel(ParallelLinearAlgebra& PLA, SolverInputs& matrices) const
{
  // Algorithm
  // Define matrices and vectors to be used in the algorithm
  ParallelLinearAlgebra::ParallelMatrix A;
  ParallelLinearAlgebra::ParallelVector B, X, X0, XMIN;
  ParallelLinearAlgebra::ParallelVector DIAG,Z;

  double tolerance =     algo_->get(Variables::TargetError).toDouble();
  int    max_iter =      algo_->get(Variables::MaxIterations).toInt();
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  int    callback_step = algo_->get_int("callback_step");
#endif
  int    niter = 0;
  int    callback_step_cnt =0;

  // Create matrices and vectors that we need for this algorithm
  if ( !PLA.add_matrix(matrices.A,A) ||
       !PLA.add_vector(matrices.b,B) ||
       !PLA.add_vector(matrices.x0,X0) ||
       !PLA.add_vector(matrices.x,XMIN) ||
       !PLA.new_vector(X) ||
       !PLA.new_vector(DIAG) ||
       !PLA.new_vector(Z))
  {
    if (PLA.first())
    {
      algo_->error("Could not allocate enough memory for algorithm");
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      algo_->set_scalar("original_error",0.0);
      algo_->set_scalar("current_error",0.0);
      algo_->set_int("iteration",niter);
#endif
    }
    PLA.wait();

    return (false);
  }

  PLA.copy(X0,X);
  PLA.copy(X0,XMIN);

  // Build a preconditioner
  PLA.diag(A,DIAG);
  double max = PLA.absmax(DIAG);
  PLA.absthreshold_invert(DIAG,DIAG,1e-6*max);

  PLA.mult(A,X,Z);
  PLA.sub(Z,B,Z);

  double bnorm = PLA.norm(B);
  double error = PLA.norm(Z);

  PLA.copy(X,XMIN);
  double xmin = error;
  double orig = error;

  if (error <= tolerance)
  {
    if (PLA.first())
    {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      algo_->set_scalar("original_error",orig);
      algo_->set_scalar("current_error",xmin);
      algo_->set_int("iteration",niter);
      if (algo_->have_callbacks()) algo_->do_callbacks();
#endif
    }
    PLA.wait();
    return (true);
  }

  int cnt = 0;
  double log_target = log(tolerance);
  double log_orig =  log(orig);
  double log_scale = log_orig - log_target;

  while (niter < max_iter)
  {
    if (error <= tolerance)
    {
      if (PLA.first())
      {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
        algo_->set_scalar("original_error",orig);
        algo_->set_scalar("current_error",xmin);
        algo_->set_int("iteration",niter);
        if (algo_->have_callbacks()) algo_->do_callbacks();
#endif
      }
      PLA.wait();
      algo_->update_progress(1);
      return (true);
    }

    PLA.mult(DIAG,Z,Z);
    PLA.sub(Z,X,X);
    PLA.mult(A,X,Z);
    PLA.sub(Z,B,Z);
    error = PLA.norm(Z) / bnorm;
    if (error < xmin) { PLA.copy(X,XMIN); xmin = error; }
    if (PLA.first()) (*convergence_)[niter] = xmin;

    niter++;

    callback_step_cnt++;
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (callback_step_cnt == callback_step)
    {
      callback_step_cnt = 0;
      if (algo_->have_callbacks())
      {
        if (PLA.first())
        {
          algo_->set_scalar("original_error",orig);
          algo_->set_scalar("current_error",xmin);
          algo_->set_int("iteration",niter);
          algo_->do_callbacks();
        }
        PLA.wait();
        tolerance = algo_->get_scalar("target_error");
        max_iter = algo_->get_int("max_iterations");
        callback_step = algo_->get_int("callback_step");
      }
    }
#endif

    cnt++;
    if (cnt == 20)
    {
      cnt = 0;
      algo_->update_progress((log_orig-log(error))/log_scale);
    }
  }

  if (PLA.first())
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    algo_->set_scalar("original_error",orig);
    algo_->set_scalar("current_error",xmin);
    algo_->set_int("iteration",niter);
    if (algo_->have_callbacks()) algo_->do_callbacks();
#endif
  }
  PLA.wait();

  algo_->update_progress(1);
  return (true);
}

bool SolveLinearSystemAlgo::run(SparseRowMatrixHandle A,
                           DenseColumnMatrixHandle b,
                           DenseColumnMatrixHandle x0,
                           DenseColumnMatrixHandle& x) const
{
  DenseColumnMatrixHandle convergence;
  return run(A,b,x0,x,convergence);
}

bool SolveLinearSystemAlgo::run(SparseRowMatrixHandle A,
  DenseColumnMatrixHandle b,
                           DenseColumnMatrixHandle x0,
                           DenseColumnMatrixHandle& x,
                           DenseColumnMatrixHandle& convergence) const
{
  ScopedAlgorithmStatusReporter ssr(this, "SolveLinearSystem");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(A, "No matrix A is given");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(b, "No matrix b is given");

  double tolerance = get(Variables::TargetError).toDouble();
  int maxIterations = get(Variables::MaxIterations).toInt();
  ENSURE_POSITIVE_DOUBLE(tolerance, "Tolerance out of range!");
  ENSURE_POSITIVE_INT(maxIterations, "Max iterations out of range!");

  if (false/*!matrixIs::sparse(A)*/)
  {
    THROW_ALGORITHM_INPUT_ERROR("Matrix A is not sparse");
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    MatrixHandle Atmp = A->sparse();
    if (Atmp.get_rep() == 0)
    {
      error();
    }
    A = Atmp;
#endif
  }

  if (false)
  {
    THROW_ALGORITHM_INPUT_ERROR("Matrix b is not a dense or column matrix");
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    MatrixHandle btmp = b->column();
    if (btmp.get_rep() == 0)
    {
      error("Matrix b is not a dense or column matrix");
      return (false);
    }
#endif
  }

  if (!x0)
  {
    // create an x0 matrix
    auto temp(boost::make_shared<DenseColumnMatrix>(b->nrows()));
    temp->setZero();
    x0 = temp;
  }

  if ((x0->ncols() != 1) || (b->ncols() != 1))
  {
    THROW_ALGORITHM_INPUT_ERROR("Matrix x0 and b need to have the same number of rows");
  }

  if (A->nrows() != A->ncols())
  {
    THROW_ALGORITHM_INPUT_ERROR("Matrix A is not square");
  }

  if (A->nrows() != b->nrows())
  {
    THROW_ALGORITHM_INPUT_ERROR("Matrix A and b do not have the same number of rows");
  }

  if (A->nrows() != x0->nrows())
  {
    THROW_ALGORITHM_INPUT_ERROR("Matrix A and x0 do not have the same number of rows");
  }

  std::string method = getOption(Variables::Method);

  DenseColumnMatrixHandle conv;
  if (method == "cg")
  {
    SolveLinearSystemCGAlgo algo(this);
    if(!algo.run(A,b,x0,x,conv))
    {
      BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("Conjugate Gradient method failed"));
    }
  }
  else if (method == "bicg")
  {
    SolveLinearSystemBICGAlgo algo(this);
    if(!(algo.run(A,b,x0,x,conv)))
    {
      BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("BiConjugate Gradient method failed"));
    }
  }
  else if (method == "jacobi")
  {
    SolveLinearSystemJACOBIAlgo algo(this);
    if(!(algo.run(A,b,x0,x,conv)))
    {
      BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("Jacobi method failed"));
    }
  }
  else if (method == "minres")
  {
    SolveLinearSystemMINRESAlgo algo(this);
    if(!(algo.run(A,b,x0,x,conv)))
    {
      BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("MINRES method failed"));
    }
  }
  else
    BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("Unknown solver method"));

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (get_bool("build_convergence"))
  {
    if (conv)
    {
      int iteration = get_int("iteration");
      convergence = conv->clone();
    }
    else
    {
      BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("No convergence matrix"));
    }
  }
#endif
  return true;
}

AlgorithmOutput SolveLinearSystemAlgo::run(const AlgorithmInput& input) const
{
  auto lhs = input.get<SparseRowMatrix>(Variables::LHS);
  auto rhs = input.get<DenseColumnMatrix>(Variables::RHS);

  DenseColumnMatrixHandle solution;

  bool success = run(lhs, rhs, DenseColumnMatrixHandle(), solution);
  if (!success)
  {
    BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("SolveLinearSystem Algo returned false--need to improve error conditions so it throws before returning."));
  }

  AlgorithmOutput output;
  output[Variables::Solution] = boost::make_shared<DenseMatrix>(solution->col(0));
  return output;
}
