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



#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>

#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Testing/Utils/SCIRunFieldSamples.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Visualization/ConvertGeometryToVtkAlgo.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/VTK/VtkGeometry.h>
#include <Core/Logging/Log.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Modules/Visualization/ShowField.h>

#include <set>
#include <tuple>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Graphics::Datatypes;

namespace
{
  struct FixedIdGenerator : GeometryIDGenerator
  {
    std::string generateGeometryID(const std::string& tag) const override { return tag; }
  };

  SpireSubPass makePass(const std::vector<SpireVBO::AttributeData>& attribs, const std::vector<float>& vertices,
    const std::vector<uint32_t>& indices, SpireIBO::PRIMITIVE prim, const std::string& program = "Shaders/Flat")
  {
    auto vboData = std::make_shared<spire::VarBuffer>(vertices.size() * sizeof(float));
    for (auto v : vertices) vboData->write(v);
    auto iboData = std::make_shared<spire::VarBuffer>(indices.size() * sizeof(uint32_t));
    for (auto i : indices) iboData->write(i);

    SpireVBO vbo("vbo", attribs, vboData, 0, Geometry::BBox(), true);
    SpireIBO ibo("ibo", prim, sizeof(uint32_t), iboData);
    RenderState state;
    return SpireSubPass("pass", "vbo", "ibo", program, ColorScheme::COLOR_UNIFORM, state,
      RenderType::RENDER_VBO_IBO, vbo, ibo, SpireText());
  }

  GeometryHandle wrap(const SpireSubPass& pass)
  {
    static FixedIdGenerator gen;
    auto geom = std::make_shared<GeometryObjectSpire>(gen, "test", false);
    geom->passes().push_back(pass);
    return geom;
  }

  VtkGeometryObjectHandle convert(GeometryBaseHandle geom)
  {
    ConvertGeometryToVtkAlgo algo;
    AlgorithmInput input;
    input[ConvertGeometryToVtkAlgo::Geometry] = geom;
    return std::dynamic_pointer_cast<VtkGeometryObject>(algo.run(input)[ConvertGeometryToVtkAlgo::VtkGeometry]);
  }

  std::vector<VtkGeometryObjectHandle> leaves(VtkGeometryObjectHandle obj)
  {
    if (auto composite = std::dynamic_pointer_cast<CompositeVtkGeometryObject>(obj))
      return composite->objects();
    return { obj };
  }

  vtkPolyData* poly(const VtkGeometryObjectHandle& obj)
  {
    return vtkPolyData::SafeDownCast(obj->dataObject);
  }
}

TEST(ConvertGeometryToVtkAlgoTests, DecodesInterleavedTrianglesWithVertexColors)
{
  const std::vector<SpireVBO::AttributeData> attribs{ {"aPos", 3 * sizeof(float)}, {"aColor", 4 * sizeof(float)} };
  const std::vector<float> verts{
    0, 0, 0,  1, 0, 0, 1,
    1, 0, 0,  0, 1, 0, 1,
    0, 1, 0,  0, 0, 1, 1,
    1, 1, 0,  1, 1, 1, 1 };
  auto obj = convert(wrap(makePass(attribs, verts, { 0, 1, 2, 2, 1, 3 }, SpireIBO::PRIMITIVE::TRIANGLES)));

  ASSERT_TRUE(obj);
  EXPECT_EQ(GeometryType::TRI_SURFACE, obj->type);
  auto pd = poly(obj);
  ASSERT_TRUE(pd);
  EXPECT_EQ(4, pd->GetNumberOfPoints());
  EXPECT_EQ(2, pd->GetNumberOfPolys());

  double p[3];
  pd->GetPoint(3, p);
  EXPECT_EQ(1.0, p[0]);
  EXPECT_EQ(1.0, p[1]);

  auto colors = vtkUnsignedCharArray::SafeDownCast(pd->GetPointData()->GetScalars());
  ASSERT_TRUE(colors);
  EXPECT_EQ(4, colors->GetNumberOfComponents());
  EXPECT_EQ(0, colors->GetTypedComponent(1, 0));
  EXPECT_EQ(255, colors->GetTypedComponent(1, 1));
  EXPECT_EQ(255, colors->GetTypedComponent(1, 3));

  EXPECT_EQ(Geometry::Point(1, 1, 0), obj->box.get_max());
}

TEST(ConvertGeometryToVtkAlgoTests, LinesAndPointsKeepPlainPolyDataType)
{
  const std::vector<SpireVBO::AttributeData> attribs{ {"aPos", 3 * sizeof(float)} };
  const std::vector<float> verts{ 0, 0, 0,  1, 0, 0,  2, 0, 0 };

  auto lines = convert(wrap(makePass(attribs, verts, { 0, 1, 1, 2 }, SpireIBO::PRIMITIVE::LINES)));
  EXPECT_EQ(GeometryType::NO_TYPE, lines->type);
  EXPECT_EQ(2, poly(lines)->GetNumberOfLines());

  auto points = convert(wrap(makePass(attribs, verts, { 0, 1, 2 }, SpireIBO::PRIMITIVE::POINTS)));
  EXPECT_EQ(3, poly(points)->GetNumberOfVerts());
}

TEST(ConvertGeometryToVtkAlgoTests, DropsOutOfRangeIndices)
{
  const std::vector<SpireVBO::AttributeData> attribs{ {"aPos", 3 * sizeof(float)} };
  const std::vector<float> verts{ 0, 0, 0,  1, 0, 0,  0, 1, 0 };
  auto obj = convert(wrap(makePass(attribs, verts, { 0, 1, 2, 0, 1, 7 }, SpireIBO::PRIMITIVE::TRIANGLES)));
  EXPECT_EQ(1, poly(obj)->GetNumberOfPolys());
}

TEST(ConvertGeometryToVtkAlgoTests, TransparencyOnlyAppliesToTransparentPasses)
{
  const std::vector<SpireVBO::AttributeData> attribs{ {"aPos", 3 * sizeof(float)} };
  const std::vector<float> verts{ 0, 0, 0,  1, 0, 0,  0, 1, 0 };
  auto pass = makePass(attribs, verts, { 0, 1, 2 }, SpireIBO::PRIMITIVE::TRIANGLES);
  pass.addUniform(SpireSubPass::Uniform("uTransparency", 0.4f));

  EXPECT_FLOAT_EQ(1.f, convert(wrap(pass))->material.opacity);

  pass.renderState.set(RenderState::ActionFlags::USE_TRANSPARENCY, true);
  EXPECT_FLOAT_EQ(0.4f, convert(wrap(pass))->material.opacity);
}

TEST(ConvertGeometryToVtkAlgoTests, ScreenSpaceTextIsNotConvertible)
{
  const std::vector<SpireVBO::AttributeData> attribs{ {"aPos", 3 * sizeof(float)}, {"aTexCoord", 2 * sizeof(float)} };
  const std::vector<float> verts{ 0, 0, 0, 0, 0,  1, 0, 0, 1, 0,  0, 1, 0, 0, 1 };
  EXPECT_THROW(convert(wrap(makePass(attribs, verts, { 0, 1, 2 }, SpireIBO::PRIMITIVE::TRIANGLES, "Shaders/TextBuilder"))),
    AlgorithmInputException);
}

class ConvertGeometryToVtkShowFieldTest : public ModuleTest
{
protected:
  void SetUp() override
  {
    Logging::LogSettings::Instance().setVerbose(false);
  }

  GeometryBaseHandle showField(FieldHandle field, bool nodes, bool edges, bool faces, ColorMapHandle colorMap = nullptr)
  {
    showField_ = makeModule("ShowField");
    showField_->setStateDefaults();
    auto state = showField_->get_state();
    state->setValue(Parameters::ShowNodes, nodes);
    state->setValue(Parameters::ShowEdges, edges);
    state->setValue(Parameters::ShowFaces, faces);
    stubPortNWithThisData(showField_, 0, field);
    if (colorMap)
      stubPortNWithThisData(showField_, 1, colorMap);
    showField_->execute();
    return std::dynamic_pointer_cast<GeometryObject>(getDataOnThisOutputPort(showField_, 0));
  }

  UseRealModuleStateFactory f_;
  ModuleHandle showField_;
};

TEST_F(ConvertGeometryToVtkShowFieldTest, CubeTriSurfFaces)
{
  auto geom = showField(CubeTriSurfLinearBasis(data_info_type::DOUBLE_E), false, false, true);
  auto parts = leaves(convert(geom));

  ASSERT_EQ(1u, parts.size());
  EXPECT_EQ(GeometryType::TRI_SURFACE, parts[0]->type);
  EXPECT_EQ(12, poly(parts[0])->GetNumberOfPolys());
  EXPECT_TRUE(poly(parts[0])->GetPointData()->GetNormals());
}

TEST_F(ConvertGeometryToVtkShowFieldTest, CubeTriSurfNodesEdgesFaces)
{
  auto geom = showField(CubeTriSurfLinearBasis(data_info_type::DOUBLE_E), true, true, true);
  vtkIdType verts = 0, lines = 0, polys = 0;
  for (const auto& part : leaves(convert(geom)))
  {
    verts += poly(part)->GetNumberOfVerts();
    lines += poly(part)->GetNumberOfLines();
    polys += poly(part)->GetNumberOfPolys();
  }
  EXPECT_EQ(8, verts);
  EXPECT_EQ(18, lines);
  EXPECT_EQ(12, polys);
}

TEST_F(ConvertGeometryToVtkShowFieldTest, ColorMappedFacesBakeColorsFromTexture)
{
  auto field = CubeTriSurfLinearBasis(data_info_type::DOUBLE_E);
  for (VMesh::index_type i = 0; i < field->vfield()->num_values(); ++i)
    field->vfield()->set_value(static_cast<double>(i) / 7.0, i);
  auto geom = showField(field, false, false, true, StandardColorMapFactory::create());

  auto parts = leaves(convert(geom));
  ASSERT_EQ(1u, parts.size());
  auto colors = vtkUnsignedCharArray::SafeDownCast(poly(parts[0])->GetPointData()->GetScalars());
  ASSERT_TRUE(colors);
  EXPECT_EQ(4, colors->GetNumberOfComponents());

  std::set<std::tuple<int, int, int>> distinct;
  for (vtkIdType i = 0; i < colors->GetNumberOfTuples(); ++i)
  {
    distinct.emplace(colors->GetTypedComponent(i, 0), colors->GetTypedComponent(i, 1), colors->GetTypedComponent(i, 2));
    EXPECT_EQ(255, colors->GetTypedComponent(i, 3));
  }
  // Eight distinct node values; opaque faces drop the colormap's own alpha.
  EXPECT_GE(distinct.size(), 6u);
}
