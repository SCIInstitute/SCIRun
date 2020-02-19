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

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Math/CollectMatrices/CollectMatricesAlgorithm.h>

#include <Testing/Utils/MatrixTestUtilities.h>

using namespace boost::assign;
using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::TestUtils;

TEST(CollectDenseMatricesAlgorithmTest, AppendColumns)
{
  CollectDenseMatricesAlgorithm algo;

  auto m1 = MAKE_DENSE_MATRIX_HANDLE((1, 2, 3)(4, 5, 6)(7, 8, 9));
  DenseMatrixHandle m2 = MAKE_DENSE_MATRIX_HANDLE((-1, -2)(-4, -5)(-7, -8));
  MatrixHandle out = algo.concat_cols(m1, m2);
  ASSERT_TRUE(matrixIs::dense(out));
  auto denseOut = castMatrix::toDense(out);

  EXPECT_MATRIX_EQ_TO(*denseOut,
    (1,2,3,-1,-2)
    (4,5,6,-4,-5)
    (7,8,9,-7,-8));
}

TEST(CollectDenseMatricesAlgorithmTest, AppendRows)
{
  CollectDenseMatricesAlgorithm algo;

  DenseMatrixHandle m1 = MAKE_DENSE_MATRIX_HANDLE((1, 2, 3)(4, 5, 6)(7, 8, 9));
  DenseMatrixHandle m2 = MAKE_DENSE_MATRIX_HANDLE((-1, -2, -3)(-4, -5, -6)(-7, -8, -9));
  MatrixHandle out = algo.concat_rows(m1, m2);
  ASSERT_TRUE(matrixIs::dense(out));
  auto denseOut = castMatrix::toDense(out);

  EXPECT_MATRIX_EQ_TO(*denseOut,
    (1,2,3)
    (4,5,6)
    (7,8,9)
    (-1,-2,-3)
    (-4,-5,-6)
    (-7,-8,-9));
}

TEST(CollectDenseMatricesAlgorithmTest, AppendZeroColumns)
{
  CollectDenseMatricesAlgorithm algo;

  DenseMatrixHandle m1 = MAKE_DENSE_MATRIX_HANDLE((1, 2, 3)(4, 5, 6)(7, 8, 9));
  DenseMatrixHandle m2(new DenseMatrix(3,0));
  MatrixHandle out = algo.concat_cols(m1, m2);
  ASSERT_TRUE(matrixIs::dense(out));
  auto denseOut = castMatrix::toDense(out);

  EXPECT_MATRIX_EQ(*denseOut, *m1);

  auto neg_m1 = boost::make_shared<DenseMatrix>(-*m1);
  out = algo.concat_cols(m2, neg_m1);
  denseOut = castMatrix::toDense(out);
  EXPECT_MATRIX_EQ(*denseOut, *neg_m1);
}

TEST(CollectDenseMatricesAlgorithmTest, AppendZeroRows)
{
  CollectDenseMatricesAlgorithm algo;

  DenseMatrixHandle m1 = MAKE_DENSE_MATRIX_HANDLE((1, 2, 3)(4, 5, 6)(7, 8, 9));
  DenseMatrixHandle m2(new DenseMatrix(0,3));
  MatrixHandle out = algo.concat_rows(m1, m2);
  EXPECT_TRUE(matrixIs::dense(out));
  auto denseOut = castMatrix::toDense(out);

  EXPECT_MATRIX_EQ(*denseOut, *m1);

  auto neg_m1 = boost::make_shared<DenseMatrix>(-*m1);
  out = algo.concat_rows(m2, neg_m1);
  denseOut = castMatrix::toDense(out);

  EXPECT_MATRIX_EQ(*denseOut, *neg_m1);
}

//TODO: repeat above for sparse matrices

TEST(CollectSparseRowMatricesAlgorithmTest, AppendColumns)
{
  CollectSparseRowMatricesAlgorithm algo;

  SparseRowMatrixHandle m1 = MAKE_SPARSE_MATRIX_HANDLE((1, 2, 3)(4, 5, 6)(7, 8, 9));
  SparseRowMatrixHandle m2 = MAKE_SPARSE_MATRIX_HANDLE((-1, -2)(-4, -5)(-7, -8));
  MatrixHandle out = algo.concat_cols(m1, m2);

  ASSERT_TRUE(matrixIs::sparse(out));
  auto sparseOut = castMatrix::toSparse(out);
  //convert to dense for comparison
  auto denseOut = makeDense(*sparseOut);

  EXPECT_MATRIX_EQ_TO(*denseOut,
    (1,2,3,-1,-2)
    (4,5,6,-4,-5)
    (7,8,9,-7,-8));
}

TEST(CollectSparseRowMatricesAlgorithmTest, AppendRows)
{
  CollectSparseRowMatricesAlgorithm algo;

  SparseRowMatrixHandle m1 = MAKE_SPARSE_MATRIX_HANDLE((1, 2, 3)(4, 5, 6)(7, 8, 9));
  SparseRowMatrixHandle m2 = MAKE_SPARSE_MATRIX_HANDLE((-1, -2, -3)(-4, -5, -6)(-7, -8, -9));
  MatrixHandle out = algo.concat_rows(m1, m2);

  ASSERT_TRUE(matrixIs::sparse(out));
  auto sparseOut = castMatrix::toSparse(out);
  //convert to dense for comparison
  auto denseOut = makeDense(*sparseOut);

  EXPECT_MATRIX_EQ_TO(*denseOut,
    (1,2,3)
    (4,5,6)
    (7,8,9)
    (-1,-2,-3)
    (-4,-5,-6)
    (-7,-8,-9));
}

TEST(CollectSparseRowMatricesAlgorithmTest, AppendZeroColumns)
{
  CollectSparseRowMatricesAlgorithm algo;

  SparseRowMatrixHandle m1 = MAKE_SPARSE_MATRIX_HANDLE((1, 2, 3)(4, 5, 6)(7, 8, 9));
  SparseRowMatrixHandle m2 = toSparseHandle(DenseMatrix(3, 0));
  MatrixHandle out = algo.concat_cols(m1, m2);
  ASSERT_TRUE(matrixIs::sparse(out));
  auto sparseOut = castMatrix::toSparse(out);
  //convert to dense for comparison
  //auto denseOut = makeDense(*sparseOut);

  EXPECT_MATRIX_EQ(*sparseOut, *m1);

  auto neg_m1 = boost::make_shared<SparseRowMatrix>(-*m1);
  out = algo.concat_cols(m2, neg_m1);
  sparseOut = castMatrix::toSparse(out);

  EXPECT_MATRIX_EQ(*sparseOut, *neg_m1);
}

TEST(CollectSparseRowMatricesAlgorithmTest, AppendZeroRows)
{
  CollectSparseRowMatricesAlgorithm algo;

  SparseRowMatrixHandle m1 = MAKE_SPARSE_MATRIX_HANDLE((1, 2, 3)(4, 5, 6)(7, 8, 9));
  SparseRowMatrixHandle m2 = toSparseHandle(DenseMatrix(0, 3));
  MatrixHandle out = algo.concat_rows(m1, m2);

  ASSERT_TRUE(matrixIs::sparse(out));
  auto sparseOut = castMatrix::toSparse(out);

  EXPECT_MATRIX_EQ(*sparseOut, *m1);

  auto neg_m1 = boost::make_shared<SparseRowMatrix>(-*m1);
  out = algo.concat_rows(m2, neg_m1);
  sparseOut = castMatrix::toSparse(out);

  EXPECT_MATRIX_EQ(*sparseOut, *neg_m1);
}
