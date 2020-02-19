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
using namespace Algorithms::Visualization::Parameters;
using namespace Visualization;
using namespace Geometry;
using namespace Graphics;
using namespace Graphics::Datatypes;

ALGORITHM_PARAMETER_DEF(Visualization, CylinderRadius);

MODULE_INFO_DEF(ShowField, Visualization, SCIRun)

namespace SCIRun {
  namespace Modules {
    namespace Visualization {
namespace detail
{
class GeometryBuilder
{
public:
  GeometryBuilder(const std::string& moduleId, ModuleStateHandle state) : moduleId_(moduleId), state_(state) {}
  /// Constructs a geometry object (essentially a spire object) from the given
  /// field data.
  GeometryHandle buildGeometryObject(
    FieldHandle field,
    boost::optional<ColorMapHandle> colorMap,
    const GeometryIDGenerator& gid,
    Interruptible* interruptible);

  /// Mesh construction. Any of the functions below can modify the renderState.
  /// This modified render state will be passed onto the renderer.
  void renderNodes(
    FieldHandle field,
    boost::optional<ColorMapHandle> colorMap,
    Interruptible* interruptible,
    RenderState state, GeometryHandle geom,
    const std::string& id);

  void renderFaces(
    FieldHandle field,
    boost::optional<ColorMapHandle> colorMap,
    Interruptible* interruptible,
    RenderState state, GeometryHandle geom,
    const std::string& id);

  void renderFacesLinear(
    FieldHandle field,
    boost::optional<ColorMapHandle> colorMap,
    Interruptible* interruptible,
    RenderState state, GeometryHandle geom,
    const std::string& id);

  void addFaceGeom(
    const std::vector<Point>  &points,
    const std::vector<Vector> &normals,
    bool withNormals,
    uint32_t& iboBufferIndex,
    spire::VarBuffer* iboBuffer,
    spire::VarBuffer* vboBuffer,
    ColorScheme colorScheme,
    const std::vector<ColorRGB> &textureCoords,
    const RenderState& state);

  void renderEdges(
    FieldHandle field,
    boost::optional<ColorMapHandle> colorMap,
    Interruptible* interruptible,
    RenderState state,
    GeometryHandle geom,
    const std::string& id);

  RenderState getNodeRenderState(boost::optional<ColorMapHandle> colorMap);
  RenderState getEdgeRenderState(boost::optional<ColorMapHandle> colorMap);
  RenderState getFaceRenderState(boost::optional<ColorMapHandle> colorMap);
private:
  float faceTransparencyValue_ = 0.65f;
  float edgeTransparencyValue_ = 0.65f;
  float nodeTransparencyValue_ = 0.65f;
  std::string moduleId_;
  ModuleStateHandle state_;
};
}}}}

using namespace detail;

ShowField::ShowField() : GeometryGeneratingModule(staticInfo_),
  builder_(new GeometryBuilder(id().id_, get_state()))
{
  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(SceneGraph);
  //INITIALIZE_PORT(OspraySceneGraph);
}

void ShowField::setStateDefaults()
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
  state->setValue(FacesColoring, 1);
  state->setValue(NodesColoring, 1);
  state->setValue(EdgesColoring, 1);
  state->setValue(SphereScaleValue, 0.03);
  state->setValue(SphereResolution, 5);
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

  state->setValue(FieldName, std::string());

  // NOTE: We need to add radio buttons for USE_DEFAULT_COLOR, COLORMAP, and
  // COLOR_CONVERT. USE_DEFAULT_COLOR is selected by default. COLOR_CONVERT
  // is more up in the air.

  getOutputPort(SceneGraph)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processMeshComponentSelection(var); });
}

void ShowField::processMeshComponentSelection(const ModuleFeedback& var)
{
  try
  {
    auto sel = dynamic_cast<const MeshComponentSelectionFeedback&>(var);
    if (sel.moduleId == id().id_)
    {
    get_state()->setValue(Name("Show" + sel.component), sel.selected);
    enqueueExecuteAgain(false);
    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
}

void ShowField::execute()
{
  auto field = getRequiredInput(Field);
  auto colorMap = getOptionalInput(ColorMapObject);

  if (needToExecute())
  {
    updateAvailableRenderOptions(field);
    auto geom = builder_->buildGeometryObject(field, colorMap, *this, this);
    sendOutput(SceneGraph, geom);
  }
}

RenderState GeometryBuilder::getNodeRenderState(
  boost::optional<boost::shared_ptr<ColorMap>> colorMap)
{
  RenderState renState;

  bool useColorMap = state_->getValue(NodesColoring).toInt() == 1;
  bool rgbConversion = state_->getValue(NodesColoring).toInt() == 2;
  renState.set(RenderState::IS_ON, state_->getValue(ShowNodes).toBool());
  renState.set(RenderState::USE_TRANSPARENT_NODES, state_->getValue(NodeTransparency).toBool());

  renState.set(RenderState::USE_SPHERE, state_->getValue(NodeAsSpheres).toInt() == 1);

  renState.defaultColor = ColorRGB(state_->getValue(DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
                           renState.defaultColor.g() > 1.0 ||
                           renState.defaultColor.b() > 1.0)?
                                ColorRGB(
                                renState.defaultColor.r() / 255.,
                                renState.defaultColor.g() / 255.,
                                renState.defaultColor.b() / 255.)
                            :   renState.defaultColor;

  if (colorMap && useColorMap)
  {
    renState.set(RenderState::USE_COLORMAP_ON_NODES, true);
  }
  else if (rgbConversion)
  {
    renState.set(RenderState::USE_COLOR_CONVERT_ON_NODES, true);
  }
  else
  {
    renState.set(RenderState::USE_DEFAULT_COLOR_NODES, true);
    state_->setValue(NodesColoring, 0);
  }

  return renState;
}

RenderState GeometryBuilder::getEdgeRenderState(boost::optional<boost::shared_ptr<ColorMap>> colorMap)
{
  RenderState renState;

  bool useColorMap = state_->getValue(EdgesColoring).toInt() == 1;
  bool rgbConversion = state_->getValue(EdgesColoring).toInt() == 2;
  renState.set(RenderState::IS_ON, state_->getValue(ShowEdges).toBool());
  renState.set(RenderState::USE_TRANSPARENT_EDGES, state_->getValue(EdgeTransparency).toBool());
  renState.set(RenderState::USE_CYLINDER, state_->getValue(EdgesAsCylinders).toInt() == 1);

  renState.defaultColor = ColorRGB(state_->getValue(DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
                           renState.defaultColor.g() > 1.0 ||
                           renState.defaultColor.b() > 1.0)?
                                ColorRGB(
                                renState.defaultColor.r() / 255.,
                                renState.defaultColor.g() / 255.,
                                renState.defaultColor.b() / 255.)
                            :   renState.defaultColor;

  edgeTransparencyValue_ = static_cast<float>(state_->getValue(EdgeTransparencyValue).toDouble());

  if (colorMap && useColorMap)
  {
    renState.set(RenderState::USE_COLORMAP_ON_EDGES, true);
  }
  else if (rgbConversion)
  {
    renState.set(RenderState::USE_COLOR_CONVERT_ON_EDGES, true);
  }
  else
  {
    renState.set(RenderState::USE_DEFAULT_COLOR_EDGES, true);
    state_->setValue(EdgesColoring, 0);
  }

  return renState;
}

RenderState GeometryBuilder::getFaceRenderState(boost::optional<boost::shared_ptr<ColorMap>> colorMap)
{
  RenderState renState;

  bool useColorMap = state_->getValue(FacesColoring).toInt() == 1;
  bool rgbConversion = state_->getValue(FacesColoring).toInt() == 2;
  renState.set(RenderState::IS_ON, state_->getValue(ShowFaces).toBool());
  renState.set(RenderState::USE_TRANSPARENCY, state_->getValue(FaceTransparency).toBool());
  renState.set(RenderState::USE_FACE_NORMALS, state_->getValue(UseFaceNormals).toBool());

  renState.defaultColor = ColorRGB(state_->getValue(DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
                           renState.defaultColor.g() > 1.0 ||
                           renState.defaultColor.b() > 1.0)?
                                ColorRGB(
                                renState.defaultColor.r() / 255.,
                                renState.defaultColor.g() / 255.,
                                renState.defaultColor.b() / 255.)
                            :   renState.defaultColor;

  faceTransparencyValue_ = static_cast<float>(state_->getValue(FaceTransparencyValue).toDouble());

  if (colorMap && useColorMap)
  {
    renState.set(RenderState::USE_COLORMAP, true);
  }
  else if (rgbConversion)
  {
    renState.set(RenderState::USE_COLOR_CONVERT, true);
  }
  else
  {
    renState.set(RenderState::USE_DEFAULT_COLOR, true);
    state_->setValue(FacesColoring, 0);
  }

  return renState;
}

GeometryHandle GeometryBuilder::buildGeometryObject(
  FieldHandle field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  const GeometryIDGenerator& gid, Interruptible* interruptible)
{
  // Function for reporting progress. TODO: use this variable somewhere!
  // auto progressFunc = getUpdaterFunc();

  // todo Determine a better way of handling all of the various object state_.
  bool showNodes = state_->getValue(ShowNodes).toBool();
  bool showEdges = state_->getValue(ShowEdges).toBool();
  bool showFaces = state_->getValue(ShowFaces).toBool();
  // Resultant geometry type (representing a spire object and a number of passes).

  std::string idname = "EntireField";
  if (!state_->getValue(FieldName).toString().empty())
  {
    idname += GeometryObject::delimiter + state_->getValue(FieldName).toString() + " (from " + moduleId_ + ")";
  }

  auto geom(boost::make_shared<GeometryObjectSpire>(gid, idname, true));

  // todo Implement inputs_changes_ ? See old scirun ShowField.cc:293.

  // todo Mind material properties (simple since we already have implemented
  //      most of this).

  // todo Handle assignment of color map. The color map will need to be
  //      available to us as we are building the meshes. Due to the way
  //      SCIRun expects meshes to be built.

  // todo render_state_ DIRTY flag? See old scirun ShowField.cc:446.

  const int dim = field->vmesh()->dimensionality();
  if (showEdges && dim < 1) { showEdges = false; }
  if (showFaces && dim < 2) { showFaces = false; }

  if (showFaces)
    renderFaces(field, colorMap, interruptible, getFaceRenderState(colorMap), geom, geom->uniqueID());

  if (showEdges)
    renderEdges(field, colorMap, interruptible, getEdgeRenderState(colorMap), geom, geom->uniqueID());

  if (showNodes)
    renderNodes(field, colorMap, interruptible, getNodeRenderState(colorMap), geom, geom->uniqueID());

  return geom;
}


void GeometryBuilder::renderFaces(
  FieldHandle field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  Interruptible* interruptible,
  RenderState state, GeometryHandle geom,
  const std::string& id)
{
  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  // Directly ported from SCIRUN 4. Unsure what 'linear' is.
  // I'm assuming it means linear interpolation as opposed to nearest neighbor
  // interpolation along the basis. But I could be wrong.
  bool doLinear = (fld->basis_order() < 2 && mesh->basis_order() < 2);

  // Todo: Check for texture -- this is indicative of volume rendering.
  // if(mesh->is_regularmesh() && mesh->is_surface() &&
  //    get_flag(render_state, USE_TEXTURE))

  if (doLinear)
  {
    return renderFacesLinear(field, colorMap, interruptible, state, geom, id);
  }
  else
  {
    std::cout << "Non linear faces not supported at this time." << std::endl;
  }
}



namespace
{
  template <typename T>
  inline void writeFloats(spire::VarBuffer* vboBuffer, std::initializer_list<T> ts)
  {
    for (const T& t : ts)
      vboBuffer->writeUnsafe(static_cast<float>(t));
  }

  inline void writeAtributeToVBO(const Point& point, spire::VarBuffer* vboBuffer)
  {
    writeFloats(vboBuffer, {point.x(), point.y(), point.z()});
  }

  inline void writeAtributeToVBO(const Vector& vector, spire::VarBuffer* vboBuffer)
  {
    writeFloats(vboBuffer, {vector.x(), vector.y(), vector.z()});
  }

  inline void writeAtributeToVBO(const glm::vec2& coords, spire::VarBuffer* vboBuffer)
  {
    writeFloats(vboBuffer, {coords.x, coords.y});
  }

  template<typename ... Params>
  void writeTri(spire::VarBuffer* vboBuffer, const Params& ... params)
  {
    for(int i = 0; i < 3; ++i)
      (void)std::initializer_list<int>{(writeAtributeToVBO(params[i], vboBuffer), 0)...};
  }

  template<typename ...Params>
  void writeQuad(spire::VarBuffer* vboBuffer, const Params& ... params)
  {
    for(int i = 0; i < 4; ++i)
      (void)std::initializer_list<int>{(writeAtributeToVBO(params[i], vboBuffer), 0)...};
  }

  enum : int
  {
    TRI, TRI_TEXCOORDS, TRI_NORMALS, TRI_NORMALS_TEXCOORDS,
    QUAD, QUAD_TEXCOORDS, QUAD_NORMALS, QUAD_NORMALS_TEXCOORDS
  };

  inline int getWriteCase(bool writeQuads, bool writeNormals, bool writeTexCoords)
  {
    return ((int)writeQuads << 2) | ((int)writeNormals << 1) | ((int)writeTexCoords);
  }

  void spiltColorMapToTextureAndCoordinates(
    const boost::optional<boost::shared_ptr<ColorMap>>& colorMap,
    ColorMapHandle& textureMap, ColorMapHandle& coordinateMap)
  {
    ColorMapHandle realColorMap = nullptr;

    if(colorMap) realColorMap = colorMap.get();
    else realColorMap = StandardColorMapFactory::create();

    textureMap = StandardColorMapFactory::create(
      realColorMap->getColorData(), realColorMap->getColorMapName(),
      realColorMap->getColorMapResolution(), realColorMap->getColorMapShift(),
      realColorMap->getColorMapInvert(), 0.5, 1.0, realColorMap->getAlphaLookup());

    coordinateMap = StandardColorMapFactory::create("Grayscale", 256, 0, false,
      realColorMap->getColorMapRescaleScale(), realColorMap->getColorMapRescaleShift());
  }
}



void GeometryBuilder::renderFacesLinear(
  FieldHandle field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
  Interruptible* interruptible,
  RenderState state,
  GeometryHandle geom,
  const std::string& id)
{
  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  mesh->synchronize(Mesh::FACES_E);

  VMesh::Face::size_type numFaces;
  mesh->size(numFaces);
  if (numFaces == 0) return;

  Point idpt;
  VMesh::Face::iterator fiter, fiterEnd;
  VMesh::Node::array_type nodes;
  mesh->get_nodes(nodes, *fiter);
  mesh->get_point(idpt, nodes[0]);
  mesh->begin(fiter);
  mesh->end(fiterEnd);
  int numNodesPerFace = nodes.size();
  bool useQuads = (numNodesPerFace == 4);
  int numAttributes = 3; //intially 3 because we will atleast be rendering verticies (vec3's)

  bool useNormals = state.get(RenderState::USE_NORMALS);
  bool useFaceNormals = state.get(RenderState::USE_FACE_NORMALS) && mesh->has_normals();
  bool invertNormals = state_->getValue(FaceInvertNormals).toBool();
  if (useNormals)
  {
    numAttributes += 3;
    mesh->synchronize(Mesh::NORMALS_E);
  }

  bool useColorMap = (fld->basis_order() >= 0 && state.get(RenderState::USE_COLORMAP));
  bool isCellData = (fld->basis_order() == 0 && mesh->dimensionality() == 3);
  bool isFaceData = (fld->basis_order() == 0 && mesh->dimensionality() == 2);
  bool isNodeData = (fld->basis_order() == 1);
  bool isScalar = fld->is_scalar();
  bool isVector = fld->is_vector();
  bool isTensor = fld->is_tensor();
  int colorMapCase = (isCellData * 0 + isFaceData * 1 + isNodeData * 2) * 3;
  colorMapCase += isScalar * 0 + isVector * 1 + isTensor * 2;

  ColorScheme colorScheme = ColorScheme::COLOR_UNIFORM;


  ColorMapHandle textureMap, coordinateMap;
  spiltColorMapToTextureAndCoordinates(colorMap, textureMap, coordinateMap);

  if (useColorMap)
  {
    numAttributes += 2;
    colorScheme = ColorScheme::COLOR_MAP;
  }

  int writeCase = getWriteCase(useQuads, useNormals, useColorMap);

  std::vector<Point> points(numNodesPerFace);
  std::vector<Vector> normals(numNodesPerFace);
  std::vector<glm::vec2> textureCoords(numNodesPerFace);
  std::vector<double> svals(numNodesPerFace);
  std::vector<Vector> vvals(numNodesPerFace);
  std::vector<Tensor> tvals(numNodesPerFace);

  size_t passNumber = 0;
  size_t facesLeft = mesh->num_faces();

  while(facesLeft > 0)
  {
    const static size_t maxFacesPerPass = 1 << 24;
    int facesLeftInThisPass = std::min(facesLeft, maxFacesPerPass);
    facesLeft -= facesLeftInThisPass;

    // Three 32 bit ints for each triangle to index into the VBO (triangles = verticies - 2)
    size_t iboSize = static_cast<size_t>(facesLeftInThisPass * sizeof(uint32_t) * (numNodesPerFace - 2) * 3);
    size_t vboSize = static_cast<size_t>(facesLeftInThisPass * sizeof(float) * numNodesPerFace * numAttributes);
    std::shared_ptr<spire::VarBuffer> iboBufferSPtr(new spire::VarBuffer(iboSize));
    std::shared_ptr<spire::VarBuffer> vboBufferSPtr(new spire::VarBuffer(vboSize));
    auto iboBuffer = iboBufferSPtr.get();
    auto vboBuffer = vboBufferSPtr.get();

    if(useQuads)
    {
      uint32_t nodesInThisPass = facesLeftInThisPass * 4;
      for(uint32_t i = 0; i < nodesInThisPass; i += 4)
      {
        iboBuffer->writeUnsafe(i+0);
        iboBuffer->writeUnsafe(i+1);
        iboBuffer->writeUnsafe(i+2);
        iboBuffer->writeUnsafe(i+2);
        iboBuffer->writeUnsafe(i+3);
        iboBuffer->writeUnsafe(i+0);
      }
    }
    else
    {
      uint32_t nodesInThisPass = facesLeftInThisPass * 3;
      for(uint32_t i = 0; i <  nodesInThisPass; i += 3)
      {
        iboBuffer->writeUnsafe(i+0);
        iboBuffer->writeUnsafe(i+1);
        iboBuffer->writeUnsafe(i+2);
      }
    }

    while (facesLeftInThisPass > 0)
    {
      interruptible->checkForInterruption();
      mesh->get_nodes(nodes, *fiter);

      for(size_t i = 0; i < numNodesPerFace; ++i)
        mesh->get_point(points[i], nodes[i]);

      if (useNormals)
      {
        if (useFaceNormals)
        {
          for(size_t i = 0; i < numNodesPerFace; ++i)
            mesh->get_normal(normals[i], nodes[i]);
        }
        else
        {
          Vector norm;
          if (useQuads)
          {
            Vector edge1 = points[1] - points[0];
            Vector edge2 = points[2] - points[1];
            Vector edge3 = points[3] - points[2];
            Vector edge4 = points[0] - points[3];
            norm = Cross(edge1, edge2) + Cross(edge2, edge3) + Cross(edge3, edge4) + Cross(edge4, edge1);
            norm.normalize();
          }
          else
          {
            Vector edge1 = points[1] - points[0];
            Vector edge2 = points[2] - points[1];
            norm = Cross(edge1, edge2);
            norm.normalize();
          }

          for(size_t i = 0; i < numNodesPerFace; ++i)
            normals[i] = norm;
        }

        if(invertNormals)
          for(size_t i = 0; i < numNodesPerFace; ++i)
            normals[i] = -normals[i];
      }

      if(useColorMap)
      {
        // Element data (Cells) so two sided faces.
        if (isCellData)
        {
          VMesh::Elem::array_type cells;
          mesh->get_elems(cells, *fiter);

          if (isScalar)
          {
            fld->get_value(svals[0], cells[0]);
            if (cells.size() > 1) fld->get_value(svals[1], cells[1]);
            else svals[1] = svals[0];

            for (size_t i = 0; i < numNodesPerFace; ++i)
            {
              textureCoords[i].x = coordinateMap->valueToIndex(svals[0]);
              textureCoords[i].y = coordinateMap->valueToIndex(svals[1]);
            }
          }
          else if (isVector)
          {
            fld->get_value(vvals[0], cells[0]);
            if (cells.size() > 1) fld->get_value(vvals[1], cells[1]);
            else svals[1] = svals[0];

            for (size_t i = 0; i < numNodesPerFace; ++i)
            {
              textureCoords[i].x = coordinateMap->valueToIndex(vvals[0]);
              textureCoords[i].y = coordinateMap->valueToIndex(vvals[1]);
            }
          }
          else if (isTensor)
          {
            fld->get_value(tvals[0], cells[0]);
            if (cells.size() > 1) fld->get_value(tvals[1], cells[1]);
            else svals[1] = svals[0];

            for (size_t i = 0; i < numNodesPerFace; ++i)
            {
              textureCoords[i].x = coordinateMap->valueToIndex(tvals[0]);
              textureCoords[i].y = coordinateMap->valueToIndex(tvals[1]);
            }
          }
        }
        // Element data (faces)
        else if (isFaceData)
        {
          if (isScalar)
          {
            fld->get_value(svals[0], *fiter);
            textureCoords[0].x = coordinateMap->valueToIndex(svals[0]);
          }
          else if (isVector)
          {
            fld->get_value(vvals[0], *fiter);
            textureCoords[0].x = coordinateMap->valueToIndex(vvals[0]);
          }
          else if (isTensor)
          {
            fld->get_value(tvals[0], *fiter);
            textureCoords[0].x = coordinateMap->valueToIndex(tvals[0]);
          }

          for (size_t i = 0; i < numNodesPerFace; ++i)
            textureCoords[i].y = textureCoords[i].x = textureCoords[0].x;
        }
        // Data at nodes
        else if (isNodeData)
        {
          if (isScalar)
          {
            for (size_t i = 0; i < numNodesPerFace; ++i)
            {
              fld->get_value(svals[i], nodes[i]);
              textureCoords[i].x = textureCoords[i].y = coordinateMap->valueToIndex(svals[i]);
            }
          }
          else if (isVector)
          {
            for (size_t i = 0; i < numNodesPerFace; ++i)
            {
              fld->get_value(vvals[i], nodes[i]);
              textureCoords[i].x = textureCoords[i].y = coordinateMap->valueToIndex(vvals[i]);
            }
          }
          else if (isTensor)
          {
            for (size_t i = 0; i < numNodesPerFace; ++i)
            {
              fld->get_value(tvals[i], nodes[i]);
              textureCoords[i].x = textureCoords[i].y = coordinateMap->valueToIndex(tvals[i]);
            }
          }
        }
      }

      switch(writeCase)
      {
        case TRI: writeTri(vboBuffer, points); break;
        case TRI_TEXCOORDS: writeTri(vboBuffer, points, textureCoords); break;
        case TRI_NORMALS: writeTri(vboBuffer, points, normals); break;
        case TRI_NORMALS_TEXCOORDS: writeTri(vboBuffer, points, normals, textureCoords); break;
        case QUAD: writeQuad(vboBuffer, points); break;
        case QUAD_TEXCOORDS: writeQuad(vboBuffer, points, textureCoords); break;
        case QUAD_NORMALS: writeQuad(vboBuffer, points, normals); break;
        case QUAD_NORMALS_TEXCOORDS: writeQuad(vboBuffer, points, normals, textureCoords); break;
      }

      ++fiter;
      --facesLeftInThisPass;
    }

    std::stringstream ss;
    ss << invertNormals << static_cast<int>(colorScheme) << faceTransparencyValue_ << "_" << passNumber;

    std::string uniqueNodeID = id + "face" + ss.str();
    std::string vboName = uniqueNodeID + "VBO";
    std::string iboName = uniqueNodeID + "IBO";
    std::string passName = uniqueNodeID + "Pass";
    std::string shader = (useNormals ? "Shaders/Phong" : "Shaders/Flat");

    std::vector<SpireVBO::AttributeData> attribs;
    std::vector<SpireSubPass::Uniform> uniforms;

    attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
    uniforms.push_back(SpireSubPass::Uniform("uUseClippingPlanes", true));
    uniforms.push_back(SpireSubPass::Uniform("uUseFog", true));
    uniforms.push_back(SpireSubPass::Uniform("uTransparency", faceTransparencyValue_));

    if (useNormals)
    {
      attribs.push_back(SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
      uniforms.push_back(SpireSubPass::Uniform("uAmbientColor", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(SpireSubPass::Uniform("uSpecularColor", glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(SpireSubPass::Uniform("uSpecularPower", 32.0f));
    }

    SpireTexture2D texture;
    if (useColorMap)
    {
      shader += "_ColorMap";
      attribs.push_back(SpireVBO::AttributeData("aTexCoords", 2 * sizeof(float)));

      const static int colorMapResolution = 256;
      for(int i = 0; i < colorMapResolution; ++i)
      {
        ColorRGB color = textureMap->valueToColor(static_cast<float>(i)/colorMapResolution * 2.0 - 1.0);
        texture.bitmap.push_back(color.r()*255.99f);
        texture.bitmap.push_back(color.g()*255.99f);
        texture.bitmap.push_back(color.b()*255.99f);
        texture.bitmap.push_back(color.a()*255.99f);
      }
      texture.name = "ColorMap";
      texture.height = 1;
      texture.width = colorMapResolution;
    }
    else
    {
      uniforms.push_back(SpireSubPass::Uniform("uDiffuseColor",
        glm::vec4(state.defaultColor.r(), state.defaultColor.g(), state.defaultColor.b(), 1.0f)));
    }

    //numVBOElements is only used in dead code and should be removed which is why its hard coded to 0
    SpireVBO geomVBO(vboName, attribs, vboBufferSPtr, 0, mesh->get_bounding_box(), true);
    geom->vbos().push_back(geomVBO);

    SpireIBO geomIBO(iboName, SpireIBO::PRIMITIVE::TRIANGLES, sizeof(uint32_t), iboBufferSPtr);
    geom->ibos().push_back(geomIBO);

    SpireText text;
    SpireSubPass pass(passName, vboName, iboName, shader,
      colorScheme, state, RenderType::RENDER_VBO_IBO, geomVBO, geomIBO, text, texture);

    for (const auto& uniform : uniforms) pass.addUniform(uniform);

    geom->passes().push_back(pass);
    ++passNumber;
  }
}



void GeometryBuilder::renderNodes(
  FieldHandle field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
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

  ColorMapHandle textureMap, coordinateMap;
  spiltColorMapToTextureAndCoordinates(colorMap, textureMap, coordinateMap);

  if (fld->basis_order() < 0 || (fld->basis_order() == 0 && mesh->dimensionality() != 0) || state.get(RenderState::USE_DEFAULT_COLOR_NODES))
    colorScheme = ColorScheme::COLOR_UNIFORM;
  else if (state.get(RenderState::USE_COLORMAP_ON_NODES))
    colorScheme = ColorScheme::COLOR_MAP;
  else
    colorScheme = ColorScheme::COLOR_IN_SITU;

  mesh->synchronize(Mesh::NODES_E);

  VMesh::Node::iterator eiter, eiter_end;
  mesh->begin(eiter);
  mesh->end(eiter_end);

  double radius = state_->getValue(SphereScaleValue).toDouble();
  double num_strips = static_cast<double>(state_->getValue(SphereResolution).toInt());
  if (radius < 0) radius = 1.;
  if (num_strips < 0) num_strips = 10.;
  std::stringstream ss;
  ss << state.get(RenderState::USE_SPHERE) << radius << num_strips << static_cast<int>(colorScheme);

  std::string uniqueNodeID = id + "node" + ss.str();

  nodeTransparencyValue_ = static_cast<float>(state_->getValue(NodeTransparencyValue).toDouble());

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
        node_color = ColorRGB(coordinateMap->valueToIndex(sval));
      }
      else if (fld->is_vector())
      {
        fld->get_value(vval, *eiter);
        node_color = ColorRGB(coordinateMap->valueToIndex(vval));
      }
      else if (fld->is_tensor())
      {
        fld->get_value(tval, *eiter);
        node_color = ColorRGB(coordinateMap->valueToIndex(tval));
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

  glyphs.buildObject(*geom, uniqueNodeID, state.get(RenderState::USE_TRANSPARENT_NODES), nodeTransparencyValue_,
    colorScheme, state, primIn, mesh->get_bounding_box(), true, textureMap);
}



void GeometryBuilder::renderEdges(
  FieldHandle field,
  boost::optional<boost::shared_ptr<ColorMap>> colorMap,
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

  ColorMapHandle textureMap, coordinateMap;
  spiltColorMapToTextureAndCoordinates(colorMap, textureMap, coordinateMap);

  if (fld->basis_order() < 0 ||
    (fld->basis_order() == 0 && mesh->dimensionality() != 0) ||
    state.get(RenderState::USE_DEFAULT_COLOR_EDGES))
    colorScheme = ColorScheme::COLOR_UNIFORM;
  else if (state.get(RenderState::USE_COLORMAP_ON_EDGES))
    colorScheme = ColorScheme::COLOR_MAP;
  else
    colorScheme = ColorScheme::COLOR_IN_SITU;

  mesh->synchronize(Mesh::EDGES_E);

  VMesh::Edge::iterator eiter, eiter_end;
  mesh->begin(eiter);
  mesh->end(eiter_end);

  double num_strips = static_cast<double>(state_->getValue(CylinderResolution).toInt());
  double radius = state_->getValue(CylinderRadius).toDouble();
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
        edge_colors[0] = ColorRGB(coordinateMap->valueToIndex(sval0));
        edge_colors[1] = ColorRGB(coordinateMap->valueToIndex(sval1));
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

        edge_colors[0] = ColorRGB(coordinateMap->valueToIndex(vval0));
        edge_colors[1] = ColorRGB(coordinateMap->valueToIndex(vval1));
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

        edge_colors[0] = ColorRGB(coordinateMap->valueToIndex(tval0));
        edge_colors[1] = ColorRGB(coordinateMap->valueToIndex(tval1));
      }
    }
    //accumulate VBO or IBO data

    if (p0 != p1)
    {
      if (state.get(RenderState::USE_CYLINDER))
      {
        glyphs.addCylinder(p0, p1, radius, num_strips, edge_colors[0], edge_colors[1]);
        glyphs.addSphere(p0, radius, num_strips, edge_colors[0]);
        glyphs.addSphere(p1, radius, num_strips, edge_colors[1]);
      }
      else
      {
        glyphs.addLine(p0, p1, edge_colors[0], edge_colors[1]);
      }
    }

    ++eiter;
  }

  glyphs.buildObject(*geom, uniqueNodeID, state.get(RenderState::USE_TRANSPARENT_EDGES), edgeTransparencyValue_,
    colorScheme, state, primIn, mesh->get_bounding_box(), true, textureMap);
}

void ShowField::updateAvailableRenderOptions(FieldHandle field)
{
  if (!field) return;

  auto vmesh = field->vmesh();
  if (!vmesh) return;

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

ALGORITHM_PARAMETER_DEF(Visualization, FieldName);
ALGORITHM_PARAMETER_DEF(Visualization, ShowNodes);
ALGORITHM_PARAMETER_DEF(Visualization, ShowEdges);
ALGORITHM_PARAMETER_DEF(Visualization, ShowFaces);
ALGORITHM_PARAMETER_DEF(Visualization, NodesAvailable);
ALGORITHM_PARAMETER_DEF(Visualization, EdgesAvailable);
ALGORITHM_PARAMETER_DEF(Visualization, FacesAvailable);
ALGORITHM_PARAMETER_DEF(Visualization, NodeTransparency);
ALGORITHM_PARAMETER_DEF(Visualization, EdgeTransparency);
ALGORITHM_PARAMETER_DEF(Visualization, FaceTransparency);
ALGORITHM_PARAMETER_DEF(Visualization, FaceInvertNormals);
ALGORITHM_PARAMETER_DEF(Visualization, NodeAsPoints);
ALGORITHM_PARAMETER_DEF(Visualization, NodeAsSpheres);
ALGORITHM_PARAMETER_DEF(Visualization, EdgesAsLines);
ALGORITHM_PARAMETER_DEF(Visualization, EdgesAsCylinders);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultMeshColor);
ALGORITHM_PARAMETER_DEF(Visualization, FaceTransparencyValue);
ALGORITHM_PARAMETER_DEF(Visualization, EdgeTransparencyValue);
ALGORITHM_PARAMETER_DEF(Visualization, NodeTransparencyValue);
ALGORITHM_PARAMETER_DEF(Visualization, FacesColoring);
ALGORITHM_PARAMETER_DEF(Visualization, NodesColoring);
ALGORITHM_PARAMETER_DEF(Visualization, EdgesColoring);
ALGORITHM_PARAMETER_DEF(Visualization, SphereScaleValue);
ALGORITHM_PARAMETER_DEF(Visualization, CylinderResolution);
ALGORITHM_PARAMETER_DEF(Visualization, SphereResolution);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultTextColor);
ALGORITHM_PARAMETER_DEF(Visualization, ShowText);
ALGORITHM_PARAMETER_DEF(Visualization, ShowDataValues);
ALGORITHM_PARAMETER_DEF(Visualization, ShowNodeIndices);
ALGORITHM_PARAMETER_DEF(Visualization, ShowEdgeIndices);
ALGORITHM_PARAMETER_DEF(Visualization, ShowFaceIndices);
ALGORITHM_PARAMETER_DEF(Visualization, ShowCellIndices);
ALGORITHM_PARAMETER_DEF(Visualization, CullBackfacingText);
ALGORITHM_PARAMETER_DEF(Visualization, TextAlwaysVisible);
ALGORITHM_PARAMETER_DEF(Visualization, RenderAsLocation);
ALGORITHM_PARAMETER_DEF(Visualization, TextSize);
ALGORITHM_PARAMETER_DEF(Visualization, TextPrecision);
ALGORITHM_PARAMETER_DEF(Visualization, TextColoring);
ALGORITHM_PARAMETER_DEF(Visualization, UseFaceNormals);
