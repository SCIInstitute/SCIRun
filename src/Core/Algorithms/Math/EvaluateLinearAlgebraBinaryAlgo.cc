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

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

EvaluateLinearAlgebraBinaryAlgorithm::EvaluateLinearAlgebraBinaryAlgorithm()
{
  addParameter(Variables::Operator, 0);
}

EvaluateLinearAlgebraBinaryAlgorithm::Outputs EvaluateLinearAlgebraBinaryAlgorithm::run(const EvaluateLinearAlgebraBinaryAlgorithm::Inputs& inputs, const EvaluateLinearAlgebraBinaryAlgorithm::Parameters& params) const
{
  DenseMatrixHandle result;
  DenseMatrixConstHandle lhs = inputs.get<0>();
  DenseMatrixConstHandle rhs = inputs.get<1>();
  ENSURE_ALGORITHM_INPUT_NOT_NULL(lhs, "lhs");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(rhs, "rhs");

  Operator oper = params;

  /// @todo: absolutely need matrix move semantics here!!!!!!!
  switch (oper)
  {
  case ADD:
    result.reset(lhs->clone());
    *result += *rhs;
    break;
  case SUBTRACT:
    result.reset(lhs->clone());
    *result -= *rhs;
    break;
  case MULTIPLY:
    result.reset(lhs->clone());
    *result *= *rhs;
    break;
  default:
    THROW_ALGORITHM_INPUT_ERROR("ERROR: unknown binary operation");
    break;
  }

  return result;
}

AlgorithmOutput EvaluateLinearAlgebraBinaryAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto LHS = input.get<DenseMatrix>(Variables::LHS);
  auto RHS = input.get<DenseMatrix>(Variables::RHS);

  auto result = run(boost::make_tuple(LHS, RHS), Operator(get(Variables::Operator).getInt()));

  AlgorithmOutput output;
  output[Variables::Result] = result;
  return output;
}