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
#include <Core/Algorithms/Visualization/RenderFieldState.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Graphics/Glyphs/GlyphGeom.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core;
using namespace Datatypes;
using namespace Thread;
using namespace Dataflow::Networks;
using namespace Algorithms;
using namespace Visualization;
using namespace Geometry;
using namespace Graphics;
using namespace Graphics::Datatypes;

ALGORITHM_PARAMETER_DEF(Visualization, CylinderRadius);

const ModuleLookupInfo ShowFieldModule::staticInfo_("ShowField", "Visualization", "SCIRun");

namespace SCIRun {
  namespace Modules {
    namespace Visualization {
namespace detail
{
class GeometryBuilder
{
public:
  //GeometryBuilder();
  /// Constructs a geometry object (essentially a spire object) from the given
  /// field data.
  GeometryHandle buildGeometryObject(
    FieldHandle field,
    boost::optional<ColorMapHandle> colorMap,
    ModuleStateHandle state,
    const GeometryIDGenerator& gid,
    Interruptible* interruptible);

  /// Mesh construction. Any of the functions below can modify the renderState.
  /// This modified render state will be passed onto the renderer.
  void renderNodes(
    FieldHandle field,
    boost::optional<ColorMapHandle> colorMap,
    ModuleStateHandle moduleState,
    Interruptible* interruptible,
    RenderState state, GeometryHandle geom,
    const std::string& id);

  void renderFaces(
    FieldHandle field,
    boost::optional<ColorMapHandle> colorMap,
    ModuleStateHandle moduleState,
    Interruptible* interruptible,
    RenderState state, GeometryHandle geom,
    unsigned int approx_div,
    const std::string& id);

  void renderFacesLinear(
    FieldHandle field,
    boost::optional<ColorMapHandle> colorMap,
    ModuleStateHandle moduleState,
    Interruptible* interruptible,
    RenderState state, GeometryHandle geom,
    unsigned int approxDiv,
    const std::string& id);

  void addFaceGeom(
    const std::vector<Point>  &points,
    const std::vector<Vector> &normals,
    bool withNormals,
    uint32_t& iboBufferIndex,
    CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer,
    CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer,
    ColorScheme colorScheme,
    const std::vector<ColorRGB> &face_colors,
    const RenderState& state);

  void renderEdges(
    FieldHandle field,
    boost::optional<ColorMapHandle> colorMap,
    ModuleStateHandle moduleState,
    Interruptible* interruptible,
    RenderState state,
    GeometryHandle geom,
    const std::string& id);

  RenderState getNodeRenderState(
    ModuleStateHandle state,
    boost::optional<ColorMapHandle> colorMap);

  RenderState getEdgeRenderState(
    ModuleStateHandle state,
    boost::optional<ColorMapHandle> colorMap);

  RenderState getFaceRenderState(
    ModuleStateHandle state,
    boost::optional<ColorMapHandle> colorMap);
private:
  float faceTransparencyValue_ = 0.65f;
  float edgeTransparencyValue_ = 0.65f;
  float nodeTransparencyValue_ = 0.65f;
};
}}}}

using namespace detail;

ShowFieldModule::ShowFieldModule() : GeometryGeneratingModule(staticInfo_),
  builder_(new GeometryBuilder)
{
  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(SceneGraph);
}

void ShowFieldModule::setStateDefaults()
{
  auto state = get_state();
  state->setValue(NodesAvailable, true);
  state->setValue(EdgesAvailable, true);
  state->setValue(FacesAvailable, true);

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

  state->setValue(DefaultTextColor, ColorRGB(1.0, 1.0, 1.0).toString());
  state->setValue(ShowText, false);
  state->setValue(ShowDataValues, true);
  state->setValue(ShowNodeIndices, false);
  state->setValue(ShowEdgeIndices, false);
  state->setValue(ShowFaceIndices, false);
  state->setValue(ShowCellIndices, false);
  state->setValue(CullBackfacingText, false);
  state->setValue(TextAlwaysVisible, false);
  state->setValue(RenderAsLocation, false);
  state->setValue(TextSize, 8);
  state->setValue(TextPrecision, 3);
  state->setValue(TextColoring, 0);

  state->setValue(UseFaceNormals, false);
  state->setValue(FaceInvertNormals, false);

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
    updateAvailableRenderOptions(field);
    GeometryHandle geom = builder_->buildGeometryObject(field, colorMap, get_state(), *this, this);
    sendOutput(SceneGraph, geom);
  }
}

RenderState GeometryBuilder::getNodeRenderState(
  ModuleStateHandle state,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap)
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

RenderState GeometryBuilder::getEdgeRenderState(
  ModuleStateHandle state,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap)
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

  edgeTransparencyValue_ = static_cast<float>(state->getValue(ShowFieldModule::EdgeTransparencyValue).toDouble());

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

RenderState GeometryBuilder::getFaceRenderState(
  ModuleStateHandle state,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap)
{
  RenderState renState;

  renState.set(RenderState::IS_ON, state->getValue(ShowFieldModule::ShowFaces).toBool());
  renState.set(RenderState::USE_TRANSPARENCY, state->getValue(ShowFieldModule::FaceTransparency).toBool()); 
  renState.set(RenderState::USE_FACE_NORMALS, state->getValue(ShowFieldModule::UseFaceNormals).toBool());

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldModule::DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
                           renState.defaultColor.g() > 1.0 ||
                           renState.defaultColor.b() > 1.0)?
                                ColorRGB(
                                renState.defaultColor.r() / 255.,
                                renState.defaultColor.g() / 255.,
                                renState.defaultColor.b() / 255.)
                            :   renState.defaultColor;

  faceTransparencyValue_ = static_cast<float>(state->getValue(ShowFieldModule::FaceTransparencyValue).toDouble());

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

GeometryHandle GeometryBuilder::buildGeometryObject(
  boost::shared_ptr<Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  ModuleStateHandle state, const GeometryIDGenerator& gid, Interruptible* interruptible)
{
  // Function for reporting progress. TODO: use this variable somewhere!
  //auto progressFunc = getUpdaterFunc();

  /// \todo Determine a better way of handling all of the various object state.
  bool showNodes = state->getValue(ShowFieldModule::ShowNodes).toBool();
  bool showEdges = state->getValue(ShowFieldModule::ShowEdges).toBool();
  bool showFaces = state->getValue(ShowFieldModule::ShowFaces).toBool();
  // Resultant geometry type (representing a spire object and a number of passes).

  std::string idname = "EntireField";
  if (state->getValue(ShowFieldModule::FieldName).toString() != "")
  {
    idname = idname + "_" + state->getValue(ShowFieldModule::FieldName).toString();
  }

  GeometryHandle geom(new GeometryObjectSpire(gid, idname, true));

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
    renderNodes(field, colorMap, state, interruptible, getNodeRenderState(state, colorMap), geom, geom->uniqueID());
  }

  if (showFaces)
  {
    int approxDiv = 1;
    renderFaces(field, colorMap, state, interruptible, getFaceRenderState(state, colorMap), geom, approxDiv, geom->uniqueID());
  }

  if (showEdges)
  {
    renderEdges(field, colorMap, state, interruptible, getEdgeRenderState(state, colorMap), geom, geom->uniqueID());
  }

  return geom;
}


void GeometryBuilder::renderFaces(
  boost::shared_ptr<Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  ModuleStateHandle moduleState,
  Interruptible* interruptible,
  RenderState state, GeometryHandle geom,
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
    return renderFacesLinear(field, colorMap, moduleState, interruptible, state, geom, approxDiv, id);
  }
  else
  {
    std::cout << "Non linear faces not supported at this time." << std::endl;
  }
}


void GeometryBuilder::renderFacesLinear(
  boost::shared_ptr<Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  ModuleStateHandle moduleState,
  Interruptible* interruptible,
  RenderState state,
  GeometryHandle geom,
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

  bool invertNormals = moduleState->getValue(ShowFieldModule::FaceInvertNormals).toBool();
  ColorScheme colorScheme = ColorScheme::COLOR_UNIFORM;
  std::vector<double> svals;
  std::vector<Vector> vvals;
  std::vector<Tensor> tvals;
  std::vector<ColorRGB> face_colors;

  if (fld->basis_order() < 0 || state.get(RenderState::USE_DEFAULT_COLOR))
  {
    colorScheme = ColorScheme::COLOR_UNIFORM;
  }
  else if (state.get(RenderState::USE_COLORMAP))
  {
    colorScheme = ColorScheme::COLOR_MAP;
  }
  else // if (fld->basis_order() >= 0)
  {
    colorScheme = ColorScheme::COLOR_IN_SITU;
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

  Point idpt;
  mesh->get_nodes(nodes, *fiter);
  mesh->get_point(idpt, nodes[0]);

  while (fiter != fiterEnd)
  {
    interruptible->checkForInterruption();

    mesh->get_nodes(nodes, *fiter);

    std::vector<Point> points(nodes.size());
    std::vector<Vector> normals(nodes.size());

    //std::cout << "Node Size: " << nodes.size() << std::endl;

    for (size_t i = 0; i < nodes.size(); i++)
    {
      mesh->get_point(points[i], nodes[i]);
    }

    //TODO fix so the withNormals tp be woth lighting is called correctly, and the meshes are fixed.
    if (withNormals)
    {
      bool useFaceNormals = state.get(RenderState::USE_FACE_NORMALS) && mesh->has_normals();
      if (useFaceNormals)
      {
        for (size_t i = 0; i < nodes.size(); i++)
        {
          auto norm = normals[i];
          normals[i] = invertNormals ? -norm : norm;
          mesh->get_normal(normals[i], nodes[i]);
        }
      }
      else
      {
        /// Fix normal of Quads
        if (points.size() == 4)
        {
          Vector edge1 = points[1] - points[0];
          Vector edge2 = points[2] - points[1];
          Vector edge3 = points[3] - points[2];
          Vector edge4 = points[0] - points[3];

          Vector norm = Cross(edge1, edge2) + Cross(edge2, edge3) + Cross(edge3, edge4) + Cross(edge4, edge1);

          norm.normalize();

          for (size_t i = 0; i < nodes.size(); i++)
          {
            normals[i] = invertNormals ? -norm : norm;
          }
        }
        /// Fix Normals of Tris
        else
        {
          Vector edge1 = points[1] - points[0];
          Vector edge2 = points[2] - points[1];
          Vector norm = Cross(edge1, edge2);

          norm.normalize();

          for (size_t i = 0; i < nodes.size(); i++)
          {
            normals[i] = invertNormals ? -norm : norm;
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
    }
    // Default color single face no matter the element data.
    if (colorScheme == ColorScheme::COLOR_UNIFORM)
    {
      addFaceGeom(points, normals, withNormals, iboIndex, iboBuffer, vboBuffer,
        colorScheme, face_colors, state);
    }
    // Element data (Cells) so two sided faces.
    else if (fld->basis_order() == 0 && mesh->dimensionality() == 3)
    {
      auto map = colorMap.get();
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
      auto map = colorMap.get();
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
      auto map = colorMap.get();
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
  ss << invertNormals << static_cast<int>(colorScheme) << faceTransparencyValue_;

  std::string uniqueNodeID = id + "face" + ss.str();
  std::string vboName = uniqueNodeID + "VBO";
  std::string iboName = uniqueNodeID + "IBO";
  std::string passName = uniqueNodeID + "Pass";

  // NOTE: Attributes will depend on the color scheme. We will want to
  // normalize the colors if the color scheme is COLOR_IN_SITU.

  // Construct VBO.
  std::string shader = "Shaders/UniformColor";
  std::vector<SpireVBO::AttributeData> attribs;
  attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  std::vector<SpireSubPass::Uniform> uniforms;
  if (withNormals)
  {
    attribs.push_back(SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
  }

  if (state.get(RenderState::USE_TRANSPARENCY))
    uniforms.push_back(SpireSubPass::Uniform("uTransparency", faceTransparencyValue_));

  if (colorScheme == ColorScheme::COLOR_MAP)
  {
    attribs.push_back(SpireVBO::AttributeData("aColor", 4 * sizeof(float)));

    if (!state.get(RenderState::IS_DOUBLE_SIDED))
    {
      if (withNormals)
      {
        // Use colormapping lit shader.
        shader = "Shaders/DirPhongCMap";
        uniforms.push_back(SpireSubPass::Uniform("uAmbientColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(SpireSubPass::Uniform("uSpecularColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(SpireSubPass::Uniform("uSpecularPower", 32.0f));
      }
      else
      {
        // Use colormapping only shader.
        shader = "Shaders/ColorMap";
      }
    }
    else
    {
      attribs.push_back(SpireVBO::AttributeData("aColorSecondary", 4 * sizeof(float)));

      if (withNormals)
      {
        // Use colormapping lit shader.
        shader = "Shaders/DblSided_DirPhongCMap";
        uniforms.push_back(SpireSubPass::Uniform("uAmbientColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(SpireSubPass::Uniform("uSpecularColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(SpireSubPass::Uniform("uSpecularPower", 32.0f));
      }
      else
      {
        // Use colormapping only shader.
        shader = "Shaders/DblSided_ColorMap";
      }
    }
  }
  else if (colorScheme == ColorScheme::COLOR_IN_SITU)
  {
    attribs.push_back(SpireVBO::AttributeData("aColor", 4 * sizeof(float), true));

    if (state.get(RenderState::IS_DOUBLE_SIDED) == false)
    {
      if (withNormals)
      {
        shader = "Shaders/InSituPhongColor";
        uniforms.push_back(SpireSubPass::Uniform("uAmbientColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(SpireSubPass::Uniform("uSpecularColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(SpireSubPass::Uniform("uSpecularPower", 32.0f));
      }
      else
      {
        // Use colormapping shader.
        shader = "Shaders/InSituColor";
      }
    }
    else
    {
      attribs.push_back(SpireVBO::AttributeData("aColorSecondary", 4 * sizeof(float), true));

      if (withNormals)
      {
        shader = "Shaders/DblSided_InSituPhongColor";
        uniforms.push_back(SpireSubPass::Uniform("uAmbientColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(SpireSubPass::Uniform("uSpecularColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(SpireSubPass::Uniform("uSpecularPower", 32.0f));
      }
      else
      {
        // Use colormapping shader.
        shader = "Shaders/DblSided_InSituColor";
      }
    }
  }
  else if (colorScheme == ColorScheme::COLOR_UNIFORM)
  {
    ColorRGB defaultColor = state.defaultColor;

    if (withNormals)
    {
      shader = "Shaders/DirPhong";
      uniforms.push_back(SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(SpireSubPass::Uniform("uSpecularPower", 32.0f));
      uniforms.push_back(SpireSubPass::Uniform(
        "uDiffuseColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), 1.0f)));
    }
    else
    {
      shader = "Shaders/UniformColor";
      uniforms.push_back(SpireSubPass::Uniform(
        "uColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), 1.0f)));
    }
  }

  SpireVBO geomVBO(vboName, attribs, vboBufferSPtr,
    numVBOElements, mesh->get_bounding_box(), true);

  geom->mVBOs.push_back(geomVBO);

  // Construct IBO.

  SpireIBO geomIBO(iboName, SpireIBO::PRIMITIVE::TRIANGLES, sizeof(uint32_t), iboBufferSPtr);

  geom->mIBOs.push_back(geomIBO);

  SpireText text;

  SpireSubPass pass(passName, vboName, iboName, shader,
    colorScheme, state, RenderType::RENDER_VBO_IBO, geomVBO, geomIBO, text);

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
void GeometryBuilder::addFaceGeom(
  const std::vector<Point>  &points,
  const std::vector<Vector> &normals,
  bool withNormals,
  uint32_t& iboIndex,
  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer,
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer,
  ColorScheme colorScheme,
  const std::vector<ColorRGB> &face_colors,
  const RenderState& state)
{
  auto writeVBOPoint = [&vboBuffer](const Point& point)
  {
    vboBuffer->write(static_cast<float>(point.x()));
    vboBuffer->write(static_cast<float>(point.y()));
    vboBuffer->write(static_cast<float>(point.z()));
  };

  auto writeVBONormal = [&vboBuffer](const Vector& normal)
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

  if (colorScheme == ColorScheme::COLOR_UNIFORM)
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
  else if (colorScheme == ColorScheme::COLOR_MAP)
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
  else if (colorScheme == ColorScheme::COLOR_IN_SITU)
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

void GeometryBuilder::renderNodes(
  boost::shared_ptr<Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  ModuleStateHandle moduleState,
  Interruptible* interruptible,
  RenderState state,
  GeometryHandle geom,
  const std::string& id)
{

  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  double sval;
  Vector vval;
  Tensor tval;

  ColorScheme colorScheme;
  ColorRGB node_color;

  if (fld->basis_order() < 0 || (fld->basis_order() == 0 && mesh->dimensionality() != 0) || state.get(RenderState::USE_DEFAULT_COLOR))
    colorScheme = ColorScheme::COLOR_UNIFORM;
  else if (state.get(RenderState::USE_COLORMAP))
    colorScheme = ColorScheme::COLOR_MAP;
  else
    colorScheme = ColorScheme::COLOR_IN_SITU;

  mesh->synchronize(Mesh::NODES_E);

  VMesh::Node::iterator eiter, eiter_end;
  mesh->begin(eiter);
  mesh->end(eiter_end);

  double radius = moduleState->getValue(ShowFieldModule::SphereScaleValue).toDouble();
  double num_strips = static_cast<double>(moduleState->getValue(ShowFieldModule::SphereResolution).toInt());
  if (radius < 0) radius = 1.;
  if (num_strips < 0) num_strips = 10.;
  std::stringstream ss;
  ss << state.get(RenderState::USE_SPHERE) << radius << num_strips << static_cast<int>(colorScheme);

  std::string uniqueNodeID = id + "node" + ss.str();

  nodeTransparencyValue_ = static_cast<float>(moduleState->getValue(ShowFieldModule::NodeTransparencyValue).toDouble());

  SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::POINTS;
  // Use spheres...
  if (state.get(RenderState::USE_SPHERE))
    primIn = SpireIBO::PRIMITIVE::TRIANGLES;

  GlyphGeom glyphs;
  while (eiter != eiter_end)
  {
    interruptible->checkForInterruption();

    Point p;
    mesh->get_point(p, *eiter);
    //coloring options
    if (colorScheme != ColorScheme::COLOR_UNIFORM)
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


void GeometryBuilder::renderEdges(
  boost::shared_ptr<Field> field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  ModuleStateHandle moduleState,
  Interruptible* interruptible,
  RenderState state,
  GeometryHandle geom,
  const std::string& id)
{
  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  double sval0, sval1;
  Vector vval0, vval1;
  Tensor tval0, tval1;

  ColorScheme colorScheme;
  ColorRGB edge_colors[2];

  if (fld->basis_order() < 0 ||
    (fld->basis_order() == 0 && mesh->dimensionality() != 0) ||
    state.get(RenderState::USE_DEFAULT_COLOR))
    colorScheme = ColorScheme::COLOR_UNIFORM;
  else if (state.get(RenderState::USE_COLORMAP))
    colorScheme = ColorScheme::COLOR_MAP;
  else
    colorScheme = ColorScheme::COLOR_IN_SITU;

  mesh->synchronize(Mesh::EDGES_E);

  VMesh::Edge::iterator eiter, eiter_end;
  mesh->begin(eiter);
  mesh->end(eiter_end);

  double num_strips = static_cast<double>(moduleState->getValue(ShowFieldModule::CylinderResolution).toInt());
  double radius = moduleState->getValue(ShowFieldModule::CylinderRadius).toDouble();
  if (num_strips < 0) num_strips = 50.;
  if (radius < 0) radius = 1.;

  std::stringstream ss;
  ss << state.get(RenderState::USE_CYLINDER) << num_strips << radius << static_cast<int>(colorScheme);

  std::string uniqueNodeID = id + "edge" + ss.str();

  SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::LINES;
  // Use cylinders...
  if (state.get(RenderState::USE_CYLINDER))
    primIn = SpireIBO::PRIMITIVE::TRIANGLES;

  GlyphGeom glyphs;
  while (eiter != eiter_end)
  {
    interruptible->checkForInterruption();

    VMesh::Node::array_type nodes;
    mesh->get_nodes(nodes, *eiter);

    Point p0, p1;
    mesh->get_point(p0, nodes[0]);
    mesh->get_point(p1, nodes[1]);
    //coloring options
    if (colorScheme != ColorScheme::COLOR_UNIFORM)
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
      glyphs.addLine(p0, p1, edge_colors[0], edge_colors[1]);
    }

    ++eiter;
  }

  glyphs.buildObject(geom, uniqueNodeID, state.get(RenderState::USE_TRANSPARENT_EDGES), edgeTransparencyValue_,
    colorScheme, state, primIn, mesh->get_bounding_box());
}

void ShowFieldModule::updateAvailableRenderOptions(FieldHandle field)
{
  if (!field)
    return;

  auto vmesh = field->vmesh();
  if (!vmesh)
    return;

  auto state = get_state();
  vmesh->synchronize(Mesh::NODES_E);
  state->setValue(NodesAvailable, 0 != vmesh->num_nodes());
  vmesh->synchronize(Mesh::EDGES_E);
  state->setValue(EdgesAvailable, 0 != vmesh->num_edges());
  vmesh->synchronize(Mesh::FACES_E);
  state->setValue(FacesAvailable, 0 != vmesh->num_faces());
  if (vmesh->is_pointcloudmesh())
  {
    state->setValue(ShowNodes, true);
    state->setValue(ShowEdges, false);
    state->setValue(ShowFaces, false);
  }
  else if (0 == vmesh->num_edges())
  {
    state->setValue(ShowEdges, false);
  }
  else if (0 == vmesh->num_faces())
  {
    state->setValue(ShowFaces, false);
  }
}

const AlgorithmParameterName ShowFieldModule::FieldName("FieldName");
const AlgorithmParameterName ShowFieldModule::ShowNodes("ShowNodes");
const AlgorithmParameterName ShowFieldModule::ShowEdges("ShowEdges");
const AlgorithmParameterName ShowFieldModule::ShowFaces("ShowFaces");
const AlgorithmParameterName ShowFieldModule::NodesAvailable("NodesAvailable");
const AlgorithmParameterName ShowFieldModule::EdgesAvailable("EdgesAvailable");
const AlgorithmParameterName ShowFieldModule::FacesAvailable("FacesAvailable");
const AlgorithmParameterName ShowFieldModule::NodeTransparency("NodeTransparency");
const AlgorithmParameterName ShowFieldModule::EdgeTransparency("EdgeTransparency");
const AlgorithmParameterName ShowFieldModule::FaceTransparency("FaceTransparency");
const AlgorithmParameterName ShowFieldModule::FaceInvertNormals("FaceInvertNormals");
const AlgorithmParameterName ShowFieldModule::NodeAsPoints("NodeAsPoints");
const AlgorithmParameterName ShowFieldModule::NodeAsSpheres("NodeAsSpheres");
const AlgorithmParameterName ShowFieldModule::EdgesAsLines("EdgesAsLines");
const AlgorithmParameterName ShowFieldModule::EdgesAsCylinders("EdgesAsCylinders");
const AlgorithmParameterName ShowFieldModule::DefaultMeshColor("DefaultMeshColor");
const AlgorithmParameterName ShowFieldModule::FaceTransparencyValue("FaceTransparencyValue");
const AlgorithmParameterName ShowFieldModule::EdgeTransparencyValue("EdgeTransparencyValue");
const AlgorithmParameterName ShowFieldModule::NodeTransparencyValue("NodeTransparencyValue");
const AlgorithmParameterName ShowFieldModule::SphereScaleValue("SphereScaleValue");
const AlgorithmParameterName ShowFieldModule::CylinderRadius("CylinderRadius");
const AlgorithmParameterName ShowFieldModule::CylinderResolution("CylinderResolution");
const AlgorithmParameterName ShowFieldModule::SphereResolution("SphereResolution");
const AlgorithmParameterName ShowFieldModule::DefaultTextColor("DefaultTextColor");
const AlgorithmParameterName ShowFieldModule::ShowText("ShowText");
const AlgorithmParameterName ShowFieldModule::ShowDataValues("ShowDataValues");
const AlgorithmParameterName ShowFieldModule::ShowNodeIndices("ShowNodeIndices");
const AlgorithmParameterName ShowFieldModule::ShowEdgeIndices("ShowEdgeIndices");
const AlgorithmParameterName ShowFieldModule::ShowFaceIndices("ShowFaceIndices");
const AlgorithmParameterName ShowFieldModule::ShowCellIndices("ShowCellIndices");
const AlgorithmParameterName ShowFieldModule::CullBackfacingText("CullBackfacingText");
const AlgorithmParameterName ShowFieldModule::TextAlwaysVisible("TextAlwaysVisible");
const AlgorithmParameterName ShowFieldModule::RenderAsLocation("RenderAsLocation");
const AlgorithmParameterName ShowFieldModule::TextSize("TextSize");
const AlgorithmParameterName ShowFieldModule::TextPrecision("TextPrecision");
const AlgorithmParameterName ShowFieldModule::TextColoring("TextColoring");
const AlgorithmParameterName ShowFieldModule::UseFaceNormals("UseFaceNormals");
