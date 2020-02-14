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
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::TestUtils;

TEST(SparseMatrixFromMapTest, BasicExample)
{
  SparseRowMatrixFromMap::Values data;

  data[0][0] = 1;
  data[2][1] = 2;
  data[1][1] = 0;

  SparseRowMatrixHandle sparseFromMap = SparseRowMatrixFromMap::make(3, 3, data);

  EXPECT_EQ(3, sparseFromMap->nrows());
  EXPECT_EQ(3, sparseFromMap->ncols());

  DenseMatrix expected = MAKE_DENSE_MATRIX(
    (1,0,0)
    (0,0,0)
    (0,2,0));

  DenseMatrixHandle actual = makeDense(*sparseFromMap);
  EXPECT_MATRIX_EQ(*actual, expected);
}

TEST(SparseMatrixFromMapTest, BasicExampleLeavingARowAllZero)
{
  SparseRowMatrixFromMap::Values data;

  data[0][0] = 1;
  data[2][1] = 2;

  SparseRowMatrixHandle sparseFromMap = SparseRowMatrixFromMap::make(3, 4, data);

  EXPECT_EQ(3, sparseFromMap->nrows());
  EXPECT_EQ(4, sparseFromMap->ncols());

  DenseMatrix expected = MAKE_DENSE_MATRIX(
    (1,0,0,0)
    (0,0,0,0)
    (0,2,0,0));

  DenseMatrixHandle actual = makeDense(*sparseFromMap);
  EXPECT_MATRIX_EQ(*actual, expected);
}

TEST(SparseMatrixFromMapTest, ZeroMatrixEmptyMap)
{
  SparseRowMatrixFromMap::Values data;
  SparseRowMatrixHandle sparseFromMap = SparseRowMatrixFromMap::make(3, 5, data);

  EXPECT_EQ(3, sparseFromMap->nrows());
  EXPECT_EQ(5, sparseFromMap->ncols());

  DenseMatrix expected(3,5,0);

  DenseMatrixHandle actual = makeDense(*sparseFromMap);
  EXPECT_MATRIX_EQ(*actual, expected);
}

TEST(SparseMatrixFromMapTest, ZeroMatrixWithSomePlacedValues)
{
  SparseRowMatrixFromMap::Values data;

  data[0][0] = 0;
  data[2][1] = 0;

  SparseRowMatrixHandle sparseFromMap = SparseRowMatrixFromMap::make(4, 3, data);

  EXPECT_EQ(4, sparseFromMap->nrows());
  EXPECT_EQ(3, sparseFromMap->ncols());

  DenseMatrix expected(4,3,0);

  DenseMatrixHandle actual = makeDense(*sparseFromMap);
  EXPECT_MATRIX_EQ(*actual, expected);
}

TEST(SparseMatrixFromMapTest, SymmetricMatrix)
{
  SparseRowMatrixFromMap::SymmetricValues data;

  data(0, 0, 1);
  data(0, 2, -1);
  data(2, 1, 2);

  SparseRowMatrixHandle sparseFromMap = SparseRowMatrixFromMap::make(3, 3, data);

  EXPECT_EQ(3, sparseFromMap->nrows());
  EXPECT_EQ(3, sparseFromMap->ncols());

  DenseMatrix expected = MAKE_DENSE_MATRIX(
    (1,0,-1)
    (0,0, 2)
    (-1,2,0));

  DenseMatrixHandle actual = makeDense(*sparseFromMap);
  EXPECT_MATRIX_EQ(*actual, expected);
}

TEST(SparseMatrixFromMapTest, CanExtendSparseMatrixWithAdditionalValues)
{
  SparseRowMatrixHandle sparseFromMap;
  {
    SparseRowMatrixFromMap::Values data;

    data[0][0] = 1;
    data[2][1] = 2;
    data[1][1] = 0;

    sparseFromMap = SparseRowMatrixFromMap::make(3, 3, data);

    EXPECT_EQ(3, sparseFromMap->nrows());
    EXPECT_EQ(3, sparseFromMap->ncols());

    DenseMatrix expected = MAKE_DENSE_MATRIX(
      (1,0,0)
      (0,0,0)
      (0,2,0));

    DenseMatrixHandle actual = makeDense(*sparseFromMap);
    EXPECT_MATRIX_EQ(*actual, expected);
  }

  SparseRowMatrixFromMap::Values additionalData;

  additionalData[4][0] = 3;
  additionalData[1][4] = -2;
  additionalData[4][4] = -1;
  additionalData[1][2] = 2;

  SparseRowMatrixHandle largerSparse = SparseRowMatrixFromMap::appendToSparseMatrix(5, 5, *sparseFromMap, additionalData);

  EXPECT_EQ(5, largerSparse->nrows());
  EXPECT_EQ(5, largerSparse->ncols());

  DenseMatrix expected = MAKE_DENSE_MATRIX(
    (1,0,0,0,0)
    (0,0,2,0,-2)
    (0,2,0,0,0)
    (0,0,0,0,0)
    (3,0,0,0,-1));

  DenseMatrixHandle actual = makeDense(*largerSparse);
  EXPECT_MATRIX_EQ(*actual, expected);
}

TEST(SparseMatrixFromMapTest, CanExtendSparseMatrixWithAdditionalValuesWithOverlapReplacing)
{
  SparseRowMatrixHandle sparseFromMap;
  {
    SparseRowMatrixFromMap::Values data;

    data[0][0] = 1;
    data[2][1] = 2;
    data[1][1] = -1;

    sparseFromMap = SparseRowMatrixFromMap::make(3, 3, data);

    EXPECT_EQ(3, sparseFromMap->nrows());
    EXPECT_EQ(3, sparseFromMap->ncols());

    DenseMatrix expected = MAKE_DENSE_MATRIX(
      (1,0,0)
      (0,-1,0)
      (0,2,0));

    DenseMatrixHandle actual = makeDense(*sparseFromMap);
    EXPECT_MATRIX_EQ(*actual, expected);
  }

  SparseRowMatrixFromMap::Values additionalData;

  additionalData[4][0] = 3;
  additionalData[1][4] = -2;
  additionalData[4][4] = -1;

  //this value will overwrite sparseFromMap's value at 1,1
  additionalData[1][1] = 2;

  SparseRowMatrixHandle largerSparse = SparseRowMatrixFromMap::appendToSparseMatrix(5, 5, *sparseFromMap, additionalData);

  EXPECT_EQ(5, largerSparse->nrows());
  EXPECT_EQ(5, largerSparse->ncols());

  DenseMatrix expected = MAKE_DENSE_MATRIX(
    (1,0,0,0,0)
    (0,2,0,0,-2)
    (0,2,0,0,0)
    (0,0,0,0,0)
    (3,0,0,0,-1));

  DenseMatrixHandle actual = makeDense(*largerSparse);
  EXPECT_MATRIX_EQ(*actual, expected);
}

//bad test/code: uses addition of different-sized sparse matrices. Crashes on Mac, but only Win32+Debug. Disabling for now, I should just delete the function.
TEST(SparseMatrixFromMapTest, DISABLED_CanExtendSparseMatrixWithAdditionalValuesWithOverlapSumming)
{
  SparseRowMatrixHandle sparseFromMap;
  {
    SparseRowMatrixFromMap::Values data;

    data[0][0] = 1;
    data[2][1] = 2;
    data[1][1] = -1;

    sparseFromMap = SparseRowMatrixFromMap::make(3, 3, data);

    EXPECT_EQ(3, sparseFromMap->nrows());
    EXPECT_EQ(3, sparseFromMap->ncols());

    DenseMatrix expected = MAKE_DENSE_MATRIX(
      (1,0,0)
      (0,-1,0)
      (0,2,0));

    DenseMatrixHandle actual = makeDense(*sparseFromMap);
    EXPECT_MATRIX_EQ(*actual, expected);
  }

  SparseRowMatrixFromMap::Values additionalData;

  additionalData[4][0] = 3;
  additionalData[1][4] = -2;
  additionalData[4][4] = -1;

  //this value will SUM with sparseFromMap's value at 1,1
  additionalData[1][1] = 2;

  SparseRowMatrixHandle largerSparse = SparseRowMatrixFromMap::appendToSparseMatrixSumming(5, 5, *sparseFromMap, additionalData);

  EXPECT_EQ(5, largerSparse->nrows());
  EXPECT_EQ(5, largerSparse->ncols());

  DenseMatrix expected = MAKE_DENSE_MATRIX(
    (1,0,0,0,0)
    (0,1,0,0,-2)
    (0,2,0,0,0)
    (0,0,0,0,0)
    (3,0,0,0,-1));

  DenseMatrixHandle actual = makeDense(*largerSparse);
  EXPECT_MATRIX_EQ(*actual, expected);
}
