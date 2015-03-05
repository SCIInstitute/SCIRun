/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <boost/foreach.hpp>

#include <glm/glm.hpp>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Core::Geometry;
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
  boost::shared_ptr<SCIRun::Field> field = getRequiredInput(Field);
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap = getOptionalInput(ColorMapObject);
  
  //set the default colormap scaling to be full range if no rescaling was provided.
  if (colorMap.get()->getColorMapRescaleScale()==1. && colorMap.get()->getColorMapRescaleShift()==0.) {
        // set the colormap min/max defaults to the data min/max.
        double actual_min = std::numeric_limits<double>::max();
        double actual_max = std::numeric_limits<double>::min();

        if (!field.get()->vfield()->minmax(actual_min, actual_max))
            error("An input field is not a scalar or vector field.");

        colorMap.get()->setColorMapRescaleShift(-actual_min);
        colorMap.get()->setColorMapRescaleScale(1. / (actual_max-actual_min));
  }
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

  // Set value ranges for color mapping fields. We should use uniforms for
  // setting the highest / lowest value ranges. We should be able to do this
  // independently of showNodes / showEdges / showFaces.
  // geom->mLowestValue = valueRangeLow;
  // geom->mHighestValue = valueRangeHigh;
  if (colorMap)
  {
    geom->mColorMap = boost::optional<std::string>((*colorMap)->getColorMapName());
  }
  else
  {
    geom->mColorMap = boost::optional<std::string>();
  }

  return geom;
}

// Borrowed from SCIRun4 -- Float to Byte
static uint8_t COLOR_FTOB(double v)
{
  const int inter = static_cast<int>(v * 255 + 0.5);
  if (inter > 255) return 255;
  if (inter < 0) return 0;
  return static_cast<uint8_t>(inter);
}

void ShowFieldModule::renderFaces(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
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
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
  RenderState state,
  Core::Datatypes::GeometryHandle geom,
  unsigned int approxDiv,
  const std::string& id)
{
  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  bool withNormals = (state.get(RenderState::USE_NORMALS));
  auto st = get_state();
  bool invertNormals = st->getValue(FaceInvertNormals).toBool();
  GeometryObject::ColorScheme colorScheme = GeometryObject::COLOR_UNIFORM;
  std::vector<double> svals(10);
  std::vector<Core::Geometry::Vector> vvals(10);
  std::vector<Core::Geometry::Tensor> tvals(10);

  std::vector<Material> vcols(10, Material());
  std::vector<double> scols(10);

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

    for (uint32_t i = 0; i < 10; ++i)
    {
      vcols[i] = Material(ColorRGB(1.0, 1.0, 1.0));

      if (state.get(RenderState::USE_TRANSPARENCY))
      {
        vcols[i].transparency = 0.75;
      }
      else
      {
        vcols[i].transparency = 1.0;
      }
    }
  }

  if (withNormals) { mesh->synchronize(Mesh::NORMALS_E); }

  mesh->synchronize(Mesh::FACES_E);
  VMesh::Face::iterator fiter, fiterEnd;
  VMesh::Node::array_type nodes;

  mesh->begin(fiter);
  mesh->end(fiterEnd);

  VMesh::Face::size_type f;
  VMesh::Cell::size_type c;

  mesh->size(f);
  mesh->size(c);

  // Attempt some form of precalculation of iboBuffer and vboBuffer size.
  // This Initial size estimation will be off quite a bit. Each face will
  // have up to 3 nodes associated.
  uint32_t iboSize = mesh->num_faces() * sizeof(uint32_t) * 3;
  uint32_t vboSize = mesh->num_faces() * sizeof(float) * 3;

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
  
  
  Core::Geometry::Point idpt;
  mesh->get_nodes(nodes, *fiter);
  mesh->get_point(idpt, nodes[0]);

  while (fiter != fiterEnd)
  {
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
        colorScheme, scols, vcols, state);
    }
    // Element data (Cells) so two sided faces.
    else if (fld->basis_order() == 0 && mesh->dimensionality() == 3)
    {
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

        valueToColor(colorScheme, svals[0], scols[0], vcols[0]);
        valueToColor(colorScheme, svals[1], scols[1], vcols[1]);
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

        valueToColor(colorScheme, vvals[0], scols[0], vcols[0]);
        valueToColor(colorScheme, vvals[1], scols[1], vcols[1]);
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

        valueToColor(colorScheme, tvals[0], scols[0], vcols[0]);
        valueToColor(colorScheme, tvals[1], scols[1], vcols[1]);
      }

      state.set(RenderState::IS_DOUBLE_SIDED, true);

      addFaceGeom(points, normals, withNormals, iboIndex, iboBuffer, vboBuffer,
        colorScheme, scols, vcols, state);
    }
    // Element data (faces)
    else if (fld->basis_order() == 0 && mesh->dimensionality() == 2)
    {
      if (fld->is_scalar())
      {
        fld->get_value(svals[0], *fiter);
        valueToColor(colorScheme, svals[0], scols[0], vcols[0]);
      }
      else if (fld->is_vector())
      {
        fld->get_value(vvals[0], *fiter);
        valueToColor(colorScheme, vvals[0], scols[0], vcols[0]);
      }
      else if (fld->is_tensor())
      {
        fld->get_value(tvals[0], *fiter);
        valueToColor(colorScheme, tvals[0], scols[0], vcols[0]);
      }

      // Same color at all corners.
      for (size_t i = 0; i<nodes.size(); ++i)
      {
        scols[i] = scols[0];
        vcols[i] = vcols[0];
      }

      addFaceGeom(points, normals, withNormals, iboIndex, iboBuffer, vboBuffer,
        colorScheme, scols, vcols, state);
    }

    // Data at nodes
    else if (fld->basis_order() == 1)
    {
      if (fld->is_scalar())
      {
        for (size_t i = 0; i<nodes.size(); i++)
        {
          fld->get_value(svals[i], nodes[i]);
          valueToColor(colorScheme, svals[i], scols[i], vcols[i]);
        }
      }
      else if (fld->is_vector())
      {
        for (size_t i = 0; i<nodes.size(); i++)
        {
          fld->get_value(vvals[i], nodes[i]);
          valueToColor(colorScheme, vvals[i], scols[i], vcols[i]);
        }
      }
      else if (fld->is_tensor())
      {
        for (size_t i = 0; i<nodes.size(); i++)
        {
          fld->get_value(tvals[i], nodes[i]);
          valueToColor(colorScheme, tvals[i], scols[i], vcols[i]);
        }
      }

      addFaceGeom(points, normals, withNormals, iboIndex, iboBuffer, vboBuffer,
        colorScheme, scols, vcols, state);
    }

    ++fiter;
    ++numVBOElements;
  }

  std::stringstream ss;
  ss << invertNormals;

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

  if (colorScheme == GeometryObject::COLOR_MAP)
  {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aFieldData", 1 * sizeof(float)));
    //push the color map parameters
    ColorMap * map = colorMap.get().get();
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMInvert",map->getColorMapInvert()?1.f:0.f));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMShift",static_cast<float>(map->getColorMapShift())));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMResolution",static_cast<float>(map->getColorMapResolution())));
    double scl = map->getColorMapRescaleScale(), shft = map->getColorMapRescaleShift();
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uRescaleScale",static_cast<float>(scl)));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uRescaleShift",static_cast<float>(shft)));

    if (state.get(RenderState::IS_DOUBLE_SIDED) == false)
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
      attribs.push_back(GeometryObject::SpireVBO::AttributeData("aFieldDataSecondary", 1 * sizeof(float)));

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

    if (state.get(RenderState::USE_TRANSPARENCY))
    {
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uTransparency", (float)(faceTransparencyValue_)));
    }
    else
    {
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uTransparency", (float)(1.0f)));
    }
  }
  else if (colorScheme == GeometryObject::COLOR_IN_SITU)
  {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 1 * sizeof(uint32_t), true));

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
      attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColorSecondary", 1 * sizeof(uint32_t), true));

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

      if (state.get(RenderState::USE_TRANSPARENCY))
      {
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform(
          "uDiffuseColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), faceTransparencyValue_)));
      }
      else
      {
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform(
          "uDiffuseColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), 1.0f)));
      }
    }
    else
    {
      shader = "Shaders/UniformColor";
      if (state.get(RenderState::USE_TRANSPARENCY))
      {
        /// \todo Add transparency slider.
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform(
          "uColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), faceTransparencyValue_)));
      }
      else
      {
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform(
          "uColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), 1.0f)));
      }
    }
  }

  GeometryObject::SpireVBO geomVBO = GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr,
    numVBOElements, mesh->get_bounding_box(), true);

  geom->mVBOs.push_back(geomVBO);

  // Construct IBO.

  GeometryObject::SpireIBO geomIBO = GeometryObject::SpireIBO(iboName, GeometryObject::SpireIBO::TRIANGLES, sizeof(uint32_t), iboBufferSPtr);

  geom->mIBOs.push_back(geomIBO);

  // Construct Pass.
  // Build pass for the edges.
  /// \todo Find an appropriate place to put program names like UniformColor.
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
  std::vector<double> &scols,
  std::vector<Material> &vcols,
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

  auto writeVBO4ByteColor = [&vboBuffer](const Material& vcol)
  {
    vboBuffer->write(COLOR_FTOB(vcol.diffuse.r()));
    vboBuffer->write(COLOR_FTOB(vcol.diffuse.g()));
    vboBuffer->write(COLOR_FTOB(vcol.diffuse.b()));
    vboBuffer->write(COLOR_FTOB(vcol.transparency));
  };

  auto writeVBOScalarValue = [&vboBuffer](double value)
  {
    vboBuffer->write(static_cast<float>(value));
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
        if (!doubleSided) { writeVBOScalarValue(scols[0]); }
        else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }

        writeVBOPoint(points[1]);
        writeVBONormal(normals[1]);
        if (!doubleSided) { writeVBOScalarValue(scols[1]); }
        else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }

        writeVBOPoint(points[2]);
        writeVBONormal(normals[2]);
        if (!doubleSided) { writeVBOScalarValue(scols[2]); }
        else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }

        writeVBOPoint(points[3]);
        writeVBONormal(normals[3]);
        if (!doubleSided) { writeVBOScalarValue(scols[3]); }
        else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }
      }
      else
      {
        writeVBOPoint(points[0]);
        if (!doubleSided) { writeVBOScalarValue(scols[0]); }
        else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }

        writeVBOPoint(points[1]);
        if (!doubleSided) { writeVBOScalarValue(scols[1]); }
        else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }

        writeVBOPoint(points[2]);
        if (!doubleSided) { writeVBOScalarValue(scols[2]); }
        else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }

        writeVBOPoint(points[3]);
        if (!doubleSided) { writeVBOScalarValue(scols[3]); }
        else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }
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
          if (!doubleSided) { writeVBOScalarValue(scols[0]); }
          else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }
          writeIBOIndex(iboIndex);

          // Apply misc user settings.
          writeVBOPoint(points[i - 1]);
          writeVBONormal(normals[i - 1]);
          if (!doubleSided) { writeVBOScalarValue(scols[i - 1]); }
          else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }
          writeIBOIndex(iboIndex + i - 1);

          writeVBOPoint(points[i]);
          writeVBONormal(normals[i]);
          if (!doubleSided) { writeVBOScalarValue(scols[i]); }
          else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }
          writeIBOIndex(iboIndex + i);
        }
        else
        {
          // Render points if we are not rendering spheres.
          writeVBOPoint(points[0]);
          if (!doubleSided) { writeVBOScalarValue(scols[0]); }
          else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }
          writeIBOIndex(iboIndex);

          writeVBOPoint(points[i - 1]);
          if (!doubleSided) { writeVBOScalarValue(scols[i - 1]); }
          else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }
          writeIBOIndex(iboIndex + i - 1);

          writeVBOPoint(points[i]);
          if (!doubleSided) { writeVBOScalarValue(scols[i]); }
          else              { writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]); }
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
        if (!doubleSided) { writeVBO4ByteColor(vcols[0]); }
        else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }

        writeVBOPoint(points[1]);
        writeVBONormal(normals[1]);
        if (!doubleSided) { writeVBO4ByteColor(vcols[1]); }
        else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }

        writeVBOPoint(points[2]);
        writeVBONormal(normals[2]);
        if (!doubleSided) { writeVBO4ByteColor(vcols[2]); }
        else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }

        writeVBOPoint(points[3]);
        writeVBONormal(normals[3]);
        if (!doubleSided) { writeVBO4ByteColor(vcols[3]); }
        else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }
      }
      else
      {
        writeVBOPoint(points[0]);
        if (!doubleSided) { writeVBO4ByteColor(vcols[0]); }
        else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }

        // Add appropriate uniforms to the pass (in this case, uColor).
        writeVBOPoint(points[1]);
        if (!doubleSided) { writeVBO4ByteColor(vcols[1]); }
        else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }

        writeVBOPoint(points[2]);
        if (!doubleSided) { writeVBO4ByteColor(vcols[2]); }
        else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }

        writeVBOPoint(points[3]);
        if (!doubleSided) { writeVBO4ByteColor(vcols[3]); }
        else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }
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
          if (!doubleSided) { writeVBO4ByteColor(vcols[0]); }
          else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }
          writeIBOIndex(iboIndex);

          writeVBOPoint(points[i - 1]);
          writeVBONormal(normals[i - 1]);
          if (!doubleSided) { writeVBO4ByteColor(vcols[i - 1]); }
          else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }
          writeIBOIndex(iboIndex + i - 1);

          writeVBOPoint(points[i]);
          writeVBONormal(normals[i]);
          if (!doubleSided) { writeVBO4ByteColor(vcols[i]); }
          else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }
          writeIBOIndex(iboIndex + i);
        }
        else
        {
          writeVBOPoint(points[0]);
          if (!doubleSided) { writeVBO4ByteColor(vcols[0]); }
          else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }
          iboBuffer->write(iboIndex);

          writeVBOPoint(points[i - 1]);
          if (!doubleSided) { writeVBO4ByteColor(vcols[i - 1]); }
          else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }
          writeIBOIndex(iboIndex + i - 1);

          writeVBOPoint(points[i]);
          if (!doubleSided) { writeVBO4ByteColor(vcols[i]); }
          else              { writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]); }
          writeIBOIndex(iboIndex + i);
        }
      }
      iboIndex += points.size();
    }
  }
}



void ShowFieldModule::renderNodes(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
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
  double scol;
  Core::Datatypes::Material vcol;

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

  // Attempt some form of precalculation of iboBuffer and vboBuffer size.
  uint32_t iboSize = 0;
  uint32_t vboSize = 0;
  auto my_state = this->get_state();
  double radius = my_state->getValue(SphereScaleValue).toDouble();
  double num_strips = static_cast<double>(my_state->getValue(SphereResolution).toInt());
  if (radius < 0) radius = 1.;
  if (num_strips < 0) num_strips = 10.;
  std::stringstream ss;
  ss << state.get(RenderState::USE_SPHERE) << radius << num_strips;

  std::string uniqueNodeID = id + "node" + ss.str();
  std::string vboName = uniqueNodeID + "VBO";
  std::string iboName = uniqueNodeID + "IBO";
  std::string passName = uniqueNodeID + "Pass";

  // NOTE: Attributes will depend on the color scheme. We will want to
  // normalize the colors if the color scheme is COLOR_IN_SITU.

  // Construct VBO.
  std::string shader = "Shaders/UniformColor";
  std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  if (state.get(RenderState::USE_SPHERE))
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
  GeometryObject::RenderType renderType = GeometryObject::RENDER_VBO_IBO;

  // If true, then the VBO will be placed on the GPU. We don't want to place
  // VBOs on the GPU when we are generating rendering lists.
  bool vboOnGPU = true;
  auto st = get_state();
  nodeTransparencyValue_ = static_cast<float>(st->getValue(NodeTransparencyValue).toDouble());
  
  std::vector<GeometryObject::SpireSubPass::Uniform> uniforms;
  
  //transparency
  if (state.get(RenderState::USE_TRANSPARENT_NODES))
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uTransparency",nodeTransparencyValue_));
  else
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uTransparency", (float)(1.0)));
  //coloring
  if (colorScheme == GeometryObject::COLOR_MAP) {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aFieldData", 1 * sizeof(float)));
    //push the color map parameters
    ColorMap * map = colorMap.get().get();
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMInvert",map->getColorMapInvert()?1.f:0.f));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMShift",static_cast<float>(map->getColorMapShift())));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMResolution",static_cast<float>(map->getColorMapResolution())));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uRescaleScale",static_cast<float>(map->getColorMapRescaleScale())));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uRescaleShift",static_cast<float>(map->getColorMapRescaleShift())));
    if (state.get(RenderState::USE_SPHERE)) {
        shader = "Shaders/DirPhongCMap" ;
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    } else {
        shader = "Shaders/ColorMap";
    }
  } else if (colorScheme == GeometryObject::COLOR_IN_SITU) {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 1 * sizeof(uint32_t), true));
    if (state.get(RenderState::USE_SPHERE)) {
      shader = "Shaders/DirPhongInSitu";
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    } else {
      shader = "Shaders/InSituColor";
    }
  } else if (colorScheme == GeometryObject::COLOR_UNIFORM) {
    ColorRGB dft = state.defaultColor;
    if (state.get(RenderState::USE_SPHERE)) {
        shader = "Shaders/DirPhong";
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
            glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDiffuseColor",
            glm::vec4(dft.r(), dft.g(), dft.b(), nodeTransparencyValue_)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
            glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    } else {
        shader = "Shaders/UniformColor";
        uniforms.emplace_back("uColor", glm::vec4(dft.r(), dft.g(), dft.b(), nodeTransparencyValue_));
    }
  }
  GeometryObject::SpireIBO::PRIMITIVE primIn = GeometryObject::SpireIBO::POINTS;
  // Use spheres...
  if (state.get(RenderState::USE_SPHERE))
    primIn = GeometryObject::SpireIBO::TRIANGLES;
  std::vector<Vector> points;
  std::vector<Vector> normals;
  std::vector<ColorRGB> colors;
  std::vector<uint32_t> indices;
  uint32_t index = 0;
  int64_t numVBOElements = 0;
  while (eiter != eiter_end) {
    Core::Geometry::Point p;
    mesh->get_point(p, *eiter);
    //coloring options
    if (colorScheme != GeometryObject::COLOR_UNIFORM) {
      if (fld->is_scalar())
      {
        fld->get_value(sval, *eiter);
        valueToColor(colorScheme, sval, scol, vcol);
      } else if (fld->is_vector()) {
        fld->get_value(vval, *eiter);
        valueToColor(colorScheme, vval, scol, vcol);
      } else if (fld->is_tensor()) {
        fld->get_value(tval, *eiter);
        valueToColor(colorScheme, tval, scol, vcol);
      }
    }
    //accumulate VBO or IBO data
    if (state.get(RenderState::USE_SPHERE)) {
        //generate triangles for the spheres
        Vector pp1, pp2;
        double theta_inc = 2. * M_PI / num_strips, phi_inc = M_PI / num_strips;
        for (double phi = 0.; phi < M_PI; phi += phi_inc) {
          for (double theta = 0.; theta <= 2. * M_PI; theta += theta_inc) {
            uint32_t offset = (uint32_t)numVBOElements;
            pp1 = Vector(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            pp2 = Vector(sin(theta) * cos(phi + phi_inc), sin(theta) * sin(phi + phi_inc), cos(theta));
            points.push_back(radius * pp1 + Vector(p));
            if (colorScheme == GeometryObject::COLOR_MAP)
              colors.push_back(ColorRGB(scol, scol, scol));
            else if (colorScheme == GeometryObject::COLOR_IN_SITU)
                colors.push_back(vcol.diffuse);
            numVBOElements++;
            points.push_back(radius * pp2 + Vector(p));
            if (colorScheme == GeometryObject::COLOR_MAP)
              colors.push_back(ColorRGB(scol, scol, scol));
            else if (colorScheme == GeometryObject::COLOR_IN_SITU)
                colors.push_back(vcol.diffuse);
            numVBOElements++;
            normals.push_back(pp1);
            normals.push_back(pp2);
            indices.push_back(0 + offset);
            indices.push_back(1 + offset);
            indices.push_back(2 + offset);
            indices.push_back(2 + offset);
            indices.push_back(1 + offset);
            indices.push_back(3 + offset);
          }
          for (int jj = 0; jj < 6; jj++) indices.pop_back();
        }
    } else {
      points.push_back(Vector(p));
      if (colorScheme == GeometryObject::COLOR_MAP)
        colors.push_back(ColorRGB(scol, scol, scol));
      else if (colorScheme == GeometryObject::COLOR_IN_SITU)
        colors.push_back(vcol.diffuse);
      indices.push_back(index);
      ++index;
      ++numVBOElements;
    }

    ++eiter;
  }

  vboSize = (uint32_t)points.size() * 3 * sizeof(float);
  vboSize += (uint32_t)normals.size() * 3 * sizeof(float);
  if (colorScheme == GeometryObject::COLOR_IN_SITU || colorScheme == GeometryObject::COLOR_MAP)
    vboSize += (uint32_t)colors.size() * 4; //add last 4 bytes for color
  iboSize = (uint32_t)indices.size() * sizeof(uint32_t);

  /// \todo To reduce memory requirements, we can use a 16bit index buffer.

  /// \todo To further reduce a large amount of memory, get rid of the index
  ///       buffer and use glDrawArrays to render without an IBO. An IBO is
  ///       a waste of space.
  ///       http://www.opengl.org/sdk/docs/man3/xhtml/glDrawArrays.xml

  /// \todo Switch to unique_ptrs and move semantics.
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr(
    new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr(
    new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));

  // Accessing the pointers like this is contrived. We only do this for
  // speed since we will be using the pointers in a tight inner loop.
  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer = iboBufferSPtr.get();
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer = vboBufferSPtr.get();

  //write to the IBO/VBOs
  for (auto a : indices)
    iboBuffer->write(a);

  for (size_t i = 0; i < points.size(); i++) {
    // Write first point on line
    vboBuffer->write(static_cast<float>(points.at(i).x()));
    vboBuffer->write(static_cast<float>(points.at(i).y()));
    vboBuffer->write(static_cast<float>(points.at(i).z()));
    // Write normal
    if (normals.size() == points.size()) {
      vboBuffer->write(static_cast<float>(normals.at(i).x()));
      vboBuffer->write(static_cast<float>(normals.at(i).y()));
      vboBuffer->write(static_cast<float>(normals.at(i).z()));
    }
    if (colorScheme == GeometryObject::COLOR_MAP)
      vboBuffer->write(static_cast<float>(colors.at(i).r()));
    else if (colorScheme == GeometryObject::COLOR_IN_SITU) {
      // Writes uint8_t out to the VBO. A total of 4 bytes.
      vboBuffer->write(COLOR_FTOB(colors.at(i).r()));
      vboBuffer->write(COLOR_FTOB(colors.at(i).g()));
      vboBuffer->write(COLOR_FTOB(colors.at(i).b()));
      vboBuffer->write(COLOR_FTOB(1.0));
    } // no color writing otherwise
  }
  state.set(RenderState::IS_ON, true);
  state.set(RenderState::HAS_DATA, true);

  GeometryObject::SpireVBO geomVBO = GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr,
    numVBOElements, mesh->get_bounding_box(), vboOnGPU);

  geom->mVBOs.push_back(geomVBO);

  // Construct IBO.

  GeometryObject::SpireIBO geomIBO = GeometryObject::SpireIBO(iboName, primIn, sizeof(uint32_t), iboBufferSPtr);

  geom->mIBOs.push_back(geomIBO);

  // Construct Pass.
  // Build pass for the edges.
  /// \todo Find an appropriate place to put program names like UniformColor.
  GeometryObject::SpireSubPass pass =
    GeometryObject::SpireSubPass(passName, vboName, iboName, shader, colorScheme, state, renderType, geomVBO, geomIBO);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  geom->mPasses.push_back(pass);
}


void ShowFieldModule::renderEdges(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
  RenderState state,
  Core::Datatypes::GeometryHandle geom,
  const std::string& id) {

  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  double sval0, sval1;
  Core::Geometry::Vector vval0, vval1;
  Core::Geometry::Tensor tval0, tval1;

  GeometryObject::ColorScheme colorScheme = GeometryObject::COLOR_UNIFORM;
  double scol0 = 0.0, scol1 = 0.0;
  Core::Datatypes::Material vcol0;
  Core::Datatypes::Material vcol1;

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

  // Attempt some form of precalculation of iboBuffer and vboBuffer size.
  uint32_t iboSize = 0;
  uint32_t vboSize = 0;
  
  auto my_state = this->get_state();
  double num_strips = double(my_state->getValue(CylinderResolution).toInt());
  double radius = my_state->getValue(CylinderRadius).toDouble();
  if (num_strips < 0) num_strips = 50.;
  if (radius < 0) radius = 1.;
  
  std::stringstream ss;
  ss << state.get(RenderState::USE_CYLINDER) << num_strips << radius;
  
  std::string uniqueNodeID = id + "edge" + ss.str();
  std::string vboName = uniqueNodeID + "VBO";
  std::string iboName = uniqueNodeID + "IBO";
  std::string passName = uniqueNodeID + "Pass";

  // NOTE: Attributes will depend on the color scheme. We will want to
  // normalize the colors if the color scheme is COLOR_IN_SITU.

  // Construct VBO.
  std::string shader = "Shaders/UniformColor";
  std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  if (state.get(RenderState::USE_CYLINDER))
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
  GeometryObject::RenderType renderType = GeometryObject::RENDER_VBO_IBO;

  // If true, then the VBO will be placed on the GPU. We don't want to place
  // VBOs on the GPU when we are generating rendering lists.
  bool vboOnGPU = true;

  std::vector<GeometryObject::SpireSubPass::Uniform> uniforms;
  //transparency
  if (state.get(RenderState::USE_TRANSPARENT_EDGES))
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uTransparency", (float)(edgeTransparencyValue_)));
  //coloring
  if (colorScheme == GeometryObject::COLOR_MAP) {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aFieldData", 1 * sizeof(float)));
    //push the color map parameters
    ColorMap * map = colorMap.get().get();
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMInvert",map->getColorMapInvert()?1.f:0.f));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMShift",static_cast<float>(map->getColorMapShift())));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uCMResolution",static_cast<float>(map->getColorMapResolution())));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uRescaleScale",static_cast<float>(map->getColorMapRescaleScale())));
    uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uRescaleShift",static_cast<float>(map->getColorMapRescaleShift())));
    if (state.get(RenderState::USE_CYLINDER)) {
        shader = "Shaders/DirPhongCMap" ;
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
          glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    } else {
        shader = "Shaders/ColorMap";
    }
  } else if (colorScheme == GeometryObject::COLOR_IN_SITU) {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 1 * sizeof(uint32_t), true));
    if (state.get(RenderState::USE_CYLINDER)) {
      shader = "Shaders/DirPhongInSitu";
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
        glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    } else {
      shader = "Shaders/InSituColor";
    }
  } else if (colorScheme == GeometryObject::COLOR_UNIFORM) {
    ColorRGB dft = state.defaultColor;
    if (state.get(RenderState::USE_CYLINDER)) {
        shader = "Shaders/DirPhong";
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uAmbientColor",
            glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uDiffuseColor",
            glm::vec4(dft.r(), dft.g(), dft.b(), (float)edgeTransparencyValue_)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularColor",
            glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
        uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uSpecularPower", 32.0f));
    } else {
        uniforms.emplace_back("uColor", glm::vec4(dft.r(), dft.g(), dft.b(), (float)edgeTransparencyValue_));
    }
  }
  GeometryObject::SpireIBO::PRIMITIVE primIn = GeometryObject::SpireIBO::LINES;
  // Use cylinders...
  if (state.get(RenderState::USE_CYLINDER))
    primIn = GeometryObject::SpireIBO::TRIANGLES;
  std::vector<Vector> points;
  std::vector<Vector> normals;
  std::vector<ColorRGB> colors;
  std::vector<uint32_t> indices;
  uint32_t index = 0;
  int64_t numVBOElements = 0;
  while (eiter != eiter_end) {
    VMesh::Node::array_type nodes;
    mesh->get_nodes(nodes, *eiter);

    Core::Geometry::Point p0, p1;
    mesh->get_point(p0, nodes[0]);
    mesh->get_point(p1, nodes[1]);
    //coloring options
    if (colorScheme != GeometryObject::COLOR_UNIFORM)
    {
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

        valueToColor(colorScheme, sval0, scol0, vcol0);
        valueToColor(colorScheme, sval1, scol1, vcol1);
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

        valueToColor(colorScheme, vval0, scol0, vcol0);
        valueToColor(colorScheme, vval1, scol1, vcol1);
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

        valueToColor(colorScheme, tval0, scol0, vcol0);
        valueToColor(colorScheme, tval1, scol1, vcol1);
      }
    }
    //accumulate VBO or IBO data
    if (state.get(RenderState::USE_CYLINDER) && p0 != p1) {
      //generate triangles for the cylinders.
      Vector n((p0 - p1).normal()), u = (10 * n + Vector(10, 10, 10)).normal();
      Vector crx = Cross(u, n).normal();
      u = Cross(crx, n).normal();
      Vector p;
      for (double strips = 0.; strips <= num_strips; strips += 1.) {
        uint32_t offset = (uint32_t)numVBOElements;
        p = std::cos(2. * M_PI * strips / num_strips) * u +
          std::sin(2. * M_PI * strips / num_strips) * crx;
        p.normalize();
        points.push_back(radius * p + Vector(p0));
        if (colorScheme == GeometryObject::COLOR_MAP)
          colors.push_back(ColorRGB(scol0, scol0, scol0));
        else if (colorScheme == GeometryObject::COLOR_IN_SITU)
          colors.push_back(vcol0.diffuse);
        numVBOElements++;
        points.push_back(radius * p + Vector(p1));
        if (colorScheme == GeometryObject::COLOR_MAP)
          colors.push_back(ColorRGB(scol1, scol1, scol1));
        else if (colorScheme == GeometryObject::COLOR_IN_SITU)
          colors.push_back(vcol1.diffuse);
        numVBOElements++;
        normals.push_back(p);
        normals.push_back(p);
        indices.push_back(0 + offset);
        indices.push_back(1 + offset);
        indices.push_back(2 + offset);
        indices.push_back(2 + offset);
        indices.push_back(1 + offset);
        indices.push_back(3 + offset);
      }
      for (int jj = 0; jj < 6; jj++) indices.pop_back();
      //generate triangles for the spheres
      Vector pp1, pp2;
      double theta_inc = 2. * M_PI / num_strips, phi_inc = M_PI / num_strips;
      std::vector<Point> epts = { { p0, p1 } };
      for (auto a : epts) {
        double col = a == p0 ? scol0 : scol1;
        Material rgbcol = a == p0 ? vcol0 : vcol1;
        for (double phi = 0.; phi <= M_PI; phi += phi_inc) {
          for (double theta = 0.; theta <= 2. * M_PI; theta += theta_inc) {
            uint32_t offset = (uint32_t)numVBOElements;
            pp1 = Vector(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            pp2 = Vector(sin(theta) * cos(phi + phi_inc), sin(theta) * sin(phi + phi_inc), cos(theta));
            points.push_back(radius * pp1 + Vector(a));
            if (colorScheme == GeometryObject::COLOR_MAP)
              colors.push_back(ColorRGB(col, col, col));
            else if (colorScheme == GeometryObject::COLOR_IN_SITU)
                colors.push_back(rgbcol.diffuse);
            numVBOElements++;
            points.push_back(radius * pp2 + Vector(a));
            if (colorScheme == GeometryObject::COLOR_MAP)
              colors.push_back(ColorRGB(col, col, col));
            else if (colorScheme == GeometryObject::COLOR_IN_SITU)
                colors.push_back(rgbcol.diffuse);
            numVBOElements++;
            normals.push_back(pp1);
            normals.push_back(pp2);
            indices.push_back(0 + offset);
            indices.push_back(1 + offset);
            indices.push_back(2 + offset);
            indices.push_back(2 + offset);
            indices.push_back(1 + offset);
            indices.push_back(3 + offset);
          }
          for (int jj = 0; jj < 6; jj++) indices.pop_back();
        }
      }
    } else {
      points.push_back(Vector(p0));
      if (colorScheme == GeometryObject::COLOR_MAP)
        colors.push_back(ColorRGB(scol0, scol0, scol0));
      else if (colorScheme == GeometryObject::COLOR_IN_SITU)
        colors.push_back(vcol0.diffuse);
      indices.push_back(index);
      ++index;
      points.push_back(Vector(p1));
      if (colorScheme == GeometryObject::COLOR_MAP)
        colors.push_back(ColorRGB(scol1, scol1, scol1));
      else if (colorScheme == GeometryObject::COLOR_IN_SITU)
        colors.push_back(vcol1.diffuse);
      indices.push_back(index);
      ++index;
      ++numVBOElements;
    }

    ++eiter;
  }

  vboSize = (uint32_t)points.size() * 3 * sizeof(float);
  vboSize += (uint32_t)normals.size() * 3 * sizeof(float);
  if (colorScheme == GeometryObject::COLOR_IN_SITU || colorScheme == GeometryObject::COLOR_MAP)
    vboSize += (uint32_t)colors.size() * 4; //add last 4 bytes for color
  iboSize = (uint32_t)indices.size() * sizeof(uint32_t);

  /// \todo To reduce memory requirements, we can use a 16bit index buffer.

  /// \todo To further reduce a large amount of memory, get rid of the index
  ///       buffer and use glDrawArrays to render without an IBO. An IBO is
  ///       a waste of space.
  ///       http://www.opengl.org/sdk/docs/man3/xhtml/glDrawArrays.xml

  /// \todo Switch to unique_ptrs and move semantics.
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr(
    new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr(
    new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));

  // Accessing the pointers like this is contrived. We only do this for
  // speed since we will be using the pointers in a tight inner loop.
  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer = iboBufferSPtr.get();
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer = vboBufferSPtr.get();

  //write to the IBO/VBOs
  for (auto a : indices)
    iboBuffer->write(a);

  for (size_t i = 0; i < points.size(); i++) {
    // Write first point on line
    vboBuffer->write(static_cast<float>(points.at(i).x()));
    vboBuffer->write(static_cast<float>(points.at(i).y()));
    vboBuffer->write(static_cast<float>(points.at(i).z()));
    // Write normal
    if (normals.size() == points.size()) {
      vboBuffer->write(static_cast<float>(normals.at(i).x()));
      vboBuffer->write(static_cast<float>(normals.at(i).y()));
      vboBuffer->write(static_cast<float>(normals.at(i).z()));
    }
    if (colorScheme == GeometryObject::COLOR_MAP)
      vboBuffer->write(static_cast<float>(colors.at(i).r()));
    else if (colorScheme == GeometryObject::COLOR_IN_SITU) {
      // Writes uint8_t out to the VBO. A total of 4 bytes.
      vboBuffer->write(COLOR_FTOB(colors.at(i).r()));
      vboBuffer->write(COLOR_FTOB(colors.at(i).g()));
      vboBuffer->write(COLOR_FTOB(colors.at(i).b()));
      vboBuffer->write(COLOR_FTOB(1.0));
    } // no color writing otherwise
  }
  state.set(RenderState::IS_ON, true);
  state.set(RenderState::HAS_DATA, true);

  GeometryObject::SpireVBO geomVBO = GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr,
    numVBOElements, mesh->get_bounding_box(), vboOnGPU);

  geom->mVBOs.push_back(geomVBO);

  // Construct IBO.

  GeometryObject::SpireIBO geomIBO = GeometryObject::SpireIBO(iboName, primIn, sizeof(uint32_t), iboBufferSPtr);

  geom->mIBOs.push_back(geomIBO);

  // Construct Pass.
  // Build pass for the edges.
  /// \todo Find an appropriate place to put program names like UniformColor.
  GeometryObject::SpireSubPass pass =
    GeometryObject::SpireSubPass(passName, vboName, iboName, shader, colorScheme, state, renderType, geomVBO, geomIBO);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  geom->mPasses.push_back(pass);
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
