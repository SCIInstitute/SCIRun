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

    // Build min-max
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

  geom->mVBOs.push_back(GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr, mesh->get_bounding_box()));

  // Construct IBO.
  geom->mIBOs.push_back(
      GeometryObject::SpireIBO(iboName, GeometryObject::SpireIBO::TRIANGLES, sizeof(uint32_t), iboBufferSPtr));

  // Construct Pass.
  // Build pass for the edges.
  /// \todo Find an appropriate place to put program names like UniformColor.
  GeometryObject::SpireSubPass pass =
      GeometryObject::SpireSubPass(passName, vboName, iboName, shader, colorScheme, state);

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
    /// \note There's some extra initialization that SCIRun4 would perform here.
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
      // Spheres.
    }

    ++index;
    ++iter; 
  }

  std::string uniqueNodeID = id + "node";
  std::string vboName      = uniqueNodeID + "VBO";
  std::string iboName      = uniqueNodeID + "IBO";
  std::string passName     = uniqueNodeID + "Pass";

  // NOTE: Attributes will depend on the color scheme. We will want to
  // normalize the colors if the color scheme is COLOR_IN_SITU.

  // Construct VBO.
  std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
  attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  if (colorScheme == GeometryObject::COLOR_MAP)
  {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aFieldData", 1 * sizeof(float)));
  }
  else if (colorScheme == GeometryObject::COLOR_IN_SITU)
  {
    attribs.push_back(GeometryObject::SpireVBO::AttributeData("aColor", 1 * sizeof(uint32_t), true));
  }

  geom->mVBOs.push_back(GeometryObject::SpireVBO(vboName, attribs, vboBufferSPtr, mesh->get_bounding_box()));

  // Construct IBO.
  geom->mIBOs.push_back(
      GeometryObject::SpireIBO(iboName, GeometryObject::SpireIBO::POINTS, sizeof(uint32_t), iboBufferSPtr));

  // Construct Pass.
  // Build pass for the edges.
  /// \todo Find an appropriate place to put program names like UniformColor.
  GeometryObject::SpireSubPass pass =
      GeometryObject::SpireSubPass(passName, vboName, iboName, "Shaders/UniformColor", colorScheme, state);

  pass.addUniform("uColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

  geom->mPasses.push_back(pass);

  /// \todo Add spheres and other glyphs as display lists. Will want to
  ///       build up to geometry / tesselation shaders if support is present.
}






























// GeometryHandle ShowFieldModule::oldBuildGeometryObject(
//     boost::shared_ptr<SCIRun::Field> field,
//     boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
//     ModuleStateHandle state, 
//     const std::string& id)
// {
//   // Function for reporting progress.
//   SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc progressFunc =
//       getUpdaterFunc();
//
//   /// \todo Determine a better way of handling all of the various object state.
//   bool showNodes = state->getValue(ShowFieldModule::ShowNodes).getBool();
//   bool showEdges = state->getValue(ShowFieldModule::ShowEdges).getBool();
//   bool showFaces = state->getValue(ShowFieldModule::ShowFaces).getBool();
//
//   // Resultant geometry type (representing a spire object and a number of passes).
//   GeometryHandle geom(new GeometryObject(field));
//   geom->objectName = id;
//
//
//   /// \todo Implement inputs_changes_ ? See old scirun ShowField.cc:293.
//
//   /// \todo Mind material properties (simple since we already have implemented
//   ///       most of this).
//
//   /// \todo Handle assignment of color map. The color map will need to be
//   ///       available to us as we are building the meshes. Due to the way
//   ///       SCIRun expects meshes to be built.
//
//   /// \todo render_state_ DIRTY flag? See old scirun ShowField.cc:446.
//
//   const int dim = field->vmesh()->dimensionality();
//   if (showEdges && dim < 1) { showEdges = false; }
//   if (showFaces && dim < 2) { showFaces = false; }
//
//   if (showNodes)
//   {
//     // Construct node geometry.
//     
//   }
//
//
//   // VMesh facade. A simpler interface to the vmesh type.
//   SCIRun::Core::Datatypes::MeshTraits<VMesh>::MeshFacadeHandle facade =
//       field->mesh()->getFacade();
//   ENSURE_NOT_NULL(facade, "Mesh facade");
//
//   // VField required only for extracting field data from the field datatype.
//   VField* vfield = field->vfield();
//
//   // Grab the vmesh object so that we can synchronize and extract normals.
//   VMesh* vmesh = field->vmesh();
//
//   // Ensure any changes made to the mesh are reflected in the normals by
//   // by synchronizing the mesh.
//   if (vmesh->has_normals())
//     vmesh->synchronize(Mesh::NORMALS_E);
//   
//   bool invertNormals = state->getValue(ShowFieldModule::FaceInvertNormals).getBool();
//   //bool nodeTransparency = state->getValue(ShowFieldModule::NodeTransparency).getBool();
//   const ColorRGB meshColor(state->getValue(ShowFieldModule::DefaultMeshColor).getString());
//   float meshRed   = static_cast<float>(meshColor.r() / 255.0f);
//   float meshGreen = static_cast<float>(meshColor.g() / 255.0f);
//   float meshBlue  = static_cast<float>(meshColor.b() / 255.0f);
//
//   /// \todo Split the mesh into chunks of about ~32,000 vertices. May be able to
//   ///       eek out better coherency and use a 16 bit index buffer instead of
//   ///       a 32 bit index buffer.
//
//   // Crude method of counting the attributes we are placing in the VBO.
//   int numFloats = 3 + 1;  // Position + field data.
//   if (vmesh->has_normals())
//   {
//     numFloats += 3;       // Position + field data + normals;
//   }
//
//   // Allocate memory for vertex buffer.
//   // Edges and faces should use the same vbo!
//   std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
//   size_t vboSize = sizeof(float) * numFloats * facade->numNodes();
//   rawVBO->resize(vboSize); // linear complexity.
//   // The C++ standard guarantees that vectors are contiguous in memory, so we
//   // grab a pointer to the first element and use that as the starting point
//   // for building our VBO.
//   float* vbo = reinterpret_cast<float*>(&(*rawVBO)[0]);
//
//   if (progressFunc) progressFunc(0.1);
//
//   // Build normalization bounds for field data. This simplifies the fragment
//   // shader which will improve rendering performance.
//   double valueRangeLow = std::numeric_limits<double>::max();
//   double valueRangeHigh = std::numeric_limits<double>::lowest();
//   BOOST_FOREACH(const NodeInfo<VMesh>& node, facade->nodes())
//   {
//     if (node.index() < vfield->num_values())
//     {
//       double val = 0.0;
//       vfield->get_value(val, node.index());
//       if (val > valueRangeHigh) valueRangeHigh = val;
//       if (val < valueRangeLow) valueRangeLow = val;
//     }
//   }
//   double valueRange = valueRangeHigh - valueRangeLow;
//   LOG_DEBUG("valueRange " << valueRange << std::endl);
//   LOG_DEBUG("valueRangeHigh " << valueRangeHigh << std::endl);
//   LOG_DEBUG("valueRangeLow " << valueRangeLow << std::endl);
//
//   // Build vertex buffer.
//   size_t i = 0;
//   Core::Geometry::BBox aabb;
//   BOOST_FOREACH(const NodeInfo<VMesh>& node, facade->nodes())
//   {
//     // Add position (aPos)
//     size_t nodeOffset = 0;
//     vbo[i+nodeOffset+0] = node.point().x();
//     vbo[i+nodeOffset+1] = node.point().y();
//     vbo[i+nodeOffset+2] = node.point().z();
//     nodeOffset += 3;
//     if (aabb.valid() == false)
//       // First point in the AABB. Extend by a small radius away from this point
//       // to ensure we have a valid AABB.
//       aabb.extend(node.point(), 0.001f);
//     else
//       // Simply extend by this point.
//       aabb.extend(node.point());
//
//     // Add optional normals (aNormal)
//     if (vmesh->has_normals())
//     {
//       Core::Geometry::Vector normal;
//       vmesh->get_normal(normal, node.index());
//
//       if (!invertNormals)
//       {
//         vbo[i+nodeOffset+0] = normal.x();
//         vbo[i+nodeOffset+1] = normal.y();
//         vbo[i+nodeOffset+2] = normal.z();
//       }
//       else
//       {
//         vbo[i+nodeOffset+0] = -normal.x();
//         vbo[i+nodeOffset+1] = -normal.y();
//         vbo[i+nodeOffset+2] = -normal.z();
//       }
//       nodeOffset += 3;
//     }
//
//     // Add field data (aFieldData)
//     if (node.index() < vfield->num_values() && valueRange > 0.0)
//     {
//       double val = 0.0;
//       vfield->get_value(val, node.index());
//       vbo[i+nodeOffset] = static_cast<float>((val - valueRangeLow) / valueRange);
//       //std::cout << "value quant: " << vbo[i+nodeOffset] << std::endl;
//       //std::cout << "value: " << val << std::endl;
//     }
//     else
//     {
//       vbo[i+nodeOffset] = 0.0f;
//     }
//     nodeOffset += 1;
//
//     i += nodeOffset;
//   }
//
//   // Set value ranges for color mapping fields.
//   geom->mLowestValue = valueRangeLow;
//   geom->mHighestValue = valueRangeHigh;
//   if (colorMap)
//   {
//     geom->mColorMap = boost::optional<std::string>((*colorMap)->getColorMapName());
//   }
//   else
//   {
//     geom->mColorMap = boost::optional<std::string>();
//   }
//
//   // Add shared VBO to the geometry object.
//   /// \note This 'primaryVBO' is dependent on the types present in the data.
//   ///       If normals are specified, then this will NOT be the primary VBO.
//   ///       Another VBO will be constructed containing normal information.
//   ///       All of this is NOT necessary if we are on OpenGL 3.2+ where we
//   ///       can compute all normals in the geometry shader (smooth and face).
//   std::string primVBOName = id + "primaryVBO";
//   std::vector<GeometryObject::SpireVBO::AttributeData> attribs;
//   attribs.push_back(GeometryObject::SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
//   if (vmesh->has_normals())
//   {
//     attribs.push_back(GeometryObject::SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
//   }
//   attribs.push_back(GeometryObject::SpireVBO::AttributeData("aFieldData", 1 * sizeof(float)));
//   geom->mVBOs.push_back(GeometryObject::SpireVBO(primVBOName, attribs, rawVBO, aabb));
//
//   if (progressFunc) progressFunc(0.25);
//
//   // Build the edges
//   if (showEdges)
//   {
//     std::string iboName = id + "edgesIBO";
//     buildEdgesIBO(facade, geom, iboName);
//
//     // Build pass for the edges.
//     /// \todo Find an appropriate place to put program names like UniformColor.
//     GeometryObject::SpireSubPass pass =
//         GeometryObject::SpireSubPass(id + "edgesPass", primVBOName, iboName,
//                                      "Shaders/UniformColor");
//
//     //spire::GPUState gpuState;
//     //gpuState.mLineWidth = 2.5f;
//     //pass.addGPUState(gpuState);
//
//     bool edgeTransparency = state->getValue(ShowFieldModule::EdgeTransparency).getBool();
//     // Add appropriate uniforms to the pass (in this case, uColor).
//     if (edgeTransparency)
//       pass.addUniform("uColor", glm::vec4(meshRed, meshGreen, meshBlue, 0.5f));
//     else
//       pass.addUniform("uColor", glm::vec4(meshRed, meshGreen, meshBlue, 1.0f));
//
//     geom->mPasses.push_back(pass);
//   }
//
//   if (progressFunc) progressFunc(0.5);
//
//   // Build the faces
//   if (showFaces)
//   {
//     //double dataMin = 0.0;
//     //double dataMax = 0.0;
//     //vfield->min(dataMin);
//     //vfield->max(dataMax);
//     const std::string iboName = id + "facesIBO";
//     buildFacesIBO(facade, geom, iboName);
//
//     // Construct construct a uniform color pass.
//     /// \todo Allow the user to select from a few different lighting routines
//     ///       and bind them here.
//     if (vmesh->has_normals())
//     {
//       std::string shaderToUse = "Shaders/DirPhong";
//       if (colorMap)
//       {
//         shaderToUse = "Shaders/DirPhongCMap";
//       }
//       GeometryObject::SpireSubPass pass = 
//           GeometryObject::SpireSubPass(id + "facesPass", primVBOName, iboName, 
//                                        shaderToUse);
//
//       bool faceTransparency = state->getValue(ShowFieldModule::FaceTransparency).getBool();
//       float transparency    = 1.0f;
//       if (faceTransparency) transparency = 0.1f;
//
//       // Add common uniforms.
//       pass.addUniform("uAmbientColor", glm::vec4(0.01f, 0.01f, 0.01f, transparency));
//
//       if (!colorMap)
//         pass.addUniform("uDiffuseColor", glm::vec4(meshRed, meshGreen, meshBlue, transparency));
//
//       pass.addUniform("uSpecularColor", glm::vec4(1.0f, 1.0f, 1.0f, transparency));
//       pass.addUniform("uSpecularPower", 32.0f);
//       geom->mPasses.push_back(pass);
//     }
//     else
//     {
//       std::string shaderToUse = "Shaders/UniformColor";
//       if (colorMap)
//       {
//         shaderToUse = "Shaders/ColorMap";
//       }
//       // No normals present in the model, construct a uniform pass
//       GeometryObject::SpireSubPass pass = 
//           GeometryObject::SpireSubPass(id + "facesPass", primVBOName, iboName,
//                                        shaderToUse);
//
//       // Apply misc user settings.
//       bool faceTransparency = state->getValue(ShowFieldModule::FaceTransparency).getBool();
//       float transparency    = 1.0f;
//       if (faceTransparency) transparency = 0.2f;
//
//       if (!colorMap)
//         pass.addUniform("uColor", glm::vec4(meshRed, meshGreen, meshBlue, transparency));
//
//       geom->mPasses.push_back(pass);
//     }
//   }
//
//   if (progressFunc) progressFunc(0.75);
//
//   // Build the nodes
//   if (showNodes)
//   {
//     const std::string iboName = id + "nodesIBO";
//     buildNodesIBO(facade, geom, iboName);
//
//     // Build pass for the nodes.
//     /// \todo Find an appropriate place to put program names like UniformColor.
//     GeometryObject::SpireSubPass pass = 
//         GeometryObject::SpireSubPass(id + "nodesPass", primVBOName, iboName,
//                                      "Shaders/UniformColor");
//
//     // Add appropriate uniforms to the pass (in this case, uColor).
//     bool nodeTransparency = state->getValue(ShowFieldModule::NodeTransparency).getBool();
//     if (nodeTransparency)
//       pass.addUniform("uColor", glm::vec4(meshRed, meshGreen, meshBlue, 0.5f));
//     else
//       pass.addUniform("uColor", glm::vec4(meshRed, meshGreen, meshBlue, 1.0f));
//
//     geom->mPasses.push_back(pass);
//   }
//
//   if (progressFunc) progressFunc(1);
//
//   return geom;
// }
//
//
//
//
// void ShowFieldModule::buildFacesIBO(
//     SCIRun::Core::Datatypes::MeshTraits<VMesh>::MeshFacadeHandle facade, 
//     GeometryHandle geom, const std::string& desiredIBOName)
// {
//   uint32_t* iboFaces = nullptr;
//
//   // Determine the size of the face structure (taking into account the varying
//   // types of faces -- only quads and triangles are currently supported).
//   size_t iboFacesSize = 0;
//   BOOST_FOREACH(const FaceInfo<VMesh>& face, facade->faces())
//   {
//     VMesh::Node::array_type nodes = face.nodeIndices();
//     if (nodes.size() == 4)
//     {
//       iboFacesSize += sizeof(uint32_t) * 6;
//     }
//     else if (nodes.size() == 3)
//     {
//       iboFacesSize += sizeof(uint32_t) * 3;
//     }
//     else
//     {
//       BOOST_THROW_EXCEPTION(SCIRun::Core::DimensionMismatch() 
//         << SCIRun::Core::ErrorMessage("Only Quads and Triangles are supported."));
//     }
//   }
//   std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
//   rawIBO->resize(iboFacesSize);   // Linear in complexity... If we need more performance,
//   // use malloc to generate buffer and then vector::assign.
//   iboFaces = reinterpret_cast<uint32_t*>(&(*rawIBO)[0]);
//   size_t i = 0;
//   BOOST_FOREACH(const FaceInfo<VMesh>& face, facade->faces())
//   {
//     VMesh::Node::array_type nodes = face.nodeIndices();
//     if (nodes.size() == 4)
//     {
//       // Winding order looks good from tests.
//       iboFaces[i  ] = static_cast<uint32_t>(nodes[0]); iboFaces[i+1] = static_cast<uint32_t>(nodes[1]); iboFaces[i+2] = static_cast<uint32_t>(nodes[2]);
//       iboFaces[i+3] = static_cast<uint32_t>(nodes[0]); iboFaces[i+4] = static_cast<uint32_t>(nodes[2]); iboFaces[i+5] = static_cast<uint32_t>(nodes[3]);
//       i += 6;
//     }
//     else if (nodes.size() == 3)
//     {
//       iboFaces[i  ] = static_cast<uint32_t>(nodes[0]); iboFaces[i+1] = static_cast<uint32_t>(nodes[1]); iboFaces[i+2] = static_cast<uint32_t>(nodes[2]);
//       i += 3;
//     }
//     // All other cases have been checked in the loop above which determines
//     // the size of the face IBO.
//   }
//
//   // Add IBO for the faces.
//   geom->mIBOs.push_back(
//       GeometryObject::SpireIBO(desiredIBOName, GeometryObject::SpireIBO::TRIANGLES, sizeof(uint32_t), rawIBO));
// }
//
//
// void ShowFieldModule::buildEdgesIBO(
//     SCIRun::Core::Datatypes::MeshTraits<VMesh>::MeshFacadeHandle facade,
//     GeometryHandle geom, const std::string& desiredIBOName)
// {
//
//   size_t iboEdgesSize = sizeof(uint32_t) * facade->numEdges() * 2;
//   uint32_t* iboEdges = nullptr;
//
//   std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
//   rawIBO->resize(iboEdgesSize);   // Linear in complexity... If we need more performance,
//   // use malloc to generate buffer and then vector::assign.
//   iboEdges = reinterpret_cast<uint32_t*>(&(*rawIBO)[0]);
//   size_t i = 0;
//   BOOST_FOREACH(const EdgeInfo<VMesh>& edge, facade->edges())
//   {
//     // There should *only* be two indicies (linestrip would be better...)
//     VMesh::Node::array_type nodes = edge.nodeIndices();
//     ENSURE_DIMENSIONS_MATCH(nodes.size(), 2, "Edges require exactly 2 indices.");
//     iboEdges[i] = static_cast<uint32_t>(nodes[0]); iboEdges[i+1] = static_cast<uint32_t>(nodes[1]);
//     i += 2;
//   }
//
//   // Add IBO for the edges.
//   geom->mIBOs.push_back(
//       GeometryObject::SpireIBO(desiredIBOName, GeometryObject::SpireIBO::LINES, sizeof(uint32_t), rawIBO));
//
// }
//
// void ShowFieldModule::buildNodesIBO(
//     SCIRun::Core::Datatypes::MeshTraits<VMesh>::MeshFacadeHandle facade,
//     GeometryHandle geom, const std::string& desiredIBOName)
// {
//   size_t iboNodesSize = sizeof(uint32_t) * facade->numNodes();
//   uint32_t* iboNodes = nullptr;
//
//   std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
//   rawIBO->resize(iboNodesSize);   // Linear in complexity... If we need more performance,
//   // use malloc to generate buffer and then vector::assign.
//   iboNodes = reinterpret_cast<uint32_t*>(&(*rawIBO)[0]);
//   size_t i = 0;
//   BOOST_FOREACH(const NodeInfo<VMesh>& node, facade->nodes())
//   {
//     // There should *only* be two indicies (linestrip would be better...)
//     //node.index()
//     //VirtualMesh::Node::array_type nodes = node.nodeIndices();
//     //ENSURE_DIMENSIONS_MATCH(nodes.size(), 2, "Edges require exactly 2 indices.");
//     iboNodes[i] = static_cast<uint32_t>(node.index());
//     i++;
//   }
//
//   // Add IBO for the nodes.
//   geom->mIBOs.push_back(
//       GeometryObject::SpireIBO(desiredIBOName, GeometryObject::SpireIBO::POINTS, sizeof(uint32_t), rawIBO));
// }

AlgorithmParameterName ShowFieldModule::ShowNodes("ShowNodes");
AlgorithmParameterName ShowFieldModule::ShowEdges("ShowEdges");
AlgorithmParameterName ShowFieldModule::ShowFaces("ShowFaces");
AlgorithmParameterName ShowFieldModule::NodeTransparency("NodeTransparency");
AlgorithmParameterName ShowFieldModule::EdgeTransparency("EdgeTransparency");
AlgorithmParameterName ShowFieldModule::FaceTransparency("FaceTransparency");
AlgorithmParameterName ShowFieldModule::FaceInvertNormals("FaceInvertNormals");
AlgorithmParameterName ShowFieldModule::DefaultMeshColor("DefaultMeshColor");


