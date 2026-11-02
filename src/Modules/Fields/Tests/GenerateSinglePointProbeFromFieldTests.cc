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


#include <Modules/Legacy/Fields/GenerateSinglePointProbeFromField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Testing/Utils/SCIRunFieldSamples.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;

class GenerateSinglePointProbeFromFieldTests : public ModuleTest
{
protected:
  Point probeAt(const Point& location, bool keepOnMesh)
  {
    auto probe = makeModule("GenerateSinglePointProbeFromField");
    // Triangle (0,0,0) (1,0,0) (0.5,1,0).
    stubPortNWithThisData(probe, 0, TriangleTriSurfLinearBasis(data_info_type::DOUBLE_E));
    auto state = probe->get_state();
    state->setValue(Parameters::XLocation, location.x());
    state->setValue(Parameters::YLocation, location.y());
    state->setValue(Parameters::ZLocation, location.z());
    state->setValue(Parameters::KeepOnMesh, keepOnMesh);

    probe->execute();

    auto output = std::dynamic_pointer_cast<Field>(getDataOnThisOutputPort(probe, 1));
    EXPECT_TRUE(output != nullptr);
    Point p;
    output->vmesh()->get_point(p, VMesh::Node::index_type(0));
    EXPECT_DOUBLE_EQ(p.x(), state->getValue(Parameters::XLocation).toDouble());
    EXPECT_DOUBLE_EQ(p.y(), state->getValue(Parameters::YLocation).toDouble());
    EXPECT_DOUBLE_EQ(p.z(), state->getValue(Parameters::ZLocation).toDouble());
    return p;
  }

  UseRealModuleStateFactory realState_;
};

TEST_F(GenerateSinglePointProbeFromFieldTests, FreeProbeStaysOffSurface)
{
  auto p = probeAt(Point(0.5, 0.3, 2.0), false);
  EXPECT_NEAR(2.0, p.z(), 1e-12);
}

TEST_F(GenerateSinglePointProbeFromFieldTests, KeepOnMeshProjectsOntoSurface)
{
  auto p = probeAt(Point(0.5, 0.3, 2.0), true);
  EXPECT_NEAR(0.5, p.x(), 1e-9);
  EXPECT_NEAR(0.3, p.y(), 1e-9);
  EXPECT_NEAR(0.0, p.z(), 1e-9);
}

TEST_F(GenerateSinglePointProbeFromFieldTests, KeepOnMeshClampsToNearestEdge)
{
  auto p = probeAt(Point(0.5, -1.0, 0.5), true);
  EXPECT_NEAR(0.5, p.x(), 1e-9);
  EXPECT_NEAR(0.0, p.y(), 1e-9);
  EXPECT_NEAR(0.0, p.z(), 1e-9);
}
