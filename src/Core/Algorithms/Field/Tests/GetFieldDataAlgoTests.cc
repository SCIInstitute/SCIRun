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

author: Moritz Dannhauer
last change: 04/10/2014
*/

#include <gtest/gtest.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Utils/StringUtil.h>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

TEST(GetFieldDataTest, TriSurfOnNodeScalar)
{
  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(TriSurfOnNodeScalar());
  DenseMatrixHandle expected_result = TriSurfOnNodeScalarMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
}


TEST(GetFieldDataTest, TriSurfOnNodeVector)
{
  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(TriSurfOnNodeVector());
  DenseMatrixHandle expected_result = TriSurfOnNodeVectorMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
}

TEST(GetFieldDataTest, TriSurfOnElemScalar)
{
  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(TriSurfOnElemScalar());
  DenseMatrixHandle expected_result = TriSurfOnElemScalarMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
}


TEST(GetFieldDataTest, TriSurfOnElemVector)
{
  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(TriSurfOnElemVector());
  DenseMatrixHandle expected_result = TriSurfOnElemVectorMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
}


TEST(GetFieldDataTest, TetMeshOnNodeScalar)
{
  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(TetMeshOnNodeScalar());
  DenseMatrixHandle expected_result = TetMeshOnNodeScalarMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
}

TEST(GetFieldDataTest, TetMeshOnNodeVector)
{
  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(TetMeshOnNodeVector());
  DenseMatrixHandle expected_result = TetMeshOnNodeVectorMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
}

TEST(GetFieldDataTest, TetMeshOnNodeTensor)
{
  /*  Does not work since SCIRun5 ReadField cannot load in that files

  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(TetMeshOnNodeTensor());
  DenseMatrixHandle expected_result = TetMeshOnNodeTensorMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
  */
}


TEST(GetFieldDataTest, TetMeshOnElemScalar)
{
  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(TetMeshOnElemScalar());
  DenseMatrixHandle expected_result = TetMeshOnElemScalarMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
}

TEST(GetFieldDataTest, TetMeshOnElemVector)
{
  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(TetMeshOnElemVector());
  DenseMatrixHandle expected_result = TetMeshOnElemVectorMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
}

TEST(GetFieldDataTest, TetMeshOnElemTensor)
{
  /*
  GetFieldDataAlgo algo; 

  DenseMatrixHandle result = algo.run(TetMeshOnElemTensor());
  DenseMatrixHandle expected_result = TetMeshOnElemTensorMat();

  std::cout << "nr:" << result->nrows() << std::endl;
  std::cout << "nc:" << result->ncols() << std::endl;

  std::cout << "enr:" << expected_result->nrows() << std::endl;
  std::cout << "enc:" << expected_result->ncols() << std::endl;

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
  */ 
}


TEST(GetFieldDataTest, PointCloudOnNodeScalar)
{
  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(PointCloudOnNodeScalar());
  DenseMatrixHandle expected_result = PointCloudOnNodeScalarMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
}

TEST(GetFieldDataTest, PointCloudOnNodeVector)
{
  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(PointCloudOnNodeVector());
  DenseMatrixHandle expected_result = PointCloudOnNodeVectorMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
}

TEST(GetFieldDataTest, PointCloudOnNodeTensor)
{
  /*
  GetFieldDataAlgo algo;

  DenseMatrixHandle result = algo.run(PointCloudOnNodeTensor());
  DenseMatrixHandle expected_result = PointCloudOnNodeTensorMat();

  EXPECT_MATRIX_EQ_TOLERANCE(*result, *expected_result, 1e-16);
  */
}


