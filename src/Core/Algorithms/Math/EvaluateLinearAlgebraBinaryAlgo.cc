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
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixMathVisitors.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Datatypes::MatrixMath;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun;

EvaluateLinearAlgebraBinaryAlgorithm::EvaluateLinearAlgebraBinaryAlgorithm()
{
  addParameter(Variables::Operator, 0);
  addParameter(Variables::FunctionString, std::string("x+y"));
}

EvaluateLinearAlgebraBinaryAlgorithm::Outputs EvaluateLinearAlgebraBinaryAlgorithm::run(const EvaluateLinearAlgebraBinaryAlgorithm::Inputs& inputs, const EvaluateLinearAlgebraBinaryAlgorithm::Parameters& params) const
{
  MatrixHandle result;
  auto lhs = inputs.get<0>();
  auto rhs = inputs.get<1>();
  ENSURE_ALGORITHM_INPUT_NOT_NULL(lhs, "lhs");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(rhs, "rhs");

  auto oper = params.op;
  switch (oper)
  {
  case ADD:
  {
    if (lhs->nrows() != rhs->nrows() || lhs->ncols() != rhs->ncols())
      THROW_ALGORITHM_INPUT_ERROR("Invalid dimensions to add matrices.");
    AddMatrices add(lhs);
    rhs->accept(add);
    return add.sum_;
  }
  case SUBTRACT:
  {
    if (lhs->nrows() != rhs->nrows() || lhs->ncols() != rhs->ncols())
      THROW_ALGORITHM_INPUT_ERROR("Invalid dimensions to subtract matrices.");
    result.reset(rhs->clone());
    NegateMatrix neg;
    result->accept(neg);
    AddMatrices add(lhs);
    result->accept(add);
    return add.sum_;
  }
  case MULTIPLY:
  {
    if (lhs->ncols() != rhs->nrows())
      THROW_ALGORITHM_INPUT_ERROR("Invalid dimensions to multiply matrices.");
    MultiplyMatrices mult(lhs);
    rhs->accept(mult);
    return mult.getProduct();
  }
  case FUNCTION:
  {
    // BUG FIX: the ArrayMathEngine is not well designed for use with sparse matrices, especially allocating proper space for the result.
    // There's no way to know ahead of time, so I'll just throw an error here and require the user to do this type of math elsewhere.
    if (matrixIs::sparse(lhs) || matrixIs::sparse(rhs))
    {
      if ((lhs->nrows() * lhs->ncols() > 10000) || (rhs->nrows() * rhs->ncols() > 10000))
        THROW_ALGORITHM_INPUT_ERROR("ArrayMathEngine needs overhaul to be used with large sparse inputs. See https://github.com/SCIInstitute/SCIRun/issues/482");
    }

    NewArrayMathEngine engine;
    MatrixHandle lhsInput(lhs->clone()), rhsInput(rhs->clone());

    if (!engine.add_input_fullmatrix("x", lhsInput))
      THROW_ALGORITHM_INPUT_ERROR("Error setting up parser");
    if (!engine.add_input_fullmatrix("y", rhsInput))
      THROW_ALGORITHM_INPUT_ERROR("Error setting up parser");

    auto function_string = params.func;

    function_string = "RESULT=" + function_string;
    engine.add_expressions(function_string);

    //bad API: how does it know what type/size the output matrix should be? Here are my guesses:
    MatrixHandle omatrix;
    if (matrixIs::sparse(lhs))
      omatrix.reset(lhs->clone());
    else if (matrixIs::sparse(rhs))
      omatrix.reset(rhs->clone());
    else if (matrixIs::dense(lhs) && matrixIs::dense(rhs))
      omatrix.reset(lhs->clone());
    else
      omatrix = convertMatrix::toSparse(lhs);

    if (!(engine.add_output_fullmatrix("RESULT", omatrix)))
      THROW_ALGORITHM_INPUT_ERROR("Error setting up parser");
    if (!(engine.run()))
      THROW_ALGORITHM_INPUT_ERROR("Error running math engine");
    result = omatrix;
  }
  break;
  default:
    THROW_ALGORITHM_INPUT_ERROR("ERROR: unknown binary operation");
    break;
  }

  return result;
}

AlgorithmOutput EvaluateLinearAlgebraBinaryAlgorithm::run(const AlgorithmInput& input) const
{
  auto LHS = input.get<Matrix>(Variables::LHS);
  auto RHS = input.get<Matrix>(Variables::RHS);
  auto func = get(Variables::FunctionString).toString();

  auto result = run(boost::make_tuple(LHS, RHS), { Operator(get(Variables::Operator).toInt()), func });

  AlgorithmOutput output;
  output[Variables::Result] = result;
  return output;
}
