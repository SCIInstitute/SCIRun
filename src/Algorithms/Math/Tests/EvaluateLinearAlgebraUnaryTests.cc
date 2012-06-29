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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <Algorithms/Math/EvaluateLinearAlgebraUnary.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>

//using namespace SCIRun;
using namespace SCIRun::Domain::Datatypes;
using namespace SCIRun::Algorithms::Math;
//using namespace SCIRun::Domain::Networks::Mocks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

//TODO DAN

namespace
{
  DenseMatrix matrix1()
  {
    DenseMatrix m (3, 3);
    for (size_t i = 0; i < m.nrows(); ++ i)
      for (size_t j = 0; j < m.ncols(); ++ j)
        m(i, j) = 3.0 * i + j;
    return m;
  }
  DenseMatrix matrixNonSquare()
  {
    DenseMatrix m (3, 4);
    for (size_t i = 0; i < m.nrows(); ++ i)
      for (size_t j = 0; j < m.ncols(); ++ j)
        m(i, j) = 3.0 * i + j;
    return m;
  }
  const DenseMatrix Zero(DenseMatrix::zero_matrix(3,3));
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanNegate)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseMatrixHandle m(matrix1().clone());
  DenseMatrixHandle result = algo.run(m, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);
  EXPECT_EQ(-*m, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanTranspose)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseMatrixHandle m(matrix1().clone());
  DenseMatrixHandle result = algo.run(m, EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE);
  DenseMatrixHandle expected(m->make_transpose());
  EXPECT_EQ(*expected, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, CanScalarMultiply)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseMatrixHandle m(matrix1().clone());
  DenseMatrixHandle result = algo.run(m, EvaluateLinearAlgebraUnaryAlgorithm::Parameters(EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY, 2.5));
  EXPECT_EQ(2.5* *m, *result);
}

TEST(EvaluateLinearAlgebraUnaryAlgorithmTests, NullInputReturnsNull)
{
  EvaluateLinearAlgebraUnaryAlgorithm algo;

  DenseMatrixHandle result = algo.run(DenseMatrixHandle(), EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);
  EXPECT_FALSE(result);
}