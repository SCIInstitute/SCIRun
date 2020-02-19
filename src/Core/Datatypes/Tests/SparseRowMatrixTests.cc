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
#include <gmock/gmock.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace ::testing;
using namespace SCIRun::TestUtils;

namespace
{
  SparseRowMatrixHandle posdef_matrix_correct()
  {
      return MAKE_SPARSE_MATRIX_HANDLE(
      ( 1.0000000000001,-0.1,-0.2,-0.3,-0.4)
      (-0.1, 2.0,-0.3,-0.4,-0.5)
      (-0.2,-0.3, 2.0,-0.5,-0.6)
      (-0.3,-0.4,-0.5, 2.0,-0.7)
      (-0.4,-0.5,-0.6,-0.7, 3.0)
      );
  }

    SparseRowMatrixHandle posdef_matrix_false0()
  {
      return MAKE_SPARSE_MATRIX_HANDLE(
      ( 1.01,-0.1,-0.2,-0.3,-0.4)
      (-0.1,  2.0,-0.3,-0.4,-0.5)
      (-0.2, -0.3, 1.6,-0.5,-0.6)
      (-0.3, -0.4,-0.5, 2.0,-0.7)
      (-0.4, -0.5,-0.6,-0.7, 3.0)
      );
  }

  SparseRowMatrixHandle posdef_matrix_false1()
  {
    return MAKE_SPARSE_MATRIX_HANDLE(
      ( -2.0,-0.1,-0.1,-0.1,-0.1)
      (-0.1, 2.0,-0.1,-0.1,-0.1)
      (-0.1,-0.1, 2.0,-0.1,-0.1)
      (-0.1,-0.1,-0.1, 2.0,-0.1)
      (-0.1,-0.1,-0.1,-0.1, 2.0)
      );

  }

  SparseRowMatrix posdef_matrix_false2()
  {
    SparseRowMatrix m(5,6);
    return m;
  }

  SparseRowMatrix posdef_matrix_false3()
  {
    SparseRowMatrix m(5,5);
    m.insert(1,3) = std::numeric_limits<double>::quiet_NaN();
    return m;
  }

  SparseRowMatrixHandle posdef_matrix_false4()
  {
     return MAKE_SPARSE_MATRIX_HANDLE(
      ( 2.0,-0.2,-0.2,-0.2,-0.2)
      (-0.1, 2.0,-0.1,-0.1,-0.1)
      (-0.1,-0.1, 2.0,-0.1,-0.1)
      (-0.1,-0.1,-0.1, 2.0,-0.1)
      (-0.1,-0.1,-0.1,-0.1, 2.0)
      );
  }

  SparseRowMatrixHandle posdef_matrix_false5()
  {
   return MAKE_SPARSE_MATRIX_HANDLE(
      ( 2.0,-0.5,-0.5,-0.5,-0.5)
      (-0.5, 2.0,-0.1,-0.1,-0.1)
      (-0.5,-0.1, 2.0,-0.1,-0.1)
      (-0.5,-0.1,-0.1, 2.0,-0.1)
      (-0.5,-0.1,-0.1,-0.1, 2.0)
      );
  }

   SparseRowMatrixHandle posdef_matrix_false6()
  {
     return MAKE_SPARSE_MATRIX_HANDLE(
      ( 2.0,-0.5,-0.5,-0.5,-0.5)
      (-0.51, 2.0,-0.1,-0.1,-0.1)
      (-0.5,-0.1, 2.0,-0.1,-0.1)
      (-0.5,-0.1,-0.1, 2.0,-0.1)
      (-0.5,-0.1,-0.1,-0.1, 2.0)
      );
  }

  SparseRowMatrix sym_matrix_correct()
  {
    SparseRowMatrix m(4,4);
    m.insert(0,0) = 1.0;
    m.insert(3,2) = 0.5;
    m.insert(2,3) = 0.5;
    return m;
  }

  SparseRowMatrix sym_matrix_false0()
  {
    SparseRowMatrix m(4,4);
    m.insert(0,1) = 1.0;
    m.insert(2,3) = 0.5;
    m.insert(3,2) = 0.5;
    return m;
  }

  SparseRowMatrix sym_matrix_false1()
  {
    SparseRowMatrix m(5,5);
    m.insert(0,0) = 1.0;
    m.insert(2,3) = 0.1;
    m.insert(4,2) = 0.5;
    return m;
  }

  SparseRowMatrix sym_matrix_false2()
  {
    SparseRowMatrix m(5,5);
    m.insert(0,0) = 1.0;
    m.insert(2,4) = 0.4999999;
    m.insert(4,2) = 0.5;
    return m;
  }

  SparseRowMatrix sym_matrix_false3()
  {
    SparseRowMatrix m(5,5);
    m.insert(0,0) = 1.0;
    m.insert(2,4) = std::numeric_limits<double>::quiet_NaN();
    m.insert(4,2) = 0.5;
    return m;
  }

  SparseRowMatrix matrix1()
  {
    SparseRowMatrix m(4,5);
    m.insert(0,0) = 1.0;
    m.insert(1,2) = -2;
    m.insert(2,3) = 0.5;
    return m;
  }
  SparseRowMatrix Zero(int r = 4, int c = 5)
  {
    SparseRowMatrix m(r,c);
    m.setZero();
    return m;
  }
  SparseRowMatrix ZeroSquare()
  {
    SparseRowMatrix m(4,4);
    m.setZero();
    return m;
  }
  SparseRowMatrix id3()
  {
    SparseRowMatrix m(3,3);
    m.insert(0,0) = 1;
    m.insert(1,1) = 1;
    m.insert(2,2) = 1;
    return m;
  }
  SparseRowMatrix matrixTdcsGood()
  {
    SparseRowMatrix m(5,5);
    m.insert(1,1) = 1;
    m.insert(2,3) = 0.5;
    return m;
  }
  SparseRowMatrix matrixTdcsBad1()
  {
    SparseRowMatrix m(5,5);
    m.insert(2,2) = 1;
    m.insert(2,1) = 0.5;
    return m;
  }
  SparseRowMatrix matrixTdcsBad2()
  {
    SparseRowMatrix m(5,5);
    m.insert(2,2) = 1;
    m.insert(1,2) = 0.5;
    return m;
  }
  SparseRowMatrix matrixTdcsBad3()
  {
    SparseRowMatrix m(5,5);
    m.insert(2,2) = 1.1;
    return m;
  }
}

#define PRINT_MATRIX(x) //std::cout << #x << " = \n" << (x) << std::endl
#define PRINT_MATRIX_BASE(x) //std::cout << #x << " = \n" << static_cast<const MatrixBase<double>&>((x)) << std::endl


TEST(SparseRowMatrixTest, CanCreateBasicMatrix)
{
  SparseRowMatrix m(matrix1());
  PRINT_MATRIX_BASE(m);
}

TEST(SparseRowMatrixTest, CanPrintInLegacyFormat)
{
  SparseRowMatrix m(matrix1());
  std::string legacy = matrix_to_string(0.5 * m);
  EXPECT_EQ("0.5 0 0 0 0 \n0 0 -1 0 0 \n0 0 0 0.25 0 \n0 0 0 0 0 \n", legacy);
}

TEST(SparseRowMatrixTest, CanDetermineSize)
{
  SparseRowMatrix m(matrix1());
  EXPECT_EQ(4, m.nrows());
  EXPECT_EQ(5, m.ncols());
}

TEST(SparseRowMatrixTest, CanCopyConstruct)
{
  SparseRowMatrix m(matrix1());
  SparseRowMatrix m2(m);
  EXPECT_EQ(m, m2);
  m.coeffRef(1,2) += 1;
  PRINT_MATRIX_BASE(m);
  PRINT_MATRIX_BASE(m2);
  EXPECT_NE(m, m2);
}

TEST(SparseRowMatrixTest, CanAssign)
{
  SparseRowMatrix m(matrix1());

  SparseRowMatrix m2;
  EXPECT_NE(m, m2);
  m2 = m;
  EXPECT_EQ(m, m2);
  m.coeffRef(1,2) += 1;
  EXPECT_NE(m, m2);
}

TEST(SparseRowMatrixUnaryOperationTests, CanNegate)
{
  SparseRowMatrix m(matrix1());

  PRINT_MATRIX_BASE(m);
  PRINT_MATRIX(-m);

  SparseRowMatrix n = - -m;
  EXPECT_EQ(m, n);
  SparseRowMatrix diff = m + (-m);
  EXPECT_EQ(diff, Zero());
}

TEST(SparseRowMatrixUnaryOperationTests, CanScalarMultiply)
{
  SparseRowMatrix m(matrix1());

  PRINT_MATRIX_BASE(m);
  PRINT_MATRIX(2*m);
  PRINT_MATRIX(m*2);
  SparseRowMatrix x = 2*m;
  SparseRowMatrix y = m*2;
  EXPECT_EQ(x,y);
}

TEST(SparseRowMatrixUnaryOperationTests, CanTranspose)
{
  SparseRowMatrix m(matrix1());

  PRINT_MATRIX_BASE(m);
  PRINT_MATRIX(m.transpose());

  SparseRowMatrix mtt = m.transpose().transpose();
  EXPECT_EQ(m,mtt);
}

TEST(SparseRowMatrixBinaryOperationTests, CanMultiply)
{
  SparseRowMatrix m(matrix1());

  PRINT_MATRIX_BASE(m);
  PRINT_MATRIX(m * m.transpose());
  SparseRowMatrix prod = m.transpose() * Zero();
  EXPECT_EQ(prod, Zero(5,5));
}

TEST(SparseRowMatrixBinaryOperationTests, CanAdd)
{
  SparseRowMatrix m(matrix1());

  PRINT_MATRIX_BASE(m);
  PRINT_MATRIX(m + m);
  SparseRowMatrix m2a = m + m;
  SparseRowMatrix m2b = 2*m;
  EXPECT_EQ(m2a, m2b);
}

TEST(SparseRowMatrixBinaryOperationTests, CanSubtract)
{
  SparseRowMatrix m(matrix1());

  PRINT_MATRIX_BASE(m);
  PRINT_MATRIX(m - m);
  SparseRowMatrix diff = m - m;
  EXPECT_EQ(diff, Zero());
}

/// @todo: compare to v4.
TEST(SparseRowMatrixBinaryOperationTests, WhatHappensWhenYouAddDifferentSizes)
{
  SparseRowMatrix sum = matrix1() + matrix1();
  std::cout << sum.rows() << std::endl;
  std::cout << sum.cols() << std::endl;
  PRINT_MATRIX(sum);
}

namespace std
{
  template <typename A, typename B>
  std::ostream& operator<<(std::ostream& o, const std::pair<A,B>& p)
  {
    return o << p.first << "," << p.second;
  }
}

namespace
{
  template <typename T>
  void printArray(const T* ts, size_t size)
  {
    std::copy(ts, ts + size, std::ostream_iterator<T>(std::cout, " "));
    std::cout << std::endl;
  }

  template <typename T>
  void printVector(const std::vector<T>& v)
  {
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
    std::cout << std::endl;
  }
}

TEST(SparseRowMatrixTest, CheckingInternalArrays)
{
  auto mat = matrix1();

  mat.makeCompressed();
  EXPECT_EQ(0, mat.innerNonZeroPtr());
  EXPECT_EQ(3, mat.nonZeros());
  EXPECT_NE(mat.rows(), mat.cols());
  EXPECT_EQ(mat.outerSize(), mat.rows());
  EXPECT_EQ(mat.innerSize(), mat.cols());

  std::vector<double> values(mat.valuePtr(), mat.valuePtr() + mat.nonZeros());
  EXPECT_THAT(values, ElementsAre(1, -2, 0.5));
  std::vector<long long> columns(mat.innerIndexPtr(), mat.innerIndexPtr() + mat.nonZeros());
  EXPECT_THAT(columns, ElementsAre(0,2,3));
  std::vector<long long> rows(mat.outerIndexPtr(), mat.outerIndexPtr() + mat.outerSize());
  EXPECT_THAT(rows, ElementsAre(0,1,2,3));
}

TEST(SparseRowMatrixTest, CheckingInternalArrays2)
{
  SparseRowMatrix mat(id3());

  mat.makeCompressed();
  EXPECT_EQ(0, mat.innerNonZeroPtr());
  EXPECT_EQ(3, mat.nonZeros());
  EXPECT_EQ(mat.rows(), mat.cols());
  EXPECT_EQ(mat.outerSize(), mat.rows());
  EXPECT_EQ(mat.innerSize(), mat.cols());

  std::vector<double> values(mat.valuePtr(), mat.valuePtr() + mat.nonZeros());
  EXPECT_THAT(values, ElementsAre(1, 1, 1));
  std::vector<long long> columns(mat.innerIndexPtr(), mat.innerIndexPtr() + mat.nonZeros());
  EXPECT_THAT(columns, ElementsAre(0,1,2));
  std::vector<long long> rows(mat.outerIndexPtr(), mat.outerIndexPtr() + mat.outerSize());
  EXPECT_THAT(rows, ElementsAre(0,1,2));
}

bool hasNElements(const SparseRowMatrix::InnerIterator& it, int n)
{
  SparseRowMatrix::InnerIterator copy(it);
  for (int i = 0; i < n && copy; ++i)
    ++copy;
  return !copy;
}

bool passesTdcsTest(const SparseRowMatrix& matrix)
{
  for (int k=0; k < matrix.outerSize(); ++k)
  {
    for (SparseRowMatrix::InnerIterator it(matrix,k); it; ++it)
    {
      if (hasNElements(it, 1))
      {
        if (it.value() == 1)
        {
          return true;
        }
      }
    }
  }
  return false;
}

TEST(SparseRowMatrixTest, DISABLED_SearchingForSingleNonzeroInRowAndColumnOnTheDiagonal)
{
  EXPECT_TRUE(passesTdcsTest(id3()));
  EXPECT_TRUE(passesTdcsTest(matrixTdcsGood()));

  EXPECT_FALSE(passesTdcsTest(Zero()));

  EXPECT_FALSE(passesTdcsTest(matrixTdcsBad1()));
  EXPECT_FALSE(passesTdcsTest(matrixTdcsBad2()));
  EXPECT_FALSE(passesTdcsTest(matrixTdcsBad3()));

}


TEST(SparseRowMatrixTest, IsSymmetricTests)
{
 auto m = matrix1();

 ASSERT_FALSE(isSymmetricMatrix(m));
 ASSERT_TRUE(isSymmetricMatrix(id3()));
 ASSERT_FALSE(isSymmetricMatrix(Zero()));
 ASSERT_TRUE(isSymmetricMatrix(ZeroSquare()));

 auto m1 = sym_matrix_correct();
 ASSERT_TRUE(isSymmetricMatrix(m1));
 ASSERT_TRUE(m1.isSymmetric());

 auto m2 = sym_matrix_false0();
 ASSERT_FALSE(isSymmetricMatrix(m2));
 ASSERT_FALSE(m2.isSymmetric());

 auto m3 = sym_matrix_false1();
 ASSERT_FALSE(isSymmetricMatrix(m3));
 ASSERT_FALSE(m3.isSymmetric());

 auto m4 = sym_matrix_false2();
 ASSERT_FALSE(isSymmetricMatrix(m4));
 ASSERT_FALSE(m4.isSymmetric());

 auto m5 = sym_matrix_false3();
 ASSERT_FALSE(isSymmetricMatrix(m5));
 ASSERT_FALSE(m5.isSymmetric());

 ASSERT_FALSE(m.isSymmetric());
 ASSERT_TRUE(id3().isSymmetric());
 ASSERT_FALSE(Zero().isSymmetric());
 ASSERT_TRUE(ZeroSquare().isSymmetric());

}

TEST(SparseRowMatrixTest, IsPositiveDefiniteTests)
{

 auto n0 = *posdef_matrix_false0();
 ASSERT_FALSE(isPositiveDefiniteMatrix(n0));

 auto n1 = *posdef_matrix_correct();
 ASSERT_TRUE(isPositiveDefiniteMatrix(n1));

 auto n2 = *posdef_matrix_false1();
 ASSERT_FALSE(isPositiveDefiniteMatrix(n2));

 auto n3 = posdef_matrix_false2();
 ASSERT_FALSE(isPositiveDefiniteMatrix(n3));

 auto n4 = posdef_matrix_false3();
 ASSERT_FALSE(isPositiveDefiniteMatrix(n4));

 auto n5 = *posdef_matrix_false4();
 ASSERT_FALSE(isPositiveDefiniteMatrix(n5));

 auto n6 = *posdef_matrix_false5();
 ASSERT_FALSE(isPositiveDefiniteMatrix(n6));

 auto n7 = *posdef_matrix_false6();
 ASSERT_FALSE(isPositiveDefiniteMatrix(n7));

}

TEST(SparseRowMatrixTest, CanBuildTripletsFromInternalArrays)
{
  int nnz = 35;
  int nrows = 7, ncols = 7;
  int rows[] = {0, 7, 11, 17, 21, 26, 30, 35};
  int cols[] = {0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 5, 0, 1, 2, 4, 5, 6, 0, 3, 4, 6, 0, 2, 3, 4, 6, 0, 1, 2, 5, 0, 2, 3, 4, 6};

  std::vector<std::pair<int,int>> nnzLocations;

  int i = 0;
  int j = 0;
  while (i < nrows)
  {
    while (j < rows[i + 1])
    {
      nnzLocations.push_back(std::make_pair(i, cols[j]));
      j++;
    }
    i++;
  }

  //printVector(nnzLocations);

  SparseRowMatrix m(nrows, ncols);
  EXPECT_EQ(nrows, m.nrows());
  EXPECT_EQ(ncols, m.ncols());
  EXPECT_EQ(0, m.nonZeros());
  std::vector<SparseRowMatrix::Triplet> triplets;
  std::transform(nnzLocations.begin(), nnzLocations.end(), std::back_inserter(triplets), [](const std::pair<int,int>& p) { return SparseRowMatrix::Triplet(p.first, p.second, 0); });
  m.setFromTriplets(triplets.begin(), triplets.end());
  EXPECT_EQ(nnz, m.nonZeros());
}

TEST(SparseRowMatrixTest, TestLegacyConstructor)
{
  int nnz = 35;
  int nrows = 7, ncols = 7;
  index_type rows[] = {0, 7, 11, 17, 21, 26, 30, 35};
  index_type cols[] = {0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 5, 0, 1, 2, 4, 5, 6, 0, 3, 4, 6, 0, 2, 3, 4, 6, 0, 1, 2, 5, 0, 2, 3, 4, 6};

  SparseRowMatrix m(nrows, ncols, rows, cols, nnz);
  EXPECT_EQ(nrows, m.nrows());
  EXPECT_EQ(ncols, m.ncols());
  EXPECT_EQ(nnz, m.nonZeros());
}

TEST(SparseRowMatrixTest, TestLegacyConstructorWithValuesCrappySetOfAssertions)
{
  int nnz = 35;
  int nrows = 7, ncols = 7;
  index_type rows[] = {0, 7, 11, 17, 21, 26, 30, 35};
  index_type cols[] = {0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 5, 0, 1, 2, 4, 5, 6, 0, 3, 4, 6, 0, 2, 3, 4, 6, 0, 1, 2, 5, 0, 2, 3, 4, 6};
  double vals[] = {1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1};

  SparseRowMatrix m(nrows, ncols, rows, cols, vals, nnz);
  EXPECT_EQ(nrows, m.nrows());
  EXPECT_EQ(ncols, m.ncols());
  EXPECT_EQ(nnz, m.nonZeros());
  EXPECT_EQ(1, *std::max_element(m.valuePtr(), m.valuePtr() + m.nonZeros()));
  EXPECT_EQ(-1, *std::min_element(m.valuePtr(), m.valuePtr() + m.nonZeros()));
}

TEST(SparseRowMatrixTest, TestLegacyConstructorWithValuesFixesBugWithBuildMappingMatrix)
{
  int nnz = 162;
  int nrows = 54, ncols = 6;
  index_type rows[] = { 0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 72, 75, 78, 81, 84, 87, 90, 93, 96, 99, 102, 105, 108, 111, 114, 117, 120, 123, 126, 129, 132, 135, 138, 141, 144, 147, 150, 153, 156, 159, 162 };
  index_type cols[] = { 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 1, 0, 3, 1, 0, 3, 1, 0, 3, 1, 0, 3, 0, 1, 2, 0, 1, 2, 0, 1, 2, 1, 0, 3, 1, 0, 3, 0, 2, 4, 0, 2, 4, 0, 2, 4, 3, 0, 4, 3, 0, 4, 0, 2, 4, 0, 2, 4, 3, 0, 4, 0, 1, 2, 0, 1, 2, 0, 1, 2, 1, 0, 3, 1, 0, 3, 0, 1, 2, 1, 0, 3, 0, 2, 4, 3, 0, 4, 0, 2, 4, 0, 2, 4, 3, 0, 4, 1, 5, 2, 1, 5, 2, 1, 5, 2, 1, 3, 5, 1, 3, 5, 2, 5, 4, 1, 3, 5, 2, 5, 4, 3, 4, 5, 2, 5, 4, 2, 5, 4, 3, 4, 5, 1, 5, 2, 1, 5, 2, 1, 3, 5, 1, 5, 2, 1, 5, 2, 1, 3, 5, 2, 5, 4, 2, 5, 4, 3, 4, 5 };
  double vals[] = { 0.444444444444445, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.444444444444444, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.444444444444444, 0.666666666666667, 0.333333333333333, 0, 0.666666666666667, 0.333333333333333, 0, 0.277777777777778, 0.444444444444444, 0.277777777777778, 0.444444444444445, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.444444444444444, 0.666666666666667, 0, 0.333333333333333, 0.333333333333333, 0, 0.666666666666667, 1, 3.70074341541719e-017, 3.70074341541719e-017, -1.11022302462516e-016, 0.666666666666667, 0.333333333333333, 0, 0.333333333333333, 0.666666666666667, 0.444444444444445, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.444444444444444, 0.277777777777778, 0.666666666666667, 0, 0.333333333333333, 0.277777777777778, 0.444444444444444, 0.277777777777778, 0.444444444444445, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.444444444444444, 0.333333333333333, 5.55111512312578e-017, 0.666666666666667, 0.277777777777778, 0.277777777777778, 0.444444444444444, -5.55111512312578e-017, 0.666666666666667, 0.333333333333333, 0, 0.333333333333333, 0.666666666666667, 0, 1, 0, 0.666666666666667, 0, 0.333333333333333, 0.333333333333333, 5.55111512312578e-017, 0.666666666666667, 0, 0, 1, 0, 5.55111512312578e-017, 1, -1.11022302462516e-016, 0.666666666666667, 0.333333333333333, 0.666666666666667, 1.11022302462516e-016, 0.333333333333333, 0, 0.333333333333333, 0.666666666666667, 0, 5.55111512312578e-017, 1, 0.333333333333333, 5.55111512312578e-017, 0.666666666666667, 0.444444444444445, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.444444444444444, 0.666666666666667, 0.333333333333333, 0, 0.444444444444445, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.444444444444444, 0.277777777777778, 0.666666666666667, 0.333333333333333, 0, 0, 0.666666666666667, 0.333333333333333, 0.444444444444445, 0.277777777777778, 0.277777777777778, 0.444444444444445, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.277777777777778, 0.444444444444444, -2.22044604925031e-016, 0.333333333333333, 0.666666666666667, 0.277777777777778, 0.444444444444444, 0.277777777777778, 0.277777777777778, 0.444444444444444, 0.277777777777778, 0.333333333333333, 0.666666666666667, 0, 0.277777777777778, 0.277777777777778, 0.444444444444444, 0, 0.666666666666667, 0.333333333333333, 0, 1, 0, -2.22044604925031e-016, 0.333333333333333, 0.666666666666667, 0.277777777777778, 0.444444444444444, 0.277777777777778, 0, 0.666666666666667, 0.333333333333333, 0.277777777777778, 0.277777777777778, 0.444444444444444};

  auto m = boost::make_shared<SparseRowMatrix>(nrows, ncols, rows, cols, vals, nnz);

  DenseMatrix expected(nrows, ncols);
  expected << 0.444444444444445, 0.277777777777778, 0.277777777777778, 0, 0, 0,
    0.277777777777778, 0.444444444444444, 0.277777777777778, 0, 0, 0,
    0.277777777777778, 0.277777777777778, 0.444444444444444, 0, 0, 0,
    0.666666666666667, 0.333333333333333, 0, 0, 0, 0,
    0.333333333333333, 0.666666666666667, 0, 0, 0, 0,
    0.444444444444444, 0.277777777777778, 0, 0.277777777777778, 0, 0,
    0.277777777777778, 0.444444444444445, 0, 0.277777777777778, 0, 0,
    0.277777777777778, 0.277777777777778, 0, 0.444444444444444, 0, 0,
    0.666666666666667, 0, 0.333333333333333, 0, 0, 0,
    0.333333333333333, 0, 0.666666666666667, 0, 0, 0,
    1, 3.70074341541719e-017, 3.70074341541719e-017, 0, 0, 0,
    0.666666666666667, -1.11022302462516e-016, 0, 0.333333333333333, 0, 0,
    0.333333333333333, 0, 0, 0.666666666666667, 0, 0,
    0.444444444444445, 0, 0.277777777777778, 0, 0.277777777777778, 0,
    0.277777777777778, 0, 0.444444444444444, 0, 0.277777777777778, 0,
    0.666666666666667, 0, 0, 0, 0.333333333333333, 0,
    0.444444444444444, 0, 0, 0.277777777777778, 0.277777777777778, 0,
    0.277777777777778, 0, 0, 0.444444444444445, 0.277777777777778, 0,
    0.277777777777778, 0, 0.277777777777778, 0, 0.444444444444444, 0,
    0.333333333333333, 0, 5.55111512312578e-017, 0, 0.666666666666667, 0,
    0.277777777777778, 0, 0, 0.277777777777778, 0.444444444444444, 0,
    -5.55111512312578e-017, 0.666666666666667, 0.333333333333333, 0, 0, 0,
    0, 0.333333333333333, 0.666666666666667, 0, 0, 0,
    0, 1, 0, 0, 0, 0,
    0, 0.666666666666667, 0, 0.333333333333333, 0, 0,
    5.55111512312578e-017, 0.333333333333333, 0, 0.666666666666667, 0, 0,
    0, 0, 1, 0, 0, 0,
    5.55111512312578e-017, 0, 0, 1, 0, 0,
    -1.11022302462516e-016, 0, 0.666666666666667, 0, 0.333333333333333, 0,
    1.11022302462516e-016, 0, 0, 0.666666666666667, 0.333333333333333, 0,
    0, 0, 0.333333333333333, 0, 0.666666666666667, 0,
    0, 0, 5.55111512312578e-017, 0, 1, 0,
    5.55111512312578e-017, 0, 0, 0.333333333333333, 0.666666666666667, 0,
    0, 0.444444444444445, 0.277777777777778, 0, 0, 0.277777777777778,
    0, 0.277777777777778, 0.444444444444444, 0, 0, 0.277777777777778,
    0, 0.666666666666667, 0, 0, 0, 0.333333333333333,
    0, 0.444444444444445, 0, 0.277777777777778, 0, 0.277777777777778,
    0, 0.277777777777778, 0, 0.444444444444444, 0, 0.277777777777778,
    0, 0, 0.666666666666667, 0, 0, 0.333333333333333,
    0, 0, 0, 0.666666666666667, 0, 0.333333333333333,
    0, 0, 0.444444444444445, 0, 0.277777777777778, 0.277777777777778,
    0, 0, 0, 0.444444444444445, 0.277777777777778, 0.277777777777778,
    0, 0, 0.277777777777778, 0, 0.444444444444444, 0.277777777777778,
    0, 0, -2.22044604925031e-016, 0, 0.666666666666667, 0.333333333333333,
    0, 0, 0, 0.277777777777778, 0.444444444444444, 0.277777777777778,
    0, 0.277777777777778, 0.277777777777778, 0, 0, 0.444444444444444,
    0, 0.333333333333333, 0, 0, 0, 0.666666666666667,
    0, 0.277777777777778, 0, 0.277777777777778, 0, 0.444444444444444,
    0, 0, 0.333333333333333, 0, 0, 0.666666666666667,
    0, 0, 0, 0, 0, 1,
    0, -2.22044604925031e-016, 0, 0.333333333333333, 0, 0.666666666666667,
    0, 0, 0.277777777777778, 0, 0.277777777777778, 0.444444444444444,
    0, 0, 0, 0, 0.333333333333333, 0.666666666666667,
    0, 0, 0, 0.277777777777778, 0.277777777777778, 0.444444444444444;
  EXPECT_EQ(nrows, m->nrows());
  EXPECT_EQ(ncols, m->ncols());
  EXPECT_EQ(nnz, m->nonZeros());
  EXPECT_MATRIX_EQ_TOLERANCE(expected, *convertMatrix::toDense(m), 1e-15);
}

TEST(SparseRowMatrixTest, CopyBlock)
{
  auto m = MAKE_SPARSE_MATRIX_HANDLE(
    (1,0,0,0)
    (0,2,0,0)
    (0,0,3,0)
    (0,0,0,4));

  SparseRowMatrixHandle block(new SparseRowMatrix(m->block(1,1,2,2)));
  DenseMatrix expected(2,2);
  expected << 2,0,  0,3;
  auto expectedSparse = toSparseHandle(expected);
  EXPECT_EQ(*expectedSparse, *block);
}
