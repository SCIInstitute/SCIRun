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
   */

#include <Python.h>
#include <boost/python.hpp>

#include <gtest/gtest.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Python/PythonDatatypeConverter.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Matlab/matlabconverter.h>

using namespace SCIRun;
using namespace Core::Python;
using namespace Testing;

class FieldConversionTests : public testing::Test
{
public:
  FieldConversionTests()
  {
    Py_Initialize();
  }
};

TEST_F(FieldConversionTests, RoundTripLatVolUsingJustMatlabConversion)
{
  auto expected = CreateEmptyLatVol();
  MatlabIO::matlabarray ma;
  {
    MatlabIO::matlabconverter mc(nullptr);
    mc.converttostructmatrix();
    mc.sciFieldTOmlArray(expected, ma);
  }

  FieldHandle actual;
  {
    MatlabIO::matlabconverter mc(nullptr);
    mc.mlArrayTOsciField(ma, actual);
  }

  ASSERT_TRUE(actual != nullptr);
  auto actualField = boost::dynamic_pointer_cast<Field>(actual);
  ASSERT_TRUE(actualField != nullptr);

  FieldInformation info(actualField);
  EXPECT_TRUE(info.is_latvolmesh());
  EXPECT_TRUE(info.is_double());
  EXPECT_TRUE(info.is_scalar());
  EXPECT_TRUE(info.is_linear());
  EXPECT_EQ("LatVolMesh<HexTrilinearLgn<Point>>", info.get_mesh_type_id());
  EXPECT_EQ("LatVolMesh", info.get_mesh_type());
  EXPECT_EQ("GenericField<LatVolMesh<HexTrilinearLgn<Point>>,HexTrilinearLgn<double>,FData3d<double,LatVolMesh<HexTrilinearLgn<Point>>>>", info.get_field_type_id());
}

TEST_F(FieldConversionTests, RoundTripLatVol)
{
  auto expected = CreateEmptyLatVol();
  auto pyField = convertFieldToPython(expected);
  EXPECT_EQ(9, len(pyField.items()));

  FieldExtractor converter(pyField);

  ASSERT_TRUE(converter.check());

  auto actual = converter();
  ASSERT_TRUE(actual != nullptr);
  auto actualField = boost::dynamic_pointer_cast<Field>(actual);
  ASSERT_TRUE(actualField != nullptr);

  FieldInformation info(actualField);
  EXPECT_TRUE(info.is_latvolmesh());
  EXPECT_TRUE(info.is_double());
  EXPECT_TRUE(info.is_scalar());
  EXPECT_TRUE(info.is_linear());
  EXPECT_EQ("LatVolMesh<HexTrilinearLgn<Point>>", info.get_mesh_type_id());
  EXPECT_EQ("LatVolMesh", info.get_mesh_type());
  EXPECT_EQ("GenericField<LatVolMesh<HexTrilinearLgn<Point>>,HexTrilinearLgn<double>,FData3d<double,LatVolMesh<HexTrilinearLgn<Point>>>>", info.get_field_type_id());
}

TEST_F(FieldConversionTests, RejectsEmptyDictionary)
{
  boost::python::dict emptyDict;
  FieldExtractor converter(emptyDict);

  ASSERT_FALSE(converter.check());
}

TEST_F(FieldConversionTests, RejectsIncompatibleDictionary)
{
  boost::python::dict dict;
  dict.setdefault(2, 5);
  ASSERT_EQ(1, len(dict));
  FieldExtractor converter(dict);

  ASSERT_FALSE(converter.check());
}
