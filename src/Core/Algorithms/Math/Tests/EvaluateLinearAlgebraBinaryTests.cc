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


#include <gtest/gtest.h>

#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Tests/MatrixTestCases.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, NullInputThrowsException)
{
  EvaluateLinearAlgebraBinaryAlgorithm algo;
  EvaluateLinearAlgebraBinaryAlgorithm::Parameters add{ EvaluateLinearAlgebraBinaryAlgorithm::ADD };
  EXPECT_THROW(algo.run(EvaluateLinearAlgebraBinaryAlgorithm::Inputs(DenseMatrixHandle(), DenseMatrixHandle(matrix1().clone())), add), AlgorithmInputException);
  EXPECT_THROW(algo.run(EvaluateLinearAlgebraBinaryAlgorithm::Inputs(DenseMatrixHandle(matrix1().clone()), DenseMatrixHandle()), add), AlgorithmInputException);
}

MatrixHandle EvalBinaryOperator(MatrixHandle lhs, MatrixHandle rhs, EvaluateLinearAlgebraBinaryAlgorithm::Parameters op)
{
  EvaluateLinearAlgebraBinaryAlgorithm algo;
  return algo.run(EvaluateLinearAlgebraBinaryAlgorithm::Inputs(lhs, rhs), op);
}

MatrixHandle getOperand(int code)
{
  switch (code)
  {
  case MatrixTypeCode::COLUMN:
    return matrix1column();
  case MatrixTypeCode::DENSE:
    return MatrixHandle(matrix1().clone());
  case MatrixTypeCode::SPARSE_ROW:
    return matrix1sparse();
  default:
    return nullptr;
  }
}

template <int LhsCode, int RhsCode>
MatrixHandle EvalOperator(EvaluateLinearAlgebraBinaryAlgorithm::Parameters op)
{
  return EvalBinaryOperator(getOperand(LhsCode), getOperand(RhsCode), op);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanAddDenseDense)
{
  auto result = castMatrix::toDense(EvalOperator<DENSE, DENSE>({ EvaluateLinearAlgebraBinaryAlgorithm::ADD }));
  EXPECT_EQ(2 * matrix1(), *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanSubtractDenseDense)
{
  auto result = castMatrix::toDense(EvalOperator<DENSE, DENSE>({ EvaluateLinearAlgebraBinaryAlgorithm::SUBTRACT }));
  EXPECT_EQ(Zero, *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanMultiplyDenseDense)
{
  auto result = castMatrix::toDense(EvalOperator<DENSE, DENSE>({ EvaluateLinearAlgebraBinaryAlgorithm::MULTIPLY }));
  EXPECT_EQ(matrix1() * matrix1(), *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanUseFunctionDenseDense)
{
  std::string functionArg = "x+y";
  auto result = castMatrix::toDense(EvalOperator<DENSE, DENSE>({ EvaluateLinearAlgebraBinaryAlgorithm::FUNCTION, functionArg }));
  EXPECT_EQ(matrix1() + matrix1(), *result);
}

////////////////////////////////////////////////////////////////////////////////////////

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanAddSparseSparse)
{
  auto result = castMatrix::toSparse(EvalOperator<SPARSE_ROW, SPARSE_ROW>({ EvaluateLinearAlgebraBinaryAlgorithm::ADD }));
  EXPECT_SPARSE_EQ(2 * *matrix1sparse(), *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanSubtractSparseSparse)
{
  auto result = castMatrix::toSparse(EvalOperator<SPARSE_ROW, SPARSE_ROW>({ EvaluateLinearAlgebraBinaryAlgorithm::SUBTRACT }));
  EXPECT_SPARSE_EQ(*convertMatrix::fromDenseToSparse(Zero), *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanMultiplySparseSparse)
{
  auto result = castMatrix::toSparse(EvalOperator<SPARSE_ROW, SPARSE_ROW>({ EvaluateLinearAlgebraBinaryAlgorithm::MULTIPLY }));
  EXPECT_SPARSE_EQ(*matrix1sparse() * *matrix1sparse(), *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanUseFunctionSparseSparse)
{
  std::string functionArg = "x+y";
  auto result = castMatrix::toSparse(EvalOperator<SPARSE_ROW, SPARSE_ROW>({ EvaluateLinearAlgebraBinaryAlgorithm::FUNCTION, functionArg }));
  EXPECT_SPARSE_EQ(*matrix1sparse() + *matrix1sparse(), *result);
}

////////////////////////////////////////////////////////////////////////////////////////

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanAddDenseSparse)
{
  auto result = castMatrix::toSparse(EvalOperator<DENSE, SPARSE_ROW>({ EvaluateLinearAlgebraBinaryAlgorithm::ADD }));
  EXPECT_SPARSE_EQ(2 * *matrix1sparse(), *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanSubtractDenseSparse)
{
  auto result = castMatrix::toSparse(EvalOperator<DENSE, SPARSE_ROW>({ EvaluateLinearAlgebraBinaryAlgorithm::SUBTRACT }));
  EXPECT_SPARSE_EQ(*convertMatrix::fromDenseToSparse(Zero), *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanMultiplyDenseSparse)
{
  auto result = castMatrix::toSparse(EvalOperator<DENSE, SPARSE_ROW>({ EvaluateLinearAlgebraBinaryAlgorithm::MULTIPLY }));
  EXPECT_SPARSE_EQ(*matrix1sparse() * *matrix1sparse(), *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanUseFunctionDenseSparse)
{
  std::string functionArg = "x+y";
  auto result = castMatrix::toSparse(EvalOperator<DENSE, SPARSE_ROW>({ EvaluateLinearAlgebraBinaryAlgorithm::FUNCTION, functionArg }));
  EXPECT_SPARSE_EQ(*matrix1sparse() + *matrix1sparse(), *result);
}

////////////////////////////////////////////////////////////////////////////////////////

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanAddSparseDense)
{
  auto result = castMatrix::toSparse(EvalOperator<SPARSE_ROW, DENSE>({ EvaluateLinearAlgebraBinaryAlgorithm::ADD }));
  EXPECT_SPARSE_EQ(2 * *matrix1sparse(), *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanSubtractSparseDense)
{
  auto result = castMatrix::toSparse(EvalOperator<SPARSE_ROW, DENSE>({ EvaluateLinearAlgebraBinaryAlgorithm::SUBTRACT }));
  EXPECT_SPARSE_EQ(*convertMatrix::fromDenseToSparse(Zero), *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanMultiplySparseDense)
{
  auto result = castMatrix::toSparse(EvalOperator<SPARSE_ROW, DENSE>({ EvaluateLinearAlgebraBinaryAlgorithm::MULTIPLY }));
  EXPECT_SPARSE_EQ(*matrix1sparse() * *matrix1sparse(), *result);
}

TEST(EvaluateLinearAlgebraBinaryAlgorithmTests, CanUseFunctionSparseDense)
{
  std::string functionArg = "x+y";
  auto result = castMatrix::toSparse(EvalOperator<SPARSE_ROW, DENSE>({ EvaluateLinearAlgebraBinaryAlgorithm::FUNCTION, functionArg }));
  EXPECT_SPARSE_EQ(*matrix1sparse() + *matrix1sparse(), *result);
}
