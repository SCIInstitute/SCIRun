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

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <stdexcept>

#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;

EvaluateLinearAlgebraUnaryAlgorithm::EvaluateLinearAlgebraUnaryAlgorithm()
{
  addParameter(Variables::Operator, 0);
  addParameter(Variables::ScalarValue, 0);
	addParameter(Variables::FunctionString, std::string("x+10"));
}

EvaluateLinearAlgebraUnaryAlgorithm::Outputs EvaluateLinearAlgebraUnaryAlgorithm::run(const EvaluateLinearAlgebraUnaryAlgorithm::Inputs& matrix, const EvaluateLinearAlgebraUnaryAlgorithm::Parameters& params) const
{
  ENSURE_ALGORITHM_INPUT_NOT_NULL(matrix, "matrix");
  MatrixHandle result;

  Operator oper = params.get<0>();

  /// @todo: absolutely need matrix move semantics here!!!!!!!
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
        THROW_ALGORITHM_INPUT_ERROR("No scalar value available to multiply!");
      double scalar = scalarOption.get();
      result.reset(matrix->clone());
      (*result) *= scalar;
    }
    break;
	case FUNCTION:
		{
				NewArrayMathEngine engine;
				MatrixHandle matrixInput;
				matrixInput.reset(matrix->clone());

				if (!(engine.add_input_fullmatrix("x", matrixInput ) ))
          THROW_ALGORITHM_INPUT_ERROR("Error setting up parser");

				boost::optional<std::string> func = params.get<2>();
				std::string function_string = func.get();

				function_string = "RESULT="+function_string;
				engine.add_expressions(function_string);

				if(!(engine.add_output_fullmatrix("RESULT",matrixInput)))
          THROW_ALGORITHM_INPUT_ERROR("Error setting up parser");
				// Actual engine call, which does the dynamic compilation, the creation of the
				// code for all the objects, as well as inserting the function and looping
				// over every data point
				if (!(engine.run()))
          THROW_ALGORITHM_INPUT_ERROR("Error running math engine");
				result = matrix_cast::as_dense(matrixInput);
		}
		break;
  default:
    THROW_ALGORITHM_INPUT_ERROR("Unknown operand");
  }

  return result;
}

AlgorithmOutput EvaluateLinearAlgebraUnaryAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto matrix = input.get<DenseMatrix>(Variables::InputMatrix);

  auto scalar = boost::make_optional(get(Variables::ScalarValue).toDouble());
	auto function = boost::make_optional(get(Variables::FunctionString).toString());

  auto result = run(matrix, boost::make_tuple(Operator(get(Variables::Operator).toInt()), scalar, function));

  AlgorithmOutput output;
  output[Variables::Result] = result;
  return output;
}
