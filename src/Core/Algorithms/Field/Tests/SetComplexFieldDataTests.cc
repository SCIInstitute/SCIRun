/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldData.h>
#include <Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Utils/StringUtil.h>
#include <Testing/Utils/SCIRunFieldSamples.h>

using namespace SCIRun;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;



TEST(SetComplexFieldDataTest, LatVolOnNodeScalarMat)
{
  SetFieldDataAlgo algo;

  auto latvol = CreateEmptyLatVol(2,2,3);

  auto matrix = TetMeshOnNodeScalarMat();

  FieldHandle result = algo.run(latvol, matrix);

  GetFieldDataAlgo algo1;

  auto resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetComplexFieldDataTest, LatVolOnElemScalarMat)
{
  FAIL() << "NEEDS COMPLEX VALUES";
  SetFieldDataAlgo algo;

  auto latvol = CreateEmptyLatVol(2, 2, 3);

  auto matrix = TetMeshOnNodeScalarMat();

  auto result = algo.run(latvol, matrix);

  GetFieldDataAlgo algo1;

  auto resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetComplexFieldDataTest, TetMeshOnNodeScalarMat)
{
  FAIL() << "NEEDS COMPLEX VALUES";
  SetFieldDataAlgo algo;

  auto tetmesh = LoadTet();

  auto matrix = TetMeshOnNodeScalarMat();

  auto result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  auto resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}

TEST(SetComplexFieldDataTest, TetMeshOnElemScalarMat)
{
  FAIL() << "NEEDS COMPLEX VALUES";
  SetFieldDataAlgo algo;

  auto tetmesh = LoadTet();

  auto matrix = TetMeshOnElemScalarMat();

  auto result = algo.run(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  auto resultmatrix = algo1.run(result);

  EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}
