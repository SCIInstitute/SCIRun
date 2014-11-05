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
#include <Core/Algorithms/Legacy/Fields/FieldData/ConvertFieldBasisType.h>
#include <Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Utils/StringUtil.h>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

TEST(ConvertFieldBasisAlgoTest, TetMeshOnNodeVectorMat)
{
  ConvertFieldBasisTypeAlgo algo;

  FieldHandle tetmesh = LoadTet();

  const int givenBasis = 1;
  EXPECT_EQ(givenBasis, tetmesh->vfield()->basis_order());

  algo.set_option(Parameters::OutputType, "Constant");

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(tetmesh, result));

  const int expectedBasis = 0;
  EXPECT_EQ(expectedBasis, result->vfield()->basis_order());

  //TODO: compare basis
  FAIL() << "todo";
}

/*
TEST(SetFieldDataTest, TetMeshOnNodeScalarMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTet();

  DenseMatrixHandle matrix = TetMeshOnNodeScalarMat();

  FieldHandle result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, DISABLED_TetMeshOnNodeTensorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTet();

  DenseMatrixHandle matrix = TetMeshOnNodeTensorMat();

  FieldHandle result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}


TEST(SetFieldDataTest, TetMeshOnElemScalarMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTet();

  DenseMatrixHandle matrix = TetMeshOnElemScalarMat();

  FieldHandle result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, TetMeshOnElemVectorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTet();

  DenseMatrixHandle matrix = TetMeshOnElemVectorMat();

  FieldHandle result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}


TEST(SetFieldDataTest, DISABLED_TetMeshOnElemTensorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTet();

  DenseMatrixHandle matrix = TetMeshOnElemTensorMat();

  FieldHandle result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}


TEST(SetFieldDataTest, TriSurfOnNodeVectorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnNodeVectorMat();

  FieldHandle result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}


TEST(SetFieldDataTest, TriSurfOnNodeScalarMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnNodeScalarMat();

  FieldHandle result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, DISABLED_TriSurfOnNodeTensorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnNodeTensorMat();

  FieldHandle result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, TriSurfOnElemVectorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnElemVectorMat();

  FieldHandle result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}


TEST(SetFieldDataTest, TriSurfOnElemScalarMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnElemScalarMat();

  FieldHandle result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetFieldDataTest, DISABLED_TriSurfOnElemTensorMat)
{
  SetFieldDataAlgo algo;

  FieldHandle tetmesh = LoadTri();

  DenseMatrixHandle matrix = TriSurfOnNodeTensorMat();

  FieldHandle result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}


*/