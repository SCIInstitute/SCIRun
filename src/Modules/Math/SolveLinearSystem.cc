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
/// @todo Documentation Modules/Math/SolveLinearSystem.cc

#include <iostream>
#include <Modules/Math/SolveLinearSystem.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Logging/ScopedTimeRemarker.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;

SolveLinearSystemModule::SolveLinearSystemModule() : Module(ModuleLookupInfo("SolveLinearSystem", "Math", "SCIRun"))
{
  INITIALIZE_PORT(LHS);
  INITIALIZE_PORT(RHS);
  INITIALIZE_PORT(Solution);
}

void SolveLinearSystemModule::setStateDefaults()
{
  auto state = get_state();
  setStateDoubleFromAlgo(Variables::TargetError);
  setStateDoubleFromAlgo(Variables::MaxIterations);
  setStateStringFromAlgoOption(Variables::Method);
  setStateStringFromAlgoOption(Variables::Preconditioner);
}

void SolveLinearSystemModule::execute()
{
  auto A = getRequiredInput(LHS);
  auto rhs = getRequiredInput(RHS);

  if (needToExecute())
  {
    /// @todo: why aren't these checks in the algo class?
    if (rhs->ncols() != 1)
      THROW_ALGORITHM_INPUT_ERROR("Right-hand side matrix must contain only one column.");
    if (!matrix_is::sparse(A))
      THROW_ALGORITHM_INPUT_ERROR("Left-hand side matrix to solve must be sparse.");

    auto rhsCol = matrix_cast::as_column(rhs);
    if (!rhsCol)
      rhsCol = matrix_convert::to_column(rhs);

    auto tolerance = get_state()->getValue(Variables::TargetError).getDouble();
    auto maxIterations = get_state()->getValue(Variables::MaxIterations).getInt();

    algo().set(Variables::TargetError, tolerance);
    algo().set(Variables::MaxIterations, maxIterations);

    auto method = get_state()->getValue(Variables::Method).getString();
    auto precond = get_state()->getValue(Variables::Preconditioner).getString();
    algo().set_option(Variables::Method, method);
    algo().set_option(Variables::Preconditioner, precond);

    std::ostringstream ostr;
    ostr << "Running algorithm Parallel " << method << " Solver with tolerance " << tolerance << " and maximum iterations " << maxIterations;
    remark(ostr.str());

    {
      ScopedTimeRemarker perf(this, "Linear solver");
      remark("Using preconditioner: " + precond);

      auto output = algo().run_generic(make_input((LHS, A)(RHS, rhsCol)));

      sendOutputFromAlgorithm(Solution, output);
    }
  }
}
