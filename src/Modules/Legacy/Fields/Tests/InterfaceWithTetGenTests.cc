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

/// Tests for the InterfaceWithTetGen module (issue #2547: upgrade Tetgen to 1.6
/// and test it). These only build/run when WITH_TETGEN is enabled, since without
/// it the module is a stub that just errors out.
///
/// Two layers of coverage:
///   1. Option-string tests  - exercise the pure SCIRun-side switch translation
///      (InterfaceWithTetGenInput::fillCommandOptions). Deterministic, no TetGen
///      call. This is the glue most likely to regress independent of TetGen.
///   2. End-to-end mesh tests - drive the real TetGen 1.6 library through the
///      module (surface in -> tet mesh out). These guard the 1.6 behavior-based
///      API port (tetgenbehavior::parse_commandline + the 5-arg tetrahedralize)
///      and the crash/hang history the module has (see #933, #1734, #1149,
///      #1125, #1538).

#include <gtest/gtest.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Dataflow/Network/ModuleExceptions.h>
#include <Modules/Legacy/Fields/InterfaceWithTetGen.h>
#include <Modules/Legacy/Fields/InterfaceWithTetGenImpl.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Testing/Utils/SCIRunFieldSamples.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Geometry;

// ---------------------------------------------------------------------------
// 1. Option-string translation (pure, deterministic, no TetGen invocation)
// ---------------------------------------------------------------------------

TEST(InterfaceWithTetGenOptionsTests, DefaultSwitchesMatchModuleDefaults)
{
  // Default-constructed input mirrors InterfaceWithTetGen::setStateDefaults():
  // piecewise (-p), suppress split (-Y), zero index (-z), quality (-q),
  // assign region attributes (-A).
  InterfaceWithTetGenInput input;

  EXPECT_EQ("pYzqA", input.fillCommandOptions(false));
  // Adding interior points prepends the -i switch.
  EXPECT_EQ("ipYzqA", input.fillCommandOptions(true));
}

TEST(InterfaceWithTetGenOptionsTests, AllSwitchesComposeInTetGenOrder)
{
  InterfaceWithTetGenInput input;
  input.piecewiseFlag_ = true;             // p
  input.suppressSplitFlag_ = true;
  input.setSplitFlag_ = true;              // YY
  input.qualityFlag_ = true;
  input.setRatioFlag_ = true;
  input.minRadius_ = 1.2;                  // q1.2
  input.volConstraintFlag_ = true;
  input.setMaxVolConstraintFlag_ = true;
  input.maxVolConstraint_ = 0.05;          // a0.05
  input.detectIntersectionsFlag_ = true;   // d
  input.assignFlag_ = true;
  input.setNonzeroAttributeFlag_ = true;   // AA
  input.moreSwitches_ = "V";               // appended verbatim

  // z is always emitted (SCIRun requires zero-based indexing).
  EXPECT_EQ("pYYzq1.2a0.05dAAV", input.fillCommandOptions(false));
}

TEST(InterfaceWithTetGenOptionsTests, AllFlagsOffLeavesOnlyZeroIndexing)
{
  InterfaceWithTetGenInput input;
  input.piecewiseFlag_ = false;
  input.suppressSplitFlag_ = false;
  input.qualityFlag_ = false;
  input.volConstraintFlag_ = false;
  input.detectIntersectionsFlag_ = false;
  input.assignFlag_ = false;
  input.moreSwitches_.clear();

  EXPECT_EQ("z", input.fillCommandOptions(false));
}

// ---------------------------------------------------------------------------
// 2. End-to-end mesh generation through the real TetGen 1.6 library
// ---------------------------------------------------------------------------

class InterfaceWithTetGenModuleTests : public ModuleTest
{
protected:
  // Assert the module output is a non-empty, well-formed TetVol field.
  static void expectValidTetVol(const DatatypeHandle& output, size_type minNodes)
  {
    ASSERT_TRUE(output != nullptr) << "TetGen produced no output field";
    auto field = std::dynamic_pointer_cast<Field>(output);
    ASSERT_TRUE(field != nullptr);

    FieldInformation info(field);
    EXPECT_TRUE(info.is_tetvolmesh());

    auto vmesh = field->vmesh();
    EXPECT_GE(vmesh->num_nodes(), minNodes) << "output dropped input vertices";
    EXPECT_GT(vmesh->num_elems(), 0) << "TetGen returned an empty tetrahedralization";
  }
};

TEST_F(InterfaceWithTetGenModuleTests, ThrowsForNullMainInput)
{
  auto tg = makeModule("InterfaceWithTetGen");
  FieldHandle nullField;
  stubPortNWithThisData(tg, 0, nullField);
  EXPECT_THROW(tg->execute(), NullHandleOnPortException);
}

// Build a valid, closed, manifold tetrahedron surface (4 nodes, 4 facets, every
// edge shared by exactly two triangles). Note: we deliberately do NOT use the
// TetrahedronTriSurf* sample helpers here - triTetrahedronGeometry() in
// SCIRunFieldSamples.cc has a copy/paste bug (adds vdata2 twice, dropping the
// 1-2-3 facet), producing an open surface that TetGen cannot fill.
static FieldHandle makeClosedTetrahedronSurface()
{
  FieldInformation fi(mesh_info_type::TRISURFMESH_E,
    databasis_info_type::LINEARDATA_E, data_info_type::DOUBLE_E);
  FieldHandle field = CreateField(fi);
  auto vmesh = field->vmesh();
  vmesh->add_point(Point(0.0, 0.0, 0.0));
  vmesh->add_point(Point(1.0, 0.0, 0.0));
  vmesh->add_point(Point(0.0, 1.0, 0.0));
  vmesh->add_point(Point(0.0, 0.0, 1.0));

  const int faces[4][3] = { {0, 2, 1}, {0, 1, 3}, {0, 3, 2}, {1, 2, 3} };
  VMesh::Node::array_type tri;
  tri.resize(3);
  for (const auto& f : faces)
  {
    tri[0] = f[0];
    tri[1] = f[1];
    tri[2] = f[2];
    vmesh->add_elem(tri);
  }
  field->vfield()->resize_values();
  return field;
}

TEST_F(InterfaceWithTetGenModuleTests, TetrahedralizesSingleTetrahedronSurface)
{
  // Simplest possible closed manifold: 4 nodes, 4 triangular facets.
  auto tg = makeModule("InterfaceWithTetGen");
  stubPortNWithThisData(tg, 0, makeClosedTetrahedronSurface());

  ASSERT_NO_THROW(tg->execute());
  expectValidTetVol(getDataOnThisOutputPort(tg, 0), 4);
}

TEST_F(InterfaceWithTetGenModuleTests, TetrahedralizesClosedCubeSurface)
{
  // Closed cube boundary: 8 nodes, 12 triangular facets -> filled volume mesh.
  auto tg = makeModule("InterfaceWithTetGen");
  stubPortNWithThisData(tg, 0, CubeTriSurfLinearBasis(data_info_type::DOUBLE_E));

  ASSERT_NO_THROW(tg->execute());
  auto output = getDataOnThisOutputPort(tg, 0);
  expectValidTetVol(output, 8);

  auto field = std::dynamic_pointer_cast<Field>(output);
  ASSERT_TRUE(field != nullptr);
  auto vmesh = field->vmesh();

  // Characterization of the TetGen 1.6 default pipeline (-pYzqA). TetGen 1.6
  // changed -p/-Y boundary recovery to insert fewer Steiner points than 1.5
  // (see the vendored TetGen CHANGELOG, "Version 1.6.0"), so the exact counts
  // here differ from what 1.5 produced. Recorded (not hard-pinned) because the
  // count can vary slightly by platform/predicate build; a silent revert to 1.5
  // would move these numbers.
  RecordProperty("cubeTetNodeCount", static_cast<int>(vmesh->num_nodes()));
  RecordProperty("cubeTetElemCount", static_cast<int>(vmesh->num_elems()));
}

TEST_F(InterfaceWithTetGenModuleTests, HonorsAddedInteriorPointsPort)
{
  // Feeding the optional Points port routes through the -i switch and the
  // 4th (addin) argument of the 1.6 tetrahedralize() signature. Guards that the
  // added-points path still produces a valid mesh rather than crashing.
  auto tg = makeModule("InterfaceWithTetGen");
  stubPortNWithThisData(tg, 0, CubeTriSurfLinearBasis(data_info_type::DOUBLE_E));

  FieldInformation pcfi(mesh_info_type::POINTCLOUDMESH_E,
    databasis_info_type::CONSTANTDATA_E, data_info_type::DOUBLE_E);
  FieldHandle points = CreateField(pcfi);
  points->vmesh()->add_point(Point(0.5, 0.5, -0.5)); // strictly inside the cube
  points->vfield()->resize_values();
  stubPortNWithThisData(tg, 1, points);

  ASSERT_NO_THROW(tg->execute());
  expectValidTetVol(getDataOnThisOutputPort(tg, 0), 8);
}
