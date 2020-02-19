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
#include <Testing/Utils/SCIRunFieldSamples.h>

using namespace SCIRun;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;


TEST(SetComplexFieldDataTest, LatVolOnNodeScalarMat)
{
  SetFieldDataAlgo algo;

  auto latvol = CreateEmptyLatVol(2,2,3);

  auto matrix = boost::make_shared<ComplexDenseMatrix>(2 * 2 * 3, 1, complex(1, 1));

  auto result = algo.runImplComplex(latvol, matrix);
  FieldInformation fi(result);

  std::cout << "SCIRun's first complex field type: " << fi.get_field_type_id() << std::endl;
  EXPECT_EQ("GenericField<LatVolMesh<HexTrilinearLgn<Point>>,HexTrilinearLgn<complex>,FData3d<complex,LatVolMesh<HexTrilinearLgn<Point>>>>", fi.get_field_type_id());

  GetFieldDataAlgo algo1;

  auto resultmatrix = algo1.runComplexMatrix(result);

  EXPECT_EQ(*resultmatrix, *matrix);
}

TEST(SetComplexFieldDataTest, LatVolOnElemScalarMat)
{
  SetFieldDataAlgo algo;

  auto latvol = CreateEmptyLatVol(2, 2, 3);

  auto matrix = boost::make_shared<ComplexDenseMatrix>(2, 1, complex(1, 1));

  auto result = algo.runImplComplex(latvol, matrix);

  GetFieldDataAlgo algo1;

  auto resultmatrix = algo1.runComplexMatrix(result);

  EXPECT_EQ(*resultmatrix, *matrix);
}

TEST(SetComplexFieldDataTest, TetMeshOnNodeScalarMat)
{
  SetFieldDataAlgo algo;

  auto tetmesh = LoadTet();

  auto matrix = boost::make_shared<ComplexDenseMatrix>(7, 1, complex(1, 1));

  auto result = algo.runImplComplex(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  auto resultmatrix = algo1.runComplexMatrix(result);

  EXPECT_EQ(*resultmatrix, *matrix);
}

TEST(SetComplexFieldDataTest, TetMeshOnElemScalarMat)
{
  SetFieldDataAlgo algo;

  auto tetmesh = LoadTet();

  auto matrix = boost::make_shared<ComplexDenseMatrix>(3, 1, complex(1, 1));

  auto result = algo.runImplComplex(tetmesh, matrix);

  GetFieldDataAlgo algo1;

  auto resultmatrix = algo1.runComplexMatrix(result);

  EXPECT_EQ(*resultmatrix, *matrix);
}
