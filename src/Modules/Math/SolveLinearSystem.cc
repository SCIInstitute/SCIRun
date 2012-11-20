/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
#include <Modules/Math/SolveLinearSystem.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystemAlgo.h>
#include <Core/Algorithms/Math/SolveLinearSystemWithEigen.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Logging/ScopedTimeRemarker.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;

SolveLinearSystemModule::SolveLinearSystemModule() : Module(ModuleLookupInfo("SolveLinearSystem", "Math", "SCIRun")) {}

void SolveLinearSystemModule::execute()
{
  auto A = getRequiredInput(Matrix);
  auto rhs = getRequiredInput(RHS);

  if (rhs->ncols() != 1)
    THROW_ALGORITHM_INPUT_ERROR("Right-hand side matrix must contain only one column.");
  if (!matrix_is::sparse(A))
    THROW_ALGORITHM_INPUT_ERROR("Left-hand side matrix to solve must be sparse.");

  auto ASparse = matrix_cast::as_sparse(A);

  auto rhsCol = matrix_cast::as_column(rhs);
  if (!rhsCol)
    rhsCol = matrix_convert::to_column(rhs);

  auto tolerance = get_state()->getValue(SolveLinearSystemAlgorithm::Tolerance).getDouble();
  auto maxIterations = get_state()->getValue(SolveLinearSystemAlgorithm::MaxIterations).getInt();

  std::ostringstream ostr;
  ostr << "Running algorithm Parallel CG Solver with tolerance " << tolerance << " and maximum iterations " << maxIterations;
  remark(ostr.str());

  SolveLinearSystemAlgo algo;
  algo.set(SolveLinearSystemAlgo::TargetError(), tolerance);
  algo.set(SolveLinearSystemAlgo::MaxIterations(), maxIterations);
  algo.setLogger(getLogger());
  algo.setUpdaterFunc(getUpdaterFunc());

  DenseColumnMatrixHandle solution;

  bool success;
  {
    ScopedTimeRemarker perf(this, "Linear solver");
    remark("Using Jacobi preconditioner");
    success = algo.run(ASparse, rhsCol, DenseColumnMatrixHandle(), solution);
  }
  if (!success)
  {
    MODULE_ERROR_WITH_TYPE(LinearAlgebraError, "SLS Algo returned false--need to improve error conditions so it throws before returning.");
  }

  //TODO: make ports/update GUI with resultant error+iterations.
  //send_output_handle(0, x.get<0>());

  //const std::string meth = method_.get();
  //if (meth == "Conjugate Gradient & Precond. (SCI)") 
  //{ 
  //  algo_.set_option("method","cg");
  //} 
  //else if (meth == "BiConjugate Gradient & Precond. (SCI)") 
  //{
  //  algo_.set_option("method","bicg");
  //} 
  //else if (meth == "Jacobi & Precond. (SCI)") 
  //{
  //  algo_.set_option("method","jacobi");
  //} 
  //else if (meth == "MINRES & Precond. (SCI)") 
  //{
  //  algo_.set_option("method","minres");
  //}   
  //else 
  //{
  //  error("Unknown method: " + meth);
  //  return;
  //}

  
  //const std::string pre = precond_.get();
  //if (pre == "jacobi")
  //{
  //  algo_.set_option("pre_conditioner",pre);
  //}
  //else
  //{
  //  algo_.set_option("pre_conditioner",pre);
  //}

  //algo_.add_callback(this);

  //MatrixHandle solution;
  //infinityReported_ = false;

  //double start_time = Time::currentSeconds();

  //TCLInterface::execute(get_id() + " reset_graph");
  //algo_.run(matrix,rhs,x0,solution);
  //TCLInterface::execute(get_id()+" finish_graph");

  //previous_solution_ = solution;

  //double end_time = Time::currentSeconds();

  //std::ostringstream perf;
  //perf << "Linear solver final residue is " << 
  //  algo_.get_scalar("current_error") << " after " << algo_.get_int("iteration") <<
  //  " iterations" << std::endl;

  //remark(perf.str());

  //MatrixHandle num_iter = new DenseMatrix(algo_.get_int("iteration"));
  //MatrixHandle residue  = new DenseMatrix(algo_.get_scalar("current_error"));





  sendOutput(Solution, solution);





  //send_output_handle(/*"Solution"*/ 0, solution);
  //send_output_handle("NumIterations",num_iter,false, false);
  //send_output_handle("Residue",residue,false, false);
}