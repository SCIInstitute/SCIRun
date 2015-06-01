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

#include <Modules/Visualization/ShowField.h>
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

#include <glm/glm.hpp>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Graphics;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(Visualization, CylinderRadius);

ModuleLookupInfo ShowFieldModule::staticInfo_("ShowField", "Visualization", "SCIRun");

ShowFieldModule::ShowFieldModule() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(SceneGraph);
}

void ShowFieldModule::setStateDefaults()
{
  auto state = get_state();
  state->setValue(ShowNodes, false);
  state->setValue(ShowEdges, true);
  state->setValue(ShowFaces, true);
  state->setValue(NodeTransparency, false);
  state->setValue(EdgeTransparency, false);
  state->setValue(FaceTransparency, false);
  state->setValue(DefaultMeshColor, ColorRGB(0.5, 0.5, 0.5).toString());

  //state->setValue(NodeAsPoints, true); //not used
  state->setValue(NodeAsSpheres, 0);
  //state->setValue(EdgesAsLines, true); //not used
  state->setValue(EdgesAsCylinders, 0);
  state->setValue(FaceTransparencyValue, 0.65f);
  state->setValue(EdgeTransparencyValue, 0.65f);
  state->setValue(NodeTransparencyValue, 0.65f);
  state->setValue(SphereScaleValue, 0.03);
  state->setValue(CylinderRadius, 0.1);
  state->setValue(CylinderResolution, 5);
  faceTransparencyValue_ = 0.65f;
  edgeTransparencyValue_ = 0.65f;
  nodeTransparencyValue_ = 0.65f;

  // NOTE: We need to add radio buttons for USE_DEFAULT_COLOR, COLORMAP, and
  // COLOR_CONVERT. USE_DEFAULT_COLOR is selected by default. COLOR_CONVERT
  // is more up in the air.
}

void ShowFieldModule::execute()
{
  auto field = getRequiredInput(Field);
  auto colorMap = getOptionalInput(ColorMapObject);

  if (needToExecute())
  {
    GeometryHandle geom = buildGeometryObject(field, colorMap, get_state());
    sendOutput(SceneGraph, geom);
  }
}

RenderState ShowFieldModule::getNodeRenderState(
  ModuleStateHandle state,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap)
{
  RenderState renState;

  renState.set(RenderState::IS_ON, state->getValue(ShowFieldModule::ShowNodes).toBool());
  renState.set(RenderState::USE_TRANSPARENT_NODES, state->getValue(ShowFieldModule::NodeTransparency).toBool());

  renState.set(RenderState::USE_SPHERE, state->getValue(ShowFieldModule::NodeAsSpheres).toInt() == 1);

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldModule::DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
                           renState.defaultColor.g() > 1.0 ||
                           renState.defaultColor.b() > 1.0)?
                                ColorRGB(
                                renState.defaultColor.r() / 255.,
                                renState.defaultColor.g() / 255.,
                                renState.defaultColor.b() / 255.)
                            :   renState.defaultColor;

  if (colorMap)
  {
    renState.set(RenderState::USE_COLORMAP, true);
  }
  else
  {
    /// \todo Set this value dependent on the radio button choice in the
    ///       dialog. Presumably this should overwrite any choice made by the
    ///       user.
    renState.set(RenderState::USE_DEFAULT_COLOR, true);
  }

  return renState;
}

RenderState ShowFieldModule::getEdgeRenderState(
  ModuleStateHandle state,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap)
{
  RenderState renState;

  renState.set(RenderState::IS_ON, state->getValue(ShowFieldModule::ShowEdges).toBool());
  renState.set(RenderState::USE_TRANSPARENT_EDGES, state->getValue(ShowFieldModule::EdgeTransparency).toBool());
  renState.set(RenderState::USE_CYLINDER, state->getValue(ShowFieldModule::EdgesAsCylinders).toInt() == 1);

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldModule::DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
                           renState.defaultColor.g() > 1.0 ||
                           renState.defaultColor.b() > 1.0)?
                                ColorRGB(
                                renState.defaultColor.r() / 255.,
                                renState.defaultColor.g() / 255.,
                                renState.defaultColor.b() / 255.)
                            :   renState.defaultColor;

  edgeTransparencyValue_ = (float)(state->getValue(ShowFieldModule::EdgeTransparencyValue).toDouble());

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

RenderState ShowFieldModule::getFaceRenderState(
  ModuleStateHandle state,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap)
{
  RenderState renState;

  renState.set(RenderState::IS_ON, state->getValue(ShowFieldModule::ShowFaces).toBool());
  renState.set(RenderState::USE_TRANSPARENCY, state->getValue(ShowFieldModule::FaceTransparency).toBool());

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldModule::DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
                           renState.defaultColor.g() > 1.0 ||
                           renState.defaultColor.b() > 1.0)?
                                ColorRGB(
                                renState.defaultColor.r() / 255.,
                                renState.defaultColor.g() / 255.,
                                renState.defaultColor.b() / 255.)
                            :   renState.defaultColor;

  faceTransparencyValue_ = (float)(state->getValue(ShowFieldModule::FaceTransparencyValue).toDouble());

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

GeometryHandle ShowFieldModule::buildGeometryObject(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
  ModuleStateHandle state)
{
  // Function for reporting progress.
  SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc progressFunc =
    getUpdaterFunc();

  /// \todo Determine a better way of handling all of the various object state.
  bool showNodes = state->getValue(ShowFieldModule::ShowNodes).toBool();
  bool showEdges = state->getValue(ShowFieldModule::ShowEdges).toBool();
  bool showFaces = state->getValue(ShowFieldModule::ShowFaces).toBool();
  // Resultant geometry type (representing a spire object and a number of passes).
  GeometryHandle geom(new GeometryObject(field, *this, "EntireField"));

  /// \todo Implement inputs_changes_ ? See old scirun ShowField.cc:293.

  /// \todo Mind material properties (simple since we already have implemented
  ///       most of this).

  /// \todo Handle assignment of color map. The color map will need to be
  ///       available to us as we are building the meshes. Due to the way
  ///       SCIRun expects meshes to be built.

  /// \todo render_state_ DIRTY flag? See old scirun ShowField.cc:446.

  const int dim = field->vmesh()->dimensionality();
  if (showEdges && dim < 1) { showEdges = false; }
  if (showFaces && dim < 2) { showFaces = false; }

  if (showNodes)
  {
    // Construct node geometry.
    renderNodes(field, colorMap, getNodeRenderState(state, colorMap), geom, geom->uniqueID());
  }

  if (showFaces)
  {
    int approxDiv = 1;
    renderFaces(field, colorMap, getFaceRenderState(state, colorMap), geom, approxDiv, geom->uniqueID());
  }

  if (showEdges)
  {
    renderEdges(field, colorMap, getEdgeRenderState(state, colorMap), geom, geom->uniqueID());
  }

  return geom;
}


void ShowFieldModule::renderFaces(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  RenderState state, Core::Datatypes::GeometryHandle geom,
  unsigned int approxDiv,
  const std::string& id)
{
  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  // Directly ported from SCIRUN 4. Unsure what 'linear' is.
  // I'm assuming it means linear interpolation as opposed to nearest neighbor
  // interpolation along the basis. But I could be wrong.
  bool doLinear = (fld->basis_order() < 2 && mesh->basis_order() < 2 && approxDiv == 1);

  // Todo: Check for texture -- this is indicative of volume rendering.
  // if(mesh->is_regularmesh() && mesh->is_surface() &&
  //    get_flag(render_state, USE_TEXTURE))

  if (doLinear)
  {
    return renderFacesLinear(field, colorMap, state, geom, approxDiv, id);
  }
  else
  {
    std::cout << "Non linear faces not supported at this time." << std::endl;
  }
}


void ShowFieldModule::renderFacesLinear(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  RenderState state,
  Core::Datatypes::GeometryHandle geom,
  unsigned int approxDiv,
  const std::string& id)
{
  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  mesh->synchronize(Mesh::FACES_E);

  VMesh::Face::size_type numFaces;

  mesh->size(numFaces);

  if (numFaces == 0)
    return;

  bool withNormals = (state.get(RenderState::USE_NORMALS));
  if (withNormals) { mesh->synchronize(Mesh::NORMALS_E); }

  auto st = get_state();
  bool invertNormals = st->getValue(FaceInvertNormals).toBool();
  GeometryObject::ColorScheme colorScheme = GeometryObject::COLOR_UNIFORM;
  std::vector<double> svals;
  std::vector<Core::Geometry::Vector> vvals;
  std::vector<Core::Geometry::Tensor> tvals;
  std::vector<ColorRGB> face_colors;

  if (fld->basis_order() < 0 || state.get(RenderState::USE_DEFAULT_COLOR))
  {
    colorScheme = GeometryObject::COLOR_UNIFORM;
  }
  else if (state.get(RenderState::USE_COLORMAP))
  {
    colorScheme = GeometryObject::COLOR_MAP;
  }
  else // if (fld->basis_order() >= 0)
  {
    colorScheme = GeometryObject::COLOR_IN_SITU;
  }

  // Three 32 bit ints to index into the VBO
  uint32_t iboSize = static_cast<uint32_t>(mesh->num_faces() * sizeof(uint32_t) * 3);
  //Seven floats per VBO: Pos (3) XYZ, and Color (4) RGBA
  uint32_t vboSize = static_cast<uint32_t>(mesh->num_faces() * sizeof(float) * 7);

  // Construct VBO and IBO that will be used to render the faces. Once again,
  // IBOs are not strictly needed. But, we may be able to optimize this code
  // somewhat.
  /// \todo Switch to unique_ptrs and move semantics.
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr(
    new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr(
    new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));

  // Accessing the pointers like this is contrived. We only do this for
  // speed since we will be using the pointers in a tight inner loop.
  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer = iboBufferSPtr.get();
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer = vboBufferSPtr.get();

  uint32_t iboIndex = 0;
  int64_t numVBOElements = 0;

  VMesh::Face::iterator fiter, fiterEnd;
  VMesh::Node::array_type nodes;

  mesh->begin(fiter);
  mesh->end(fiterEnd);

  Core::Geometry::Point idpt;
  mesh->get_nodes(nodes, *fiter);
  mesh->get_point(idpt, nodes[0]);

  while (fiter != fiterEnd)
  {
    checkForInterruption();

    mesh->get_nodes(nodes, *fiter);

    std::vector<Core::Geometry::Point> points(nodes.size());
    std::vector<Core::Geometry::Vector> normals(nodes.size());

    //std::cout << "Node Size: " << nodes.size() << std::endl;

    for (size_t i = 0; i < nodes.size(); i++)
    {
      mesh->get_point(points[i], nodes[i]);
    }

    //TODO fix so the withNormals tp be woth lighting is called correctly, and the meshes are fixed.
    if (withNormals)
    {
      /// Fix normal of Quads
      if (points.size() == 4)
      {
        Core::Geometry::Vector edge1 = points[1] - points[0];
        Core::Geometry::Vector edge2 = points[2] - points[1];
        Core::Geometry::Vector edge3 = points[3] - points[2];
        Core::Geometry::Vector edge4 = points[0] - points[3];

        Core::Geometry::Vector norm = Cross(edge1, edge2) + Cross(edge2, edge3) + Cross(edge3, edge4) + Cross(edge4, edge1);

        norm.normalize();

        for (size_t i = 0; i < nodes.size(); i++)
        {
          normals[i] = invertNormals?-norm:norm;
        }
      }
      /// Fix Normals of Tris
      else
      {
        Core::Geometry::Vector edge1 = points[1] - points[0];
        Core::Geometry::Vector edge2 = points[2] - points[1];
        Core::Geometry::Vector norm = Cross(edge1, edge2);

        norm.normalize();

        for (size_t i = 0; i < nodes.size(); i++)
        {
          normals[i] = invertNormals?-norm:norm;
        }
        //For future reference for a try at smoother rendering
        /*
        for (size_t i = 0; i < nodes.size(); i++)
        {
        mesh->get_normal(normals[i], nodes[i]);
        }
        */
      }
    }

    // Default color single face no matter the element data.
    if (colorScheme == GeometryObject::COLOR_UNIFORM)
    {
      addFaceGeom(points, normals, withNormals, iboIndex, iboBuffer, vboBuffer,
        colorScheme, face_colors, state);
    }
    // Element data (Cells) so two sided faces.
    else if (fld->basis_order() == 0 && mesh->dimensionality() == 3)
    {
      ColorMap * map = colorMap.get().get();
      //two possible colors.
      svals.resize(2);
      vvals.resize(2);
      tvals.resize(2);
      face_colors = {{ColorRGB(1.,1.,1.),ColorRGB(1.,1.,1.)}};

      VMesh::Elem::array_type cells;
      mesh->get_elems(cells, *fiter);

      if (fld->is_scalar())
      {
        fld->get_value(svals[0], cells[0]);

        if (cells.size() > 1)
        {
          fld->get_value(svals[1], cells[1]);
        }
        else
        {
          svals[1] = svals[0];
        }
        face_colors[0] = map->valueToColor(svals[0]);
        face_colors[1] = map->valueToColor(svals[1]);
      }
      else if (fld->is_vector())
      {
        fld->get_value(vvals[0], cells[0]);

        if (cells.size() > 1)
        {
          fld->get_value(vvals[1], cells[1]);
        }
        else
        {
          svals[1] = svals[0];
        }

        face_colors[0] = map->valueToColor(vvals[0]);
        face_colors[1] = map->valueToColor(vvals[1]);
      }
      else if (fld->is_tensor())
      {
        fld->get_value(tvals[0], cells[0]);

        if (cells.size() > 1)
        {
          fld->get_value(tvals[1], cells[1]);
        }
        else
        {
          svals[1] = svals[0];
        }

        face_colors[0] = map->valueToColor(tvals[0]);
        face_colors[1] = map->valueToColor(tvals[1]);
      }

      state.set(RenderState::IS_DOUBLE_SIDED, true);

      addFaceGeom(points, normals, withNormals, iboIndex, iboBuffer, vboBuffer,
        colorScheme, face_colors, state);
    }
    // Element data (faces)
    else if (fld->basis_order() == 0 && mesh->dimensionality() == 2)
    {
      ColorMap * map = colorMap.get().get();
      //one possible color, each node that color.
      svals.resize(1);
      vvals.resize(1);
      tvals.resize(1);
      face_colors.resize(nodes.size());
      if (fld->is_scalar())
      {
        fld->get_value(svals[0], *fiter);
        face_colors[0] = map->valueToColor(svals[0]);
      }
      else if (fld->is_vector())
      {
        fld->get_value(vvals[0], *fiter);
        face_colors[0] = map->valueToColor(vvals[0]);
      }
      else if (fld->is_tensor())
      {
        fld->get_value(tvals[0], *fiter);
        face_colors[0] = map->valueToColor(tvals[0]);
      }

      // Same color at all corners.
      for (size_t i = 0; i<nodes.size(); ++i)
      {
        face_colors[i] = face_colors[0];
      }

      addFaceGeom(points, normals, withNormals, iboIndex, iboBuffer, vboBuffer,
        colorScheme, face_colors,  state);
    }

    // Data at nodes
    else if (fld->basis_order() == 1)
    {
      ColorMap * map = colorMap.get().get();
      svals.resize(nodes.size());
      vvals.resize(nodes.size());
      tvals.resize(nodes.size());
      face_colors.resize(nodes.size());
      //node.size() possible colors.
      if (fld->is_scalar())
      {
        for (size_t i = 0; i<nodes.size(); i++)
        {
          fld->get_value(svals[i], nodes[i]);
          face_colors[i] = map->valueToColor(svals[i]);
        }
      }
      else if (fld->is_vector())
      {
        for (size_t i = 0; i<nodes.size(); i++)
        {
          fld->get_value(vvals[i], nodes[i]);
          face_colors[i] = map->valueToColor(vvals[i]);
        }
      }
      else if (fld->is_tensor())
      {
        for (size_t i = 0; i<nodes.size(); i++)
        {
          fld->get_value(tvals[i], nodes[i]);
          face_colors[i] = map->valueToColor(tvals[i]);
        }
      }

      addFaceGeom(points, normals, withNormals, iboIndex, iboBuffer, vboBuffer,
        colorScheme, face_colors, state);
    }

    ++fiter;
    ++numVBOElements;
  }

  std::stringstream ss;
  ss << invertNormals << colorScheme << faceTransparencyValue_;

  std::string uniqueNodeID = id + "face" + ss.str();
  std::string vboName = uniqueNodeID + "VBO";
  std::string iboName = uniqueNodeID + "IBO";
  std::string passName = uniqueNodeID + "Pass";

  // NOTE: Attributes will depend on the color scheme. We will want to
  // normalize the colors if the color scheme is COLOR_IN_SITU.

  // Construct VBO.
  std::string shader = "Shaders/UniformColor";
  std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  std::vector<GeometryObject::SpireSubPass::Uniform> uniforms;
  if (withNormals)
  {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
  }

  if (state.get(RenderState::USE_TRANSPARENCY))
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uTransparency", faceTransparencyValue_));

  if (colorScheme == GeometryObject::COLOR_MAP)
  {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 4 * sizeof(float)));

    if (!state.get(RenderState::IS_DOUBLE_SIDED))
    {
      if (withNormals)
      {
        // Use colormapping lit shader.
        shader = "Shaders/DirPhongCMap";
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
      }
      else
      {
        // Use colormapping only shader.
        shader = "Shaders/ColorMap";
      }
    }
    else
    {
      attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColorSecondary", 4 * sizeof(float)));

      if (withNormals)
      {
        // Use colormapping lit shader.
        shader = "Shaders/DblSided_DirPhongCMap";
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
      }
      else
      {
        // Use colormapping only shader.
        shader = "Shaders/DblSided_ColorMap";
      }
    }
  }
  else if (colorScheme == GeometryObject::COLOR_IN_SITU)
  {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 4 * sizeof(float), true));

    if (state.get(RenderState::IS_DOUBLE_SIDED) == false)
    {
      if (withNormals)
      {
        shader = "Shaders/InSituPhongColor";
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
      }
      else
      {
        // Use colormapping shader.
        shader = "Shaders/InSituColor";
      }
    }
    else
    {
      attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColorSecondary", 4 * sizeof(float), true));

      if (withNormals)
      {
        shader = "Shaders/DblSided_InSituPhongColor";
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
      }
      else
      {
        // Use colormapping shader.
        shader = "Shaders/DblSided_InSituColor";
      }
    }
  }
  else if (colorScheme == GeometryObject::COLOR_UNIFORM)
  {
    ColorRGB defaultColor = state.defaultColor;

    if (withNormals)
    {
      shader = "Shaders/DirPhong";
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform(
        "uDiffuseColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), 1.0f)));
    }
    else
    {
      shader = "Shaders/UniformColor";
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform(
        "uColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), 1.0f)));
    }
  }

  GeometryObject::SpireVBO geomVBO = GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr,
    numVBOElements, mesh->get_bounding_box(), true);

  geom->mVBOs.push_back(geomVBO);

  // Construct IBO.

  GeometryObject::SpireIBO geomIBO = GeometryObject::SpireIBO(iboName,
                GeometryObject::SpireIBO::TRIANGLES, sizeof(uint32_t), iboBufferSPtr);

  geom->mIBOs.push_back(geomIBO);

  GeometryObject::SpireSubPass pass =
    GeometryObject::SpireSubPass(passName, vboName, iboName, shader,
    colorScheme, state, GeometryObject::RENDER_VBO_IBO, geomVBO, geomIBO);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  geom->mPasses.push_back(pass);

  /// \todo Add spheres and other glyphs as display lists. Will want to
  ///       build up to geometry / tessellation shaders if support is present.
}

// This function needs to be reorganized.
// The fact that we are only rendering triangles helps us dramatically and
// we get rid of the quads renderer pointers. Additionally, we can re-order
// the triangles in ES and perform different rendering based on the
// transparency of the triangles.
void ShowFieldModule::addFaceGeom(
  const std::vector<Core::Geometry::Point>  &points,
  const std::vector<Core::Geometry::Vector> &normals,
  bool withNormals,
  uint32_t& iboIndex,
  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer,
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer,
  GeometryObject::ColorScheme colorScheme,
  const std::vector<ColorRGB> &face_colors,
  const RenderState& state)
{
  auto writeVBOPoint = [&vboBuffer](const Core::Geometry::Point& point)
  {
    vboBuffer->write(static_cast<float>(point.x()));
    vboBuffer->write(static_cast<float>(point.y()));
    vboBuffer->write(static_cast<float>(point.z()));
  };

  auto writeVBONormal = [&vboBuffer](const Core::Geometry::Vector& normal)
  {
    vboBuffer->write(static_cast<float>(normal.x()));
    vboBuffer->write(static_cast<float>(normal.y()));
    vboBuffer->write(static_cast<float>(normal.z()));
  };

  auto writeVBOColorValue = [&vboBuffer](ColorRGB value)
  {
    vboBuffer->write(static_cast<float>(value.r()));
    vboBuffer->write(static_cast<float>(value.g()));
    vboBuffer->write(static_cast<float>(value.b()));
    vboBuffer->write(static_cast<float>(1.f));
  };

  auto writeIBOIndex = [&iboBuffer](uint32_t index)
  {
    iboBuffer->write(index);
  };

  bool doubleSided = state.get(RenderState::IS_DOUBLE_SIDED);

  if (colorScheme == GeometryObject::COLOR_UNIFORM)
  {
    if (points.size() == 4)
    {
      if (withNormals)
      {
        writeVBOPoint(points[0]);
        writeVBONormal(normals[0]);

        writeVBOPoint(points[1]);
        writeVBONormal(normals[1]);

        writeVBOPoint(points[2]);
        writeVBONormal(normals[2]);

        writeVBOPoint(points[3]);
        writeVBONormal(normals[3]);
      }
      else
      {
        writeVBOPoint(points[0]);
        writeVBOPoint(points[1]);
        writeVBOPoint(points[2]);
        writeVBOPoint(points[3]);
      }
      writeIBOIndex(iboIndex);
      writeIBOIndex(iboIndex + 1);
      writeIBOIndex(iboIndex + 2);

      writeIBOIndex(iboIndex + 2);
      writeIBOIndex(iboIndex + 3);
      writeIBOIndex(iboIndex + 0);

      iboIndex += 4;
    }
    else
    {
      for (size_t i = 2; i < points.size(); i++)
      {
        if (withNormals)
        {
          // Render points if we are not rendering spheres.
          writeVBOPoint(points[0]);
          writeVBONormal(normals[0]);
          writeIBOIndex(iboIndex);

          writeVBOPoint(points[i - 1]);
          writeVBONormal(normals[i - 1]);
          writeIBOIndex(iboIndex + i - 1);

          writeVBOPoint(points[i]);
          writeVBONormal(normals[i]);
          writeIBOIndex(iboIndex + i);
        }
        else
        {
          writeVBOPoint(points[0]);
          writeIBOIndex(iboIndex);

          writeVBOPoint(points[i - 1]);
          writeIBOIndex(iboIndex + i - 1);

          writeVBOPoint(points[i]);
          writeIBOIndex(iboIndex + i);
        }
      }
      iboIndex += points.size();
    }
  }
  else if (colorScheme == GeometryObject::COLOR_MAP)
  {
    if (points.size() == 4)
    {
      // Note:  For the double sided case, the 0 and 1 indices are not a typo.
      //        It is a direct translation from old scirun.
      if (withNormals)
      {
        writeVBOPoint(points[0]);
        writeVBONormal(normals[0]);
        if (!doubleSided) { writeVBOColorValue(face_colors[0]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        writeVBOPoint(points[1]);
        writeVBONormal(normals[1]);
        if (!doubleSided) { writeVBOColorValue(face_colors[1]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        writeVBOPoint(points[2]);
        writeVBONormal(normals[2]);
        if (!doubleSided) { writeVBOColorValue(face_colors[2]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        writeVBOPoint(points[3]);
        writeVBONormal(normals[3]);
        if (!doubleSided) { writeVBOColorValue(face_colors[3]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
      }
      else
      {
        writeVBOPoint(points[0]);
        if (!doubleSided) { writeVBOColorValue(face_colors[0]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        writeVBOPoint(points[1]);
        if (!doubleSided) { writeVBOColorValue(face_colors[1]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        writeVBOPoint(points[2]);
        if (!doubleSided) { writeVBOColorValue(face_colors[2]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        writeVBOPoint(points[3]);
        if (!doubleSided) { writeVBOColorValue(face_colors[3]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
      }
      writeIBOIndex(iboIndex);
      writeIBOIndex(iboIndex + 1);
      writeIBOIndex(iboIndex + 2);

      writeIBOIndex(iboIndex + 2);
      writeIBOIndex(iboIndex + 3);
      writeIBOIndex(iboIndex + 0);

      iboIndex += 4;
    }
    else
    {
      for (size_t i = 2; i < points.size(); i++)
      {
        // Note:  For the double sided case, the 0 and 1 indices are not a typo.
        //        It is a direct translation from old scirun.
        if (withNormals)
        {
          // Render points if we are not rendering spheres.
          writeVBOPoint(points[0]);
          writeVBONormal(normals[0]);
        if (!doubleSided) { writeVBOColorValue(face_colors[0]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          writeIBOIndex(iboIndex);

          // Apply misc user settings.
          writeVBOPoint(points[i - 1]);
          writeVBONormal(normals[i - 1]);
        if (!doubleSided) { writeVBOColorValue(face_colors[i-1]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          writeIBOIndex(iboIndex + i - 1);

          writeVBOPoint(points[i]);
          writeVBONormal(normals[i]);
        if (!doubleSided) { writeVBOColorValue(face_colors[i]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          writeIBOIndex(iboIndex + i);
        }
        else
        {
          // Render points if we are not rendering spheres.
          writeVBOPoint(points[0]);
        if (!doubleSided) { writeVBOColorValue(face_colors[0]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          writeIBOIndex(iboIndex);

          writeVBOPoint(points[i - 1]);
        if (!doubleSided) { writeVBOColorValue(face_colors[i-1]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          writeIBOIndex(iboIndex + i - 1);

          writeVBOPoint(points[i]);
        if (!doubleSided) { writeVBOColorValue(face_colors[i]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          writeIBOIndex(iboIndex + i);
        }
      }
      iboIndex += points.size();
    }
  }
  else if (colorScheme == GeometryObject::COLOR_IN_SITU)
  {
    if (points.size() == 4)
    {
      if (withNormals)
      {
        writeVBOPoint(points[0]);
        writeVBONormal(normals[0]);
        if (!doubleSided) { writeVBOColorValue(face_colors[0]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        writeVBOPoint(points[1]);
        writeVBONormal(normals[1]);
        if (!doubleSided) { writeVBOColorValue(face_colors[1]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        writeVBOPoint(points[2]);
        writeVBONormal(normals[2]);
        if (!doubleSided) { writeVBOColorValue(face_colors[2]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        writeVBOPoint(points[3]);
        writeVBONormal(normals[3]);
        if (!doubleSided) { writeVBOColorValue(face_colors[3]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
      }
      else
      {
        writeVBOPoint(points[0]);
        if (!doubleSided) { writeVBOColorValue(face_colors[0]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        // Add appropriate uniforms to the pass (in this case, uColor).
        writeVBOPoint(points[1]);
        if (!doubleSided) { writeVBOColorValue(face_colors[1]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        writeVBOPoint(points[2]);
        if (!doubleSided) { writeVBOColorValue(face_colors[2]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }

        writeVBOPoint(points[3]);
        if (!doubleSided) { writeVBOColorValue(face_colors[3]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
      }
      writeIBOIndex(iboIndex);
      writeIBOIndex(iboIndex + 1);
      writeIBOIndex(iboIndex + 2);

      writeIBOIndex(iboIndex + 2);
      writeIBOIndex(iboIndex + 3);
      writeIBOIndex(iboIndex + 0);

      iboIndex += 4;
    }
    else
    {
      for (size_t i = 2; i < points.size(); i++)
      {
        if (withNormals)
        {
          writeVBOPoint(points[0]);
          writeVBONormal(normals[0]);
        if (!doubleSided) { writeVBOColorValue(face_colors[0]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          writeIBOIndex(iboIndex);

          writeVBOPoint(points[i - 1]);
          writeVBONormal(normals[i - 1]);
        if (!doubleSided) { writeVBOColorValue(face_colors[i - 1]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          writeIBOIndex(iboIndex + i - 1);

          writeVBOPoint(points[i]);
          writeVBONormal(normals[i]);
        if (!doubleSided) { writeVBOColorValue(face_colors[i]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          writeIBOIndex(iboIndex + i);
        }
        else
        {
          writeVBOPoint(points[0]);
        if (!doubleSided) { writeVBOColorValue(face_colors[0]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          iboBuffer->write(iboIndex);

          writeVBOPoint(points[i - 1]);
        if (!doubleSided) { writeVBOColorValue(face_colors[i - 1]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          writeIBOIndex(iboIndex + i - 1);

          writeVBOPoint(points[i]);
        if (!doubleSided) { writeVBOColorValue(face_colors[i]); }
        else              { writeVBOColorValue(face_colors[0]); writeVBOColorValue(face_colors[1]); }
          writeIBOIndex(iboIndex + i);
        }
      }
      iboIndex += points.size();
    }
  }
}

void ShowFieldModule::renderNodes(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  RenderState state,
  Core::Datatypes::GeometryHandle geom,
  const std::string& id)
{

  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  double sval;
  Core::Geometry::Vector vval;
  Core::Geometry::Tensor tval;

  GeometryObject::ColorScheme colorScheme = GeometryObject::COLOR_UNIFORM;
  ColorRGB node_color;

  if (fld->basis_order() < 0 ||
    (fld->basis_order() == 0 && mesh->dimensionality() != 0) ||
    state.get(RenderState::USE_DEFAULT_COLOR))
    colorScheme = GeometryObject::COLOR_UNIFORM;
  else if (state.get(RenderState::USE_COLORMAP))
    colorScheme = GeometryObject::COLOR_MAP;
  else
    colorScheme = GeometryObject::COLOR_IN_SITU;

  mesh->synchronize(Mesh::NODES_E);

  VMesh::Node::iterator eiter, eiter_end;
  mesh->begin(eiter);
  mesh->end(eiter_end);

  auto my_state = this->get_state();
  double radius = my_state->getValue(SphereScaleValue).toDouble();
  double num_strips = static_cast<double>(my_state->getValue(SphereResolution).toInt());
  if (radius < 0) radius = 1.;
  if (num_strips < 0) num_strips = 10.;
  std::stringstream ss;
  ss << state.get(RenderState::USE_SPHERE) << radius << num_strips << colorScheme;

  std::string uniqueNodeID = id + "node" + ss.str();

  auto st = get_state();
  nodeTransparencyValue_ = static_cast<float>(st->getValue(NodeTransparencyValue).toDouble());

  GeometryObject::SpireIBO::PRIMITIVE primIn = GeometryObject::SpireIBO::POINTS;
  // Use spheres...
  if (state.get(RenderState::USE_SPHERE))
    primIn = GeometryObject::SpireIBO::TRIANGLES;

  GlyphGeom glyphs;
  while (eiter != eiter_end)
  {
    checkForInterruption();

    Core::Geometry::Point p;
    mesh->get_point(p, *eiter);
    //coloring options
    if (colorScheme != GeometryObject::COLOR_UNIFORM)
    {
      ColorMapHandle map = colorMap.get();
      if (fld->is_scalar())
      {
        fld->get_value(sval, *eiter);
        node_color = map->valueToColor(sval);
      }
      else if (fld->is_vector())
      {
        fld->get_value(vval, *eiter);
        node_color = map->valueToColor(vval);
      }
      else if (fld->is_tensor())
      {
        fld->get_value(tval, *eiter);
        node_color = map->valueToColor(tval);
      }
    }
    //accumulate VBO or IBO data
    if (state.get(RenderState::USE_SPHERE))
    {
      glyphs.addSphere(p, radius, num_strips, node_color);
    }
    else
    {
      glyphs.addPoint(p, node_color);
    }

    ++eiter;
  }

  glyphs.buildObject(geom, uniqueNodeID, state.get(RenderState::USE_TRANSPARENT_NODES), nodeTransparencyValue_,
    colorScheme, state, primIn, mesh->get_bounding_box());
}


void ShowFieldModule::renderEdges(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  RenderState state,
  Core::Datatypes::GeometryHandle geom,
  const std::string& id)
{
  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  double sval0, sval1;
  Core::Geometry::Vector vval0, vval1;
  Core::Geometry::Tensor tval0, tval1;

  GeometryObject::ColorScheme colorScheme = GeometryObject::COLOR_UNIFORM;
  ColorRGB edge_colors[2];

  if (fld->basis_order() < 0 ||
    (fld->basis_order() == 0 && mesh->dimensionality() != 0) ||
    state.get(RenderState::USE_DEFAULT_COLOR))
    colorScheme = GeometryObject::COLOR_UNIFORM;
  else if (state.get(RenderState::USE_COLORMAP))
    colorScheme = GeometryObject::COLOR_MAP;
  else
    colorScheme = GeometryObject::COLOR_IN_SITU;

  mesh->synchronize(Mesh::EDGES_E);

  VMesh::Edge::iterator eiter, eiter_end;
  mesh->begin(eiter);
  mesh->end(eiter_end);

  auto my_state = this->get_state();
  double num_strips = double(my_state->getValue(CylinderResolution).toInt());
  double radius = my_state->getValue(CylinderRadius).toDouble();
  if (num_strips < 0) num_strips = 50.;
  if (radius < 0) radius = 1.;

  std::stringstream ss;
  ss << state.get(RenderState::USE_CYLINDER) << num_strips << radius << colorScheme;

  std::string uniqueNodeID = id + "edge" + ss.str();

  GeometryObject::SpireIBO::PRIMITIVE primIn = GeometryObject::SpireIBO::LINES;
  // Use cylinders...
  if (state.get(RenderState::USE_CYLINDER))
    primIn = GeometryObject::SpireIBO::TRIANGLES;

  GlyphGeom glyphs;
  while (eiter != eiter_end)
  {
    checkForInterruption();

    VMesh::Node::array_type nodes;
    mesh->get_nodes(nodes, *eiter);

    Core::Geometry::Point p0, p1;
    mesh->get_point(p0, nodes[0]);
    mesh->get_point(p1, nodes[1]);
    //coloring options
    if (colorScheme != GeometryObject::COLOR_UNIFORM)
    {
      ColorMapHandle map = colorMap.get();
      if (fld->is_scalar())
      {
        if (fld->basis_order() == 1)
        {
          fld->get_value(sval0, nodes[0]);
          fld->get_value(sval1, nodes[1]);
        }
        else //if (mesh->dimensionality() == 1)
        {
          fld->get_value(sval0, *eiter);

          sval1 = sval0;
        }
        edge_colors[0] = map->valueToColor(sval0);
        edge_colors[1] = map->valueToColor(sval1);
      }
      else if (fld->is_vector())
      {
        if (fld->basis_order() == 1)
        {
          fld->get_value(vval0, nodes[0]);
          fld->get_value(vval1, nodes[1]);
        }
        else //if (mesh->dimensionality() == 1)
        {
          fld->get_value(vval0, *eiter);
          vval1 = vval0;
        }

        edge_colors[0] = map->valueToColor(vval0);
        edge_colors[1] = map->valueToColor(vval1);
      }
      else if (fld->is_tensor())
      {
        if (fld->basis_order() == 1)
        {
          fld->get_value(tval0, nodes[0]);
          fld->get_value(tval1, nodes[1]);
        }
        else //if (mesh->dimensionality() == 1)
        {
          fld->get_value(tval0, *eiter);
          tval1 = tval0;
        }

        edge_colors[0] = map->valueToColor(tval0);
        edge_colors[1] = map->valueToColor(tval1);
      }
    }
    //accumulate VBO or IBO data
    if (state.get(RenderState::USE_CYLINDER) && p0 != p1)
    {
      glyphs.addCylinder(p0, p1, radius, num_strips, edge_colors[0], edge_colors[1]);
      glyphs.addSphere(p0, radius, num_strips, edge_colors[0]);
      glyphs.addSphere(p1, radius, num_strips, edge_colors[1]);
    }
    else
    {
      glyphs.addNeedle(p0, p1, edge_colors[0], edge_colors[1]);
    }

    ++eiter;
  }

  glyphs.buildObject(geom, uniqueNodeID, state.get(RenderState::USE_TRANSPARENT_EDGES), edgeTransparencyValue_,
    colorScheme, state, primIn, mesh->get_bounding_box());
}

AlgorithmParameterName ShowFieldModule::ShowNodes("ShowNodes");
AlgorithmParameterName ShowFieldModule::ShowEdges("ShowEdges");
AlgorithmParameterName ShowFieldModule::ShowFaces("ShowFaces");
AlgorithmParameterName ShowFieldModule::NodeTransparency("NodeTransparency");
AlgorithmParameterName ShowFieldModule::EdgeTransparency("EdgeTransparency");
AlgorithmParameterName ShowFieldModule::FaceTransparency("FaceTransparency");
AlgorithmParameterName ShowFieldModule::FaceInvertNormals("FaceInvertNormals");
AlgorithmParameterName ShowFieldModule::NodeAsPoints("NodeAsPoints");
AlgorithmParameterName ShowFieldModule::NodeAsSpheres("NodeAsSpheres");
AlgorithmParameterName ShowFieldModule::EdgesAsLines("EdgesAsLines");
AlgorithmParameterName ShowFieldModule::EdgesAsCylinders("EdgesAsCylinders");
AlgorithmParameterName ShowFieldModule::DefaultMeshColor("DefaultMeshColor");
AlgorithmParameterName ShowFieldModule::FaceTransparencyValue("FaceTransparencyValue");
AlgorithmParameterName ShowFieldModule::EdgeTransparencyValue("EdgeTransparencyValue");
AlgorithmParameterName ShowFieldModule::NodeTransparencyValue("NodeTransparencyValue");
AlgorithmParameterName ShowFieldModule::SphereScaleValue("SphereScaleValue");
AlgorithmParameterName ShowFieldModule::CylinderRadius("CylinderRadius");
AlgorithmParameterName ShowFieldModule::CylinderResolution("CylinderResolution");
AlgorithmParameterName ShowFieldModule::SphereResolution("SphereResolution");
