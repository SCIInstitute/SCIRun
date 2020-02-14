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

#include <sci_debug.h>
#include <Core/Datatypes/Tests/MatrixTestCases.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/BlockMatrix.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Geometry;
using namespace boost::assign;
//#define DO_PRINTING

#ifdef DO_PRINTING
#define PRINT_MATRIX(x) std::cout << #x << " = \n" << (x) << std::endl
#else
#define PRINT_MATRIX(x)
#endif

TEST(DenseMatrixTest, CanCreateBasicMatrix)
{
  DenseMatrix m(matrix1());
  PRINT_MATRIX(m);
}

TEST(DenseMatrixTest, CanPrintInLegacyFormat)
{
  DenseMatrix m(matrix1());
  std::string legacy = matrix_to_string(0.5 * m);
  std::cout << legacy << std::endl;
  EXPECT_EQ("  0 0.5   1\n1.5   2 2.5\n  3 3.5   4", legacy);
}

TEST(DenseMatrixTest, CanDetermineSize)
{
  DenseMatrix m(matrixNonSquare());
  EXPECT_EQ(3, m.rows());
  EXPECT_EQ(4, m.cols());
}

TEST(DenseMatrixTest, CanCopyConstruct)
{
  DenseMatrix m(matrixNonSquare());
  DenseMatrix m2(m);
  EXPECT_EQ(m, m2);
  m(1,2) += 1;
  EXPECT_NE(m, m2);
}

TEST(DenseMatrixTest, CanAssign)
{
  DenseMatrix m(matrixNonSquare());

  DenseMatrix m2;
  EXPECT_NE(m, m2);
  m2 = m;
  EXPECT_EQ(m, m2);
  m(1,2) += 1;
  EXPECT_NE(m, m2);
}

TEST(DenseMatrixUnaryOperationTests, CanNegate)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(-m);

  DenseMatrix n = - -m;
  EXPECT_EQ(m, n);
  EXPECT_EQ(m + (-m), Zero);
}

TEST(DenseMatrixUnaryOperationTests, CanScalarMultiply)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(2*m);
  PRINT_MATRIX(m*2);
  EXPECT_EQ(2*m, m*2);
}

TEST(DenseMatrixUnaryOperationTests, CanTranspose)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(m.transpose());

  EXPECT_EQ(m, m.transpose().transpose());
}

TEST(DenseMatrixBinaryOperationTests, CanMultiply)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(m * m);
  EXPECT_EQ(Zero * m, Zero);
}

TEST(DenseMatrixBinaryOperationTests, CanAdd)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(m + m);
  EXPECT_EQ(m + m, 2*m);
}

TEST(DenseMatrixBinaryOperationTests, CanSubtract)
{
  DenseMatrix m(matrix1());

  PRINT_MATRIX(m);
  PRINT_MATRIX(m - m);
  EXPECT_EQ(m - m, Zero);
}

#if !DEBUG
/// @todo: compare to v4.
TEST(DenseMatrixBinaryOperationTests, WhatHappensWhenYouAddDifferentSizes)
{
  DenseMatrix sum = matrix1() + matrixNonSquare();
  std::cout << sum.rows() << std::endl;
  std::cout << sum.cols() << std::endl;
  PRINT_MATRIX(sum);
}
#endif

TEST(DenseMatrixTests, CanConstructFromTransform)
{
  Transform t;
  t.set_mat_val(2,1,2);
  t.set_mat_val(0,3,-1);
  DenseMatrix m(t);
  PRINT_MATRIX(m);
  EXPECT_EQ(4, m.rows());
  EXPECT_EQ(4, m.cols());
  DenseMatrix expected(4,4);
  expected << 1,0,0,-1,  0,1,0,0,  0,2,1,0,  0,0,0,1;
  EXPECT_EQ(expected, m);
}

TEST(DenseMatrixTests, OldMacroStillWorks)
{
  DenseMatrix actual = MAKE_DENSE_MATRIX(
    (1,0,0)
    (0,0,0)
    (0,2,0));
  DenseMatrix expected(3,3);
  expected << 1,0,0,  0,0,0,  0,2,0;
  EXPECT_EQ(expected, actual);
}

TEST(DenseMatrixTests, HasNanTests)
{
  DenseMatrix actual = MAKE_DENSE_MATRIX(
    (1,0,0)
    (0,0,0)
    (0,2,0));

  EXPECT_FALSE(actual.hasNaN());
  EXPECT_TRUE(actual.allFinite());
}

TEST(DenseMatrixTests, ConcatenateRowsFromIndices)
{
  DenseMatrix actual = MAKE_DENSE_MATRIX(
    (1,0,0)
    (0,2,0)
    (0,0,3)
    (4,0,0)
    (0,5,0)
    (0,0,6));

  std::vector<int> rows;
  rows += 1,3,5;

  // select inputs are rows{1,3,5}
  // (1, 0, 0)
  // (0, 2, 0)

  DenseMatrix selectedRows(rows.size(),actual.cols());

  for (int i = 0; i < rows.size(); ++i)
    selectedRows.row(i) = actual.row(rows[i]);

  DenseMatrix expected(3,3);
  expected << 0,2,0,  4,0,0,  0,0,6;
  EXPECT_EQ(expected, selectedRows);
}

TEST(DenseMatrixTests, CopyBlock)
{
  DenseMatrix m = MAKE_DENSE_MATRIX(
    (1,0,0,0)
    (0,2,0,0)
    (0,0,3,0)
    (0,0,0,4));

  DenseMatrixHandle block(new DenseMatrix(m.block(1,1,2,2)));
  DenseMatrix expected(2,2);
  expected << 2,0,  0,3;
  EXPECT_EQ(expected, *block);
}

TEST(DenseMatrixTests, CanReadNaNs)
{
  std::string nanMatrix = "1 2\nNaN 3\n";
  DenseMatrix m;
  std::stringstream istr(nanMatrix);
  istr >> m;
  //std::cout << m << std::endl;

  DenseMatrix expected(2,2);
  expected << 1,2, std::numeric_limits<double>::quiet_NaN(), 3;
  EXPECT_TRUE(m.hasNaN());
  //std::cout << "diff: \n" << expected - m << std::endl;
  EXPECT_TRUE(m.hasNaN());
  EXPECT_TRUE((expected - m).hasNaN());
  //comparison won't work with NaNs.
  //EXPECT_EQ(expected, m);
}

TEST(BlockMatrixTest, CanConstructFromBlockSizes)
{
  std::vector<int> rowBlocks, colBlocks;
  rowBlocks += 2,3,4;
  colBlocks += 5,2;

  DenseBlockMatrix blocks(rowBlocks, colBlocks);

  EXPECT_EQ(9, blocks.matrix().nrows());
  EXPECT_EQ(7, blocks.matrix().ncols());
}

TEST(BlockMatrixTest, CanAccessBlockByIndex)
{
  std::vector<int> rowBlocks, colBlocks;
  rowBlocks += 2,3,4;
  colBlocks += 5,2;

  DenseBlockMatrix blocks(rowBlocks, colBlocks);

  for (int i = 0; i < rowBlocks.size(); ++i)
  {
    for (int j = 0; j < colBlocks.size(); ++j)
    {
      auto blockIJ = blocks.blockRef(i,j);
      EXPECT_EQ(rowBlocks[i], blockIJ.rows());
      EXPECT_EQ(colBlocks[j], blockIJ.cols());
      //std::cout << "block size " << i << "," << j << ": " << blockIJ.rows() << " x " << blockIJ.cols() << std::endl;
    }
  }
}

TEST(ComplexMatrixTests, CanPrintComplexMatrix)
{
  ComplexDenseMatrix cm(2, 2, {0,1});
  std::cout << cm << std::endl;
}

namespace
{
  std::complex<double> c(double r, double i) { return{ r, i }; }
}

TEST(ComplexMatrixTests, CanConstructComplexMatrixFromTwoRealMatrices)
{
  DenseMatrix real = MAKE_DENSE_MATRIX(
    (1, 0, 0)
    (0, 0, 0)
    (0, 2, 0));
  DenseMatrix imaginary(3, 3);
  imaginary << -1, 0, 0.5, 0, 0, 5, 0, 2, 1;

  auto actual = makeComplexMatrix(real, imaginary);

  ComplexDenseMatrix expected(3, 3);
  expected << c(1, -1), c(0, 0), c(0,0.5),
    c(0, 0), c(0, 0), c(0, 5),
    c(0, 0), c(2, 2), c(0, 1);
  EXPECT_EQ(expected, actual);

}
