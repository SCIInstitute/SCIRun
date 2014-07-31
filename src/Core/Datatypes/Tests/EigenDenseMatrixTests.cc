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

#include <Eigen/Dense>

typedef Eigen::MatrixXd DenseMatrix;

namespace
{
  DenseMatrix matrix1()
  {
    DenseMatrix m(3, 3);
    for (int i = 0; i < m.rows(); ++ i)
      for (int j = 0; j < m.cols(); ++ j)
        m(i, j) = 3.0 * i + j;
    return m;
  }
  DenseMatrix matrixNonSquare()
  {
    DenseMatrix m (3, 4);
    for (int i = 0; i < m.rows(); ++ i)
      for (int j = 0; j < m.cols(); ++ j)
        m(i, j) = 3.0 * i + j;
    return m;
  }
  const DenseMatrix Zero(DenseMatrix::Zero(3,3));
}

#define PRINT_MATRIX(x) //std::cout << #x << " = \n" << (x) << std::endl

TEST(EigenDenseMatrixTest, CanCreateBasicMatrix)
{
  DenseMatrix m(matrix1());
  PRINT_MATRIX(m);
}

TEST(EigenDenseMatrixTest, CanPrintInLegacyFormat)
{
  DenseMatrix m(matrix1());
  std::ostringstream o;
  o << 0.5*m;
  std::string legacy = o.str();
  EXPECT_EQ("  0 0.5   1\n1.5   2 2.5\n  3 3.5   4", legacy);
}

TEST(EigenDenseMatrixTest, CanDetermineSize)
{
  DenseMatrix m(matrixNonSquare());
  EXPECT_EQ(3, m.rows());
  EXPECT_EQ(4, m.cols());
}

TEST(EigenDenseMatrixTest, CanCopyConstruct)
{
  DenseMatrix m(matrixNonSquare());
  DenseMatrix m2(m);
  EXPECT_EQ(m, m2);
  m(1,2) += 1;
  EXPECT_NE(m, m2);
}

TEST(EigenDenseMatrixTest, CanAssign)
{
  DenseMatrix m(matrixNonSquare());
  
  DenseMatrix m2(3,4);
  EXPECT_NE(m, m2);
  m2 = m;
  EXPECT_EQ(m, m2);
  m(1,2) += 1;
  EXPECT_NE(m, m2);
}

TEST(EigenDenseMatrixUnaryOperationTests, CanNegate)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(-m);

  DenseMatrix n = - -m;
  EXPECT_EQ(m, n);
  EXPECT_EQ(m + (-m), Zero);
}

TEST(EigenDenseMatrixUnaryOperationTests, CanScalarMultiply)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(2*m);
  PRINT_MATRIX(m*2);
  EXPECT_EQ(2*m, m*2);
}

TEST(EigenDenseMatrixUnaryOperationTests, CanTranspose)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(m.transpose());

  EXPECT_EQ(m, m.transpose().transpose());
}

TEST(EigenDenseMatrixBinaryOperationTests, CanMultiply)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(m * m);
  EXPECT_EQ(Zero * m, Zero);
}

TEST(EigenDenseMatrixBinaryOperationTests, CanAdd)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(m + m);
  EXPECT_EQ(m + m, 2*m);
}

TEST(EigenDenseMatrixBinaryOperationTests, CanSubtract)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(m - m);
  EXPECT_EQ(m - m, Zero);
}
