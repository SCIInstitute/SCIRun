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


#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Modules/Visualization/ShowField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Utils/Exception.h>
#include <Core/Logging/Log.h>
#include <Core/Datatypes/ColorMap.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core;
using namespace SCIRun;
using namespace SCIRun::Core::Logging;
using ::testing::Values;
using ::testing::Combine;
using ::testing::Range;
using ::testing::NotNull;
using ::testing::TestWithParam;
using ::testing::Bool;






//--------------------------------------------------------------------------------------------------
//---------------- Brute Force ---------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//27527 ms
//115 ms
class ShowFieldBruteForceTest : public ParameterizedModuleTest<std::tuple<bool, bool, bool, bool, bool, bool,
  double, int, int>>
{
protected:
  enum ShowFieldParams
  {
    SHOW_NODES,
    SHOW_EDGES,
    SHOW_FACES,
    NODE_TRANSPARENCY,
    EDGE_TRANSPARENCY,
    FACE_TRANSPARENCY,
    TRANSPARENCY_VALUE,
    NODES_AS_SPHERES,
    EDGES_AS_CYLINDERS
  };

  static void TestFixtureSetUp()
  {
    LogSettings::Instance().setVerbose(false);
  }

  virtual void SetUp()
  {
    LogSettings::Instance().setVerbose(false);

    if (!showField)
    {
      showField = makeModule("ShowField");
    }

    auto params = GetParam();

    showField->setStateDefaults();
    auto state = showField->get_state();
    state->setValue(Parameters::ShowNodes, std::get<SHOW_NODES>(params));
    state->setValue(Parameters::ShowEdges, std::get<SHOW_EDGES>(params));
    state->setValue(Parameters::ShowFaces, std::get<SHOW_FACES>(params));
    state->setValue(Parameters::NodeTransparency, std::get<NODE_TRANSPARENCY>(params));
    state->setValue(Parameters::EdgeTransparency, std::get<EDGE_TRANSPARENCY>(params));
    state->setValue(Parameters::FaceTransparency, std::get<FACE_TRANSPARENCY>(params));
    state->setValue(Parameters::NodeTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(Parameters::EdgeTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(Parameters::FaceTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(Parameters::NodeAsSpheres, std::get<NODES_AS_SPHERES>(params));
    state->setValue(Parameters::EdgesAsCylinders, std::get<EDGES_AS_CYLINDERS>(params));
    //...ran out of gtest options at 10 due to tuple.

    //data--to loop over
    if (vectorOfInputData.empty())
    {
      vectorOfInputData =
      {
        CreateEmptyLatVol(2, 3, 4),
        loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tri.fld"), // trisurf
        loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tet.fld"), // tetvol
        loadFieldFromFile(TestResources::rootDir() / "Fields/test_image_node.fld"),                            // imagemesh
        loadFieldFromFile(TestResources::rootDir() / "Fields/hexvol.fld"),                                     // hexvol
        loadFieldFromFile(TestResources::rootDir() / "Fields/quadsurf.fld"),                                   // quadsurf
        //loadFieldFromFile(TestResources::rootDir() / "Fields/curveMesh.fld"),                                  // curvemesh
        //loadFieldFromFile(TestResources::rootDir() / "Fields/points.fld"),                                     // point cloud
      };
    }
  }

  UseRealModuleStateFactory f;
  static ModuleHandle showField;
  static std::vector<FieldHandle> vectorOfInputData;
};

ModuleHandle ShowFieldBruteForceTest::showField;
std::vector<FieldHandle> ShowFieldBruteForceTest::vectorOfInputData;

// 768 cases in 1 second/field
// 768 tests from ConstructLatVolGeometry/ShowFieldBruteForceTest (2746 ms total)
TEST_P(ShowFieldBruteForceTest, DISABLED_BruteForceExecuteTest)
{
  LogSettings::Instance().setVerbose(false);

  //std::cout << testing::PrintToString(GetParam()) << "\n";

  for (auto& field : vectorOfInputData)
  {
    stubPortNWithThisData(showField, 0, field);
    showField->execute();
    auto geom = getDataOnThisOutputPort(showField, 0);
    ASSERT_TRUE(geom != nullptr);
  }
}

INSTANTIATE_TEST_CASE_P(
  ConstructLatVolGeometry,
  ShowFieldBruteForceTest,
  Combine(Bool(), Bool(), Bool(), Bool(), Bool(), Bool(),
  Values(0.0, 0.5, 1.0), Values(0, 1), Values(0, 1))
);






//--------------------------------------------------------------------------------------------------
//---------------- Faces ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//87ms
class ShowFieldFaceTest : public ParameterizedModuleTest<std::tuple<int, bool, double, int, bool, bool>>
{
protected:
  enum ShowFieldParams
  {
    FIELD,
    TRANSPARENCY,
    TRANSPARENCY_VALUE,
    COLORING,
    INVERT_NORMALS,
    USE_FACE_NORMALS
  };

  static void TestFixtureSetUp()
  {
    LogSettings::Instance().setVerbose(false);
  }

  virtual void SetUp()
  {
    LogSettings::Instance().setVerbose(false);

    if (!showField)
    {
      showField = makeModule("ShowField");
      showField->setStateDefaults();

      auto state = showField->get_state();
      state->setValue(Parameters::ShowFaces, true);
      state->setValue(Parameters::ShowEdges, false);
      state->setValue(Parameters::ShowNodes, false);

      colorMap = StandardColorMapFactory::create();
      stubPortNWithThisData(showField, 1, colorMap);
    }

    //data--to loop over
    if (vectorOfInputData.empty())
    {
      vectorOfInputData =
      {
        CreateEmptyLatVol(2, 3, 4),
        loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tri.fld"), // trisurf
        loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tet.fld"), // tetvol
        loadFieldFromFile(TestResources::rootDir() / "Fields/test_image_node.fld"),                            // imagemesh
        loadFieldFromFile(TestResources::rootDir() / "Fields/hexvol.fld"),                                     // hexvol
        loadFieldFromFile(TestResources::rootDir() / "Fields/quadsurf.fld"),                                   // quadsurf
        //loadFieldFromFile(TestResources::rootDir() / "Fields/curveMesh.fld"),                                  // curvemesh
        //loadFieldFromFile(TestResources::rootDir() / "Fields/points.fld"),                                     // point cloud
      };
    }

    auto params = GetParam();
    auto state = showField->get_state();

    state->setValue(Parameters::FaceTransparency, std::get<TRANSPARENCY>(params));
    state->setValue(Parameters::FaceTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(Parameters::FacesColoring, std::get<COLORING>(params));
    state->setValue(Parameters::FaceInvertNormals, std::get<INVERT_NORMALS>(params));
    state->setValue(Parameters::UseFaceNormals, std::get<USE_FACE_NORMALS>(params));
  }

  UseRealModuleStateFactory f;
  static ModuleHandle showField;
  static ColorMapHandle colorMap;
  static std::vector<FieldHandle> vectorOfInputData;
};

ModuleHandle ShowFieldFaceTest::showField;
ColorMapHandle ShowFieldFaceTest::colorMap;
std::vector<FieldHandle> ShowFieldFaceTest::vectorOfInputData;

TEST_P(ShowFieldFaceTest, FaceGenerationTest)
{
  LogSettings::Instance().setVerbose(false);

  auto params = GetParam();
  for (auto& field : vectorOfInputData)
  {
    stubPortNWithThisData(showField, 0, field);
    showField->execute();

    auto state = showField->get_state();
    auto geom = getDataOnThisOutputPort(showField, 0);

    ASSERT_TRUE(geom != nullptr);

    auto spireGeom = boost::dynamic_pointer_cast<SCIRun::Graphics::Datatypes::GeometryObjectSpire>(geom);

    int numAttributes = 2;
    if(std::get<COLORING>(params) == 1) ++numAttributes;
    ASSERT_TRUE(spireGeom->vbos().front().attributes.size() == numAttributes);

    if(std::get<TRANSPARENCY>(params))
    {
      bool transparencyFound = false;
      for(auto uniform: spireGeom->passes().front().mUniforms)
      {
        if(uniform.name == "uTransparency")
        {
          transparencyFound = true;
          ASSERT_TRUE(std::abs(uniform.data.x - std::get<TRANSPARENCY_VALUE>(params)) < 0.001);
          break;
        }
      }
      ASSERT_TRUE(transparencyFound);
    }
  }
}

INSTANTIATE_TEST_CASE_P(
  ConstructLatVolGeometry,
  ShowFieldFaceTest,
  Combine(Values(0), Bool(), Values(0.0, 0.25, 0.5, 1.0), Values(0, 1), Bool(), Bool())
);






//--------------------------------------------------------------------------------------------------
//---------------- Edges ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//2799 ms
class ShowFieldEdgeTest : public ParameterizedModuleTest<std::tuple<bool, double, int, int, double, int>>
{
protected:
  enum ShowFieldParams
  {
    TRANSPARENCY,
    TRANSPARENCY_VALUE,
    COLORING,
    USE_CYLINDERS,
    CYLINDER_RADIUS,
    CYLYNDER_RESOLUTION
  };

  static void TestFixtureSetUp()
  {
    LogSettings::Instance().setVerbose(false);
  }

  virtual void SetUp()
  {
    LogSettings::Instance().setVerbose(false);

    if (!showField)
    {
      showField = makeModule("ShowField");
      showField->setStateDefaults();

      auto state = showField->get_state();
      state->setValue(Parameters::ShowFaces, false);
      state->setValue(Parameters::ShowEdges, true);
      state->setValue(Parameters::ShowNodes, false);

      colorMap = StandardColorMapFactory::create();
      stubPortNWithThisData(showField, 1, colorMap);
    }

    //data--to loop over
    if (vectorOfInputData.empty())
    {
      vectorOfInputData =
      {
        CreateEmptyLatVol(2, 3, 4),
        loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tri.fld"), // trisurf
        loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tet.fld"), // tetvol
        loadFieldFromFile(TestResources::rootDir() / "Fields/test_image_node.fld"),                            // imagemesh
        loadFieldFromFile(TestResources::rootDir() / "Fields/hexvol.fld"),                                     // hexvol
        loadFieldFromFile(TestResources::rootDir() / "Fields/quadsurf.fld"),                                   // quadsurf
        loadFieldFromFile(TestResources::rootDir() / "Fields/curveMesh.fld"),                                  // curvemesh
        //loadFieldFromFile(TestResources::rootDir() / "Fields/points.fld"),                                     // point cloud
      };
    }

    auto params = GetParam();
    auto state = showField->get_state();

    state->setValue(Parameters::EdgeTransparency, std::get<TRANSPARENCY>(params));
    state->setValue(Parameters::EdgeTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(Parameters::EdgesColoring, std::get<COLORING>(params));
    state->setValue(Parameters::EdgesAsCylinders, std::get<USE_CYLINDERS>(params));
    state->setValue(Parameters::CylinderRadius, std::get<CYLINDER_RADIUS>(params));
    state->setValue(Parameters::CylinderResolution, std::get<CYLYNDER_RESOLUTION>(params));
  }

  UseRealModuleStateFactory f;
  static ModuleHandle showField;
  static ColorMapHandle colorMap;
  static std::vector<FieldHandle> vectorOfInputData;
};

ModuleHandle ShowFieldEdgeTest::showField;
ColorMapHandle ShowFieldEdgeTest::colorMap;
std::vector<FieldHandle> ShowFieldEdgeTest::vectorOfInputData;

TEST_P(ShowFieldEdgeTest, EdgeGenerationTest)
{
  LogSettings::Instance().setVerbose(false);

  auto params = GetParam();
  for (auto& field : vectorOfInputData)
  {
    stubPortNWithThisData(showField, 0, field);
    showField->execute();

    auto state = showField->get_state();
    auto geom = getDataOnThisOutputPort(showField, 0);

    ASSERT_TRUE(geom != nullptr);

    auto spireGeom = boost::dynamic_pointer_cast<SCIRun::Graphics::Datatypes::GeometryObjectSpire>(geom);

    int numAttributes = 1;
    if(std::get<COLORING>(params) == 1) ++numAttributes;
    if(std::get<USE_CYLINDERS>(params) == 1) ++numAttributes;
    ASSERT_TRUE(spireGeom->vbos().front().attributes.size() == numAttributes);

    if(std::get<TRANSPARENCY>(params))
    {
      bool transparencyFound = false;
      for(auto uniform: spireGeom->passes().front().mUniforms)
      {
        if(uniform.name == "uTransparency")
        {
          transparencyFound = true;
          ASSERT_TRUE(std::abs(uniform.data.x - std::get<TRANSPARENCY_VALUE>(params)) < 0.001);
          break;
        }
      }
      ASSERT_TRUE(transparencyFound);
    }
  }
}

INSTANTIATE_TEST_CASE_P(
  ConstructLatVolGeometry,
  ShowFieldEdgeTest,
  Combine(Bool(), Values(0.0, 0.25, 0.5, 1.0), Values(0, 1), Values(0, 1), Values(0.05), Values(5))
);






//--------------------------------------------------------------------------------------------------
//---------------- Nodes ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//717 ms
class ShowFieldNodeTest : public ParameterizedModuleTest<std::tuple<bool, double, int, int, double, int>>
{
protected:
  enum ShowFieldParams
  {
    TRANSPARENCY,
    TRANSPARENCY_VALUE,
    COLORING,
    USE_SPHERES,
    SPHERE_SCALE,
    SPHERE_RESOLUTION
  };

  static void TestFixtureSetUp()
  {
    LogSettings::Instance().setVerbose(false);
  }

  virtual void SetUp()
  {
    LogSettings::Instance().setVerbose(false);

    if (!showField)
    {
      showField = makeModule("ShowField");
      showField->setStateDefaults();

      auto state = showField->get_state();
      state->setValue(Parameters::ShowFaces, false);
      state->setValue(Parameters::ShowEdges, false);
      state->setValue(Parameters::ShowNodes, true);

      colorMap = StandardColorMapFactory::create();
      stubPortNWithThisData(showField, 1, colorMap);
    }

    //data--to loop over
    if (vectorOfInputData.empty())
    {
      vectorOfInputData =
      {
        CreateEmptyLatVol(2, 3, 4),
        loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tri.fld"), // trisurf
        loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tet.fld"), // tetvol
        loadFieldFromFile(TestResources::rootDir() / "Fields/test_image_node.fld"),                            // imagemesh
        loadFieldFromFile(TestResources::rootDir() / "Fields/hexvol.fld"),                                     // hexvol
        loadFieldFromFile(TestResources::rootDir() / "Fields/quadsurf.fld"),                                   // quadsurf
        loadFieldFromFile(TestResources::rootDir() / "Fields/curveMesh.fld"),                                  // curvemesh
        //loadFieldFromFile(TestResources::rootDir() / "Fields/points.fld"),                                     // point cloud
      };
    }

    auto params = GetParam();
    auto state = showField->get_state();

    state->setValue(Parameters::NodeTransparency, std::get<TRANSPARENCY>(params));
    state->setValue(Parameters::NodeTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(Parameters::NodesColoring, std::get<COLORING>(params));
    state->setValue(Parameters::NodeAsSpheres, std::get<USE_SPHERES>(params));
    state->setValue(Parameters::SphereScaleValue, std::get<SPHERE_SCALE>(params));
    state->setValue(Parameters::SphereResolution, std::get<SPHERE_RESOLUTION>(params));
  }

  UseRealModuleStateFactory f;
  static ModuleHandle showField;
  static ColorMapHandle colorMap;
  static std::vector<FieldHandle> vectorOfInputData;
};

ModuleHandle ShowFieldNodeTest::showField;
ColorMapHandle ShowFieldNodeTest::colorMap;
std::vector<FieldHandle> ShowFieldNodeTest::vectorOfInputData;

TEST_P(ShowFieldNodeTest, NodeGenerationTest)
{
  LogSettings::Instance().setVerbose(false);

  auto params = GetParam();
  for (auto& field : vectorOfInputData)
  {
    stubPortNWithThisData(showField, 0, field);
    showField->execute();

    auto state = showField->get_state();
    auto geom = getDataOnThisOutputPort(showField, 0);

    ASSERT_TRUE(geom != nullptr);

    auto spireGeom = boost::dynamic_pointer_cast<SCIRun::Graphics::Datatypes::GeometryObjectSpire>(geom);

    int numAttributes = 1;
    if(std::get<COLORING>(params) == 1) ++numAttributes;
    if(std::get<USE_SPHERES>(params) == 1) ++numAttributes;
    ASSERT_TRUE(spireGeom->vbos().front().attributes.size() == numAttributes);

    if(std::get<TRANSPARENCY>(params))
    {
      bool transparencyFound = false;
      for(auto uniform: spireGeom->passes().front().mUniforms)
      {
        if(uniform.name == "uTransparency")
        {
          transparencyFound = true;
          ASSERT_TRUE(std::abs(uniform.data.x - std::get<TRANSPARENCY_VALUE>(params)) < 0.001);
          break;
        }
      }
      ASSERT_TRUE(transparencyFound);
    }
  }
}

INSTANTIATE_TEST_CASE_P(
  ConstructLatVolGeometry,
  ShowFieldNodeTest,
  Combine(Bool(), Values(0.0, 0.25, 0.5, 1.0), Values(0, 1), Values(0, 1), Values(0.05), Values(5))
);
