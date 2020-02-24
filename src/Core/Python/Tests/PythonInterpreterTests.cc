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


#include <Python.h>
#include <boost/python.hpp>

#include <gtest/gtest.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Python/PythonDatatypeConverter.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Matlab/matlabconverter.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/SCIRunFieldSamples.h>
#ifdef WIN32
#ifndef DEBUG
#include <Core/Python/PythonInterpreter.h>
#endif
#endif

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace Core::Python;
using namespace Testing;
using namespace TestUtils;

class FieldConversionTests : public testing::Test
{
protected:
  virtual void SetUp() override
  {
  #ifdef WIN32
  #ifndef DEBUG
    PythonInterpreter::Instance().initialize(false, "Core_Python_Tests", boost::filesystem::current_path().string());
    PythonInterpreter::Instance().importSCIRunLibrary();
  #endif
  #else
    Py_Initialize();
  #endif
  }

  static FieldHandle roundTripThroughMatlabConverter(FieldHandle field)
  {
    MatlabIO::matlabarray ma;
    {
      MatlabIO::matlabconverter mc(nullptr);
      mc.converttostructmatrix();
      mc.sciFieldTOmlArray(field, ma);
    }

    FieldHandle actual;
    {
      MatlabIO::matlabconverter mc(nullptr);
      mc.mlArrayTOsciField(ma, actual);
    }
    return actual;
  }

  static FieldHandle TetMesh1()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_mapfielddatafromelemtonode.fld");
  }

  static FieldHandle TetMesh2()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_mapfielddatafromnodetoelem.fld");
  }

  static FieldHandle CreateTriSurfScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.fld");
  }
  static FieldHandle CreateTriSurfVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/vector/tri_vector_on_node.fld");
  }

  static FieldHandle CreateTetMeshVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/vector/tet_vector_on_node.fld");
  }
  static FieldHandle CreateTetMeshScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
  static FieldHandle CreateTetMeshScalarOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_elem/scalar/tet_scalar_on_elem.fld");
  }
  static FieldHandle CreateTetMeshTensorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/tensor/tet_tensor_on_node.fld");
  }

  static FieldHandle CreatePointCloudScalar()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/point_cloud/scalar/pts_scalar.fld");
  }

  static FieldHandle CreateCurveMeshElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_curve_elem.fld");
  }

  static FieldHandle CreateImageNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_image_node.fld");
  }

  static FieldHandle CreateTriSurfFromCVRTI()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/CVRTI_cappedTorso/CappedTorso_192.fld");
  }

  static std::vector<FieldHandle> fileExamples()
  {
    return{ TetMesh1(), TetMesh2(), CreateTriSurfScalarOnNode(), CreateTriSurfVectorOnNode(), CreateTetMeshVectorOnNode(),
      CreateTetMeshScalarOnElem(), CreateTetMeshScalarOnNode(), CreateTetMeshTensorOnNode(), CreatePointCloudScalar(),
      CreateCurveMeshElem(), CreateImageNode()
    };
  }
};

TEST_F(FieldConversionTests, RoundTripLatVolUsingJustMatlabConversion)
{
  auto expected = CreateEmptyLatVol(2,2,2);
  FieldInformation expectedInfo(expected);
  auto actual = roundTripThroughMatlabConverter(expected);

  ASSERT_TRUE(actual != nullptr);
  auto actualField = boost::dynamic_pointer_cast<Field>(actual);
  ASSERT_TRUE(actualField != nullptr);

  FieldInformation info(actualField);
  ASSERT_EQ(expectedInfo, info);
  EXPECT_TRUE(info.is_latvolmesh());
  EXPECT_TRUE(info.is_double());
  EXPECT_TRUE(info.is_scalar());
  EXPECT_TRUE(info.is_linear());
  EXPECT_EQ("LatVolMesh<HexTrilinearLgn<Point>>", info.get_mesh_type_id());
  EXPECT_EQ("LatVolMesh", info.get_mesh_type());
  EXPECT_EQ("GenericField<LatVolMesh<HexTrilinearLgn<Point>>,HexTrilinearLgn<double>,FData3d<double,LatVolMesh<HexTrilinearLgn<Point>>>>", info.get_field_type_id());
  auto expectedTransform = expected->vmesh()->get_transform();
  auto actualTransform = actualField->vmesh()->get_transform();
  ASSERT_EQ(expectedTransform, actualTransform);
}

TEST_F(FieldConversionTests, RoundTripTetVolUsingJustMatlabConversion)
{
  auto expected = TetMesh1();
  FieldInformation expectedInfo(expected);
  auto actual = roundTripThroughMatlabConverter(expected);

  ASSERT_TRUE(actual != nullptr);
  auto actualField = boost::dynamic_pointer_cast<Field>(actual);
  ASSERT_TRUE(actualField != nullptr);

  FieldInformation info(actualField);
  ASSERT_EQ(expectedInfo, info);
  EXPECT_TRUE(info.is_tetvolmesh());
  EXPECT_TRUE(info.is_double());
  EXPECT_TRUE(info.is_scalar());
  EXPECT_TRUE(info.is_constantdata());
  EXPECT_EQ("TetVolMesh<TetLinearLgn<Point>>", info.get_mesh_type_id());
  EXPECT_EQ("TetVolMesh", info.get_mesh_type());
  EXPECT_EQ("GenericField<TetVolMesh<TetLinearLgn<Point>>,ConstantBasis<double>,vector<double>>", info.get_field_type_id());
}

TEST_F(FieldConversionTests, RoundTripTriSurfUsingJustMatlabConversion)
{
  auto expected = CreateTriSurfScalarOnNode();
  FieldInformation expectedInfo(expected);
  auto actual = roundTripThroughMatlabConverter(expected);

  ASSERT_TRUE(actual != nullptr);
  auto actualField = boost::dynamic_pointer_cast<Field>(actual);
  ASSERT_TRUE(actualField != nullptr);

  FieldInformation info(actualField);
  ASSERT_EQ(expectedInfo, info);
  EXPECT_TRUE(info.is_trisurf());
  EXPECT_TRUE(info.is_double());
  EXPECT_TRUE(info.is_scalar());
  EXPECT_TRUE(info.is_linear());
  EXPECT_EQ("TriSurfMesh<TriLinearLgn<Point>>", info.get_mesh_type_id());
  EXPECT_EQ("TriSurfMesh", info.get_mesh_type());
  EXPECT_EQ("GenericField<TriSurfMesh<TriLinearLgn<Point>>,TriLinearLgn<double>,vector<double>>", info.get_field_type_id());
}

TEST_F(FieldConversionTests, LoopThroughFieldFilesMatlabOnly)
{
  for (const auto& field : fileExamples())
  {
    auto expected = field;
    FieldInformation expectedInfo(expected);
    auto actual = roundTripThroughMatlabConverter(expected);

    ASSERT_TRUE(actual != nullptr);
    auto actualField = boost::dynamic_pointer_cast<Field>(actual);
    ASSERT_TRUE(actualField != nullptr);

    FieldInformation info(actualField);
    ASSERT_EQ(expectedInfo, info);
  }
}

TEST_F(FieldConversionTests, LoopThroughFieldFiles)
{
  for (const auto& field : fileExamples())
  {
    auto expected = field;
    FieldInformation expectedInfo(expected);
    // std::cout << "Converting " << expectedInfo.get_field_type_id() << " to python." << std::endl;
    auto pyField = convertFieldToPython(expected);
    FieldExtractor converter(pyField);

    ASSERT_TRUE(converter.check());

    // std::cout << "Converting " << expectedInfo.get_field_type_id() << " from python." << std::endl;
    auto actual = converter();
    ASSERT_TRUE(actual != nullptr);
    auto actualField = boost::dynamic_pointer_cast<Field>(actual);
    ASSERT_TRUE(actualField != nullptr);

    FieldInformation info(actualField);
    ASSERT_EQ(expectedInfo, info);

    ASSERT_TRUE(compareNodes(expected, actualField));

    std::cout << "Done testing " << expectedInfo.get_field_type_id() << "." << std::endl;
  }
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

TEST_F(FieldConversionTests, RoundTripTriSurf)
{
  auto expected = CreateTriSurfScalarOnNode();
  auto pyField = convertFieldToPython(expected);
  EXPECT_EQ(10, len(pyField.items()));

  FieldExtractor converter(pyField);

  ASSERT_TRUE(converter.check());

  auto actual = converter();
  ASSERT_TRUE(actual != nullptr);
  auto actualField = boost::dynamic_pointer_cast<Field>(actual);
  ASSERT_TRUE(actualField != nullptr);

  FieldInformation info(actualField);
  EXPECT_TRUE(info.is_trisurf());
  EXPECT_TRUE(info.is_double());
  EXPECT_TRUE(info.is_scalar());
  EXPECT_TRUE(info.is_linear());
  EXPECT_EQ("TriSurfMesh<TriLinearLgn<Point>>", info.get_mesh_type_id());
  EXPECT_EQ("TriSurfMesh", info.get_mesh_type());
  EXPECT_EQ("GenericField<TriSurfMesh<TriLinearLgn<Point>>,TriLinearLgn<double>,vector<double>>", info.get_field_type_id());
}

TEST_F(FieldConversionTests, RoundTripTetVolNode)
{
  auto expected = CreateTetMeshScalarOnNode();
  auto pyField = convertFieldToPython(expected);
  EXPECT_EQ(10, len(pyField.items()));

  FieldExtractor converter(pyField);

  ASSERT_TRUE(converter.check());

  auto actual = converter();
  ASSERT_TRUE(actual != nullptr);
  auto actualField = boost::dynamic_pointer_cast<Field>(actual);
  ASSERT_TRUE(actualField != nullptr);

  FieldInformation info(actualField);
  EXPECT_TRUE(info.is_tetvolmesh());
  EXPECT_TRUE(info.is_double());
  EXPECT_TRUE(info.is_scalar());
  EXPECT_TRUE(info.is_linear());
  EXPECT_EQ("TetVolMesh<TetLinearLgn<Point>>", info.get_mesh_type_id());
  EXPECT_EQ("TetVolMesh", info.get_mesh_type());
  EXPECT_EQ("GenericField<TetVolMesh<TetLinearLgn<Point>>,TetLinearLgn<double>,vector<double>>", info.get_field_type_id());
}

// TODO: found a workaround for Brett's failing mesh (need to set data to all zeros)
TEST_F(FieldConversionTests, DISABLED_RoundTripTriSurfCVRTI)
{
  auto expected = CreateTriSurfFromCVRTI();
  {
    FieldInformation info(expected);
    EXPECT_TRUE(info.is_trisurf());
    EXPECT_TRUE(info.is_double());
    EXPECT_TRUE(info.is_scalar());
    EXPECT_TRUE(info.is_linear());
    EXPECT_EQ("TriSurfMesh<TriLinearLgn<Point>>", info.get_mesh_type_id());
    EXPECT_EQ("TriSurfMesh", info.get_mesh_type());
    EXPECT_EQ("GenericField<TriSurfMesh<TriLinearLgn<Point>>,TriLinearLgn<double>,vector<double>>", info.get_field_type_id());
  }

  auto pyField = convertFieldToPython(expected);
  EXPECT_EQ(10, len(pyField.items()));
  {
    boost::python::extract<boost::python::dict> e(pyField);
    auto pyMatlabDict = e();

    auto length = len(pyMatlabDict);

    auto keys = pyMatlabDict.keys();
    auto values = pyMatlabDict.values();

    for (int i = 0; i < length; ++i)
    {
      boost::python::extract<std::string> key_i(keys[i]);

      boost::python::extract<std::string> value_i_string(values[i]);
      boost::python::extract<boost::python::list> value_i_list(values[i]);
      auto fieldName = key_i();
      std::cout << "setting field " << fieldName << " " << (value_i_string.check() ? value_i_string() : "NOT A STRING") << std::endl;
    }
  }

  FieldExtractor converter(pyField);

  ASSERT_TRUE(converter.check());

  auto actual = converter();
  ASSERT_TRUE(actual != nullptr);
  auto actualField = boost::dynamic_pointer_cast<Field>(actual);
  ASSERT_TRUE(actualField != nullptr);

  FieldInformation info(actualField);
  EXPECT_TRUE(info.is_trisurf());
  EXPECT_TRUE(info.is_double());
  EXPECT_TRUE(info.is_scalar());
  EXPECT_TRUE(info.is_linear());
  EXPECT_EQ("TriSurfMesh<TriLinearLgn<Point>>", info.get_mesh_type_id());
  EXPECT_EQ("TriSurfMesh", info.get_mesh_type());
  EXPECT_EQ("GenericField<TriSurfMesh<TriLinearLgn<Point>>,TriLinearLgn<double>,vector<double>>", info.get_field_type_id());
}

TEST_F(FieldConversionTests, RoundTripTetVolCell)
{
  auto expected = CreateTetMeshScalarOnElem();
  auto pyField = convertFieldToPython(expected);
  EXPECT_EQ(10, len(pyField.items()));

  FieldExtractor converter(pyField);

  ASSERT_TRUE(converter.check());

  auto actual = converter();
  ASSERT_TRUE(actual != nullptr);
  auto actualField = boost::dynamic_pointer_cast<Field>(actual);
  ASSERT_TRUE(actualField != nullptr);

  FieldInformation info(actualField);
  EXPECT_TRUE(info.is_tetvolmesh());
  EXPECT_TRUE(info.is_double());
  EXPECT_TRUE(info.is_scalar());
  EXPECT_TRUE(info.is_constantdata());
  EXPECT_EQ("TetVolMesh<TetLinearLgn<Point>>", info.get_mesh_type_id());
  EXPECT_EQ("TetVolMesh", info.get_mesh_type());
  EXPECT_EQ("GenericField<TetVolMesh<TetLinearLgn<Point>>,ConstantBasis<double>,vector<double>>", info.get_field_type_id());
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
