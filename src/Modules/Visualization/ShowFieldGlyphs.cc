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
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Algorithms/Visualization/RenderFieldState.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core;
using namespace Core::Datatypes;
using namespace Core::Thread;
using namespace Dataflow::Networks;
using namespace Algorithms;
using namespace Geometry;
using namespace Graphics;
using namespace Graphics::Datatypes;

const ModuleLookupInfo ShowFieldGlyphs::staticInfo_("ShowFieldGlyphs", "Visualization", "SCIRun");

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      class GlyphBuilder
      {
      public:
        /// Constructs a geometry object (essentially a spire object) from the given
        /// field data.
        /// \param field    Field from which to construct geometry.
        /// \param state
        /// \param id       Ends up becoming the name of the spire object.
        GeometryHandle buildGeometryObject(
          FieldHandle field,
          boost::optional<ColorMapHandle> colorMap,
          Interruptible* interruptible,
          ModuleStateHandle state,
          const GeometryIDGenerator& idgen);

        void renderVectors(
          FieldHandle field,
          boost::optional<ColorMapHandle> colorMap,
          ModuleStateHandle state,
          Interruptible* interruptible,
          const RenderState& renState,
          GeometryHandle geom,
          const std::string& id);

        void renderScalars(
          FieldHandle field,
          boost::optional<ColorMapHandle> colorMap,
          ModuleStateHandle state,
          Interruptible* interruptible,
          const RenderState& renState,
          GeometryHandle geom,
          const std::string& id);

        void renderTensors(
          FieldHandle field,
          boost::optional<ColorMapHandle> colorMap,
          ModuleStateHandle state,
          Interruptible* interruptible,
          const RenderState& renState,
          GeometryHandle geom,
          const std::string& id);

        RenderState getVectorsRenderState(
          ModuleStateHandle state,
          boost::optional<ColorMapHandle> colorMap);

        RenderState getScalarsRenderState(
          ModuleStateHandle state,
          boost::optional<ColorMapHandle> colorMap);

        RenderState getTensorsRenderState(
          ModuleStateHandle state,
          boost::optional<ColorMapHandle> colorMap);
      };
    }
  }
}


ShowFieldGlyphs::ShowFieldGlyphs() : GeometryGeneratingModule(staticInfo_), builder_(new GlyphBuilder)
{
  INITIALIZE_PORT(PrimaryData);
  INITIALIZE_PORT(PrimaryColorMap);
  //INITIALIZE_PORT(SecondaryData);
  //INITIALIZE_PORT(SecondaryColorMap);
  //INITIALIZE_PORT(TertiaryData);
  //INITIALIZE_PORT(TertiaryColorMap);
  INITIALIZE_PORT(SceneGraph);
}

void ShowFieldGlyphs::setStateDefaults()
{
  auto state = get_state();

  // Vectors
  state->setValue(ShowVectors, false);
  state->setValue(VectorsTransparency, false);
  state->setValue(VectorsTransparencyValue, 0.65);
  state->setValue(VectorsScale, 1.0);
  state->setValue(VectorsResolution, 5);
  state->setValue(VectorsColoring, 0);
  state->setValue(VectorsDisplayType, 0);
  state->setValue(ShowVectorTab, false);

  // Scalars
  state->setValue(ShowScalars, false);
  state->setValue(ScalarsTransparency, false);
  state->setValue(ScalarsTransparencyValue, 0.65);
  state->setValue(ScalarsScale, 1.0);
  state->setValue(ScalarsResolution, 10);
  state->setValue(ScalarsColoring, 0);
  state->setValue(ScalarsDisplayType, 0);
  state->setValue(ShowScalarTab, false);

  // Tensors
  state->setValue(ShowTensors, false);
  state->setValue(TensorsTransparency, false);
  state->setValue(TensorsTransparencyValue, 0.65);
  state->setValue(TensorsScale, 0.1);
  state->setValue(TensorsResolution, 10);
  state->setValue(TensorsColoring, 0);
  state->setValue(TensorsDisplayType, 2);
  state->setValue(ShowTensorTab, false);

  // Secondary Tab
  state->setValue(ShowSecondaryTab, false);

  // Tertiary Tab
  state->setValue(ShowTertiaryTab, false);


  state->setValue(DefaultMeshColor, ColorRGB(0.5, 0.5, 0.5).toString());
}

void ShowFieldGlyphs::execute()
{
  auto pfield = getRequiredInput(PrimaryData);
  auto pcolorMap = getOptionalInput(PrimaryColorMap);
  //boost::optional<boost::shared_ptr<SCIRun::Field>> sfield = getOptionalInput(SecondaryData);
  //boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> scolorMap = getOptionalInput(SecondaryColorMap);
  //boost::optional<boost::shared_ptr<SCIRun::Field>> tfield = getOptionalInput(TertiaryData);
  //boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> tcolorMap = getOptionalInput(TertiaryColorMap);

  if (needToExecute())
  {
    //configureInputs(pfield, sfield, tfield, pcolorMap, scolorMap, tcolorMap);
    auto geom = builder_->buildGeometryObject(pfield, pcolorMap, this, get_state(), *this);
    sendOutput(SceneGraph, geom);
  }
}

void ShowFieldGlyphs::configureInputs(
  FieldHandle pfield,
  boost::optional<FieldHandle> sfield,
  boost::optional<FieldHandle> tfield,
  boost::optional<ColorMapHandle> pcolormap,
  boost::optional<ColorMapHandle> scolormap,
  boost::optional<ColorMapHandle> tcolormap)
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

GeometryHandle GlyphBuilder::buildGeometryObject(
  FieldHandle field,
  boost::optional<ColorMapHandle> colorMap,
  Interruptible* interruptible,
  ModuleStateHandle state,
  const GeometryIDGenerator& idgen)
{
  // Function for reporting progress.
  //SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc progressFunc = getUpdaterFunc();

  bool showVectors = state->getValue(ShowFieldGlyphs::ShowVectors).toBool();
  bool showScalars = state->getValue(ShowFieldGlyphs::ShowScalars).toBool();
  bool showTensors = state->getValue(ShowFieldGlyphs::ShowTensors).toBool();

  GeometryHandle geom(new GeometryObjectSpire(idgen, "EntireGlyphField", true));

  FieldInformation finfo(field);

  if (finfo.is_vector())
  {
    state->setValue(ShowFieldGlyphs::ShowVectorTab, true);
    if (showVectors)
    {
      renderVectors(field, colorMap, state, interruptible, getVectorsRenderState(state, colorMap), geom, geom->uniqueID());
    }
  }
  else
  {
    state->setValue(ShowFieldGlyphs::ShowVectorTab, false);
  }

  if (finfo.is_scalar())
  {
    state->setValue(ShowFieldGlyphs::ShowScalarTab, true);
    if (showScalars)
    {
      renderScalars(field, colorMap, state, interruptible, getScalarsRenderState(state, colorMap), geom, geom->uniqueID());
    }
  }
  else
  {
    state->setValue(ShowFieldGlyphs::ShowScalarTab, false);
  }

  if (finfo.is_tensor())
  {
    state->setValue(ShowFieldGlyphs::ShowTensorTab, true);
    if (showTensors)
    {
      renderTensors(field, colorMap, state, interruptible, getTensorsRenderState(state, colorMap), geom, geom->uniqueID());
    }
  }
  else
  {
    state->setValue(ShowFieldGlyphs::ShowTensorTab, false);
  }

  return geom;
}

void GlyphBuilder::renderVectors(
  FieldHandle field,
  boost::optional<ColorMapHandle> colorMap,
  ModuleStateHandle state,
  Interruptible* interruptible,
  const RenderState& renState,
  GeometryHandle geom,
  const std::string& id)
{
  FieldInformation finfo(field);

  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  ColorScheme colorScheme = COLOR_UNIFORM;
  ColorRGB node_color;  

  if (fld->basis_order() < 0 || renState.get(RenderState::USE_DEFAULT_COLOR))
  {
    colorScheme = COLOR_UNIFORM;
  }
  else if (renState.get(RenderState::USE_COLORMAP))
  {
    colorScheme = COLOR_MAP;
  }
  else
  {
    colorScheme = COLOR_IN_SITU;
  }

  mesh->synchronize(Mesh::EDGES_E);

  bool useLines = renState.mGlyphType == RenderState::GlyphType::LINE_GLYPH || renState.mGlyphType == RenderState::GlyphType::NEEDLE_GLYPH;

  SpireIBO::PRIMITIVE primIn = SpireIBO::TRIANGLES;;
  // Use Lines
  if (useLines)
  {
    primIn = SpireIBO::LINES;
  }

  double scale = state->getValue(ShowFieldGlyphs::VectorsScale).toDouble();
  double resolution = state->getValue(ShowFieldGlyphs::VectorsResolution).toInt();
  double secondaryScalar = 0.25; // to be replaced with data from secondary field.
  if (scale < 0) scale = 1.0;
  if (resolution < 3) resolution = 5;  
  
  GlyphGeom glyphs;
  auto facade(field->mesh()->getFacade());
  
  //Temporary fix for cloud field data until after IBBM
  bool done = false;
  // Render cell data
  if (!finfo.is_linear())
  {
    for (const auto& cell : facade->cells())
    {
      interruptible->checkForInterruption();
      Vector v, inputVector;
      fld->get_value(inputVector, cell.index());
      v = inputVector * scale;
      double radius = v.length() * secondaryScalar;
      Point p1 = cell.center();
      Point p2 = p1 + v;
      //std::cout << "center: " << p1 << " end: " << p2 << std::endl;
      //std::cout << "radius: " << radius << std::endl;
      if (colorScheme != COLOR_UNIFORM)
      {
        if (colorScheme == COLOR_MAP)
        {
          ColorMapHandle map = colorMap.get();
          node_color = map->valueToColor(inputVector);
        }
        if (colorScheme == COLOR_IN_SITU)
        {
          Vector colorVector = inputVector.normal();
          node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
        }
      }
      switch (renState.mGlyphType)
      {
      case RenderState::GlyphType::LINE_GLYPH:
        glyphs.addLine(p1, p2, node_color, node_color);
        break;
      case RenderState::GlyphType::NEEDLE_GLYPH:
        glyphs.addNeedle(p1, p2, node_color, node_color);
        break;
      case RenderState::GlyphType::COMET_GLYPH:
        glyphs.addSphere(p2, radius, resolution, node_color);
        glyphs.addCone(p2, p1, radius, resolution, node_color, node_color);
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
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Ring Geom is not supported yet."));
        break;
      case RenderState::GlyphType::SPRING_GLYPH:
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Spring Geom is not supported yet."));
        break;
      default:
        if (useLines)
          glyphs.addLine(p1, p2, node_color, node_color);
        else
          glyphs.addArrow(p1, p2, radius, resolution, node_color, node_color);
        break;
      }
      done = true;
    }
  }

  // Render linear data
  if (!done)
  {
    //std::cout << "in vector linear pre loop" << std::endl;

    if ((fld->basis_order() == 0 && mesh->dimensionality() != 0))
    {
      colorScheme = COLOR_UNIFORM;
    }

    for (const auto& node : facade->nodes())
    {
      interruptible->checkForInterruption();
      Vector v, inputVector;
      fld->get_value(inputVector, node.index());
      v = inputVector * scale;
      double radius = v.length() * secondaryScalar;
      Point p1 = node.point();
      Point p2 = p1 + v;
      //std::cout << "center: " << p1 << " end: " << p2 << std::endl;
      //std::cout << "radius: " << radius << std::endl;
      //std::cout << "resolution: " << resolution << std::endl;
      if (colorScheme != COLOR_UNIFORM)
      {
        if (colorScheme == COLOR_MAP)
        {
          ColorMapHandle map = colorMap.get();
          node_color = map->valueToColor(inputVector);
        }
        if (colorScheme == COLOR_IN_SITU)
        {
          Vector colorVector = inputVector.normal();
          node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
        }
      }
      else
      {
        node_color = renState.defaultColor;
      }
      switch (renState.mGlyphType)
      {
      case RenderState::GlyphType::LINE_GLYPH:
        //std::cout << "LINE_GLYPH" << std::endl;
        glyphs.addLine(p1, p2, node_color, node_color);
        break;
      case RenderState::GlyphType::NEEDLE_GLYPH:
        //std::cout << "NEEDLE_GLYPH" << std::endl;
        glyphs.addNeedle(p1, p2, node_color, node_color);
        break;
      case RenderState::GlyphType::COMET_GLYPH:
        //std::cout << "COMET_GLYPH" << std::endl;
        glyphs.addSphere(p2, radius, resolution, node_color);
        glyphs.addCone(p2, p1, radius, resolution, node_color, node_color);
        break;
      case RenderState::GlyphType::CONE_GLYPH:
        //std::cout << "CONE_GLYPH" << std::endl;
        glyphs.addCone(p1, p2, radius, resolution, node_color, node_color);
        break;
      case RenderState::GlyphType::ARROW_GLYPH:
        //std::cout << "ARROW_GLYPH" << std::endl;
        glyphs.addArrow(p1, p2, radius, resolution, node_color, node_color);
        break;
      case RenderState::GlyphType::DISK_GLYPH:
        //std::cout << "DISK_GLYPH" << std::endl;
        glyphs.addCylinder(p1, p2, radius, resolution, node_color, node_color);
        break;
      case RenderState::GlyphType::RING_GLYPH:
        //std::cout << "RING_GLYPH" << std::endl;
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Ring Geom is not supported yet."));
        break;
      case RenderState::GlyphType::SPRING_GLYPH:
        //std::cout << "SPRING_GLYPH" << std::endl;
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Spring Geom is not supported yet."));
        break;
      default:
        //std::cout << "default" << std::endl;
        if (useLines)
          glyphs.addLine(p1, p2, node_color, node_color);
        else
          glyphs.addArrow(p1, p2, radius, resolution, node_color, node_color);
        break;
      }
    }
  }
 
  std::stringstream ss;
  ss << renState.mGlyphType << resolution << scale << colorScheme;

  std::string uniqueNodeID = id + "vector_glyphs" + ss.str();

  glyphs.buildObject(geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENT_EDGES),
    state->getValue(ShowFieldGlyphs::VectorsTransparencyValue).toDouble(), colorScheme, renState, primIn, mesh->get_bounding_box());
}

void GlyphBuilder::renderScalars(
  FieldHandle field,
  boost::optional<ColorMapHandle> colorMap,
  ModuleStateHandle state,
  Interruptible* interruptible,
  const RenderState& renState,
  GeometryHandle geom,
  const std::string& id)
{
  FieldInformation finfo(field);

  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  ColorScheme colorScheme = COLOR_UNIFORM;
  ColorRGB node_color;

  if (fld->basis_order() < 0 || renState.get(RenderState::USE_DEFAULT_COLOR))
  {
    colorScheme = COLOR_UNIFORM;
  }
  else if (renState.get(RenderState::USE_COLORMAP))
  {
    colorScheme = COLOR_MAP;
  }
  else
  {
    colorScheme = COLOR_IN_SITU;
  }

  mesh->synchronize(Mesh::NODES_E);

  double scale = state->getValue(ShowFieldGlyphs::ScalarsScale).toDouble();
  double resolution = state->getValue(ShowFieldGlyphs::ScalarsResolution).toInt();
  if (scale < 0) scale = 1.0;
  if (resolution < 3) resolution = 5;
    
  bool usePoints = renState.mGlyphType == RenderState::GlyphType::POINT_GLYPH;
   
  SpireIBO::PRIMITIVE primIn = SpireIBO::TRIANGLES;;
  // Use Points
  if (usePoints)
  {
    primIn = SpireIBO::POINTS;
  }

  GlyphGeom glyphs;
  auto facade(field->mesh()->getFacade());

  bool done = false;
  // Render cell data
  if (!finfo.is_linear())
  {
    for (const auto& cell : facade->cells())
    {
      interruptible->checkForInterruption();
      double v;
      fld->get_value(v, cell.index());
      Point p = cell.center();
      double radius = std::abs(v) * scale;

      if (colorScheme != COLOR_UNIFORM)
      {
        if (colorScheme == COLOR_MAP)
        {
          ColorMapHandle map = colorMap.get();
          node_color = map->valueToColor(v);
        }
        if (colorScheme == COLOR_IN_SITU)
        {
          Vector colorVector = Vector(p.x(), p.y(), p.z()).normal();
          node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
        }
      }
      switch (renState.mGlyphType)
      {
      case RenderState::GlyphType::POINT_GLYPH:
        glyphs.addPoint(p, node_color);
        break;
      case RenderState::GlyphType::SPHERE_GLYPH:
        glyphs.addSphere(p, radius, resolution, node_color);
        break;
      case RenderState::GlyphType::BOX_GLYPH:
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Box Geom is not supported yet."));
        break;
      case RenderState::GlyphType::AXIS_GLYPH:
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Axis Geom is not supported yet."));
        break;
      default:
        if (usePoints)
          glyphs.addPoint(p, node_color);
        else
          glyphs.addSphere(p, radius, resolution, node_color);
        break;
      }
      done = true;
    }
  }

  // Render linear data
  if (!done)
  {
    if ((fld->basis_order() == 0 && mesh->dimensionality() != 0))
    {
      colorScheme = COLOR_UNIFORM;
    }

    for (const auto& node : facade->nodes())
    {
      interruptible->checkForInterruption();
      double v;
      fld->get_value(v, node.index());
      Point p = node.point();
      double radius = std::abs(v) * scale;

      if (colorScheme != COLOR_UNIFORM)
      {
        if (colorScheme == COLOR_MAP)
        {
          ColorMapHandle map = colorMap.get();
          node_color = map->valueToColor(v);
        }
        if (colorScheme == COLOR_IN_SITU)
        {
          Vector colorVector = Vector(p.x(), p.y(), p.z()).normal();
          node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
        }
      }
      switch (renState.mGlyphType)
      {
      case RenderState::GlyphType::POINT_GLYPH:
        glyphs.addPoint(p, node_color);
        break;
      case RenderState::GlyphType::SPHERE_GLYPH:
        glyphs.addSphere(p, radius, resolution, node_color);
        break;
      case RenderState::GlyphType::BOX_GLYPH:
        //glyphs.addEllipsoid(p, radius, 2*radius, resolution, node_color);
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Box Geom is not supported yet."));
        break;
      case RenderState::GlyphType::AXIS_GLYPH:
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Axis Geom is not supported yet."));
        break;
      default:
        if (usePoints)
          glyphs.addPoint(p, node_color);
        else
          glyphs.addSphere(p, radius, resolution, node_color);
        break;
      }
    }
  }

  std::stringstream ss;
  ss << renState.mGlyphType << resolution << scale << colorScheme;

  std::string uniqueNodeID = id + "scalar_glyphs" + ss.str();

  glyphs.buildObject(geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENT_NODES),
    state->getValue(ShowFieldGlyphs::ScalarsTransparencyValue).toDouble(), colorScheme, renState, primIn, mesh->get_bounding_box());
}

void GlyphBuilder::renderTensors(
  FieldHandle field,
  boost::optional<ColorMapHandle> colorMap,
  ModuleStateHandle state,
  Interruptible* interruptible,
  const RenderState& renState,
  GeometryHandle geom,
  const std::string& id)
{
  FieldInformation finfo(field);

  VField* fld = field->vfield();
  VMesh*  mesh = field->vmesh();

  ColorScheme colorScheme = COLOR_UNIFORM;
  ColorRGB node_color;

  if (fld->basis_order() < 0 || (fld->basis_order() == 0 && mesh->dimensionality() != 0) || renState.get(RenderState::USE_DEFAULT_COLOR))
  {
    colorScheme = COLOR_UNIFORM;
  }
  else if (renState.get(RenderState::USE_COLORMAP))
  {
    colorScheme = COLOR_MAP;
  }
  else
  {
    colorScheme = COLOR_IN_SITU;
  }

  mesh->synchronize(Mesh::NODES_E);

  double radius = state->getValue(ShowFieldGlyphs::TensorsScale).toDouble();
  double resolution = state->getValue(ShowFieldGlyphs::TensorsResolution).toInt();
  if (radius < 0) radius = 0.1;
  if (resolution < 3) resolution = 5;

  std::stringstream ss;
  ss << renState.mGlyphType << resolution << radius << colorScheme;

  std::string uniqueNodeID = id + "tensor_glyphs" + ss.str();
  
  SpireIBO::PRIMITIVE primIn = SpireIBO::TRIANGLES;;
 
  GlyphGeom glyphs;
  auto facade(field->mesh()->getFacade());
  // Render linear data
  if (finfo.is_linear())
  {
    for (const auto& node : facade->nodes())
    {
      interruptible->checkForInterruption();
      Tensor t;
      fld->get_value(t, node.index());
      Point p = node.point();
      double eigen1, eigen2, eigen3;
      t.get_eigenvalues(eigen1, eigen2, eigen3);

      if (colorScheme != COLOR_UNIFORM)
      {
        if (colorScheme == COLOR_MAP)
        {
          ColorMapHandle map = colorMap.get();
          node_color = map->valueToColor(t);
        }
        if (colorScheme == COLOR_IN_SITU)
        {
          Vector colorVector = t.get_eigenvector1().normal();
          node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
        }        
      }
      switch (renState.mGlyphType)
      {
      case RenderState::GlyphType::BOX_GLYPH:
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Box Geom is not supported yet."));
        break;
      case RenderState::GlyphType::SPHERE_GLYPH:
        glyphs.addSphere(p, radius, resolution, node_color);
        break;
      default:
        
        break;
      }
    }
  }
  // Render cell data
  else
  {
    for (const auto& cell : facade->cells())
    {
      interruptible->checkForInterruption();
      Tensor t;
      fld->get_value(t, cell.index());
      Point p = cell.center();
      double eigen1, eigen2, eigen3;
      t.get_eigenvalues(eigen1, eigen2, eigen3);

      if (colorScheme != COLOR_UNIFORM)
      {
        if (colorScheme == COLOR_MAP)
        {
          ColorMapHandle map = colorMap.get();
          node_color = map->valueToColor(t);
        }
        if (colorScheme == COLOR_IN_SITU)
        {
          Vector colorVector = t.get_eigenvector1().normal();
          node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
        }
      }
      switch (renState.mGlyphType)
      {
      case RenderState::GlyphType::BOX_GLYPH:
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Box Geom is not supported yet."));
        break;
      case RenderState::GlyphType::SPHERE_GLYPH:
        glyphs.addSphere(p, radius, resolution, node_color);
        break;
      default:
        
        break;
      }
    }
  }

  glyphs.buildObject(geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENCY),
    state->getValue(ShowFieldGlyphs::TensorsTransparencyValue).toDouble(), colorScheme, renState, primIn, mesh->get_bounding_box());
}

RenderState GlyphBuilder::getVectorsRenderState(
  ModuleStateHandle state,
  boost::optional<ColorMapHandle> colorMap)
{
  RenderState renState;

  bool useColorMap = state->getValue(ShowFieldGlyphs::VectorsColoring).toInt() == 1;
  bool rgbConversion = state->getValue(ShowFieldGlyphs::VectorsColoring).toInt() == 2;
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
                  ColorRGB(renState.defaultColor.r() / 255.,
                           renState.defaultColor.g() / 255.,
                           renState.defaultColor.b() / 255.) : renState.defaultColor;
  
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
  }

  return renState;
}

RenderState GlyphBuilder::getScalarsRenderState(
  ModuleStateHandle state,
  boost::optional<ColorMapHandle> colorMap)
{
  RenderState renState;

  bool useColorMap = state->getValue(ShowFieldGlyphs::ScalarsColoring).toInt() == 1;
  bool rgbConversion = state->getValue(ShowFieldGlyphs::ScalarsColoring).toInt() == 2;
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
  else if (rgbConversion)
  {
    renState.set(RenderState::USE_COLOR_CONVERT, true);
  }
  else
  {
    renState.set(RenderState::USE_DEFAULT_COLOR, true);
  }

  return renState;
}

RenderState GlyphBuilder::getTensorsRenderState(
  ModuleStateHandle state,
  boost::optional<ColorMapHandle> colorMap)
{
  RenderState renState;

  bool useColorMap = state->getValue(ShowFieldGlyphs::TensorsColoring).toInt() == 1;
  renState.set(RenderState::USE_NORMALS, true);

  renState.set(RenderState::IS_ON, state->getValue(ShowFieldGlyphs::ShowTensors).toBool());
  renState.set(RenderState::USE_TRANSPARENCY, state->getValue(ShowFieldGlyphs::TensorsTransparency).toBool());

  switch (state->getValue(ShowFieldGlyphs::TensorsDisplayType).toInt())
  {
  case 0:
    renState.mGlyphType = RenderState::GlyphType::BOX_GLYPH;
    break;
  case 1:
    renState.mGlyphType = RenderState::GlyphType::BOX_GLYPH;
    break;
  case 2:
    renState.mGlyphType = RenderState::GlyphType::SPHERE_GLYPH;
    break;
  case 3:
    renState.mGlyphType = RenderState::GlyphType::SPHERE_GLYPH;
    break;
  default:
    renState.mGlyphType = RenderState::GlyphType::BOX_GLYPH;
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

// Vector Controls
const AlgorithmParameterName ShowFieldGlyphs::ShowVectors("ShowVectors");
const AlgorithmParameterName ShowFieldGlyphs::VectorsTransparency("VectorsTransparency");
const AlgorithmParameterName ShowFieldGlyphs::VectorsTransparencyValue("VectorsTransparencyValue");
const AlgorithmParameterName ShowFieldGlyphs::VectorsScale("VectorsScale");
const AlgorithmParameterName ShowFieldGlyphs::VectorsResolution("VectorsResolution");
const AlgorithmParameterName ShowFieldGlyphs::VectorsColoring("VectorsColoring");
const AlgorithmParameterName ShowFieldGlyphs::VectorsDisplayType("VectorsDisplayType");
// Scalar Controls
const AlgorithmParameterName ShowFieldGlyphs::ShowScalars("ShowScalars");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsTransparency("ScalarsTransparency");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsTransparencyValue("ScalarsTransparencyValue");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsScale("ScalarsScale");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsResolution("ScalarsResolution");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsColoring("ScalarsColoring");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsDisplayType("ScalarsDisplayType");
// Tensor Controls
const AlgorithmParameterName ShowFieldGlyphs::ShowTensors("ShowTensors");
const AlgorithmParameterName ShowFieldGlyphs::TensorsTransparency("TensorsTransparency");
const AlgorithmParameterName ShowFieldGlyphs::TensorsTransparencyValue("TensorsTransparencyValue");
const AlgorithmParameterName ShowFieldGlyphs::TensorsScale("TensorsScale");
const AlgorithmParameterName ShowFieldGlyphs::TensorsResolution("TensorsResolution");
const AlgorithmParameterName ShowFieldGlyphs::TensorsColoring("TensorsColoring");
const AlgorithmParameterName ShowFieldGlyphs::TensorsDisplayType("TensorsDisplayType");
// Mesh Color
const AlgorithmParameterName ShowFieldGlyphs::DefaultMeshColor("DefaultMeshColor");
// Tab Controls
const AlgorithmParameterName ShowFieldGlyphs::ShowVectorTab("ShowVectorTab");
const AlgorithmParameterName ShowFieldGlyphs::ShowScalarTab("ShowScalarTab");
const AlgorithmParameterName ShowFieldGlyphs::ShowTensorTab("ShowTensorTab");
const AlgorithmParameterName ShowFieldGlyphs::ShowSecondaryTab("ShowSecondaryTab");
const AlgorithmParameterName ShowFieldGlyphs::ShowTertiaryTab("ShowTertiaryTab");