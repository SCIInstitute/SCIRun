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

#include <Core/Datatypes/Tests/MatrixTestCases.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, NullInputThrowsException)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  EXPECT_THROW(algo.run(nullptr, { EvaluateLinearAlgebraUnaryAlgorithm::NEGATE }), AlgorithmInputException);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanNegateDense)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseMatrixHandle m(matrix1().clone());
  DenseMatrixHandle result = castMatrix::toDense(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::NEGATE }));
  EXPECT_EQ(-*m, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanTransposeDense)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseMatrixHandle m(matrix1().clone());
  DenseMatrixHandle result = castMatrix::toDense(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE }));
  DenseMatrix expected(m->transpose());
  EXPECT_EQ(expected, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanScalarMultiplyDense)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseMatrixHandle m(matrix1().clone());
  DenseMatrixHandle result = castMatrix::toDense(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY, 2.5 }));
  EXPECT_EQ(2.5* *m, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanUseFunctionDense)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseMatrixHandle m(matrix1().clone());
	std::string functionArg = "x+5";
  DenseMatrixHandle result = castMatrix::toDense(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::FUNCTION, 0.0, functionArg }));
  EXPECT_EQ( (m->array()+5).matrix(), *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanNegateSparse)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  SparseRowMatrixHandle m(matrix1sparse()->clone());
  SparseRowMatrixHandle result = castMatrix::toSparse(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::NEGATE }));
  EXPECT_SPARSE_EQ(-*m, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanTransposeSparse)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  SparseRowMatrixHandle m(matrix1sparse()->clone());
  SparseRowMatrixHandle result = castMatrix::toSparse(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE }));
  SparseRowMatrix expected(m->transpose());
  EXPECT_SPARSE_EQ(expected, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanScalarMultiplySparse)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  SparseRowMatrixHandle m(matrix1sparse()->clone());
  SparseRowMatrixHandle result = castMatrix::toSparse(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY, 2.5 }));
  EXPECT_SPARSE_EQ(2.5* *m, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanUseFunctionSparse)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  SparseRowMatrixHandle m(matrix1sparse()->clone());
  std::string functionArg = "x+5";
  SparseRowMatrixHandle result = castMatrix::toSparse(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::FUNCTION, 0.0, functionArg }));
  SparseRowMatrixHandle expected = convertMatrix::toSparse(boost::make_shared<DenseMatrix>((matrix1().array() + 5).matrix()));
  EXPECT_SPARSE_EQ(*expected, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanNegateColumn)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseColumnMatrixHandle m(matrix1column()->clone());
  DenseColumnMatrixHandle result = castMatrix::toColumn(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::NEGATE }));
  EXPECT_EQ(-*m, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, DISABLED_CanTransposeColumn)
{
  //TODO: does transpose work for column type?
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseColumnMatrixHandle m(matrix1column()->clone());
  DenseMatrixHandle result = convertMatrix::toDense(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE }));
  DenseMatrix expected(m->transpose());
  EXPECT_EQ(expected, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanScalarMultiplyColumn)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseColumnMatrixHandle m(matrix1column()->clone());
  DenseColumnMatrixHandle result = castMatrix::toColumn(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY, 2.5 }));
  EXPECT_EQ(2.5* *m, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanUseFunctionColumn)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseColumnMatrixHandle m(matrix1column()->clone());
  std::string functionArg = "x+5";
  DenseColumnMatrixHandle result = castMatrix::toColumn(algo.run(m, { EvaluateLinearAlgebraUnaryAlgorithm::FUNCTION, 0.0, functionArg }));
  EXPECT_EQ((m->array() + 5).matrix(), *result);
}
