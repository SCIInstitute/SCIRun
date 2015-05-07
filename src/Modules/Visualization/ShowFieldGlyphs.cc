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
}

void ShowFieldGlyphs::execute()
{
  boost::shared_ptr<SCIRun::Field> field = getRequiredInput(PrimaryData);
  //boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap = getOptionalInput(PrimaryColorMap);
  
  //std::cout << "enter execute" << std::endl;
  //if (needToExecute())
  {
    GeometryHandle geom = buildGeometryObject(field, nullptr, getRenderState(get_state(), nullptr));
    //GeometryHandle geom = buildGeometryObject(field, colorMap, getRenderState(get_state(), colorMap));
    sendOutput(SceneGraph, geom);
  }
}

GeometryHandle ShowFieldGlyphs::buildGeometryObject(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  RenderState state)
{
  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();
  FieldInformation finfo(field);

  //std::cout << "enter buildgeometry" << std::endl;

  if (!finfo.is_vector())
  {
    THROW_ALGORITHM_INPUT_ERROR("Field is not a vector field");
  }

  //std::cout << "is vector" << std::endl;

  GeometryObject::ColorScheme colorScheme = GeometryObject::COLOR_UNIFORM;
  //GeometryHandle geom(new GeometryObject(field, *this, "EntireGlyphField"));

  /*
  if (fld->basis_order() < 0 || (fld->basis_order() == 0 && mesh->dimensionality() != 0) || state.get(RenderState::USE_DEFAULT_COLOR))
    colorScheme = GeometryObject::COLOR_UNIFORM;
  else if (state.get(RenderState::USE_COLORMAP))
    colorScheme = GeometryObject::COLOR_MAP;
  else
    colorScheme = GeometryObject::COLOR_IN_SITU;
  */

  auto facade(field->mesh()->getFacade());
  std::ostringstream ostr;
  
  //for (const auto& node : facade->nodes())
  if (!finfo.is_linear())
  {
    THROW_ALGORITHM_INPUT_ERROR("only able to handle data on nodes at this point");
  }

  //std::cout << "is linear" << std::endl;

  GlyphGeom* glyphs = new GlyphGeom();

  BOOST_FOREACH(const NodeInfo<VMesh>& node, facade->nodes())
  {
    Vector v;
    fld->get_value(v, node.index());
    Point p1 = node.point();
    Point p2 = p1 + v;

    glyphs->addArrow(p1, p2, 0.2, 10);

  }
  
  //std::cout << "out of loop" << std::endl;

  uint32_t iboSize = 0;
  uint32_t vboSize = 0;

  int64_t numVBOElements = 0;
  std::vector<Vector> points;
  std::vector<Vector> normals;
  std::vector<ColorRGB> colors;
  std::vector<uint32_t> indices;

  glyphs->getBufferInfo(numVBOElements, points, normals, colors, indices);;

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

  //std::cout << "vboBuffer written" << std::endl;

  GeometryHandle geom(new GeometryObject(field, *this, "EntireGlyphField"));

  std::stringstream ss;
  for (auto a : points) ss << a.x() << a.y() << a.z();

  std::string uniqueNodeID = geom->uniqueID() + "glyph_arrow_cylinders" + std::string(ss.str().c_str());
  std::string vboName = uniqueNodeID + "VBO";
  std::string iboName = uniqueNodeID + "IBO";
  std::string passName = uniqueNodeID + "Pass";

  // Construct VBO.
  std::string shader = "Shaders/DirPhong";
  std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
  GeometryObject::RenderType renderType = GeometryObject::RENDER_VBO_IBO;

  // If true, then the VBO will be placed on the GPU. We don't want to place
  // VBOs on the GPU when we are generating rendering lists.
  GeometryObject::SpireVBO geomVBO(vboName, attribs, vboBufferSPtr, numVBOElements, mesh->get_bounding_box(), true);

  // Construct IBO.
  GeometryObject::SpireIBO geomIBO(iboName, GeometryObject::SpireIBO::TRIANGLES, sizeof(uint32_t), iboBufferSPtr);
  
  state.set(RenderState::IS_ON, true);
  state.set(RenderState::HAS_DATA, true);
  
  // Construct Pass.
  GeometryObject::SpireSubPass pass(passName, vboName, iboName, shader, colorScheme, state, renderType, geomVBO, geomIBO);

  // Add all uniforms generated above to the pass.
  std::vector<GeometryObject::SpireSubPass::Uniform> uniforms;
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDiffuseColor", glm::vec4(1.f, 1.f, 1.f, 1.f)));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
  uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }
    
  geom->mVBOs.push_back(geomVBO);
  geom->mIBOs.push_back(geomIBO);
  geom->mPasses.push_back(pass);

  return geom;
}

RenderState ShowFieldGlyphs::getRenderState(
  ModuleStateHandle state,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap)
{
  RenderState renState;

  renState.set(RenderState::USE_TRANSPARENCY, false);

  renState.defaultColor = ColorRGB(1, 1, 1);
  renState.set(RenderState::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::USE_NORMALS, true);

  /*
  renState.set(RenderState::IS_ON, state->getValue(ShowFieldModule::ShowEdges).toBool());
  renState.set(RenderState::USE_TRANSPARENT_EDGES, state->getValue(ShowFieldModule::EdgeTransparency).toBool());
  renState.set(RenderState::USE_CYLINDER, state->getValue(ShowFieldModule::EdgesAsCylinders).toInt() == 1);

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldModule::DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
    renState.defaultColor.g() > 1.0 ||
    renState.defaultColor.b() > 1.0) ?
    ColorRGB(
    renState.defaultColor.r() / 255.,
    renState.defaultColor.g() / 255.,
    renState.defaultColor.b() / 255.)
    : renState.defaultColor;

  edgeTransparencyValue_ = (float)(state->getValue(ShowFieldModule::EdgeTransparencyValue).toDouble());
  */
  if (colorMap)
  {
    renState.set(RenderState::USE_COLORMAP, true);
  }
  else
  {
    renState.set(RenderState::USE_DEFAULT_COLOR, true);
  }

  return renState;
}