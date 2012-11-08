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

#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnary.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <stdexcept>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;

AlgorithmParameterName EvaluateLinearAlgebraUnaryAlgorithm::OperatorName("Operator");
AlgorithmParameterName EvaluateLinearAlgebraUnaryAlgorithm::ScalarValue("Scalar");

EvaluateLinearAlgebraUnaryAlgorithm::Outputs EvaluateLinearAlgebraUnaryAlgorithm::run(const EvaluateLinearAlgebraUnaryAlgorithm::Inputs& matrix, const EvaluateLinearAlgebraUnaryAlgorithm::Parameters& params) const
{
  ENSURE_ALGORITHM_INPUT_NOT_NULL(matrix, "matrix");
  DenseMatrixHandle result;

  Operator oper = params.get<0>();

  //TODO: absolutely need matrix move semantics here!!!!!!!
  switch (oper)
  {
  case NEGATE:
    result.reset(matrix->clone());
    (*result) *= -1;
    break;
  case TRANSPOSE:
    result.reset(matrix->clone());
    result->transposeInPlace();
    break;
  case SCALAR_MULTIPLY:
    {
      boost::optional<double> scalarOption = params.get<1>();
      if (!scalarOption)
        ALGORITHM_INPUT_ERROR("No scalar value available to multiply!");
      double scalar = scalarOption.get();
      result.reset(matrix->clone());
      (*result) *= scalar;
    }
    break;
  default:
    ALGORITHM_INPUT_ERROR("Unknown operand");
  }

  return result;
}