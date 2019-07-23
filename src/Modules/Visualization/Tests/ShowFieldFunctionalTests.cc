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
    state->setValue(ShowField::ShowNodes, std::get<SHOW_NODES>(params));
    state->setValue(ShowField::ShowEdges, std::get<SHOW_EDGES>(params));
    state->setValue(ShowField::ShowFaces, std::get<SHOW_FACES>(params));
    state->setValue(ShowField::NodeTransparency, std::get<NODE_TRANSPARENCY>(params));
    state->setValue(ShowField::EdgeTransparency, std::get<EDGE_TRANSPARENCY>(params));
    state->setValue(ShowField::FaceTransparency, std::get<FACE_TRANSPARENCY>(params));
    state->setValue(ShowField::NodeTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(ShowField::EdgeTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(ShowField::FaceTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(ShowField::NodeAsSpheres, std::get<NODES_AS_SPHERES>(params));
    state->setValue(ShowField::EdgesAsCylinders, std::get<EDGES_AS_CYLINDERS>(params));
    //...ran out of gtest options at 10 due to tuple.

    //data--to loop over
    if (vectorOfInputData.empty())
    {
      vectorOfInputData =
      {
        CreateEmptyLatVol(2, 2, 2),
        CreateEmptyLatVol(3, 4, 5),
        // trisurf
        // quadsurf
        // hexvol
        // curvemesh
        // imagemesh
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
TEST_P(ShowFieldBruteForceTest, BruteForceExecuteTest)
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
//57ms
class ShowFieldFaceTest : public ParameterizedModuleTest<std::tuple<FieldHandle, bool, double, int, bool, bool>>
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
      state->setValue(ShowField::ShowFaces, true);
      state->setValue(ShowField::ShowEdges, false);
      state->setValue(ShowField::ShowNodes, false);
    }

    //data--to loop over
    //if (vectorOfInputData.empty())
    //{
    //  vectorOfInputData =
    //  {
    //    CreateEmptyLatVol(2, 2, 2),
    //    CreateEmptyLatVol(3, 4, 5),
    //    // trisurf
    //    // quadsurf
    //    // hexvol
    //    // curvemesh
    //    // imagemesh
    //  };
    //}

    auto params = GetParam();
    auto state = showField->get_state();

    state->setValue(ShowField::FaceTransparency, std::get<TRANSPARENCY>(params));
    state->setValue(ShowField::FaceTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(ShowField::FacesColoring, std::get<COLORING>(params));
    state->setValue(ShowField::FaceInvertNormals, std::get<INVERT_NORMALS>(params));
    state->setValue(ShowField::UseFaceNormals, std::get<USE_FACE_NORMALS>(params));
  }

  UseRealModuleStateFactory f;
  static ModuleHandle showField;
  //static std::vector<FieldHandle> vectorOfInputData;
};

ModuleHandle ShowFieldFaceTest::showField;
//std::vector<FieldHandle> ShowFieldFaceTest::vectorOfInputData;

TEST_P(ShowFieldFaceTest, FaceGenerationTest)
{
  LogSettings::Instance().setVerbose(false);

  auto params = GetParam();
  auto field = std::get<FIELD>(params);
  auto colorMap = StandardColorMapFactory::create();

  stubPortNWithThisData(showField, 0, field);
  stubPortNWithThisData(showField, 1, colorMap);
  showField->execute();

  auto state = showField->get_state();
  auto geom = getDataOnThisOutputPort(showField, 0);

  ASSERT_TRUE(geom != nullptr);

  auto spireGeom = boost::dynamic_pointer_cast<SCIRun::Graphics::Datatypes::GeometryObjectSpire>(geom);

  if(std::get<COLORING>(params) == 0)
  {
    ASSERT_TRUE(spireGeom->vbos().front().attributes.size() == 2);
  }
  else if(std::get<COLORING>(params) == 1)
  {
    ASSERT_TRUE(spireGeom->vbos().front().attributes.size() == 3);
  }
}

INSTANTIATE_TEST_CASE_P(
  ConstructLatVolGeometry,
  ShowFieldFaceTest,
  Combine(Values(
    CreateEmptyLatVol(2, 2, 2),
    CreateEmptyLatVol(3, 4, 5),
    loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tri.fld"), // trisurf
    loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tet.fld") // tetvol
    //loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tet.fld")// quadsurf
    // hexvol
    // curvemesh
    // imagemesh
    // point cloud
  ), Bool(), Values(0.0, 0.25, 0.5, 1.0), Values(0, 1), Bool(), Bool())
);






//--------------------------------------------------------------------------------------------------
//---------------- Edges ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

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
      state->setValue(ShowField::ShowFaces, false);
      state->setValue(ShowField::ShowEdges, true);
      state->setValue(ShowField::ShowNodes, false);
    }

    //data--to loop over
    if (vectorOfInputData.empty())
    {
      vectorOfInputData =
      {
        CreateEmptyLatVol(2, 2, 2),
        CreateEmptyLatVol(3, 4, 5),
        // trisurf
        // quadsurf
        // hexvol
        // curvemesh
        // imagemesh
      };
    }

    auto params = GetParam();
    auto state = showField->get_state();

    state->setValue(ShowField::EdgeTransparency, std::get<TRANSPARENCY>(params));
    state->setValue(ShowField::EdgeTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(ShowField::EdgesColoring, std::get<COLORING>(params));
    state->setValue(ShowField::EdgesAsCylinders, std::get<USE_CYLINDERS>(params));
    state->setValue(ShowField::CylinderRadius, std::get<CYLINDER_RADIUS>(params));
    state->setValue(ShowField::CylinderResolution, std::get<CYLYNDER_RESOLUTION>(params));
  }

  UseRealModuleStateFactory f;
  static ModuleHandle showField;
  static std::vector<FieldHandle> vectorOfInputData;
};

ModuleHandle ShowFieldEdgeTest::showField;
std::vector<FieldHandle> ShowFieldEdgeTest::vectorOfInputData;

TEST_P(ShowFieldEdgeTest, EdgeGenerationTest)
{
  LogSettings::Instance().setVerbose(false);

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
  ShowFieldEdgeTest,
  Combine(Bool(), Values(0.0, 0.25, 0.5, 1.0), Values(0, 1), Values(0, 1), Values(0.05, 0.01), Values(5, 10))
);






//--------------------------------------------------------------------------------------------------
//---------------- Nodes ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

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
      state->setValue(ShowField::ShowFaces, false);
      state->setValue(ShowField::ShowEdges, false);
      state->setValue(ShowField::ShowNodes, true);
    }

    //data--to loop over
    if (vectorOfInputData.empty())
    {
      vectorOfInputData =
      {
        CreateEmptyLatVol(2, 2, 2),
        CreateEmptyLatVol(3, 4, 5),
        // trisurf
        // quadsurf
        // hexvol
        // curvemesh
        // imagemesh
      };
    }

    auto params = GetParam();
    auto state = showField->get_state();

    state->setValue(ShowField::NodeTransparency, std::get<TRANSPARENCY>(params));
    state->setValue(ShowField::NodeTransparencyValue, std::get<TRANSPARENCY_VALUE>(params));
    state->setValue(ShowField::NodesColoring, std::get<COLORING>(params));
    state->setValue(ShowField::NodeAsSpheres, std::get<USE_SPHERES>(params));
    state->setValue(ShowField::SphereScaleValue, std::get<SPHERE_SCALE>(params));
    state->setValue(ShowField::SphereResolution, std::get<SPHERE_RESOLUTION>(params));
  }

  UseRealModuleStateFactory f;
  static ModuleHandle showField;
  static std::vector<FieldHandle> vectorOfInputData;
};

ModuleHandle ShowFieldNodeTest::showField;
std::vector<FieldHandle> ShowFieldNodeTest::vectorOfInputData;

TEST_P(ShowFieldNodeTest, NodeGenerationTest)
{
  LogSettings::Instance().setVerbose(false);

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
  ShowFieldNodeTest,
  Combine(Bool(), Values(0.0, 0.25, 0.5, 1.0), Values(0, 1), Values(0, 1), Values(0.05, 0.01), Values(5, 10))
);
