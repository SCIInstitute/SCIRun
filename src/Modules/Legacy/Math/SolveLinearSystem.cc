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


#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystem.h>
#include <Core/Math/MiscMath.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Thread/Time.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <iostream>
#include <sstream>

namespace SCIRun {

using namespace SCIRunAlgo;

/// @class SolveLinearSystem
/// @brief This module is used to solve the linear system Ax=b, where A is a
/// given Matrix, b is a given right-hand-side vector, and the user wants to
/// find the solution vector x.

class SolveLinearSystem : public Module, public AlgoCallBack {

  public:
    SolveLinearSystem(GuiContext* ctx);
    virtual ~SolveLinearSystem() {}

    virtual void execute();
    virtual bool callback();

    GuiDouble target_error_;
    GuiDouble orig_error_;
    GuiDouble current_error_;

    GuiString method_;
    GuiString precond_;

    GuiInt iteration_;
    GuiInt maxiter_;

    GuiInt use_previous_solution_;
    GuiInt emit_partial_;
    GuiInt emit_iter_;

  private:

    int    last_iter_;
    int    next_partial_;
    int    callback_step_;

    SCIRunAlgo::SolveLinearSystemAlgo algo_;

    MatrixHandle previous_solution_;

    bool checkErrorForInfinityBeforeSetting(GuiDouble& errorGui, const std::string& name, double errorVal);
    bool infinityReported_;
};

bool SolveLinearSystem::checkErrorForInfinityBeforeSetting(GuiDouble& errorGui, const std::string& name, double errorVal)
{
  if (IsInfinite(errorVal))
  {
    if (!infinityReported_)
    {
      infinityReported_ = true;
      error("Error value " + name + " has reached infinity, setting error to std::numeric_limits<double>::max.");
    }
    errorGui.set(std::numeric_limits<double>::max());
    return false;
  }
  else
  {
    errorGui.set(errorVal);
    return true;
  }
}

bool
SolveLinearSystem::callback()
{
  MatrixHandle convergence;
  algo_.get_handle("convergence",convergence);
  double* error = convergence->get_data_pointer();

  double target_error = algo_.get_scalar("target_error");
  double current_error = algo_.get_scalar("current_error");
  double orig_error = algo_.get_scalar("original_error");
  int iter = algo_.get_int("iteration");

  iteration_.set(iter);
  if (!checkErrorForInfinityBeforeSetting(orig_error_, "Original Error", orig_error))
    return false;
  if (!checkErrorForInfinityBeforeSetting(current_error_, "Current Error", current_error))
    return false;

  if (last_iter_ < 0)
  {
    last_iter_ = 0;
    emit_iter_.reset();
    next_partial_ = emit_iter_.get();
  }

  std::ostringstream str;
  str << get_id() << " append_graph " << iter << " \"";

  const double ERROR_THRESHOLD = 1000000;
  for (int i = last_iter_; i < iter; i++)
  {
    double err = error[i];
    if (0.0 == err)
      err = 1e-20;
    str << i << " " << (err < ERROR_THRESHOLD ? log10(err) : ERROR_THRESHOLD) << " ";
  }

  str << "\" \"";

  for (int i = last_iter_; i < iter; i++)
  {
    str << i << " " << log10(target_error) << " ";
  }
  str << "\" ; update idletasks";
  TCLInterface::execute(str.str());

  last_iter_ = iter;

  // Set new target
  target_error_.reset();
  algo_.set_scalar("target_error",target_error_.get());

  int callback_step = callback_step_;

  emit_partial_.reset();
  if (emit_partial_.get())
  {
    if (iter >= next_partial_)
    {

      // Send intermediate solutions
      MatrixHandle solution;
      algo_.get_handle("solution",solution);
      solution.detach();
      MatrixHandle num_iter = new DenseMatrix(iter);
      MatrixHandle residue = new DenseMatrix(current_error);

      send_output_handle("Solution", solution, true, true);
      send_output_handle("NumIterations",num_iter, true, true);
      send_output_handle("Residue",residue, true, true);
      emit_iter_.reset();

      next_partial_ = iter + Max(1,emit_iter_.get());
    }
  }
  else
  {
    next_partial_ = -1;
  }

  if (next_partial_ > 0)
  {
    if ((next_partial_-iter) < callback_step) callback_step = next_partial_-iter;
  }

  algo_.set_int("callback_step",callback_step);
  return (true);
}

DECLARE_MAKER(SolveLinearSystem)

SolveLinearSystem::SolveLinearSystem(GuiContext* ctx)
  : Module("SolveLinearSystem", ctx, Filter, "Math", "SCIRun"),
    target_error_(get_ctx()->subVar("target_error"), 0.00001),
    orig_error_(get_ctx()->subVar("orig_error"), 0.0),
    current_error_(get_ctx()->subVar("current_error"), 0.0),
    method_(get_ctx()->subVar("method"), "Conjugate Gradient & Precond. (SCI)"),
    precond_(get_ctx()->subVar("precond"), "jacobi"),
    iteration_(get_ctx()->subVar("iteration"), 0),
    maxiter_(get_ctx()->subVar("maxiter"), 500),
    use_previous_solution_(get_ctx()->subVar("use_previous_soln"), 1),
    emit_partial_(get_ctx()->subVar("emit_partial"), 1),
    emit_iter_(get_ctx()->subVar("emit_iter"), 50),
    infinityReported_(false)
{
  algo_.set_progress_reporter(this);
}


void
SolveLinearSystem::execute()
{
  MatrixHandle matrix, rhs, x0;

  get_input_handle("Matrix", matrix,true);
  get_input_handle("RHS", rhs, true);

  if (rhs->is_zero())
  {
    ColumnMatrixHandle zeroSolution(new ColumnMatrix(rhs->nrows()));
    zeroSolution->zero();
    send_output_handle("Solution", zeroSolution);
    warning("Returning zero solution vector immediately since input RHS equals the zero vector.");
    return;
  }

  if (inputs_changed_ || target_error_.changed() ||
      orig_error_.changed() || current_error_.changed() ||
      method_.changed() || precond_.changed() ||
      iteration_.changed() || maxiter_.changed() ||
      use_previous_solution_.changed() || emit_partial_.changed() ||
      emit_iter_.changed())
  {
    if (use_previous_solution_.get() &&
        previous_solution_.get_rep() && rhs.get_rep() &&
        previous_solution_->nrows() == rhs->nrows())
    {
      x0 = previous_solution_;
    }

    last_iter_ = 0;

    algo_.set_int("max_iterations",maxiter_.get());
    algo_.set_scalar("current_error",current_error_.get());
    algo_.set_scalar("target_error",target_error_.get());
    callback_step_ = 10;
    algo_.set_int("callback_step",callback_step_);

    const std::string meth = method_.get();
    if (meth == "Conjugate Gradient & Precond. (SCI)")
    {
      algo_.set_option("method","cg");
    }
    else if (meth == "BiConjugate Gradient & Precond. (SCI)")
    {
      algo_.set_option("method","bicg");
    }
    else if (meth == "Jacobi & Precond. (SCI)")
    {
      algo_.set_option("method","jacobi");
    }
    else if (meth == "MINRES & Precond. (SCI)")
    {
      algo_.set_option("method","minres");
    }
    else
    {
      error("Unknown method: " + meth);
      return;
    }

    const std::string pre = precond_.get();
    if (pre == "jacobi")
    {
      algo_.set_option("pre_conditioner",pre);
    }
    else
    {
      algo_.set_option("pre_conditioner",pre);
    }

    algo_.add_callback(this);

    MatrixHandle solution;
    infinityReported_ = false;

    double start_time = Time::currentSeconds();

    TCLInterface::execute(get_id() + " reset_graph");
    algo_.run(matrix,rhs,x0,solution);
    TCLInterface::execute(get_id()+" finish_graph");

    previous_solution_ = solution;

    double end_time = Time::currentSeconds();

    std::ostringstream perf;
    perf << "Linear solver took " <<(end_time-start_time)<< " seconds" <<  std::endl;
    perf << "Linear solver final residue is " <<
      algo_.get_scalar("current_error") << " after " << algo_.get_int("iteration") <<
      " iterations" << std::endl;

    remark(perf.str());

    MatrixHandle num_iter = new DenseMatrix(algo_.get_int("iteration"));
    MatrixHandle residue  = new DenseMatrix(algo_.get_scalar("current_error"));

    send_output_handle("Solution", solution, false, false);
    send_output_handle("NumIterations",num_iter,false, false);
    send_output_handle("Residue",residue,false, false);
  }
}

} // End namespace SCIRun
