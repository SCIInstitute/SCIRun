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

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystem.h>
#include <Core/Algorithms/Math/ParallelAlgebra/ParallelLinearAlgebra.h>

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

namespace SCIRunAlgo {

using namespace SCIRun;


//-------------------------------------------------------
// ParallelSolver base

class SolveLinearSystemParallelAlgo : public ParallelLinearAlgebraBase 
{
  public:
    bool run(AlgoBase* base, MatrixHandle a, MatrixHandle b,
             MatrixHandle x0, MatrixHandle& x,
             MatrixHandle& convergence);
             
 protected: 

  std::string pre_conditioner_;
  double*     convergence_;
  AlgoBase*   algo_;
};


bool
SolveLinearSystemParallelAlgo::run(AlgoBase* algo, 
                                   MatrixHandle a, MatrixHandle b, 
                                   MatrixHandle x0, MatrixHandle& x, 
                                   MatrixHandle& convergence)
{
  // Create vector with matrices that need to be processed
  std::vector<MatrixHandle> matrix(4);
  // Copy pointers of input matrices
  matrix[0] = a; 
  matrix[1] = b; 
  matrix[2] = x0;
  
  // Create output matrix
  size_type size = x0->nrows();
  x = new ColumnMatrix(size);
  if (x.get_rep() == 0)
  {
    algo->error("Could not allocate output matrix");
    return (false);
  }
  
  // Copy output matrix pointer
  matrix[3] = x;

  // Create convergence matrix
  int num_iter = algo->get_int("max_iterations");
  convergence = new ColumnMatrix(num_iter);
  if (convergence.get_rep() == 0)
  {
    algo->error("Could not allocate convergence matrix");
    return (false);
  }
  convergence_ = convergence->get_data_pointer();
  
  // Set intermediate solution handle
  algo_ = algo;
  algo->set_handle("solution",x);
  algo->set_handle("convergence",convergence);
  
  pre_conditioner_ = algo_->get_option("pre_conditioner");
  
  if(!(start_parallel(matrix)))
  {
    algo->error("Encountered an error while running parallel linear algebra");
    return (false);
  }

  return (true);
}  

//------------------------------------------------------------------
// CG Solver with simple preconditioner

class SolveLinearSystemCGAlgo : public SolveLinearSystemParallelAlgo 
{
  public:
             
    virtual bool parallel(ParallelLinearAlgebra& PLA, std::vector<MatrixHandle>& matrix);
};


bool
SolveLinearSystemCGAlgo::
parallel(ParallelLinearAlgebra& PLA, std::vector<MatrixHandle>& matrix)
{
  // Algorithm
  
  // Define matrices and vectors to be used in the algorithm
  ParallelLinearAlgebra::ParallelMatrix A; 
  ParallelLinearAlgebra::ParallelVector B, X, X0, XMIN, DIAG, R, Z, P;

  double tolerance =     algo_->get_scalar("target_error");
  int    max_iter =      algo_->get_int("max_iterations");
  int    callback_step = algo_->get_int("callback_step");
  int    niter = 0;
  int    callback_step_cnt =0;

  // Create matrices and vectors that we need for this algorithm
  if ( !PLA.add_matrix(matrix[0],A) ||
       !PLA.add_vector(matrix[1],B) ||
       !PLA.add_vector(matrix[2],X0) ||
       !PLA.add_vector(matrix[3],XMIN))
  {
    if (PLA.first()) 
    {
      algo_->error("Could not link matrices");
      algo_->set_scalar("original_error",0.0);
      algo_->set_scalar("current_error",0.0);
      algo_->set_int("iteration",niter);
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
      algo_->set_scalar("original_error",0.0);
      algo_->set_scalar("current_error",0.0);
      algo_->set_int("iteration",niter);
    }
    PLA.wait();
    return (false);
  }

  PLA.copy(X0,X);
  PLA.copy(X0,XMIN);
  
  // Build a preconditioner
  if (pre_conditioner_ == "jacobi")
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
      algo_->set_scalar("original_error",orig);
      algo_->set_scalar("current_error",xmin);
      algo_->set_int("iteration",niter);
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
        algo_->set_scalar("original_error",orig);
        algo_->set_scalar("current_error",xmin);
        algo_->set_int("iteration",niter);
        if (algo_->have_callbacks()) algo_->do_callbacks();
      }
      PLA.wait();
      return (true);
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
        
    if (error < xmin) { PLA.copy(X,XMIN); xmin = error; }
    if (PLA.first()) convergence_[niter] = xmin;

    niter++;
    
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
    algo_->set_scalar("original_error",orig);
    algo_->set_scalar("current_error",xmin);
    algo_->set_int("iteration",niter);
    if (algo_->have_callbacks()) algo_->do_callbacks();
  }
  PLA.wait();
  
  return (true);
}
 
//------------------------------------------------------------------
// BICG Solver with simple preconditioner


class SolveLinearSystemBICGAlgo : public SolveLinearSystemParallelAlgo 
{
  public:
    virtual bool parallel(ParallelLinearAlgebra& PLA, 
                          std::vector<MatrixHandle>& matrix);
};


bool
SolveLinearSystemBICGAlgo::
parallel(ParallelLinearAlgebra& PLA, std::vector<MatrixHandle>& matrix)
{
  // Algorithm
  
  // Define matrices and vectors to be used in the algorithm
  ParallelLinearAlgebra::ParallelMatrix A; 
  ParallelLinearAlgebra::ParallelVector B, X, X0, XMIN; 
  ParallelLinearAlgebra::ParallelVector DIAG, R, R1, Z, Z1, P, P1;

  double tolerance =     algo_->get_scalar("target_error");
  int    max_iter =      algo_->get_int("max_iterations");
  int    callback_step = algo_->get_int("callback_step");
  int    niter = 0;
  int    callback_step_cnt =0;

  // Create matrices and vectors that we need for this algorithm
  if ( !PLA.add_matrix(matrix[0],A) ||
       !PLA.add_vector(matrix[1],B) ||
       !PLA.add_vector(matrix[2],X0) ||
       !PLA.add_vector(matrix[3],XMIN) ||
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
      algo_->set_scalar("original_error",0.0);
      algo_->set_scalar("current_error",0.0);
      algo_->set_int("iteration",niter);
    }
    PLA.wait();

    return (false);
  }

  PLA.copy(X0,X);
  PLA.copy(X0,XMIN);
  
  // Build a preconditioner
  // Build a preconditioner
  if (pre_conditioner_ == "jacobi")
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
      algo_->set_scalar("original_error",orig);
      algo_->set_scalar("current_error",xmin);
      algo_->set_int("iteration",niter);
      if (algo_->have_callbacks()) algo_->do_callbacks();
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
        algo_->set_scalar("original_error",orig);
        algo_->set_scalar("current_error",xmin);
        algo_->set_int("iteration",niter);
        if (algo_->have_callbacks()) algo_->do_callbacks();
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
        algo_->set_scalar("original_error",orig);
        algo_->set_scalar("current_error",xmin);
        algo_->set_int("iteration",niter);
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
    if (PLA.first()) convergence_[niter] = xmin;
    
    niter++;
    
    callback_step_cnt++;
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
    cnt++;
    if (cnt == 20) 
    {
      cnt = 0;
      algo_->update_progress((log_orig-log(error))/log_scale);
    }
  }
  
  if (PLA.first())
  {
    algo_->set_scalar("original_error",orig);
    algo_->set_scalar("current_error",xmin);
    algo_->set_int("iteration",niter);
    if (algo_->have_callbacks()) algo_->do_callbacks();
  }
  PLA.wait();
  
  return (true);
}


//------------------------------------------------------------------
// BICG Solver with simple preconditioner


class SolveLinearSystemMINRESAlgo : public SolveLinearSystemParallelAlgo 
{
  public:
    virtual bool parallel(ParallelLinearAlgebra& PLA, 
                          std::vector<MatrixHandle>& matrix);
};


bool
SolveLinearSystemMINRESAlgo::
parallel(ParallelLinearAlgebra& PLA, std::vector<MatrixHandle>& matrix)
{
  // Algorithm
  
  // Define matrices and vectors to be used in the algorithm
  ParallelLinearAlgebra::ParallelMatrix A; 
  ParallelLinearAlgebra::ParallelVector B, X, X0, XMIN; 
  ParallelLinearAlgebra::ParallelVector DIAG, R, V, VOLD, VV;
  ParallelLinearAlgebra::ParallelVector VOLDER, M, MOLD, MOLDER, XCG;

  double tolerance =     algo_->get_scalar("target_error");
  int    max_iter =      algo_->get_int("max_iterations");
  int    callback_step = algo_->get_int("callback_step");
  int    niter = 0;
  int    callback_step_cnt =0;

  // Create matrices and vectors that we need for this algorithm
  if ( !PLA.add_matrix(matrix[0],A) ||
       !PLA.add_vector(matrix[1],B) ||
       !PLA.add_vector(matrix[2],X0) ||
       !PLA.add_vector(matrix[3],XMIN) ||
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
      algo_->set_scalar("original_error",0.0);
      algo_->set_scalar("current_error",0.0);
      algo_->set_int("iteration",niter);
    }
    PLA.wait();

    return (false);
  }

  PLA.copy(X0,X);
  PLA.copy(X0,XMIN);
  
  // Build a preconditioner
  // Build a preconditioner
  if (pre_conditioner_ == "jacobi")
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
      algo_->set_scalar("original_error",orig);
      algo_->set_scalar("current_error",xmin);
      algo_->set_int("iteration",niter);
      if (algo_->have_callbacks()) algo_->do_callbacks();
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

  error = Abs(snprod);  
  double errorcg = error/(Abs(cs)*bnorm);
  
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
        algo_->set_scalar("original_error",orig);
        algo_->set_scalar("current_error",xmin);
        algo_->set_int("iteration",niter);
        if (algo_->have_callbacks()) algo_->do_callbacks();
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
          algo_->set_scalar("original_error",orig);
          algo_->set_scalar("current_error",xmin);
          algo_->set_int("iteration",niter);
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
          algo_->set_scalar("original_error",orig);
          algo_->set_scalar("current_error",xmin);
          algo_->set_int("iteration",niter);
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
          algo_->set_scalar("original_error",orig);
          algo_->set_scalar("current_error",xmin);
          algo_->set_int("iteration",niter);
        }
        PLA.wait();
        return (true);       
      }
    }
    
    if (error < xmin) { PLA.copy(X,XMIN); xmin = error; }
    if (PLA.first()) convergence_[niter] = xmin;
    
    niter++;
    
    callback_step_cnt++;
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
    ucnt++;
    if (ucnt == 20) 
    {
      ucnt = 0;
      algo_->update_progress((log_orig-log(error))/log_scale);
    }
  }
  
  if (PLA.first())
  {
    algo_->set_scalar("original_error",orig);
    algo_->set_scalar("current_error",xmin);
    algo_->set_int("iteration",niter);
    if (algo_->have_callbacks()) algo_->do_callbacks();
  }
  PLA.wait();
  
  return (true);
}



//------------------------------------------------------------------
// JACOBI Solver with simple preconditioner


class SolveLinearSystemJACOBIAlgo : public SolveLinearSystemParallelAlgo 
{
  public:
    virtual bool parallel(ParallelLinearAlgebra& PLA, 
                          std::vector<MatrixHandle>& matrix);
};


bool
SolveLinearSystemJACOBIAlgo::
parallel(ParallelLinearAlgebra& PLA, std::vector<MatrixHandle>& matrix)
{
  // Algorithm
  // Define matrices and vectors to be used in the algorithm
  ParallelLinearAlgebra::ParallelMatrix A; 
  ParallelLinearAlgebra::ParallelVector B, X, X0, XMIN; 
  ParallelLinearAlgebra::ParallelVector DIAG,Z;

  double tolerance =     algo_->get_scalar("target_error");
  int    max_iter =      algo_->get_int("max_iterations");
  int    callback_step = algo_->get_int("callback_step");
  int    niter = 0;
  int    callback_step_cnt =0;

  // Create matrices and vectors that we need for this algorithm
  if ( !PLA.add_matrix(matrix[0],A) ||
       !PLA.add_vector(matrix[1],B) ||
       !PLA.add_vector(matrix[2],X0) ||
       !PLA.add_vector(matrix[3],XMIN) ||
       !PLA.new_vector(X) ||
       !PLA.new_vector(DIAG) ||
       !PLA.new_vector(Z))
  {
    if (PLA.first()) 
    {
      algo_->error("Could not allocate enough memory for algorithm");
      algo_->set_scalar("original_error",0.0);
      algo_->set_scalar("current_error",0.0);
      algo_->set_int("iteration",niter);
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
      algo_->set_scalar("original_error",orig);
      algo_->set_scalar("current_error",xmin);
      algo_->set_int("iteration",niter);
      if (algo_->have_callbacks()) algo_->do_callbacks();
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
        algo_->set_scalar("original_error",orig);
        algo_->set_scalar("current_error",xmin);
        algo_->set_int("iteration",niter);
        if (algo_->have_callbacks()) algo_->do_callbacks();
      }
      PLA.wait();
      return (true);
    }
    
    PLA.mult(DIAG,Z,Z);
    PLA.sub(Z,X,X);
    PLA.mult(A,X,Z);
    PLA.sub(Z,B,Z);
    error = PLA.norm(Z) / bnorm;
    if (error < xmin) { PLA.copy(X,XMIN); xmin = error; }
    if (PLA.first()) convergence_[niter] = xmin;
    
    niter++;
    
    callback_step_cnt++;
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
    
    cnt++;
    if (cnt == 20) 
    {
      cnt = 0;
      algo_->update_progress((log_orig-log(error))/log_scale);
    }
  }
  
  if (PLA.first())
  {
    algo_->set_scalar("original_error",orig);
    algo_->set_scalar("current_error",xmin);
    algo_->set_int("iteration",niter);
    if (algo_->have_callbacks()) algo_->do_callbacks();
  }
  PLA.wait();
  
  return (true);
}


bool
SolveLinearSystemAlgo::run(MatrixHandle A,
                           MatrixHandle b,
                           MatrixHandle x0, 
                           MatrixHandle& x)
{
  MatrixHandle convergence;
  return(run(A,b,x0,x,convergence));
}

bool
SolveLinearSystemAlgo::run(MatrixHandle A,
                           MatrixHandle b,
                           MatrixHandle x0, 
                           MatrixHandle& x,
                           MatrixHandle& convergence)
{
  algo_start("SolveLinearSystem");
  
  if (A.get_rep() == 0)
  {
    error("No matrix A is given");
    algo_end(); return (false);
  }

  if (b.get_rep() == 0)
  {
    error("No matrix b is given");
    algo_end(); return (false);
  }

  if (!(matrix_is::sparse(A)))
  {
    MatrixHandle Atmp = A->sparse();
    if (Atmp.get_rep() == 0)
    {
      error("Matrix A is not sparse");
      algo_end(); return (false);
    }
    A = Atmp;
  }

  if (!(matrix_is::dense(b)) && !(matrix_is::column(b)))
  {
    MatrixHandle btmp = b->column();
    if (btmp.get_rep() == 0)
    {
      error("Matrix b is not a dense or column matrix");
      algo_end(); return (false);
    }
  }

  if (x0.get_rep() == 0)
  {
    // create an x0 matrix
    x0 = b;
    x0.detach();
    x0->zero();
  }

  if (!(matrix_is::dense(x0)) && !(matrix_is::column(x0)))
  {
    MatrixHandle x0tmp = x0->column();
    if (x0tmp.get_rep() == 0)
    {
      error("Matrix x0 is not a dense or column matrix");
      algo_end(); return (false);
    }
  }
  
  if ((x0->ncols() != 1) || (b->ncols() != 1))
  {
    error("Matrix x0 and b need to have the same number of rows");
    algo_end(); return (false);
  }
  
  if (A->nrows() != A->ncols())
  {
    error("Matrix A is not square");
    algo_end(); return (false);
  }
  
  if (A->nrows() != b->nrows())
  {
    error("Matrix A and b do not have the same number of rows");
    algo_end(); return (false);
  }

  if (A->nrows() != x0->nrows())
  {
    error("Matrix A and b do not have the same number of rows");
    algo_end(); return (false);
  }
  
  std::string method = get_option("method");
  
  MatrixHandle conv;
  if (method == "cg")
  {
    SolveLinearSystemCGAlgo algo;
    if(!(algo.run(this,A,b,x0,x,conv)))
    {
      error("Conjungate Gradient method failed");
      algo_end(); return (false);
    }
  }
  else if (method == "bicg")
  {
    SolveLinearSystemBICGAlgo algo;
    if(!(algo.run(this,A,b,x0,x,conv)))
    {
      error("BiConjungate Gradient method failed");
      algo_end(); return (false);
    }
  }
  else if (method == "jacobi")
  {
    SolveLinearSystemJACOBIAlgo algo;
    if(!(algo.run(this,A,b,x0,x,conv)))
    {
      error("Jacobi method failed");
      algo_end(); return (false);
    }
  }
  else if (method == "minres")
  {
    SolveLinearSystemMINRESAlgo algo;
    if(!(algo.run(this,A,b,x0,x,conv)))
    {
      error("MINRES method failed");
      algo_end(); return (false);
    }
  }
      
  if (get_bool("build_convergence"))
  {
    if (conv.get_rep())
    {
      int iteration = get_int("iteration");
      convergence = new ColumnMatrix(iteration);
      if (convergence.get_rep() == 0)
      {
        error("Could not allocate output matrix");
        algo_end(); return (false);
      }
      double* odata = convergence->get_data_pointer();
      double* idata = conv->get_data_pointer();
      
      for (index_type i=0; i<iteration; i++)
        odata[i] = idata[i];
    }
    else
    {
      error("No convergence matrix");
      algo_end(); return (false);
    }
  }
  algo_end(); return (true);
}
    
}

