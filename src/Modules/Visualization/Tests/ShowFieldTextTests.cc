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
#include <Testing/Utils/SCIRunFieldSamples.h>
#include <Modules/Visualization/ShowField.h>
#include <Core/Algorithms/Visualization/FieldTextLabels.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Dataflow::Networks;

namespace
{
  FieldTextLabelOptions nothingOn()
  {
    FieldTextLabelOptions o;
    o.showDataValues = false;
    return o;
  }

  std::vector<std::string> texts(const FieldTextLabels& labels)
  {
    std::vector<std::string> t;
    for (const auto& l : labels.labels)
      t.push_back(l.text);
    return t;
  }
}

TEST(FieldTextLabelsTest, NodeIndicesSitOnTheNodes)
{
  auto field = TetrahedronTriSurfLinearBasis(data_info_type::DOUBLE_E);
  auto o = nothingOn();
  o.showNodeIndices = true;

  const auto labels = buildFieldTextLabels(field, nullptr, o);

  EXPECT_EQ((std::vector<std::string>{ "0", "1", "2", "3" }), texts(labels));
  EXPECT_EQ(size_t(0), labels.omitted);
  for (VMesh::Node::index_type i = 0; i < 4; ++i)
  {
    Point p;
    field->vmesh()->get_center(p, i);
    EXPECT_EQ(p, labels.labels[i].anchor);
    EXPECT_FALSE(labels.labels[i].normal);
    EXPECT_FALSE(labels.labels[i].color);
  }
}

TEST(FieldTextLabelsTest, DataValuesFollowTheBasisAndPrecision)
{
  auto onNodes = TetrahedronTriSurfLinearBasis(data_info_type::DOUBLE_E);
  onNodes->vfield()->set_value(3.14159, VMesh::Node::index_type(0));
  FieldTextLabelOptions o;
  o.precision = 3;
  auto labels = buildFieldTextLabels(onNodes, nullptr, o);
  ASSERT_EQ(size_t(4), labels.labels.size());
  EXPECT_EQ("3.14", labels.labels[0].text);

  o.precision = 5;
  EXPECT_EQ("3.1416", buildFieldTextLabels(onNodes, nullptr, o).labels[0].text);

  auto onElems = TetrahedronTriSurfConstantBasis(data_info_type::DOUBLE_E);
  onElems->vfield()->set_value(-2.5, VMesh::Elem::index_type(1));
  labels = buildFieldTextLabels(onElems, nullptr, o);
  ASSERT_EQ(static_cast<size_t>(onElems->vmesh()->num_elems()), labels.labels.size());
  EXPECT_EQ("-2.5", labels.labels[1].text);
  Point center;
  onElems->vmesh()->get_center(center, VMesh::Elem::index_type(1));
  EXPECT_EQ(center, labels.labels[1].anchor);
}

TEST(FieldTextLabelsTest, VectorDataPrintsAllComponents)
{
  auto field = TetrahedronTriSurfLinearBasis(data_info_type::VECTOR_E);
  field->vfield()->set_value(Vector(1, 2.5, -3), VMesh::Node::index_type(2));
  EXPECT_EQ("[1 2.5 -3]", buildFieldTextLabels(field, nullptr, {}).labels[2].text);
}

TEST(FieldTextLabelsTest, ElementIndicesDependOnMeshDimension)
{
  auto volume = CubeTetVolLinearBasis(data_info_type::DOUBLE_E);
  auto o = nothingOn();
  o.showEdgeIndices = true;
  // Building the labels synchronizes the edges, so count them afterwards.
  const auto edgeLabels = buildFieldTextLabels(volume, nullptr, o);
  EXPECT_EQ(static_cast<size_t>(volume->vmesh()->num_edges()), edgeLabels.labels.size());
  EXPECT_EQ(size_t(19), edgeLabels.labels.size());

  o = nothingOn();
  o.showFaceIndices = true;
  // Six tets in a cube: twelve boundary triangles plus six shared ones.
  EXPECT_EQ(size_t(18), buildFieldTextLabels(volume, nullptr, o).labels.size());

  o = nothingOn();
  o.showCellIndices = true;
  EXPECT_EQ(size_t(6), buildFieldTextLabels(volume, nullptr, o).labels.size());

  auto surface = TetrahedronTriSurfLinearBasis(data_info_type::DOUBLE_E);
  EXPECT_TRUE(buildFieldTextLabels(surface, nullptr, o).labels.empty());
}

TEST(FieldTextLabelsTest, LocationsReplaceIndices)
{
  auto field = TetrahedronTriSurfLinearBasis(data_info_type::DOUBLE_E);
  auto o = nothingOn();
  o.showNodeIndices = true;
  o.renderAsLocation = true;

  const auto labels = buildFieldTextLabels(field, nullptr, o);

  for (const auto& l : labels.labels)
  {
    std::ostringstream expected;
    expected.precision(o.precision);
    expected << l.anchor;
    EXPECT_EQ(expected.str(), l.text);
  }
}

TEST(FieldTextLabelsTest, LabelLimitCountsWhatItDrops)
{
  auto field = CubeTetVolLinearBasis(data_info_type::DOUBLE_E);
  FieldTextLabelOptions o;
  o.showNodeIndices = true;
  o.maxLabels = 5;

  const auto labels = buildFieldTextLabels(field, nullptr, o);

  EXPECT_EQ(size_t(5), labels.labels.size());
  // Eight data values plus eight node indices.
  EXPECT_EQ(size_t(11), labels.omitted);
}

TEST(FieldTextLabelsTest, ColorMapOnlyColorsLabelsThatHaveData)
{
  auto field = TetrahedronTriSurfConstantBasis(data_info_type::DOUBLE_E);
  auto colorMap = StandardColorMapFactory::create();
  FieldTextLabelOptions o;
  o.showNodeIndices = true;
  o.useColorMap = true;

  const auto labels = buildFieldTextLabels(field, colorMap, o);

  const auto numElems = static_cast<size_t>(field->vmesh()->num_elems());
  ASSERT_EQ(numElems + 4, labels.labels.size());
  // Data is on the faces, so the face values are colored and the node indices are not.
  for (size_t i = 0; i < labels.labels.size(); ++i)
    EXPECT_EQ(i < numElems, labels.labels[i].color.has_value()) << "label " << i;

  // No color map connected: fall back to the default color.
  for (const auto& l : buildFieldTextLabels(field, nullptr, o).labels)
    EXPECT_FALSE(l.color);
}

TEST(FieldTextLabelsTest, CullingAttachesNodeNormalsOnSurfaces)
{
  auto surface = TetrahedronTriSurfLinearBasis(data_info_type::DOUBLE_E);
  FieldTextLabelOptions o;
  o.cullBackfacing = true;
  for (const auto& l : buildFieldTextLabels(surface, nullptr, o).labels)
    EXPECT_TRUE(l.normal);

  // A volume mesh has no normals to cull against.
  auto volume = CubeTetVolLinearBasis(data_info_type::DOUBLE_E);
  for (const auto& l : buildFieldTextLabels(volume, nullptr, o).labels)
    EXPECT_FALSE(l.normal);
}

class ShowFieldTextTest : public ModuleTest
{
protected:
  UseRealModuleStateFactory f_;
};

TEST_F(ShowFieldTextTest, ExecutesWithEveryTextOptionOn)
{
  using namespace SCIRun::Core::Algorithms::Visualization::Parameters;
  auto showField = makeModule("ShowField");
  showField->setStateDefaults();
  auto state = showField->get_state();
  for (const auto& name : { ShowText, ShowDataValues, ShowNodeIndices, ShowEdgeIndices, ShowFaceIndices,
    ShowCellIndices, CullBackfacingText, TextAlwaysVisible })
    state->setValue(name, true);
  stubPortNWithThisData(showField, 0, CubeTetVolLinearBasis(data_info_type::DOUBLE_E));

  EXPECT_NO_THROW(showField->execute());
}
