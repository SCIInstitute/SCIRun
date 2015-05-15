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

#include <Modules/Visualization/ShowFieldGlyphs.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/Datatypes/Material.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/Logging/Log.h>
#include <Core/Math/MiscMath.h>
#include <Core/Algorithms/Visualization/DataConversions.h>
#include <Core/Algorithms/Visualization/RenderFieldState.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include <boost/foreach.hpp>

#include <glm/glm.hpp>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Graphics;
using namespace SCIRun;

ModuleLookupInfo ShowFieldGlyphs::staticInfo_("ShowFieldGlyphs", "Visualization", "SCIRun");

ShowFieldGlyphs::ShowFieldGlyphs() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(PrimaryData);
  INITIALIZE_PORT(PrimaryColorMap);
  INITIALIZE_PORT(SecondaryData);
  INITIALIZE_PORT(SecondaryColorMap);
  INITIALIZE_PORT(TertiaryData);
  INITIALIZE_PORT(TertiaryColorMap);
  INITIALIZE_PORT(SceneGraph);
}

void ShowFieldGlyphs::setStateDefaults()
{
  auto state = get_state();

  // Vectors
  state->setValue(ShowVectors, false);
  state->setValue(VectorsTransparency, false);
  state->setValue(VectorsTransparencyValue, 0.65);
  state->setValue(VectorsScale, 0.1);
  state->setValue(VectorsResolution, 3);
  state->setValue(VectorsColoring, 0);
  state->setValue(VectorsDisplayType, 0);

  // Scalars
  state->setValue(ShowScalars, false);
  state->setValue(ScalarsTransparency, false);
  state->setValue(ScalarsTransparencyValue, 0.65);
  state->setValue(ScalarsScale, 0.1);
  state->setValue(ScalarsResolution, 3);
  state->setValue(ScalarsColoring, 0);
  state->setValue(ScalarsDisplayType, 0);


  state->setValue(DefaultMeshColor, ColorRGB(0.5, 0.5, 0.5).toString());
}

void ShowFieldGlyphs::execute()
{
  boost::shared_ptr<SCIRun::Field> pfield = getRequiredInput(PrimaryData);
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> pcolorMap = getOptionalInput(PrimaryColorMap);
  boost::optional<boost::shared_ptr<SCIRun::Field>> sfield = getOptionalInput(SecondaryData);
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> scolorMap = getOptionalInput(SecondaryColorMap);
  boost::optional<boost::shared_ptr<SCIRun::Field>> tfield = getOptionalInput(TertiaryData);
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> tcolorMap = getOptionalInput(TertiaryColorMap);

  if (needToExecute())
  {
    //configureInputs(pfield, sfield, tfield, pcolorMap, scolorMap, tcolorMap);
    GeometryHandle geom = buildGeometryObject(pfield, pcolorMap, get_state());
    sendOutput(SceneGraph, geom);
  }
}

void ShowFieldGlyphs::configureInputs(
  boost::shared_ptr<SCIRun::Field> pfield,
  boost::optional<boost::shared_ptr<SCIRun::Field>> sfield,
  boost::optional<boost::shared_ptr<SCIRun::Field>> tfield,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> pcolormap,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> scolormap,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> tcolormap)
{
  FieldInformation pfinfo(pfield);

  if (!pfinfo.is_svt())
  {
    THROW_ALGORITHM_INPUT_ERROR("No Scalar, Vector, or Tensor data found in the primary data field.");
  }

  if (!sfield)
  {
    *sfield = pfield;
  }

  if (!tfield)
  {
    *tfield = pfield;
  }

  if (*sfield != pfield || *tfield != pfield)
  {
    FieldInformation sfinfo(*sfield);
    FieldInformation tfinfo(*tfield);

    if (!sfinfo.is_svt())
    {
      THROW_ALGORITHM_INPUT_ERROR("No Scalar, Vector, or Tensor data found in the secondary data field.");
    }

    if (!tfinfo.is_svt())
    {
      THROW_ALGORITHM_INPUT_ERROR("No Scalar, Vector, or Tensor data found in the tertiary data field.");
    }


  }
}

GeometryHandle ShowFieldGlyphs::buildGeometryObject(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  ModuleStateHandle state)
{
  // Function for reporting progress.
  //SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc progressFunc = getUpdaterFunc();

  bool showVectors = state->getValue(ShowFieldGlyphs::ShowVectors).toBool();
  bool showScalars = state->getValue(ShowFieldGlyphs::ShowScalars).toBool();

  GeometryHandle geom(new GeometryObject(field, *this, "EntireGlyphField"));

  FieldInformation finfo(field);

  if (finfo.is_vector() && showVectors)
  {
    renderVectors(field, colorMap, getVectorsRenderState(state, colorMap), geom, geom->uniqueID());
  }

  if (finfo.is_scalar() && showScalars)
  {
    renderScalars(field, colorMap, getVectorsRenderState(state, colorMap), geom, geom->uniqueID());
  }

  return geom;
}

void ShowFieldGlyphs::renderVectors(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
  RenderState state,
  Core::Datatypes::GeometryHandle geom,
  const std::string& id)
{
  FieldInformation finfo(field);

  if (!finfo.is_vector())
  {
    THROW_ALGORITHM_INPUT_ERROR("Field is not a vector field");
  }  

  if (!finfo.is_linear())
  {
    THROW_ALGORITHM_INPUT_ERROR("only able to handle data on nodes at this point");
  }
  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  GeometryObject::ColorScheme colorScheme = GeometryObject::COLOR_UNIFORM;
  ColorRGB node_color;  

  if (fld->basis_order() < 0 || (fld->basis_order() == 0 && mesh->dimensionality() != 0) || state.get(RenderState::USE_DEFAULT_COLOR))
    colorScheme = GeometryObject::COLOR_UNIFORM;
  else if (state.get(RenderState::USE_COLORMAP))
    colorScheme = GeometryObject::COLOR_MAP;
  else
    colorScheme = GeometryObject::COLOR_IN_SITU;

  mesh->synchronize(Mesh::EDGES_E);

  auto my_state = get_state();
  double radius = my_state->getValue(VectorsScale).toDouble();
  double resolution = static_cast<double>(my_state->getValue(VectorsResolution).toInt());
  if (radius < 0) radius = 0.1;
  if (resolution < 3) resolution = 5;

  std::stringstream ss;
  ss << state.mGlyphType << resolution << radius << colorScheme;

  std::string uniqueNodeID = id + "vector_glyphs" + ss.str();
  std::string vboName = uniqueNodeID + "VBO";
  std::string iboName = uniqueNodeID + "IBO";
  std::string passName = uniqueNodeID + "Pass";

  bool useLines = state.mGlyphType == RenderState::GlyphType::LINE_GLYPH || state.mGlyphType == RenderState::GlyphType::NEEDLE_GLYPH;

  // Construct VBO.
  std::string shader = "Shaders/UniformColor";
  std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  if (!useLines)
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
  GeometryObject::RenderType renderType = GeometryObject::RENDER_VBO_IBO;


  std::vector<GeometryObject::SpireSubPass::Uniform> uniforms;
  auto myState = get_state();
  double transparencyValue = myState->getValue(VectorsTransparencyValue).toDouble();
  if (state.get(RenderState::USE_TRANSPARENT_EDGES))
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uTransparency", (float)(transparencyValue)));
  // TODO: add colormapping options
  if (colorScheme == GeometryObject::COLOR_MAP)
  {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 4 * sizeof(float)));
    if (!useLines)
    {
      shader = "Shaders/DirPhongCMap";
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    }
    else
    {
      shader = "Shaders/ColorMap";
    }
  }
  else if (colorScheme == GeometryObject::COLOR_IN_SITU)
  {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 1 * sizeof(uint32_t), true));
    if (!useLines)
    {
      shader = "Shaders/DirPhongInSitu";
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    }
    else
    {
      shader = "Shaders/InSituColor";
    }
  }
  else if (colorScheme == GeometryObject::COLOR_UNIFORM)
  {
    ColorRGB dft = state.defaultColor;
    if (!useLines)
    {
      shader = "Shaders/DirPhong";
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDiffuseColor",
        glm::vec4(dft.r(), dft.g(), dft.b(), (float)transparencyValue)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    }
    else
    {
      uniforms.emplace_back("uColor", glm::vec4(dft.r(), dft.g(), dft.b(), (float)transparencyValue));
    }
  }

  GeometryObject::SpireIBO::PRIMITIVE primIn = GeometryObject::SpireIBO::TRIANGLES;;
  // Use Lines
  if (useLines)
  {
    primIn = GeometryObject::SpireIBO::LINES;
  }

  GlyphGeom glyphs;
  auto facade(field->mesh()->getFacade());
  //for (const auto& node : facade->nodes())
  BOOST_FOREACH(const NodeInfo<VMesh>& node, facade->nodes())
  {
    Vector v;
    fld->get_value(v, node.index());
    Point p1 = node.point();
    Point p2 = p1 + v;
    if (colorScheme != GeometryObject::COLOR_UNIFORM)
    {
      ColorMapHandle map = colorMap.get();
      node_color = map->valueToColor(v);
    }
    switch (state.mGlyphType)
    {
    case RenderState::GlyphType::LINE_GLYPH:
      glyphs.addNeedle(p1, p2, node_color, node_color);
      break;
    case RenderState::GlyphType::NEEDLE_GLYPH:
      glyphs.addNeedle(p1, p2, node_color, node_color);
      break;
    case RenderState::GlyphType::COMET_GLYPH:
      THROW_ALGORITHM_INPUT_ERROR("Comet Geom is not supported yet.");
      break;
    case RenderState::GlyphType::CONE_GLYPH:
      glyphs.addCone(p1, p2, radius, resolution, node_color, node_color);
      break;
    case RenderState::GlyphType::ARROW_GLYPH:
      glyphs.addArrow(p1, p2, radius, resolution, node_color, node_color);
      break;
    case RenderState::GlyphType::DISK_GLYPH:
      glyphs.addCylinder(p1, p2, radius, resolution, node_color, node_color);
      break;
    case RenderState::GlyphType::RING_GLYPH:
      THROW_ALGORITHM_INPUT_ERROR("Ring Geom is not supported yet.");
      break;
    case RenderState::GlyphType::SPRING_GLYPH:
      THROW_ALGORITHM_INPUT_ERROR("Spring Geom is not supported yet.");
      break;
    default:
      if (useLines)
        glyphs.addNeedle(p1, p2, node_color, node_color);
      else
        glyphs.addArrow(p1, p2, radius, resolution, node_color, node_color);
      break;
    }

  }

  uint32_t iboSize = 0;
  uint32_t vboSize = 0;

  int64_t numVBOElements = 0;
  std::vector<Vector> points;
  std::vector<Vector> normals;
  std::vector<ColorRGB> colors;
  std::vector<uint32_t> indices;

  glyphs.getBufferInfo(numVBOElements, points, normals, colors, indices);

  vboSize = (uint32_t)points.size() * 3 * sizeof(float);
  vboSize += (uint32_t)normals.size() * 3 * sizeof(float);
  if (colorScheme == GeometryObject::COLOR_IN_SITU || colorScheme == GeometryObject::COLOR_MAP)
    vboSize += (uint32_t)colors.size() * 4 * sizeof(float); //RGBA
  iboSize = (uint32_t)indices.size() * sizeof(uint32_t);
  /// \todo To reduce memory requirements, we can use a 16bit index buffer.

  /// \todo To further reduce a large amount of memory, get rid of the index
  ///       buffer and use glDrawArrays to render without an IBO. An IBO is
  ///       a waste of space.
  ///       http://www.opengl.org/sdk/docs/man3/xhtml/glDrawArrays.xml

  /// \todo Switch to unique_ptrs and move semantics.
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr(new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr(new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));

  // Accessing the pointers like this is contrived. We only do this for
  // speed since we will be using the pointers in a tight inner loop.
  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer = iboBufferSPtr.get();
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer = vboBufferSPtr.get();

  //write to the IBO/VBOs

  for (auto a : indices)
    iboBuffer->write(a);

  for (size_t i = 0; i < points.size(); i++)
  {
    // Write first point on line
    vboBuffer->write(static_cast<float>(points.at(i).x()));
    vboBuffer->write(static_cast<float>(points.at(i).y()));
    vboBuffer->write(static_cast<float>(points.at(i).z()));
    // Write normal
    if (normals.size() == points.size())
    {
      vboBuffer->write(static_cast<float>(normals.at(i).x()));
      vboBuffer->write(static_cast<float>(normals.at(i).y()));
      vboBuffer->write(static_cast<float>(normals.at(i).z()));
    }
    if (colorScheme == GeometryObject::COLOR_MAP || colorScheme == GeometryObject::COLOR_IN_SITU)
    {
      vboBuffer->write(static_cast<float>(colors.at(i).r()));
      vboBuffer->write(static_cast<float>(colors.at(i).g()));
      vboBuffer->write(static_cast<float>(colors.at(i).b()));
      vboBuffer->write(static_cast<float>(1.f));
    } // no color writing otherwise
  }

  // If true, then the VBO will be placed on the GPU. We don't want to place
  // VBOs on the GPU when we are generating rendering lists.
  GeometryObject::SpireVBO geomVBO(vboName, attribs, vboBufferSPtr, numVBOElements, mesh->get_bounding_box(), true);

  // Construct IBO.
  GeometryObject::SpireIBO geomIBO(iboName, primIn, sizeof(uint32_t), iboBufferSPtr);

  state.set(RenderState::IS_ON, true);
  state.set(RenderState::HAS_DATA, true);

  // Construct Pass.
  GeometryObject::SpireSubPass pass(passName, vboName, iboName, shader, colorScheme, state, renderType, geomVBO, geomIBO);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  geom->mVBOs.push_back(geomVBO);
  geom->mIBOs.push_back(geomIBO);
  geom->mPasses.push_back(pass);
}

void ShowFieldGlyphs::renderScalars(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
  RenderState state,
  Core::Datatypes::GeometryHandle geom,
  const std::string& id)
{
  FieldInformation finfo(field);

  if (!finfo.is_scalar())
  {
    THROW_ALGORITHM_INPUT_ERROR("Field is not a scalar field");
  }

  if (!finfo.is_linear())
  {
    THROW_ALGORITHM_INPUT_ERROR("only able to handle data on nodes at this point");
  }
  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  GeometryObject::ColorScheme colorScheme = GeometryObject::COLOR_UNIFORM;
  ColorRGB node_color;

  if (fld->basis_order() < 0 || (fld->basis_order() == 0 && mesh->dimensionality() != 0) || state.get(RenderState::USE_DEFAULT_COLOR))
    colorScheme = GeometryObject::COLOR_UNIFORM;
  else if (state.get(RenderState::USE_COLORMAP))
    colorScheme = GeometryObject::COLOR_MAP;
  else
    colorScheme = GeometryObject::COLOR_IN_SITU;

  mesh->synchronize(Mesh::NODES_E);

  auto my_state = get_state();
  double radius = my_state->getValue(ScalarsScale).toDouble();
  double resolution = static_cast<double>(my_state->getValue(ScalarsResolution).toInt());
  if (radius < 0) radius = 0.1;
  if (resolution < 3) resolution = 5;

  std::stringstream ss;
  ss << state.mGlyphType << resolution << radius << colorScheme;

  std::string uniqueNodeID = id + "scalar_glyphs" + ss.str();
  std::string vboName = uniqueNodeID + "VBO";
  std::string iboName = uniqueNodeID + "IBO";
  std::string passName = uniqueNodeID + "Pass";

  bool usePoints = state.mGlyphType == RenderState::GlyphType::POINT_GLYPH;

  // Construct VBO.
  std::string shader = "Shaders/UniformColor";
  std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  if (!usePoints)
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
  GeometryObject::RenderType renderType = GeometryObject::RENDER_VBO_IBO;


  std::vector<GeometryObject::SpireSubPass::Uniform> uniforms;
  auto myState = get_state();
  double transparencyValue = myState->getValue(ScalarsTransparencyValue).toDouble();
  if (state.get(RenderState::USE_TRANSPARENT_NODES))
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uTransparency", (float)(transparencyValue)));
  // TODO: add colormapping options
  if (colorScheme == GeometryObject::COLOR_MAP)
  {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 4 * sizeof(float)));
    if (!usePoints)
    {
      shader = "Shaders/DirPhongCMap";
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    }
    else
    {
      shader = "Shaders/ColorMap";
    }
  }
  else if (colorScheme == GeometryObject::COLOR_IN_SITU)
  {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 1 * sizeof(uint32_t), true));
    if (!usePoints)
    {
      shader = "Shaders/DirPhongInSitu";
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    }
    else
    {
      shader = "Shaders/InSituColor";
    }
  }
  else if (colorScheme == GeometryObject::COLOR_UNIFORM)
  {
    ColorRGB dft = state.defaultColor;
    if (!usePoints)
    {
      shader = "Shaders/DirPhong";
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDiffuseColor",
        glm::vec4(dft.r(), dft.g(), dft.b(), (float)transparencyValue)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    }
    else
    {
      uniforms.emplace_back("uColor", glm::vec4(dft.r(), dft.g(), dft.b(), (float)transparencyValue));
    }
  }

  GeometryObject::SpireIBO::PRIMITIVE primIn = GeometryObject::SpireIBO::TRIANGLES;;
  // Use Points
  if (usePoints)
  {
    primIn = GeometryObject::SpireIBO::POINTS;
  }

  GlyphGeom glyphs;
  auto facade(field->mesh()->getFacade());
  //for (const auto& node : facade->nodes())
  BOOST_FOREACH(const NodeInfo<VMesh>& node, facade->nodes())
  {
    Vector v;
    fld->get_value(v, node.index());
    Point p = node.point();

    if (colorScheme != GeometryObject::COLOR_UNIFORM)
    {
      ColorMapHandle map = colorMap.get();
      node_color = map->valueToColor(v);
    }
    switch (state.mGlyphType)
    {
    case RenderState::GlyphType::POINT_GLYPH:
      glyphs.addPoint(p, node_color);
      std::cout << "addPoint" << std::endl;
      break;
    case RenderState::GlyphType::SPHERE_GLYPH:
      glyphs.addSphere(p, radius, resolution, node_color);
      std::cout << "addSphere" << std::endl;
      break;
    case RenderState::GlyphType::BOX_GLYPH:
      THROW_ALGORITHM_INPUT_ERROR("Box Geom is not supported yet.");
      break;
    case RenderState::GlyphType::AXIS_GLYPH:
      THROW_ALGORITHM_INPUT_ERROR("Axis Geom is not supported yet.");
      break;
    default:
      if (usePoints)
        glyphs.addPoint(p, node_color);
      else
        glyphs.addSphere(p, radius, resolution, node_color);
      break;
    }

  }

  uint32_t iboSize = 0;
  uint32_t vboSize = 0;

  int64_t numVBOElements = 0;
  std::vector<Vector> points;
  std::vector<Vector> normals;
  std::vector<ColorRGB> colors;
  std::vector<uint32_t> indices;

  glyphs.getBufferInfo(numVBOElements, points, normals, colors, indices);

  vboSize = (uint32_t)points.size() * 3 * sizeof(float);
  vboSize += (uint32_t)normals.size() * 3 * sizeof(float);
  if (colorScheme == GeometryObject::COLOR_IN_SITU || colorScheme == GeometryObject::COLOR_MAP)
    vboSize += (uint32_t)colors.size() * 4 * sizeof(float); //RGBA
  iboSize = (uint32_t)indices.size() * sizeof(uint32_t);
  /// \todo To reduce memory requirements, we can use a 16bit index buffer.

  /// \todo To further reduce a large amount of memory, get rid of the index
  ///       buffer and use glDrawArrays to render without an IBO. An IBO is
  ///       a waste of space.
  ///       http://www.opengl.org/sdk/docs/man3/xhtml/glDrawArrays.xml

  /// \todo Switch to unique_ptrs and move semantics.
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr(new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr(new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));

  // Accessing the pointers like this is contrived. We only do this for
  // speed since we will be using the pointers in a tight inner loop.
  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer = iboBufferSPtr.get();
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer = vboBufferSPtr.get();

  //write to the IBO/VBOs

  for (auto a : indices)
    iboBuffer->write(a);

  for (size_t i = 0; i < points.size(); i++)
  {
    // Write first point on line
    vboBuffer->write(static_cast<float>(points.at(i).x()));
    vboBuffer->write(static_cast<float>(points.at(i).y()));
    vboBuffer->write(static_cast<float>(points.at(i).z()));
    // Write normal
    if (normals.size() == points.size())
    {
      vboBuffer->write(static_cast<float>(normals.at(i).x()));
      vboBuffer->write(static_cast<float>(normals.at(i).y()));
      vboBuffer->write(static_cast<float>(normals.at(i).z()));
    }
    if (colorScheme == GeometryObject::COLOR_MAP || colorScheme == GeometryObject::COLOR_IN_SITU)
    {
      vboBuffer->write(static_cast<float>(colors.at(i).r()));
      vboBuffer->write(static_cast<float>(colors.at(i).g()));
      vboBuffer->write(static_cast<float>(colors.at(i).b()));
      vboBuffer->write(static_cast<float>(1.f));
    } // no color writing otherwise
  }

  // If true, then the VBO will be placed on the GPU. We don't want to place
  // VBOs on the GPU when we are generating rendering lists.
  GeometryObject::SpireVBO geomVBO(vboName, attribs, vboBufferSPtr, numVBOElements, mesh->get_bounding_box(), true);

  // Construct IBO.
  GeometryObject::SpireIBO geomIBO(iboName, primIn, sizeof(uint32_t), iboBufferSPtr);

  state.set(RenderState::IS_ON, true);
  state.set(RenderState::HAS_DATA, true);

  // Construct Pass.
  GeometryObject::SpireSubPass pass(passName, vboName, iboName, shader, colorScheme, state, renderType, geomVBO, geomIBO);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  geom->mVBOs.push_back(geomVBO);
  geom->mIBOs.push_back(geomIBO);
  geom->mPasses.push_back(pass);
}


RenderState ShowFieldGlyphs::getVectorsRenderState(
  ModuleStateHandle state,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap)
{
  RenderState renState;

  bool useColorMap = state->getValue(ShowFieldGlyphs::VectorsColoring).toInt() == 1;
  renState.set(RenderState::USE_NORMALS, true);
  
  renState.set(RenderState::IS_ON, state->getValue(ShowFieldGlyphs::ShowVectors).toBool());
  renState.set(RenderState::USE_TRANSPARENT_EDGES, state->getValue(ShowFieldGlyphs::VectorsTransparency).toBool());

  switch (state->getValue(ShowFieldGlyphs::VectorsDisplayType).toInt())
  {
  case 0:
    renState.mGlyphType = RenderState::GlyphType::LINE_GLYPH;
    break;
  case 1:
    renState.mGlyphType = RenderState::GlyphType::NEEDLE_GLYPH;
    break;
  case 2:
    renState.mGlyphType = RenderState::GlyphType::COMET_GLYPH;
    break;
  case 3:
    renState.mGlyphType = RenderState::GlyphType::CONE_GLYPH;
    break;
  case 4:
    renState.mGlyphType = RenderState::GlyphType::ARROW_GLYPH;
    break;
  case 5:
    renState.mGlyphType = RenderState::GlyphType::DISK_GLYPH;
    break;
  case 6:
    renState.mGlyphType = RenderState::GlyphType::RING_GLYPH;
    break;
  case 7:
    renState.mGlyphType = RenderState::GlyphType::SPRING_GLYPH;
    break;
  default:
    renState.mGlyphType = RenderState::GlyphType::LINE_GLYPH;
    break;
  }

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldGlyphs::DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
    renState.defaultColor.g() > 1.0 ||
    renState.defaultColor.b() > 1.0) ?
    ColorRGB(
    renState.defaultColor.r() / 255.,
    renState.defaultColor.g() / 255.,
    renState.defaultColor.b() / 255.)
    : renState.defaultColor;
  
  if (colorMap && useColorMap)
  {
    renState.set(RenderState::USE_COLORMAP, true);
  }
  else
  {
    renState.set(RenderState::USE_DEFAULT_COLOR, true);
  }

  return renState;
}

RenderState ShowFieldGlyphs::getScalarsRenderState(
  ModuleStateHandle state,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap)
{
  RenderState renState;

  bool useColorMap = state->getValue(ShowFieldGlyphs::ScalarsColoring).toInt() == 1;
  renState.set(RenderState::USE_NORMALS, true);

  renState.set(RenderState::IS_ON, state->getValue(ShowFieldGlyphs::ShowScalars).toBool());
  renState.set(RenderState::USE_TRANSPARENT_NODES, state->getValue(ShowFieldGlyphs::ScalarsTransparency).toBool());

  switch (state->getValue(ShowFieldGlyphs::ScalarsDisplayType).toInt())
  {
  case 0:
    renState.mGlyphType = RenderState::GlyphType::POINT_GLYPH;
    break;
  case 1:
    renState.mGlyphType = RenderState::GlyphType::SPHERE_GLYPH;
    break;
  case 2:
    renState.mGlyphType = RenderState::GlyphType::BOX_GLYPH;
    break;
  case 3:
    renState.mGlyphType = RenderState::GlyphType::AXIS_GLYPH;
    break;
  default:
    renState.mGlyphType = RenderState::GlyphType::POINT_GLYPH;
    break;
  }

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldGlyphs::DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
    renState.defaultColor.g() > 1.0 ||
    renState.defaultColor.b() > 1.0) ?
    ColorRGB(
    renState.defaultColor.r() / 255.,
    renState.defaultColor.g() / 255.,
    renState.defaultColor.b() / 255.)
    : renState.defaultColor;

  if (colorMap && useColorMap)
  {
    renState.set(RenderState::USE_COLORMAP, true);
  }
  else
  {
    renState.set(RenderState::USE_DEFAULT_COLOR, true);
  }

  return renState;
}

AlgorithmParameterName ShowFieldGlyphs::ShowVectors("ShowVectors");
AlgorithmParameterName ShowFieldGlyphs::VectorsTransparency("VectorsTransparency");
AlgorithmParameterName ShowFieldGlyphs::VectorsTransparencyValue("VectorsTransparencyValue");
AlgorithmParameterName ShowFieldGlyphs::VectorsScale("VectorsScale");
AlgorithmParameterName ShowFieldGlyphs::VectorsResolution("VectorsResolution");
AlgorithmParameterName ShowFieldGlyphs::VectorsColoring("VectorsColoring");
AlgorithmParameterName ShowFieldGlyphs::VectorsDisplayType("VectorsDisplayType");
AlgorithmParameterName ShowFieldGlyphs::ShowScalars("ShowScalars");
AlgorithmParameterName ShowFieldGlyphs::ScalarsTransparency("ScalarsTransparency");
AlgorithmParameterName ShowFieldGlyphs::ScalarsTransparencyValue("ScalarsTransparencyValue");
AlgorithmParameterName ShowFieldGlyphs::ScalarsScale("ScalarsScale");
AlgorithmParameterName ShowFieldGlyphs::ScalarsResolution("ScalarsResolution");
AlgorithmParameterName ShowFieldGlyphs::ScalarsColoring("ScalarsColoring");
AlgorithmParameterName ShowFieldGlyphs::ScalarsDisplayType("ScalarsDisplayType");
AlgorithmParameterName ShowFieldGlyphs::DefaultMeshColor("DefaultMeshColor");