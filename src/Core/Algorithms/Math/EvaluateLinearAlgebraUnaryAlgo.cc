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
 

#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixMathVisitors.h>
#include <stdexcept>

#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes::MatrixMath;
using namespace SCIRun::Core::Algorithms;

EvaluateLinearAlgebraUnaryAlgorithm::EvaluateLinearAlgebraUnaryAlgorithm()
{
  addParameter(Variables::Operator, 0);
  addParameter(Variables::ScalarValue, 0);
	addParameter(Variables::FunctionString, std::string("x+10"));
}

namespace impl
{
  class TransposeMatrix : public Matrix::Visitor
  {
  public:
    virtual void visit(DenseMatrixGeneric<double>& dense) override
    {
      dense.transposeInPlace();
    }
    virtual void visit(SparseRowMatrixGeneric<double>& sparse) override
    {
      sparse = sparse.transpose();
    }
    virtual void visit(DenseColumnMatrixGeneric<double>& column) override
    {
      column = column.transpose();
    }
  };
}

EvaluateLinearAlgebraUnaryAlgorithm::Outputs EvaluateLinearAlgebraUnaryAlgorithm::run(const EvaluateLinearAlgebraUnaryAlgorithm::Inputs& matrix, const EvaluateLinearAlgebraUnaryAlgorithm::Parameters& params) const
{
  ENSURE_ALGORITHM_INPUT_NOT_NULL(matrix, "matrix");
  MatrixHandle result;

  Operator oper = params.op;

  /// @todo: absolutely need matrix move semantics here!!!!!!!
  switch (oper)
  {
  case NEGATE:
  {
    result.reset(matrix->clone());
    NegateMatrix negate;
    result->accept(negate);
    break;
  }
  case TRANSPOSE:
  {
    result.reset(matrix->clone());
    impl::TransposeMatrix tr;
    result->accept(tr);
    break;
  }
  case SCALAR_MULTIPLY:
  {
    auto scalar = params.scalar;
    result.reset(matrix->clone());
    ScalarMultiplyMatrix mult(scalar);
    result->accept(mult);
  }
  break;
  case FUNCTION:
  {
    // BUG FIX: the ArrayMathEngine is not well designed for use with sparse matrices, especially allocating proper space for the result.
    // There's no way to know ahead of time, so I'll just throw an error here and require the user to do this type of math elsewhere.
    if (matrixIs::sparse(matrix) && (matrix->nrows() * matrix->ncols() > 10000))
    {
      THROW_ALGORITHM_INPUT_ERROR("ArrayMathEngine needs overhaul to be used with large sparse inputs. See https://github.com/SCIInstitute/SCIRun/issues/482");
    }
    NewArrayMathEngine engine;
    result.reset(matrix->clone());

    if (!(engine.add_input_fullmatrix("x", matrix)))
      THROW_ALGORITHM_INPUT_ERROR("Error setting up parser");

    auto function_string = params.func;

    function_string = "RESULT=" + function_string;
    engine.add_expressions(function_string);

    if (!(engine.add_output_fullmatrix("RESULT", result)))
      THROW_ALGORITHM_INPUT_ERROR("Error setting up parser");
    // Actual engine call, which does the dynamic compilation, the creation of the
    // code for all the objects, as well as inserting the function and looping
    // over every data point
    if (!engine.run())
      THROW_ALGORITHM_INPUT_ERROR("Error running math engine");
  }
  break;
  default:
    THROW_ALGORITHM_INPUT_ERROR("Unknown operand");
  }

  return result;
}

AlgorithmOutput EvaluateLinearAlgebraUnaryAlgorithm::run(const AlgorithmInput& input) const
{
  auto matrix = input.get<Matrix>(Variables::InputMatrix);

  auto scalar = get(Variables::ScalarValue).toDouble();
	auto function = get(Variables::FunctionString).toString();

  auto result = run(matrix, { Operator(get(Variables::Operator).toInt()), scalar, function });

  AlgorithmOutput output;
  output[Variables::Result] = result;
  return output;
}
