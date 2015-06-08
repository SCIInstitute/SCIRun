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
  state->setValue(VectorsScale, 0.1);
  state->setValue(VectorsResolution, 3);
  state->setValue(VectorsColoring, 0);
  state->setValue(VectorsDisplayType, 0);
  state->setValue(ShowVectorTab, false);

  // Scalars
  state->setValue(ShowScalars, false);
  state->setValue(ScalarsTransparency, false);
  state->setValue(ScalarsTransparencyValue, 0.65);
  state->setValue(ScalarsScale, 0.1);
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
  boost::shared_ptr<SCIRun::Field> pfield = getRequiredInput(PrimaryData);
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> pcolorMap = getOptionalInput(PrimaryColorMap);
  //boost::optional<boost::shared_ptr<SCIRun::Field>> sfield = getOptionalInput(SecondaryData);
  //boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> scolorMap = getOptionalInput(SecondaryColorMap);
  //boost::optional<boost::shared_ptr<SCIRun::Field>> tfield = getOptionalInput(TertiaryData);
  //boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> tcolorMap = getOptionalInput(TertiaryColorMap);

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
  bool showTensors = state->getValue(ShowFieldGlyphs::ShowTensors).toBool();

  GeometryHandle geom(new GeometryObject(field, *this, "EntireGlyphField"));

  FieldInformation finfo(field);

  if (finfo.is_vector())
  {
    state->setValue(ShowFieldGlyphs::ShowVectorTab, true);
    if (showVectors)
    {
      renderVectors(field, colorMap, getVectorsRenderState(state, colorMap), geom, geom->uniqueID());
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
      renderScalars(field, colorMap, getScalarsRenderState(state, colorMap), geom, geom->uniqueID());
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
      renderTensors(field, colorMap, getTensorsRenderState(state, colorMap), geom, geom->uniqueID());
    }
  }
  else
  {
    state->setValue(ShowFieldGlyphs::ShowTensorTab, false);
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

  bool useLines = state.mGlyphType == RenderState::GlyphType::LINE_GLYPH || state.mGlyphType == RenderState::GlyphType::NEEDLE_GLYPH;

  GeometryObject::SpireIBO::PRIMITIVE primIn = GeometryObject::SpireIBO::TRIANGLES;;
  // Use Lines
  if (useLines)
  {
    primIn = GeometryObject::SpireIBO::LINES;
  }

  auto my_state = get_state();
  double radius = my_state->getValue(VectorsScale).toDouble();
  double resolution = static_cast<double>(my_state->getValue(VectorsResolution).toInt());
  if (radius < 0) radius = 0.1;
  if (resolution < 3) resolution = 5;  

  GlyphGeom glyphs;
  auto facade(field->mesh()->getFacade());
  // Render linear data
  if (finfo.is_linear())
  {
    for (const auto& node : facade->nodes())
    {
      checkForInterruption();
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
  }
  // Render cell data
  else
  {
    for (const auto& cell : facade->cells())
    {
      checkForInterruption();
      Vector v;
      fld->get_value(v, cell.index());
      Point p1 = cell.center();
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
  }

  std::stringstream ss;
  ss << state.mGlyphType << resolution << radius << colorScheme;

  std::string uniqueNodeID = id + "vector_glyphs" + ss.str();

  glyphs.buildObject(geom, uniqueNodeID, state.get(RenderState::USE_TRANSPARENT_EDGES), 
    my_state->getValue(VectorsTransparencyValue).toDouble(), colorScheme, state, primIn, mesh->get_bounding_box());
}

void ShowFieldGlyphs::renderScalars(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
  RenderState state,
  Core::Datatypes::GeometryHandle geom,
  const std::string& id)
{
  FieldInformation finfo(field);

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
 
  bool usePoints = state.mGlyphType == RenderState::GlyphType::POINT_GLYPH;
   
  GeometryObject::SpireIBO::PRIMITIVE primIn = GeometryObject::SpireIBO::TRIANGLES;;
  // Use Points
  if (usePoints)
  {
    primIn = GeometryObject::SpireIBO::POINTS;
  }

  GlyphGeom glyphs;
  auto facade(field->mesh()->getFacade());
  // Render linear data
  if (finfo.is_linear())
  {
    for (const auto& node : facade->nodes())
    {
      checkForInterruption();
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
        break;
      case RenderState::GlyphType::SPHERE_GLYPH:
        glyphs.addSphere(p, radius, resolution, node_color);
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
  }
  // Render cell data
  else 
  {
    for (const auto& cell : facade->cells())
    {
      checkForInterruption();
      Vector v;
      fld->get_value(v, cell.index());
      Point p = cell.center();

      if (colorScheme != GeometryObject::COLOR_UNIFORM)
      {
        ColorMapHandle map = colorMap.get();
        node_color = map->valueToColor(v);
      }
      switch (state.mGlyphType)
      {
      case RenderState::GlyphType::POINT_GLYPH:
        glyphs.addPoint(p, node_color);
        break;
      case RenderState::GlyphType::SPHERE_GLYPH:
        glyphs.addSphere(p, radius, resolution, node_color);
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
  }

  glyphs.buildObject(geom, uniqueNodeID, state.get(RenderState::USE_TRANSPARENT_NODES),
    my_state->getValue(ScalarsTransparencyValue).toDouble(), colorScheme, state, primIn, mesh->get_bounding_box());
}

void ShowFieldGlyphs::renderTensors(
  boost::shared_ptr<SCIRun::Field> field,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap,
  RenderState state,
  Core::Datatypes::GeometryHandle geom,
  const std::string& id)
{
  FieldInformation finfo(field);

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
  double radius = my_state->getValue(TensorsScale).toDouble();
  double resolution = static_cast<double>(my_state->getValue(TensorsResolution).toInt());
  if (radius < 0) radius = 0.1;
  if (resolution < 3) resolution = 5;

  double radius2 = radius * 1.5;

  std::stringstream ss;
  ss << state.mGlyphType << resolution << radius << colorScheme;

  std::string uniqueNodeID = id + "tensor_glyphs" + ss.str();
  
  GeometryObject::SpireIBO::PRIMITIVE primIn = GeometryObject::SpireIBO::TRIANGLES;;
 
  GlyphGeom glyphs;
  auto facade(field->mesh()->getFacade());
  // Render linear data
  if (finfo.is_linear())
  {
    for (const auto& node : facade->nodes())
    {
      checkForInterruption();
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
      case RenderState::GlyphType::BOX_GLYPH:
        THROW_ALGORITHM_INPUT_ERROR("Box Geom is not supported yet.");
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
      checkForInterruption();
      Vector v;
      fld->get_value(v, cell.index());
      Point p = cell.center();

      if (colorScheme != GeometryObject::COLOR_UNIFORM)
      {
        ColorMapHandle map = colorMap.get();
        node_color = map->valueToColor(v);
      }
      switch (state.mGlyphType)
      {
      case RenderState::GlyphType::BOX_GLYPH:
        THROW_ALGORITHM_INPUT_ERROR("Box Geom is not supported yet.");
        break;
      case RenderState::GlyphType::SPHERE_GLYPH:
        glyphs.addSphere(p, radius, resolution, node_color);
        break;
      default:
        
        break;
      }
    }
  }

  glyphs.buildObject(geom, uniqueNodeID, state.get(RenderState::USE_TRANSPARENCY),
    my_state->getValue(TensorsTransparencyValue).toDouble(), colorScheme, state, primIn, mesh->get_bounding_box());
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

RenderState ShowFieldGlyphs::getTensorsRenderState(
  ModuleStateHandle state,
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> colorMap)
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
AlgorithmParameterName ShowFieldGlyphs::ShowVectors("ShowVectors");
AlgorithmParameterName ShowFieldGlyphs::VectorsTransparency("VectorsTransparency");
AlgorithmParameterName ShowFieldGlyphs::VectorsTransparencyValue("VectorsTransparencyValue");
AlgorithmParameterName ShowFieldGlyphs::VectorsScale("VectorsScale");
AlgorithmParameterName ShowFieldGlyphs::VectorsResolution("VectorsResolution");
AlgorithmParameterName ShowFieldGlyphs::VectorsColoring("VectorsColoring");
AlgorithmParameterName ShowFieldGlyphs::VectorsDisplayType("VectorsDisplayType");
// Scalar Controls
AlgorithmParameterName ShowFieldGlyphs::ShowScalars("ShowScalars");
AlgorithmParameterName ShowFieldGlyphs::ScalarsTransparency("ScalarsTransparency");
AlgorithmParameterName ShowFieldGlyphs::ScalarsTransparencyValue("ScalarsTransparencyValue");
AlgorithmParameterName ShowFieldGlyphs::ScalarsScale("ScalarsScale");
AlgorithmParameterName ShowFieldGlyphs::ScalarsResolution("ScalarsResolution");
AlgorithmParameterName ShowFieldGlyphs::ScalarsColoring("ScalarsColoring");
AlgorithmParameterName ShowFieldGlyphs::ScalarsDisplayType("ScalarsDisplayType");
// Tensor Controls
AlgorithmParameterName ShowFieldGlyphs::ShowTensors("ShowTensors");
AlgorithmParameterName ShowFieldGlyphs::TensorsTransparency("TensorsTransparency");
AlgorithmParameterName ShowFieldGlyphs::TensorsTransparencyValue("TensorsTransparencyValue");
AlgorithmParameterName ShowFieldGlyphs::TensorsScale("TensorsScale");
AlgorithmParameterName ShowFieldGlyphs::TensorsResolution("TensorsResolution");
AlgorithmParameterName ShowFieldGlyphs::TensorsColoring("TensorsColoring");
AlgorithmParameterName ShowFieldGlyphs::TensorsDisplayType("TensorsDisplayType");
// Mesh Color
AlgorithmParameterName ShowFieldGlyphs::DefaultMeshColor("DefaultMeshColor");
// Tab Controls
AlgorithmParameterName ShowFieldGlyphs::ShowVectorTab("ShowVectorTab");
AlgorithmParameterName ShowFieldGlyphs::ShowScalarTab("ShowScalarTab");
AlgorithmParameterName ShowFieldGlyphs::ShowTensorTab("ShowTensorTab");
AlgorithmParameterName ShowFieldGlyphs::ShowSecondaryTab("ShowSecondaryTab");
AlgorithmParameterName ShowFieldGlyphs::ShowTertiaryTab("ShowTertiaryTab");