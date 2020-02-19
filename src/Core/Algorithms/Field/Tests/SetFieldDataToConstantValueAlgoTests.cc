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
#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldDataToConstantValue.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Utils/StringUtil.h>

#include <Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h>
#include <Testing/Utils/SCIRunFieldSamples.h>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;

TEST(SetFieldDataToConstantValueAlgoTest, NullInput)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle nullField;

  const double value = 0;
  algo.set(Parameters::Value, value);

  FieldHandle result;
  ASSERT_FALSE(algo.runImpl(nullField, result));
}

TEST(SetFieldDataToConstantValueAlgoTest, NullInputDoesNotThrow)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle nullField;

  const double value = 0;
  algo.set(Parameters::Value, value);

  FieldHandle result;
  ASSERT_NO_THROW(algo.runImpl(nullField, result));
}

TEST(SetFieldDataToConstantValueAlgoTest, InvalidType)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle cubeTriSurf = CubeTriSurfLinearBasis(INT_E);

  const int value = 5;
  algo.set(Parameters::Value, value);
  ASSERT_THROW(algo.setOption(Parameters::DataType, std::string("notatype")), AlgorithmParameterNotFound);
}

TEST(SetFieldDataToConstantValueAlgoTest, InvalidBasis)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle cubeTriSurf = CubeTriSurfLinearBasis(INT_E);

  const int value = 5;
  algo.set(Parameters::Value, value);
  ASSERT_THROW(algo.setOption(Parameters::BasisOrder, std::string("notabasis")), AlgorithmParameterNotFound);
}

TEST(SetFieldDataToConstantValueAlgoTest, EmptyFieldNoValue)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle emptyTetVol = EmptyTetVolFieldConstantBasis(DOUBLE_E);

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(emptyTetVol, result));

  ASSERT_EQ(result->vfield()->get_data_type(), emptyTetVol->vfield()->get_data_type());
  ASSERT_EQ(result->vfield()->basis_order(), emptyTetVol->vfield()->basis_order());
  ASSERT_TRUE(result->vfield()->is_double());
  ASSERT_TRUE(result->vfield()->is_constantdata());

  GetFieldDataAlgo getData;
  DenseMatrixHandle data = getData.runMatrix(result);
  ASSERT_TRUE(data != nullptr);

  // empty matrix
  EXPECT_EQ(1, data->ncols());
  EXPECT_EQ(0, data->nrows());
}

TEST(SetFieldDataToConstantValueAlgoTest, NoValue)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle tetTetVol = TetrahedronTetVolLinearBasis(DOUBLE_E);

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(tetTetVol, result));

  ASSERT_EQ(result->vfield()->get_data_type(), tetTetVol->vfield()->get_data_type());
  ASSERT_EQ(result->vfield()->basis_order(), tetTetVol->vfield()->basis_order());
  ASSERT_TRUE(result->vfield()->is_double());
  ASSERT_TRUE(result->vfield()->is_lineardata());

  GetFieldDataAlgo getData;
  DenseMatrixHandle data = getData.runMatrix(result);
  ASSERT_TRUE(data != nullptr);

  EXPECT_EQ(1, data->ncols());
  EXPECT_EQ(4, data->nrows());
  for (int i = 0; i < data->nrows(); ++i)
    ASSERT_EQ(0, (*data)(i,0));
}

TEST(SetFieldDataToConstantValueAlgoTest, IntValue)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle cubeTriSurf = CubeTriSurfLinearBasis(INT_E);

  const int value = 5;
  algo.set(Parameters::Value, value);

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(cubeTriSurf, result));

  ASSERT_EQ(result->vfield()->get_data_type(), cubeTriSurf->vfield()->get_data_type());
  ASSERT_EQ(result->vfield()->basis_order(), cubeTriSurf->vfield()->basis_order());
  ASSERT_TRUE(result->vfield()->is_int());
  ASSERT_TRUE(result->vfield()->is_lineardata());

  GetFieldDataAlgo getData;
  DenseMatrixHandle data = getData.runMatrix(result);
  ASSERT_TRUE(data != nullptr);

  EXPECT_EQ(1, data->ncols());
  EXPECT_EQ(8, data->nrows());
  for (int i = 0; i < data->nrows(); ++i)
    ASSERT_EQ(value, (*data)(i,0));
}

// LoadTet():
//- see Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h
//- loads TetVolMesh field with 7 nodes, 3 elements, linear basis, Vector data type
TEST(SetFieldDataToConstantValueAlgoTest, PositiveValue)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle tetmesh = LoadTet();
  GetFieldDataAlgo getData;

  const double value = 3.14;
  algo.set(Parameters::Value, value);

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(tetmesh, result));

  ASSERT_NE(result->vfield()->get_data_type(), tetmesh->vfield()->get_data_type());
  ASSERT_EQ(result->vfield()->basis_order(), tetmesh->vfield()->basis_order());
  ASSERT_TRUE(result->vfield()->is_double());
  ASSERT_TRUE(result->vfield()->is_lineardata());

  DenseMatrixHandle data = getData.runMatrix(result);
  ASSERT_TRUE(data != nullptr);

  EXPECT_EQ(1, data->ncols());
  EXPECT_EQ(7, data->nrows());
  for (int i = 0; i < data->nrows(); ++i)
    ASSERT_EQ(value, (*data)(i,0));
}

TEST(SetFieldDataToConstantValueAlgoTest, NegativeValue)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle tetmesh = LoadTet();
  GetFieldDataAlgo getData;

  const double value = -5.7;
  algo.set(Parameters::Value, value);

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(tetmesh, result));

  ASSERT_NE(result->vfield()->get_data_type(), tetmesh->vfield()->get_data_type());
  ASSERT_EQ(result->vfield()->basis_order(), tetmesh->vfield()->basis_order());
  ASSERT_TRUE(result->vfield()->is_double());
  ASSERT_TRUE(result->vfield()->is_lineardata());

  DenseMatrixHandle data = getData.runMatrix(result);
  ASSERT_TRUE(data != nullptr);

  EXPECT_EQ(1, data->ncols());
  EXPECT_EQ(7, data->nrows());
  for (int i = 0; i < data->nrows(); ++i)
    ASSERT_EQ(value, (*data)(i,0));
}

//TODO DAN FIX TEST
TEST(SetFieldDataToConstantValueAlgoTest, DISABLED_DataTypeIntLinear)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle cubeTriSurf = CubeTriSurfLinearBasis(INT_E);

  const int value = -200;
  algo.set(Parameters::Value, value);

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(cubeTriSurf, result));

  ASSERT_EQ(result->vfield()->get_data_type(), cubeTriSurf->vfield()->get_data_type());
  ASSERT_EQ(result->vfield()->basis_order(), cubeTriSurf->vfield()->basis_order());
  ASSERT_TRUE(result->vfield()->is_int());
  ASSERT_TRUE(result->vfield()->is_lineardata());

  GetFieldDataAlgo getData;
  DenseMatrixHandle data = getData.runMatrix(result);
  ASSERT_TRUE(data != nullptr);

  EXPECT_EQ(1, data->ncols());
  EXPECT_EQ(8, data->nrows());
  for (int i = 0; i < data->nrows(); ++i)
    ASSERT_EQ(value, (*data)(i,0));
}

//TODO DAN FIX TEST
TEST(SetFieldDataToConstantValueAlgoTest, DISABLED_DataTypeIntConstant)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle cubeTriSurf = CubeTriSurfConstantBasis(INT_E);

  const int value = -200;
  algo.set(Parameters::Value, value);

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(cubeTriSurf, result));

  ASSERT_EQ(result->vfield()->get_data_type(), cubeTriSurf->vfield()->get_data_type());
  ASSERT_EQ(result->vfield()->basis_order(), cubeTriSurf->vfield()->basis_order());
  ASSERT_TRUE(result->vfield()->is_int());
  ASSERT_TRUE(result->vfield()->is_constantdata());

  GetFieldDataAlgo getData;
  DenseMatrixHandle data = getData.runMatrix(result);
  ASSERT_TRUE(data != nullptr);

  EXPECT_EQ(1, data->ncols());
  EXPECT_EQ(12, data->nrows());
  for (int i = 0; i < data->nrows(); ++i)
    ASSERT_EQ(value, (*data)(i,0));
}

TEST(SetFieldDataToConstantValueAlgoTest, DataTypeDoubleToInt)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle cubeTriSurf = CubeTriSurfLinearBasis(DOUBLE_E);

  const int value = 5;
  algo.set(Parameters::Value, value);
  algo.setOption(Parameters::DataType, std::string("int"));

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(cubeTriSurf, result));

  ASSERT_NE(result->vfield()->get_data_type(), cubeTriSurf->vfield()->get_data_type());
  ASSERT_EQ(result->vfield()->basis_order(), cubeTriSurf->vfield()->basis_order());
  ASSERT_TRUE(result->vfield()->is_int());
  ASSERT_TRUE(result->vfield()->is_lineardata());

  GetFieldDataAlgo getData;
  DenseMatrixHandle data = getData.runMatrix(result);
  ASSERT_TRUE(data != nullptr);

  EXPECT_EQ(1, data->ncols());
  EXPECT_EQ(8, data->nrows());
  for (int i = 0; i < data->nrows(); ++i)
    ASSERT_EQ(value, (*data)(i,0));
}

TEST(SetFieldDataToConstantValueAlgoTest, DataTypeDoubleToIntFromDouble)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle cubeTriSurf = CubeTriSurfLinearBasis(DOUBLE_E);

  const double value = 5.2;
  const int intValue = 5;
  algo.set(Parameters::Value, value);
  algo.setOption(Parameters::DataType, std::string("int"));

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(cubeTriSurf, result));

  ASSERT_NE(result->vfield()->get_data_type(), cubeTriSurf->vfield()->get_data_type());
  ASSERT_EQ(result->vfield()->basis_order(), cubeTriSurf->vfield()->basis_order());
  ASSERT_TRUE(result->vfield()->is_int());
  ASSERT_TRUE(result->vfield()->is_lineardata());

  GetFieldDataAlgo getData;
  DenseMatrixHandle data = getData.runMatrix(result);
  ASSERT_TRUE(data != nullptr);

  EXPECT_EQ(1, data->ncols());
  EXPECT_EQ(8, data->nrows());
  for (int i = 0; i < data->nrows(); ++i)
    ASSERT_EQ(intValue, (*data)(i,0));
}

TEST(SetFieldDataToConstantValueAlgoTest, BasisOrderLinearToConstantTet)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle tetmesh = LoadTet();
  GetFieldDataAlgo getData;

  const double value = -5.7;
  algo.set(Parameters::Value, value);
  algo.setOption(Parameters::BasisOrder, std::string("constant"));

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(tetmesh, result));
  ASSERT_EQ(0, result->vfield()->basis_order());

  ASSERT_NE(result->vfield()->get_data_type(), tetmesh->vfield()->get_data_type());
  ASSERT_TRUE(result->vfield()->is_double());
  ASSERT_TRUE(tetmesh->vfield()->is_lineardata());
  ASSERT_TRUE(result->vfield()->is_constantdata());

  DenseMatrixHandle data = getData.runMatrix(result);

  ASSERT_TRUE(data != nullptr);

  EXPECT_EQ(1, data->ncols());
  EXPECT_EQ(3, data->nrows());
  for (int i = 0; i < data->nrows(); ++i)
    ASSERT_EQ(value, (*data)(i,0));
}

TEST(SetFieldDataToConstantValueAlgoTest, BasisOrderLinearToConstantTri)
{
  SetFieldDataToConstantValueAlgo algo;
  FieldHandle cubeTriSurf = CubeTriSurfLinearBasis(INT_E);

  const int value = 5;
  algo.set(Parameters::Value, value);
  algo.setOption(Parameters::BasisOrder, std::string("constant"));

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(cubeTriSurf, result));

  ASSERT_EQ(result->vfield()->get_data_type(), cubeTriSurf->vfield()->get_data_type());
  ASSERT_TRUE(result->vfield()->is_int());
  ASSERT_TRUE(cubeTriSurf->vfield()->is_lineardata());
  ASSERT_TRUE(result->vfield()->is_constantdata());

  GetFieldDataAlgo getData;
  DenseMatrixHandle data = getData.runMatrix(result);
  ASSERT_TRUE(data != nullptr);

  EXPECT_EQ(1, data->ncols());
  EXPECT_EQ(12, data->nrows());
  for (int i = 0; i < data->nrows(); ++i)
    ASSERT_EQ(value, (*data)(i,0));
}
