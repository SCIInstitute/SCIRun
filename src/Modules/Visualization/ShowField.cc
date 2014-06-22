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
#include <Core/Algorithms/Visualization/DataConversions.h>
#include <Core/Algorithms/Visualization/RenderFieldState.h>

#include <boost/foreach.hpp>

#include <glm/glm.hpp>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

ModuleLookupInfo ShowFieldModule::staticInfo_("ShowField", "Visualization", "SCIRun");

ShowFieldModule::ShowFieldModule() : 
    Module(staticInfo_)
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
  state->setValue(DefaultMeshColor, ColorRGB(1.0, 1.0, 1.0).toString());

  // NOTE: We need to add radio buttons for USE_DEFAULT_COLOR, COLORMAP, and
  // COLOR_CONVERT. USE_DEFAULT_COLOR is selected by default. COLOR_CONVERT
  // is more up in the air.
}

void ShowFieldModule::execute()
{
  boost::shared_ptr<SCIRun::Field> field = getRequiredInput(Field);
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap = getOptionalInput(ColorMapObject);
  GeometryHandle geom = buildGeometryObject(field, colorMap, get_state(), get_id());
  sendOutput(SceneGraph, geom);
}

RenderState ShowFieldModule::getNodeRenderState(
    ModuleStateHandle state,
    boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap)
{
  RenderState renState;

  renState.set(RenderState::IS_ON, state->getValue(ShowFieldModule::ShowNodes).getBool());
  renState.set(RenderState::USE_TRANSPARENCY, state->getValue(ShowFieldModule::NodeTransparency).getBool());

  renState.set(RenderState::USE_SPHERE, state->getValue(ShowFieldModule::NodeAsSpheres).getBool());
  if (state->getValue(ShowFieldModule::NodeAsSpheres).getBool())
  {
    std::cout << "Nodes as spheres!" << std::endl;
  }

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldModule::DefaultMeshColor).getString());

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

  renState.set(RenderState::IS_ON, state->getValue(ShowFieldModule::ShowEdges).getBool());
  renState.set(RenderState::USE_TRANSPARENCY, state->getValue(ShowFieldModule::EdgeTransparency).getBool());

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldModule::DefaultMeshColor).getString());

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

  renState.set(RenderState::IS_ON, state->getValue(ShowFieldModule::ShowFaces).getBool());
  renState.set(RenderState::USE_TRANSPARENCY, state->getValue(ShowFieldModule::FaceTransparency).getBool());

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldModule::DefaultMeshColor).getString());

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
    ModuleStateHandle state, 
    const std::string& id)
{
  // Function for reporting progress.
  SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc progressFunc =
      getUpdaterFunc();

  /// \todo Determine a better way of handling all of the various object state.
  bool showNodes = state->getValue(ShowFieldModule::ShowNodes).getBool();
  bool showEdges = state->getValue(ShowFieldModule::ShowEdges).getBool();
  bool showFaces = state->getValue(ShowFieldModule::ShowFaces).getBool();

  // Resultant geometry type (representing a spire object and a number of passes).
  GeometryHandle geom(new GeometryObject(field));
  geom->objectName = id;

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
    renderNodes(field, colorMap, getNodeRenderState(state, colorMap), geom, id);
  }

  if (showFaces)
  {
    int approxDiv = 1;
    renderFaces(field, colorMap, getFaceRenderState(state, colorMap), geom, approxDiv, id);
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

void ShowFieldModule::applyColorMapScaling(
    boost::shared_ptr<SCIRun::Field> field,
    GeometryObject::SpireSubPass& pass)
{
  // Rescale color maps if that is the input paradigm we are using.
  // initialize the following so that the compiler will stop
  // warning us about possibly using unitialized variables
  double minv = std::numeric_limits<double>::max();
  double maxv = std::numeric_limits<double>::lowest();

  VField* fld   = field->vfield();
  if (!(field->vfield()->minmax(minv, maxv)))
  {
    std::cerr << "Input field is not a scalar or vector field." << std::endl;
    return;
  } 

  pass.addUniform("uMinVal", minv);
  pass.addUniform("uMaxVal", maxv);

  // if ( gui_make_symmetric_.get() ) 
  // {
  //   float biggest = Max(Abs(minmax_.first), Abs(minmax_.second));
  //   minmax_.first  = -biggest;
  //   minmax_.second =  biggest;
  // }
}

void ShowFieldModule::renderFaces(
    boost::shared_ptr<SCIRun::Field> field,
    boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
    RenderState state, Core::Datatypes::GeometryHandle geom, 
    unsigned int approxDiv,
    const std::string& id)
{
  VField* fld   = field->vfield();
  VMesh*  mesh  = field->vmesh();

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
  VField* fld   = field->vfield();
  VMesh*  mesh  = field->vmesh();

  bool withNormals = (state.get(RenderState::USE_NORMALS) && mesh->has_normals());

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

  if (withNormals) {mesh->synchronize(Mesh::NORMALS_E);}

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
  uint32_t iboSize = mesh->num_faces() * sizeof(uint32_t);
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
  while (fiter != fiterEnd) 
  {
    mesh->get_nodes(nodes, *fiter);
 
    std::vector<Core::Geometry::Point> points(nodes.size());
    std::vector<Core::Geometry::Vector> normals(nodes.size());

    for (size_t i = 0; i < nodes.size(); i++)
    {
      mesh->get_point(points[i], nodes[i]);
    }

    if (withNormals) 
    {
      for (size_t i = 0; i < nodes.size(); i++)
      {
        mesh->get_normal(normals[i], nodes[i]);
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
        
        valueToColor( colorScheme, svals[0], scols[0], vcols[0] );
        valueToColor( colorScheme, svals[1], scols[1], vcols[1] );
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
        
        valueToColor( colorScheme, vvals[0], scols[0], vcols[0] );
        valueToColor( colorScheme, vvals[1], scols[1], vcols[1] );
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
        
        valueToColor( colorScheme, tvals[0], scols[0], vcols[0] );
        valueToColor( colorScheme, tvals[1], scols[1], vcols[1] );
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
        valueToColor( colorScheme, svals[0], scols[0], vcols[0] );
      }
      else if (fld->is_vector())
      {
        fld->get_value(vvals[0], *fiter);
        valueToColor( colorScheme, vvals[0], scols[0], vcols[0] );
      }
      else if (fld->is_tensor())
      {
        fld->get_value(tvals[0], *fiter);
        valueToColor( colorScheme, tvals[0], scols[0], vcols[0] );
      }

      // Same color at all corners.
      for(size_t i=0; i<nodes.size(); ++i)
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
        for (size_t i=0; i<nodes.size(); i++)
        {
          fld->get_value(svals[i], nodes[i]);
          valueToColor( colorScheme, svals[i], scols[i], vcols[i] );
        }
      }
      else if (fld->is_vector())
      {
        for (size_t i=0; i<nodes.size(); i++)
        {
          fld->get_value(vvals[i], nodes[i]);
          valueToColor( colorScheme, vvals[i], scols[i], vcols[i] );
        }
      }      
      else if (fld->is_tensor())
      {
        for (size_t i=0; i<nodes.size(); i++)
        {
          fld->get_value(tvals[i], nodes[i]);
          valueToColor( colorScheme, tvals[i], scols[i], vcols[i] );
        }
      }
      
      addFaceGeom(points, normals, withNormals, iboIndex, iboBuffer, vboBuffer,
                  colorScheme, scols, vcols, state);
    }

    ++fiter;
    ++numVBOElements;
  }

  std::string uniqueNodeID = id + "face";
  std::string vboName      = uniqueNodeID + "VBO";
  std::string iboName      = uniqueNodeID + "IBO";
  std::string passName     = uniqueNodeID + "Pass";

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

    if (state.get(RenderState::IS_DOUBLE_SIDED) == false)
    {
      if (withNormals)
      {
        // Use colormapping lit shader.
        shader = "Shaders/DirPhongCMap";
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
      }
      else
      {
        // Use colormapping only shader.
        shader = "Shaders/DblSided_ColorMap";
      }
    }

    if (state.get(RenderState::USE_TRANSPARENCY))
    {
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uTransparency", (float)(0.75f)));
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
        std::cerr << "ERROR - In situ phong not implemented." << std::endl;
        shader = "Shaders/InSituPhongColor";
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
        std::cerr << "ERROR - Double sided in situ phong not implemented." << std::endl;
        shader = "Shaders/DblSided_InSituPhongColor";
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

    shader = "Shaders/UniformColor";

    if (state.get(RenderState::USE_TRANSPARENCY))
    {
      /// \todo Add transparency slider.
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform(
              "uColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), 0.7f)));
    }
    else
    {
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform(
              "uColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), 1.0f)));
    }
  }

  geom->mVBOs.push_back(GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr,
                                                 numVBOElements, mesh->get_bounding_box(), true));

  // Construct IBO.
  geom->mIBOs.push_back(
      GeometryObject::SpireIBO(iboName, GeometryObject::SpireIBO::TRIANGLES, sizeof(uint32_t), iboBufferSPtr));

  // Construct Pass.
  // Build pass for the edges.
  /// \todo Find an appropriate place to put program names like UniformColor.
  GeometryObject::SpireSubPass pass =
      GeometryObject::SpireSubPass(passName, vboName, iboName, shader,
                                   colorScheme, state, GeometryObject::RENDER_VBO_IBO);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  if (colorScheme == GeometryObject::COLOR_MAP)
  {
    applyColorMapScaling(field, pass);
  }

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

          writeVBOPoint(points[i-1]);
          writeVBONormal(normals[i-1]);
          writeIBOIndex(iboIndex + i-1);

          writeVBOPoint(points[i]);
          writeVBONormal(normals[i]);
          writeIBOIndex(iboIndex + i);
        }
        else
        {
          writeVBOPoint(points[0]);
          writeIBOIndex(iboIndex);

          writeVBOPoint(points[i-1]);
          writeIBOIndex(iboIndex + i-1);

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
        if (!doubleSided) {writeVBOScalarValue(scols[0]);}
        else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}

        writeVBOPoint(points[1]);
        writeVBONormal(normals[1]);
        writeVBOScalarValue(scols[1]);
        if (!doubleSided) {writeVBOScalarValue(scols[1]);}
        else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}

        writeVBOPoint(points[2]);
        writeVBONormal(normals[2]);
        if (!doubleSided) {writeVBOScalarValue(scols[2]);}
        else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}

        writeVBOPoint(points[3]);
        writeVBONormal(normals[3]);
        if (!doubleSided) {writeVBOScalarValue(scols[3]);}
        else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}
      }
      else
      {
        writeVBOPoint(points[0]);
        if (!doubleSided) {writeVBOScalarValue(scols[0]);}
        else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}

        writeVBOPoint(points[1]);
        if (!doubleSided) {writeVBOScalarValue(scols[1]);}
        else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}

        writeVBOPoint(points[2]);
        if (!doubleSided) {writeVBOScalarValue(scols[2]);}
        else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}

        writeVBOPoint(points[3]);
        if (!doubleSided) {writeVBOScalarValue(scols[3]);}
        else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}
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
          if (!doubleSided) {writeVBOScalarValue(scols[0]);}
          else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}
          writeIBOIndex(iboIndex);

          writeVBOPoint(points[i-1]);
          writeVBONormal(normals[i-1]);
          if (!doubleSided) {writeVBOScalarValue(scols[i-1]);}
          else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}
          writeIBOIndex(iboIndex + i-1);

          writeVBOPoint(points[i]);
          writeVBONormal(normals[i]);
          if (!doubleSided) {writeVBOScalarValue(scols[i]);}
          else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}
          writeIBOIndex(iboIndex + i);
        }
        else
        {
          // Render points if we are not rendering spheres.
          writeVBOPoint(points[0]);
          if (!doubleSided) {writeVBOScalarValue(scols[0]);}
          else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}
          writeIBOIndex(iboIndex);

          writeVBOPoint(points[i-1]);
          if (!doubleSided) {writeVBOScalarValue(scols[i-1]);}
          else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}
          writeIBOIndex(iboIndex + i-1);

          writeVBOPoint(points[i]);
          if (!doubleSided) {writeVBOScalarValue(scols[i]);}
          else              {writeVBOScalarValue(scols[0]); writeVBOScalarValue(scols[1]);}
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
        if (!doubleSided) {writeVBO4ByteColor(vcols[0]);}
        else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}

        writeVBOPoint(points[1]);
        writeVBONormal(normals[1]);
        if (!doubleSided) {writeVBO4ByteColor(vcols[1]);}
        else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}

        writeVBOPoint(points[2]);
        writeVBONormal(normals[2]);
        if (!doubleSided) {writeVBO4ByteColor(vcols[2]);}
        else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}

        writeVBOPoint(points[3]);
        writeVBONormal(normals[3]);
        if (!doubleSided) {writeVBO4ByteColor(vcols[3]);}
        else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}
      }
      else
      {
        writeVBOPoint(points[0]);
        if (!doubleSided) {writeVBO4ByteColor(vcols[0]);}
        else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}

        writeVBOPoint(points[1]);
        if (!doubleSided) {writeVBO4ByteColor(vcols[1]);}
        else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}

        writeVBOPoint(points[2]);
        if (!doubleSided) {writeVBO4ByteColor(vcols[2]);}
        else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}

        writeVBOPoint(points[3]);
        if (!doubleSided) {writeVBO4ByteColor(vcols[3]);}
        else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}
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
          if (!doubleSided) {writeVBO4ByteColor(vcols[0]);}
          else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}
          writeIBOIndex(iboIndex);

          writeVBOPoint(points[i-1]);
          writeVBONormal(normals[i-1]);
          if (!doubleSided) {writeVBO4ByteColor(vcols[i-1]);}
          else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}
          writeIBOIndex(iboIndex + i-1);

          writeVBOPoint(points[i]);
          writeVBONormal(normals[i]);
          if (!doubleSided) {writeVBO4ByteColor(vcols[i]);}
          else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}
          writeIBOIndex(iboIndex + i);
        }
        else
        {
          writeVBOPoint(points[0]);
          if (!doubleSided) {writeVBO4ByteColor(vcols[0]);}
          else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}
          iboBuffer->write(iboIndex);

          writeVBOPoint(points[i-1]);
          if (!doubleSided) {writeVBO4ByteColor(vcols[i-1]);}
          else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}
          writeIBOIndex(iboIndex + i-1);

          writeVBOPoint(points[i]);
          if (!doubleSided) {writeVBO4ByteColor(vcols[i]);}
          else              {writeVBO4ByteColor(vcols[0]); writeVBO4ByteColor(vcols[1]);}
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
  VField* fld   = field->vfield();
  VMesh*  mesh  = field->vmesh();

  double sval;
  Core::Geometry::Vector vval;
  Core::Geometry::Tensor tval;

  GeometryObject::ColorScheme colorScheme = GeometryObject::COLOR_UNIFORM;
  double scol;
  Core::Datatypes::Material vcol;

  if (fld->basis_order() < 0 ||
      (fld->basis_order() == 0 && mesh->dimensionality() != 0) ||
      state.get(RenderState::USE_DEFAULT_COLOR))
  {
    colorScheme = GeometryObject::COLOR_UNIFORM;
  }
  else if (state.get(RenderState::USE_COLORMAP))
  {
    colorScheme = GeometryObject::COLOR_MAP;
  }
  else
  {
    colorScheme = GeometryObject::COLOR_IN_SITU;
  }

  mesh->synchronize(Mesh::NODES_E);

  VMesh::Node::iterator iter, iter_end;
  mesh->begin(iter);
  mesh->end(iter_end);

  // Attempt some form of precalculation of iboBuffer and vboBuffer size.
  int iboSize = mesh->num_nodes() * sizeof(uint32_t);
  int vboSize = mesh->num_nodes() * sizeof(float) * 3;

  if (colorScheme == GeometryObject::COLOR_MAP)
  {
    vboSize += mesh->num_nodes() * sizeof(float);     // For the data (color map lookup).
  }
  else if (colorScheme == GeometryObject::COLOR_IN_SITU)
  {
    vboSize += mesh->num_nodes() * sizeof(uint32_t); // For full color in the elements.
  }

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

  uint32_t index = 0;
  int64_t numVBOElements = 0;
  while (iter != iter_end)
  {
    Core::Geometry::Point p;
    mesh->get_point(p, *iter);

    if (colorScheme != GeometryObject::COLOR_UNIFORM)
    {
      if (fld->is_scalar())
      {
        fld->get_value(sval, *iter);
        valueToColor(colorScheme, sval, scol, vcol);
      }
      if (fld->is_vector())
      {
        fld->get_value(vval, *iter);
        valueToColor(colorScheme, vval, scol, vcol);
      }
      if (fld->is_tensor())
      {
        fld->get_value(tval, *iter);
        valueToColor(colorScheme, tval, scol, vcol);
      }
    }

    if (!state.get(RenderState::USE_SPHERE))
    {
      // Render points if we are not rendering spheres.
      vboBuffer->write(static_cast<float>(p.x()));
      vboBuffer->write(static_cast<float>(p.y()));
      vboBuffer->write(static_cast<float>(p.z()));

      if (colorScheme == GeometryObject::COLOR_MAP)
      {
        vboBuffer->write(static_cast<float>(scol));
      }
      else if (colorScheme == GeometryObject::COLOR_IN_SITU)
      {
        // Writes uint8_t out to the VBO. A total of 4 bytes.
        vboBuffer->write(COLOR_FTOB(vcol.diffuse.r()));
        vboBuffer->write(COLOR_FTOB(vcol.diffuse.g()));
        vboBuffer->write(COLOR_FTOB(vcol.diffuse.b()));
        vboBuffer->write(COLOR_FTOB(vcol.transparency));
      }

      iboBuffer->write(static_cast<uint32_t>(index));
    }
    else
    {
      // Spheres. I don't believe we need this.
    }

    ++index;
    ++iter;
    ++numVBOElements;
  }

  std::string uniqueNodeID = id + "node";
  std::string vboName      = uniqueNodeID + "VBO";
  std::string iboName      = uniqueNodeID + "IBO";
  std::string passName     = uniqueNodeID + "Pass";

  // NOTE: Attributes will depend on the color scheme. We will want to
  // normalize the colors if the color scheme is COLOR_IN_SITU.

  // Construct VBO.
  std::string shader = "Shaders/UniformColor";
  std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  GeometryObject::RenderType renderType = GeometryObject::RENDER_VBO_IBO;

  // If true, then the VBO will be placed on the GPU. We don't want to place
  // VBOs on the GPU when we are generating rendering lists.
  bool vboOnGPU = true;

  std::vector<GeometryObject::SpireSubPass::Uniform> uniforms;

  if (colorScheme == GeometryObject::COLOR_MAP)
  {
    shader = "Shaders/ColorMap";
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aFieldData", 1 * sizeof(float)));

    if (state.get(RenderState::USE_TRANSPARENCY))
    {
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uTransparency", (float)(0.75f)));
    }
    else
    {
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform("uTransparency", (float)(1.0f)));
    }
  }
  else if (colorScheme == GeometryObject::COLOR_IN_SITU)
  {
    shader = "Shaders/InSituColor";
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 1 * sizeof(uint32_t), true));
  }
  else if (colorScheme == GeometryObject::COLOR_UNIFORM)
  {
    ColorRGB defaultColor = state.defaultColor;

    shader = "Shaders/UniformColor";

    if (state.get(RenderState::USE_TRANSPARENCY))
    {
      /// \todo Add transparency slider.
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform(
              "uColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), 0.7f)));
    }
    else
    {
      uniforms.push_back(GeometryObject::SpireSubPass::Uniform(
              "uColor", glm::vec4(defaultColor.r(), defaultColor.g(), defaultColor.b(), 1.0f)));
    }
  }

  if (state.get(RenderState::USE_SPHERE))
  {
    renderType = GeometryObject::RENDER_RLIST_SPHERE;
    vboOnGPU = false;
  }

  geom->mVBOs.push_back(GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr,
                                                 numVBOElements, mesh->get_bounding_box(), vboOnGPU));

  // Construct IBO.
  geom->mIBOs.push_back(
      GeometryObject::SpireIBO(iboName, GeometryObject::SpireIBO::POINTS, sizeof(uint32_t), iboBufferSPtr));

  // Construct Pass.
  // Build pass for the edges.
  /// \todo Find an appropriate place to put program names like UniformColor.
  GeometryObject::SpireSubPass pass =
      GeometryObject::SpireSubPass(passName, vboName, iboName, shader,
                                   colorScheme, state, renderType);

  pass.addUniform("uColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

  geom->mPasses.push_back(pass);

  /// \todo Add spheres and other glyphs as display lists. Will want to
  ///       build up to geometry / tesselation shaders if support is present.
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
AlgorithmParameterName ShowFieldModule::DefaultMeshColor("DefaultMeshColor");


