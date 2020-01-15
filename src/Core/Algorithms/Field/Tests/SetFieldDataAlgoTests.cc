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

   Author:              Moritz Dannhauer
   Last Modification:   April 10 2014
*/


#include <gtest/gtest.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldData.h>
#include <Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Utils/StringUtil.h>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

//TODO (DAN) : No way to import (ReadField) TriSurf (on nodes or elements) with defined tensors

TEST(SetFieldDataTest, TetMeshOnNodeVectorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTet();

  DenseMatrixHandle matrix = TetMeshOnNodeVectorMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, TetMeshOnNodeScalarMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTet();

  DenseMatrixHandle matrix = TetMeshOnNodeScalarMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, DISABLED_TetMeshOnNodeTensorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTet();

  DenseMatrixHandle matrix = TetMeshOnNodeTensorMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}


TEST(SetFieldDataTest, TetMeshOnElemScalarMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTet();

  DenseMatrixHandle matrix = TetMeshOnElemScalarMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, TetMeshOnElemVectorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTet();

  DenseMatrixHandle matrix = TetMeshOnElemVectorMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}


TEST(SetFieldDataTest, DISABLED_TetMeshOnElemTensorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTet();

  DenseMatrixHandle matrix = TetMeshOnElemTensorMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}


TEST(SetFieldDataTest, TriSurfOnNodeVectorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnNodeVectorMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}


TEST(SetFieldDataTest, TriSurfOnNodeScalarMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnNodeScalarMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, DISABLED_TriSurfOnNodeTensorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnNodeTensorMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, TriSurfOnElemVectorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnElemVectorMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}


TEST(SetFieldDataTest, TriSurfOnElemScalarMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnElemScalarMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, DISABLED_TriSurfOnElemTensorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnNodeTensorMat();

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, TriSurfOnNodeScalarMatrixTooSmall)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnNodeScalarMat();
  matrix->resize(matrix->rows() - 1, matrix->cols());

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

//TODO: issue made, requirements need refining to see if this one is actually a problem.
TEST(SetFieldDataTest, DISABLED_TriSurfOnNodeScalarMatrixTooLarge)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnNodeScalarMat();
  matrix->resize(matrix->rows() + 1, matrix->cols());

  FieldHandle result = algo.runImpl(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.runMatrix(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}
