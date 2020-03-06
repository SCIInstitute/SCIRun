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


#include <Modules/Visualization/ShowFieldGlyphsPortHandler.h>
#include <Modules/Visualization/ShowFieldGlyphs.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Color.h>
#include <Graphics/Datatypes/GeometryImpl.h>

#define _USE_MATH_DEFINES
#include <math.h>

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

MODULE_INFO_DEF(ShowFieldGlyphs, Visualization, SCIRun)

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      class GlyphBuilder
      {
      public:
        GlyphBuilder(const std::string& moduleId) : moduleId_(moduleId){}
        /// Constructs a geometry object (essentially a spire object) from the given
        /// field data.
        /// \param field    Field from which to construct geometry.
        /// \param state
        /// \param id       Ends up becoming the name of the spire object.
        GeometryHandle buildGeometryObject(
          FieldHandle pfield,
          boost::optional<FieldHandle> sfield,
          boost::optional<FieldHandle> tfield,
          boost::optional<ColorMapHandle> pcolormap,
          boost::optional<ColorMapHandle> scolormap,
          boost::optional<ColorMapHandle> tcolormap,
          Interruptible* interruptible,
          ModuleStateHandle state,
          const GeometryIDGenerator& idgen,
          const Module* module);

        void renderVectors(
          ModuleStateHandle state,
          Interruptible* interruptible,
          const RenderState& renState,
          GeometryHandle geom,
          const std::string& id);

        void renderScalars(
          ModuleStateHandle state,
          Interruptible* interruptible,
          const RenderState& renState,
          GeometryHandle geom,
          const std::string& id);

        void renderTensors(
          ModuleStateHandle state,
          Interruptible* interruptible,
          const RenderState& renState,
          GeometryHandle geom,
          const std::string& id,
          const Module* module_);

      RenderState getVectorsRenderState(ModuleStateHandle state);

      RenderState getScalarsRenderState(ModuleStateHandle state);

      RenderState getTensorsRenderState(ModuleStateHandle state);

      private:
        std::string moduleId_;
        ColorScheme getColoringType(const RenderState& renState, VField* fld);
        void getPoints(VMesh* mesh, std::vector<int>& indices, std::vector<Point>& points);
        std::unique_ptr<ShowFieldGlyphsPortHandler> portHandler_;
        RenderState::InputPort getInput(const std::string& port_name);
        void addGlyph(
          GlyphGeom& glyphs,
          int glyph_type,
          Point& p1,
          Vector& dir,
          double radius,
          double scale,
          double ratio,
          int resolution,
          ColorRGB& node_color,
          bool use_lines,
          bool render_base1,
          bool render_base2);

      };
    }
  }
}

enum SecondaryVectorParameterScalingTypeEnum
{
  UNIFORM,
  USE_INPUT
};

enum FieldDataType
{
  node,
  edge,
  face,
  cell
};

ColorScheme GlyphBuilder::getColoringType(const RenderState& renState, VField* fld)
{
  if(fld->basis_order() < 0 || renState.get(RenderState::USE_DEFAULT_COLOR))
  {
    return ColorScheme::COLOR_UNIFORM;
  }
  else if(renState.get(RenderState::USE_COLORMAP))
  {
    return ColorScheme::COLOR_MAP;
  }
  else
  {
    return ColorScheme::COLOR_IN_SITU;
  }
}

void GlyphBuilder::addGlyph(
  GlyphGeom& glyphs,
  int glyph_type,
  Point& p1,
  Vector& dir,
  double radius,
  double scale,
  double ratio,
  int resolution,
  ColorRGB& node_color,
  bool use_lines,
  bool render_base1 = false,
  bool render_base2 = false)
{
  Point p2 = p1 + dir * scale;
  double scaled_radius = scale * radius;
  switch (glyph_type)
  {
    case RenderState::GlyphType::LINE_GLYPH:
      glyphs.addLine(p1, p2, node_color, node_color);
      break;
    case RenderState::GlyphType::NEEDLE_GLYPH:
      glyphs.addNeedle(p1, p2, node_color, node_color);
      break;
    case RenderState::GlyphType::COMET_GLYPH:
    {
      static const double sphere_extrusion = 0.0625f;
      glyphs.addComet(p1-(dir*scale), p1, scaled_radius, resolution, node_color, node_color, sphere_extrusion);
      break;
    }
    case RenderState::GlyphType::CONE_GLYPH:
      glyphs.addCone(p1, p2, scaled_radius, resolution, render_base1, node_color, node_color);
      break;
    case RenderState::GlyphType::ARROW_GLYPH:
      glyphs.addArrow(p1, p2, scaled_radius, ratio, resolution, node_color, node_color, render_base1, render_base2);
      break;
    case RenderState::GlyphType::DISK_GLYPH:
    {
      Point new_p2 = p1 + dir.normal() * scaled_radius * 2.0;
      double new_radius = dir.length() * scale * 0.5;
      glyphs.addDisk(p1, new_p2, new_radius, resolution, node_color, node_color);
      break;
    }
    case RenderState::GlyphType::RING_GLYPH:
    {
      double major_radius = dir.length() * scale * 0.5;
      glyphs.addTorus(p1, p2, major_radius, scaled_radius, resolution, node_color, node_color);
      break;
    }
    case RenderState::GlyphType::SPRING_GLYPH:
      BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Spring Geom is not supported yet."));
      break;
    default:
      if (use_lines)
        glyphs.addLine(p1, p2, node_color, node_color);
      else
        glyphs.addArrow(p1, p2, scaled_radius, ratio, resolution, node_color, node_color, render_base1, render_base2);
  }
}


ShowFieldGlyphs::ShowFieldGlyphs() : GeometryGeneratingModule(staticInfo_), builder_(new GlyphBuilder(id().id_))
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

  // General Options
  state->setValue(DefaultMeshColor, ColorRGB(0.5, 0.5, 0.5).toString());
  state->setValue(FieldName, std::string());

  // Vectors
  state->setValue(ShowVectorTab, false);
  state->setValue(ShowVectors, false);
  state->setValue(VectorsDisplayType, std::string("Lines"));
  state->setValue(VectorsColoring, std::string("Default"));
  state->setValue(VectorsColoringDataInput, std::string("Primary"));
  state->setValue(VectorsTransparency, 0);
  state->setValue(VectorsUniformTransparencyValue, 0.65);
  //  state->setValue(VectorsTransparencyDataInput, std::string("Primary"));
  state->setValue(SecondaryVectorParameterScalingType, SecondaryVectorParameterScalingTypeEnum::USE_INPUT);
  state->setValue(SecondaryVectorParameterDataInput, std::string("Primary"));
  state->setValue(SecondaryVectorParameterScale, 0.25);
  state->setValue(NormalizeVectors, false);
  state->setValue(VectorsScale, 1.0);
  state->setValue(RenderVectorsBelowThreshold, true);
  state->setValue(VectorsThreshold, 0.0);
  state->setValue(RenderBases, false);
  state->setValue(RenderBidirectionaly, false);
  state->setValue(ArrowHeadRatio, 0.5);
  state->setValue(VectorsResolution, 5);
  // Scalars
  state->setValue(ShowScalarTab, false);
  state->setValue(ShowScalars, false);
  state->setValue(ScalarsDisplayType, std::string("Points"));
  state->setValue(ScalarsColoring, std::string("Default"));
  state->setValue(ScalarsColoringDataInput, std::string("Primary"));
  state->setValue(ScalarsTransparency, 0);
  state->setValue(ScalarsUniformTransparencyValue, 0.65);
  //  state->setValue(ScalarsTransparencyDataInput, std::string("Primary"));
  state->setValue(ScalarsScale, 1.0);
  state->setValue(ScalarsThreshold, 0.0);
  state->setValue(ScalarsResolution, 10);
  // Tensors
  state->setValue(ShowTensorTab, false);
  state->setValue(ShowTensors, false);
  state->setValue(TensorsDisplayType, std::string("Ellipsoids"));
  state->setValue(TensorsColoring, std::string("Default"));
  state->setValue(TensorsColoringDataInput, std::string("Primary"));
  state->setValue(TensorsTransparency, 0);
  state->setValue(TensorsUniformTransparencyValue, 0.65);
  state->setValue(SuperquadricEmphasis, 0.85);
  //  state->setValue(TensorsTransparencyDataInput, std::string("Primary"));
  state->setValue(NormalizeTensors, false);
  state->setValue(TensorsScale, 1.0);
  state->setValue(RenderTensorsBelowThreshold, true);
  state->setValue(TensorsThreshold, 0.0);
  state->setValue(TensorsResolution, 10);
}

void ShowFieldGlyphs::execute()
{
  auto pfield = getRequiredInput(PrimaryData);
  auto pcolorMap = getOptionalInput(PrimaryColorMap);
  boost::optional<boost::shared_ptr<SCIRun::Field>> sfield = getOptionalInput(SecondaryData);
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> scolorMap = getOptionalInput(SecondaryColorMap);
  boost::optional<boost::shared_ptr<SCIRun::Field>> tfield = getOptionalInput(TertiaryData);
  boost::optional<boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap>> tcolorMap = getOptionalInput(TertiaryColorMap);

  if (needToExecute())
  {
    configureInputs(pfield, sfield, tfield, pcolorMap, scolorMap, tcolorMap);

    auto geom = builder_->buildGeometryObject(pfield, sfield, tfield, pcolorMap, scolorMap, tcolorMap,
                                              this, get_state(), *this, this);
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

  if (sfield)
  {
    FieldInformation sfinfo(*sfield);
    if (!sfinfo.is_svt())
    {
      THROW_ALGORITHM_INPUT_ERROR("No Scalar, Vector, or Tensor data found in the secondary data field.");
    }
  }

  if (tfield)
  {
    FieldInformation tfinfo(*tfield);
    if (!tfinfo.is_svt())
    {
      THROW_ALGORITHM_INPUT_ERROR("No Scalar, Vector, or Tensor data found in the tertiary data field.");
    }
  }
}

RenderState::InputPort GlyphBuilder::getInput(const std::string& port_name)
{
  if(port_name == "Primary")
  {
    return RenderState::PRIMARY_PORT;
  }
  else if(port_name == "Secondary")
  {
    return RenderState::SECONDARY_PORT;
  }
  else
  {
    return RenderState::TERTIARY_PORT;
  }
}

GeometryHandle GlyphBuilder::buildGeometryObject(
  FieldHandle pfield,
  boost::optional<FieldHandle> sfield,
  boost::optional<FieldHandle> tfield,
  boost::optional<ColorMapHandle> pcolormap,
  boost::optional<ColorMapHandle> scolormap,
  boost::optional<ColorMapHandle> tcolormap,
  Interruptible* interruptible,
  ModuleStateHandle state,
  const GeometryIDGenerator& idgen,
  const Module* module)
{
  // Function for reporting progress.
  //SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc progressFunc = getUpdaterFunc();

  FieldInformation finfo(pfield);

  // Show or hide tabs
  state->setValue(ShowFieldGlyphs::ShowScalarTab, finfo.is_scalar());
  state->setValue(ShowFieldGlyphs::ShowVectorTab, finfo.is_vector());
  state->setValue(ShowFieldGlyphs::ShowTensorTab, finfo.is_tensor());

  // Shows glyphs if data is present and user has selected
  bool showScalars = state->getValue(ShowFieldGlyphs::ShowScalars).toBool() && finfo.is_scalar();
  bool showVectors = state->getValue(ShowFieldGlyphs::ShowVectors).toBool() && finfo.is_vector();
  bool showTensors = state->getValue(ShowFieldGlyphs::ShowTensors).toBool() && finfo.is_tensor();

  // Creates id
  std::string idname = "EntireGlyphField";
  if(!state->getValue(ShowFieldGlyphs::FieldName).toString().empty())
    idname += GeometryObject::delimiter + state->getValue(ShowFieldGlyphs::FieldName).toString() + " (from " + moduleId_ +")";

  auto geom(boost::make_shared<GeometryObjectSpire>(idgen, idname, true));

  if(!showScalars)
    state->setValue(ShowFieldGlyphs::ShowScalars, showScalars);
  if(!showVectors)
    state->setValue(ShowFieldGlyphs::ShowVectors, showVectors);
  if(!showTensors)
    state->setValue(ShowFieldGlyphs::ShowTensors, showTensors);

  // Don't render if no data given
  if(!(showScalars || showVectors || showTensors)) return geom;

  // Get render state
  RenderState renState;
  if(showScalars)
  {
    renState = getScalarsRenderState(state);
  }
  else if(showVectors)
  {
    renState = getVectorsRenderState(state);
  }
  else if(showTensors)
  {
    renState = getTensorsRenderState(state);
  }

  // Create port handler and check for errors
  portHandler_.reset(new ShowFieldGlyphsPortHandler(module, state, renState, pfield, sfield, tfield,
                                                    pcolormap, scolormap, tcolormap));
  try
  {
    portHandler_->checkForErrors();
  } catch(const std::invalid_argument& e)
  {
    // If error is given, post it and return empty geom object
    module->error(e.what());
    return geom;
  }

  // Render glyphs
  if (finfo.is_scalar() && showScalars)
  {
    renderScalars(state, interruptible, renState, geom, geom->uniqueID());
  }
  else if (finfo.is_vector() && showVectors)
  {
    renderVectors(state, interruptible, renState, geom, geom->uniqueID());
  }
  else if (finfo.is_tensor() && showTensors)
  {
    renderTensors(state, interruptible, renState, geom, geom->uniqueID(), module);
  }

  return geom;
}

void GlyphBuilder::getPoints(VMesh* mesh, std::vector<int>& indices, std::vector<Point>& points)
{
    // Collect indices and points from facades
  FieldDataType fieldLocation;
  FieldInformation pfinfo = portHandler_->getPrimaryFieldInfo();
  if (pfinfo.is_point() || pfinfo.is_linear())
    fieldLocation = FieldDataType::node;
  else if (pfinfo.is_line())
    fieldLocation = FieldDataType::edge;
  else if (pfinfo.is_surface())
    fieldLocation = FieldDataType::face;
  else
    fieldLocation = FieldDataType::cell;

  switch(fieldLocation)
  {
    case FieldDataType::node:
      for (const auto& node : portHandler_->getPrimaryFacade()->nodes())
      {
        indices.push_back(node.index());
        Point p;
        mesh->get_center(p, node.index());
        points.push_back(p);
      }
      break;
    case FieldDataType::edge:
      for (const auto& edge : portHandler_->getPrimaryFacade()->edges())
      {
        indices.push_back(edge.index());
        Point p;
        mesh->get_center(p, edge.index());
        points.push_back(p);
      }
      break;
    case FieldDataType::face:
      for (const auto& face : portHandler_->getPrimaryFacade()->faces())
      {
        indices.push_back(face.index());
        Point p;
        mesh->get_center(p, face.index());
        points.push_back(p);
      }
      break;
    case FieldDataType::cell:
      for (const auto& cell : portHandler_->getPrimaryFacade()->cells())
      {
        indices.push_back(cell.index());
        Point p;
        mesh->get_center(p, cell.index());
        points.push_back(p);
      }
      break;
  }
}

void GlyphBuilder::renderVectors(
  ModuleStateHandle state,
  Interruptible* interruptible,
  const RenderState& renState,
  GeometryHandle geom,
  const std::string& id)
{
  VMesh* mesh = portHandler_->getMesh();
  mesh->synchronize(Mesh::EDGES_E);
  FieldInformation pfinfo = portHandler_->getPrimaryFieldInfo();

  bool useLines = renState.mGlyphType == RenderState::GlyphType::LINE_GLYPH || renState.mGlyphType == RenderState::GlyphType::NEEDLE_GLYPH;

  SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::TRIANGLES;
  // Use Lines
  if (useLines)
  {
    primIn = SpireIBO::PRIMITIVE::LINES;
  }

  // Gets user set data
  ColorScheme colorScheme = portHandler_->getColorScheme();
  double scale = state->getValue(ShowFieldGlyphs::VectorsScale).toDouble();
  double radiusWidthScale = state->getValue(ShowFieldGlyphs::SecondaryVectorParameterScale).toDouble();
  int resolution = state->getValue(ShowFieldGlyphs::VectorsResolution).toInt();
  double arrowHeadRatio = state->getValue(ShowFieldGlyphs::ArrowHeadRatio).toDouble();

  bool normalizeGlyphs = state->getValue(ShowFieldGlyphs::NormalizeVectors).toBool();
  bool renderBidirectionaly = state->getValue(ShowFieldGlyphs::RenderBidirectionaly).toBool();
  bool renderBases = state->getValue(ShowFieldGlyphs::RenderBases).toBool();
  bool renderGlphysBelowThreshold = state->getValue(ShowFieldGlyphs::RenderVectorsBelowThreshold).toBool();
  float threshold = state->getValue(ShowFieldGlyphs::VectorsThreshold).toDouble();

  // Make sure scale and resolution are not below minimum values
  if (scale < 0) scale = 1.0;
  if (resolution < 3) resolution = 5;
  if(arrowHeadRatio < 0) arrowHeadRatio = 0;
  if(arrowHeadRatio > 1) arrowHeadRatio = 1;

  auto indices = std::vector<int>();
  auto points = std::vector<Point>();
  getPoints(mesh, indices, points);

  GlyphGeom glyphs;
  // Render every item from facade
  for(int i = 0; i < indices.size(); i++)
  {
    interruptible->checkForInterruption();
    Vector v, pinputVector; Point p2, p3; double radius;

    pinputVector = portHandler_->getPrimaryVector(indices[i]);

      // Normalize/Scale
    Vector dir = pinputVector;
    if(normalizeGlyphs)
    dir.normalize();
    // v = pinputVector.normal() * scale;
    // else
    // v = pinputVector * scale;

    // Calculate points
    // p2 = points[i] + v;
    // p3 = points[i] - v;

    // Get radius
    // radius = scale * radiusWidthScale / 2.0;
    radius = radiusWidthScale / 2.0;
    if(state->getValue(ShowFieldGlyphs::SecondaryVectorParameterScalingType).toInt() == SecondaryVectorParameterScalingTypeEnum::USE_INPUT)
      radius *= portHandler_->getSecondaryVectorParameter(indices[i]);

    ColorRGB node_color = portHandler_->getNodeColor(indices[i]);

    if(renderGlphysBelowThreshold || pinputVector.length() >= threshold)
    {
      // No need to render cylinder base if arrow is bidirectional
      bool render_cylinder_base = renderBases && !renderBidirectionaly;
      addGlyph(glyphs, renState.mGlyphType, points[i], dir, radius, scale, arrowHeadRatio,
               resolution, node_color, useLines, render_cylinder_base, renderBases);

      if(renderBidirectionaly)
      {
        Vector neg_dir = -dir;
        addGlyph(glyphs, renState.mGlyphType, points[i], neg_dir, radius, scale, arrowHeadRatio,
                 resolution, node_color, useLines, render_cylinder_base, renderBases);
      }
    }
  }

  std::stringstream ss;
  ss << renState.mGlyphType << resolution << scale << static_cast<int>(colorScheme);

  std::string uniqueNodeID = id + "vector_glyphs" + ss.str();

  glyphs.buildObject(*geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENT_EDGES),
                     state->getValue(ShowFieldGlyphs::VectorsUniformTransparencyValue).toDouble(),
                     colorScheme, renState, primIn, mesh->get_bounding_box(), true, portHandler_->getTextureMap());
}

void GlyphBuilder::renderScalars(
  ModuleStateHandle state,
  Interruptible* interruptible,
  const RenderState& renState,
  GeometryHandle geom,
  const std::string& id)
{
  VMesh* mesh = portHandler_->getMesh();
  mesh->synchronize(Mesh::NODES_E);

  // Gets user set data
  ColorScheme colorScheme = portHandler_->getColorScheme();
  double scale = state->getValue(ShowFieldGlyphs::ScalarsScale).toDouble();
  int resolution = state->getValue(ShowFieldGlyphs::ScalarsResolution).toInt();
  if (scale < 0) scale = 1.0;
  if (resolution < 3) resolution = 5;

  bool usePoints = renState.mGlyphType == RenderState::GlyphType::POINT_GLYPH;

  SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::TRIANGLES;;
  // Use Points
  if (usePoints)
  {
    primIn = SpireIBO::PRIMITIVE::POINTS;
  }

  auto indices = std::vector<int>();
  auto points = std::vector<Point>();
  getPoints(mesh, indices, points);

  GlyphGeom glyphs;
  // Render every item from facade
  for(int i = 0; i < indices.size(); i++)
  {
    interruptible->checkForInterruption();

    double v = portHandler_->getPrimaryScalar(indices[i]);
    ColorRGB node_color = portHandler_->getNodeColor(indices[i]);
    double radius = std::abs(v) * scale;

    switch (renState.mGlyphType)
    {
      case RenderState::GlyphType::POINT_GLYPH:
        glyphs.addPoint(points[i], node_color);
        break;
      case RenderState::GlyphType::SPHERE_GLYPH:
        glyphs.addSphere(points[i], radius, resolution, node_color);
        break;
      case RenderState::GlyphType::BOX_GLYPH:
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Box Geom is not supported yet."));
        break;
      case RenderState::GlyphType::AXIS_GLYPH:
        BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Axis Geom is not supported yet."));
        break;
      default:
        if (usePoints)
          glyphs.addPoint(points[i], node_color);
        else
          glyphs.addSphere(points[i], radius, resolution, node_color);
        break;
    }
  }

  std::stringstream ss;
  ss << renState.mGlyphType << resolution << scale << static_cast<int>(colorScheme);

  std::string uniqueNodeID = id + "scalar_glyphs" + ss.str();

  glyphs.buildObject(*geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENT_NODES),
                     state->getValue(ShowFieldGlyphs::ScalarsUniformTransparencyValue).toDouble(),
                     colorScheme, renState, primIn, mesh->get_bounding_box(), true,
                     portHandler_->getTextureMap());
}

double map_emphasis(double old)
{
  if (old < 0.0) old = 0.0;
  else if (old > 1.0) old = 1.0;
  return tan(old * (M_PI / 2.0 * 0.999));
  // Map old 3.5 value onto new 0.825 value.
  //return tan(old * (atan(3.5) / (0.825 * 4.0)));
}

void GlyphBuilder::renderTensors(
  ModuleStateHandle state,
  Interruptible* interruptible,
  const RenderState& renState,
  GeometryHandle geom,
  const std::string& id,
  const Module* module_)
{
  FieldInformation pfinfo = portHandler_->getPrimaryFieldInfo();

  VMesh* mesh = portHandler_->getMesh();
  mesh->synchronize(Mesh::NODES_E);

  auto indices = std::vector<int>();
  auto points = std::vector<Point>();
  getPoints(mesh, indices, points);

  // Gets user set data
  ColorScheme colorScheme = portHandler_->getColorScheme();
  double scale = state->getValue(ShowFieldGlyphs::TensorsScale).toDouble();
  int resolution = state->getValue(ShowFieldGlyphs::TensorsResolution).toInt();
  bool normalizeGlyphs = state->getValue(ShowFieldGlyphs::NormalizeTensors).toBool();
  bool renderGlyphsBelowThreshold = state->getValue(ShowFieldGlyphs::RenderTensorsBelowThreshold).toBool();
  float threshold = state->getValue(ShowFieldGlyphs::TensorsThreshold).toDouble();
  if (resolution < 3) resolution = 5;

  std::stringstream ss;
  ss << renState.mGlyphType << resolution << scale << static_cast<int>(colorScheme);

  // Separate id's are needed for lines and points if rendered
  std::string uniqueNodeID = id + "tensor_glyphs" + ss.str();
  std::string uniqueLineID = id + "tensor_line_glyphs" + ss.str();
  std::string uniquePointID = id + "tensor_point_glyphs" + ss.str();

  SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::TRIANGLES;

  GlyphGeom tensor_line_glyphs;
  GlyphGeom point_glyphs;

  int neg_eigval_count = 0;
  int tensorcount = 0;
  static const double vectorThreshold = 0.001;
  static const double pointThreshold = 0.01;
  static const double epsilon = pow(2, -52);

  GlyphGeom glyphs;
  // Render every item from facade
  for(int i = 0; i < indices.size(); i++)
  {
    interruptible->checkForInterruption();

    Tensor t = portHandler_->getPrimaryTensor(indices[i]);

    double eigen1, eigen2, eigen3;
    t.get_eigenvalues(eigen1, eigen2, eigen3);
    Vector eigvals(fabs(eigen1), fabs(eigen2), fabs(eigen3));

    // Counter for negative eigen values
    if(eigen1 < -epsilon || eigen2 < -epsilon || eigen3 < -epsilon) ++neg_eigval_count;

    Vector eigvec1, eigvec2, eigvec3;
    t.get_eigenvectors(eigvec1, eigvec2, eigvec3);

    // Checks to see if eigenvalues are below defined threshold
    bool vector_eig_x_0 = eigvals.x() <= vectorThreshold;
    bool vector_eig_y_0 = eigvals.y() <= vectorThreshold;
    bool vector_eig_z_0 = eigvals.z() <= vectorThreshold;
    bool point_eig_x_0 = eigvals.x() <= pointThreshold;
    bool point_eig_y_0 = eigvals.y() <= pointThreshold;
    bool point_eig_z_0 = eigvals.z() <= pointThreshold;

    bool order0Tensor = (point_eig_x_0 && point_eig_y_0 && point_eig_z_0);
    bool order1Tensor = (vector_eig_x_0 + vector_eig_y_0 + vector_eig_z_0) >= 2;

    ColorRGB node_color = portHandler_->getNodeColor(indices[i]);

    // Do not render tensors that are too small - because surfaces
    // are not renderd at least two of the scales must be non zero.
    if(!renderGlyphsBelowThreshold && t.magnitude() < threshold) continue;

    if(order0Tensor)
    {
      point_glyphs.addPoint(points[i], node_color);
    }
    else if(order1Tensor)
    {
      Vector dir;
      if(vector_eig_x_0 && vector_eig_y_0)
        dir = eigvec3 * eigvals[2];
      else if(vector_eig_y_0 && vector_eig_z_0)
        dir = eigvec1 * eigvals[0];
      else if(vector_eig_x_0 && vector_eig_z_0)
        dir = eigvec2 * eigvals[1];
      // Point p1 = points[i];
      // Point p2 = points[i] + dir;
      addGlyph(tensor_line_glyphs, RenderState::GlyphType::LINE_GLYPH, points[i], dir, scale, scale, scale, resolution, node_color, true);
    }
    // Render as order 2 or 3 tensor
    else
    {
      switch (renState.mGlyphType)
      {
        case RenderState::GlyphType::BOX_GLYPH:
          glyphs.addBox(points[i], t, scale, node_color, normalizeGlyphs);
          break;
        case RenderState::GlyphType::ELLIPSOID_GLYPH:
          glyphs.addEllipsoid(points[i], t, scale, resolution, node_color, normalizeGlyphs);
          break;
        case RenderState::GlyphType::SUPERQUADRIC_TENSOR_GLYPH:
        {
          double emphasis = state->getValue(ShowFieldGlyphs::SuperquadricEmphasis).toDouble();
          if(emphasis > 0.0)
            glyphs.addSuperquadricTensor(points[i], t, scale, resolution, node_color, normalizeGlyphs, emphasis);
          else
            glyphs.addEllipsoid(points[i], t, scale, resolution, node_color, normalizeGlyphs);
        }
        default:
          break;
      }
      tensorcount++;
    }
  }

  // Prints warning if there are negative eigen values
  if(neg_eigval_count > 0) {
      module_->warning(std::to_string(neg_eigval_count) + " negative eigen values in data.");
  }

  glyphs.buildObject(*geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENCY),
                     state->getValue(ShowFieldGlyphs::TensorsUniformTransparencyValue).toDouble(),
                     colorScheme, renState, primIn, mesh->get_bounding_box(), true,
                     portHandler_->getTextureMap());

  // Render lines(2 eigenvalues equalling 0)
  RenderState lineRenState = getVectorsRenderState(state);
  tensor_line_glyphs.buildObject(*geom, uniqueLineID, lineRenState.get(RenderState::USE_TRANSPARENT_EDGES),
                                 state->getValue(ShowFieldGlyphs::TensorsUniformTransparencyValue).toDouble(),
                                 colorScheme, lineRenState, SpireIBO::PRIMITIVE::LINES, mesh->get_bounding_box(),
                                 true, portHandler_->getTextureMap());

  // Render scalars(3 eigenvalues equalling 0)
  RenderState pointRenState = getScalarsRenderState(state);
  point_glyphs.buildObject(*geom, uniquePointID, pointRenState.get(RenderState::USE_TRANSPARENT_NODES),
                           state->getValue(ShowFieldGlyphs::TensorsUniformTransparencyValue).toDouble(),
                           colorScheme, pointRenState, SpireIBO::PRIMITIVE::POINTS, mesh->get_bounding_box(),
                           true, portHandler_->getTextureMap());
}

void ShowFieldGlyphs::setSuperquadricEmphasis(int emphasis)
{
  double mapped_emphasis = map_emphasis((double) emphasis * 0.01);
  get_state()->setValue(ShowFieldGlyphs::SuperquadricEmphasis, mapped_emphasis);
}

RenderState GlyphBuilder::getVectorsRenderState(ModuleStateHandle state)
{
  RenderState renState;

  renState.set(RenderState::USE_NORMALS, true);

  renState.set(RenderState::IS_ON, state->getValue(ShowFieldGlyphs::ShowVectors).toBool());

  // Transparency
  renState.set(RenderState::USE_TRANSPARENT_EDGES, state->getValue(ShowFieldGlyphs::VectorsTransparency).toInt() == 1);

  std::string g_type = state->getValue(ShowFieldGlyphs::VectorsDisplayType).toString();
  if(g_type == "Lines")
    renState.mGlyphType = RenderState::GlyphType::LINE_GLYPH;
  else if(g_type == "Needles")
    renState.mGlyphType = RenderState::GlyphType::NEEDLE_GLYPH;
  else if(g_type == "Comets")
    renState.mGlyphType = RenderState::GlyphType::COMET_GLYPH;
  else if(g_type == "Cones")
    renState.mGlyphType = RenderState::GlyphType::CONE_GLYPH;
  else if(g_type == "Arrows")
    renState.mGlyphType = RenderState::GlyphType::ARROW_GLYPH;
  else if(g_type == "Disks")
    renState.mGlyphType = RenderState::GlyphType::DISK_GLYPH;
  else if(g_type == "Rings")
    renState.mGlyphType = RenderState::GlyphType::RING_GLYPH;
  else if(g_type == "Springs")
    renState.mGlyphType = RenderState::GlyphType::SPRING_GLYPH;
  else
    renState.mGlyphType = RenderState::GlyphType::LINE_GLYPH;

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldGlyphs::DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
                           renState.defaultColor.g() > 1.0 ||
                           renState.defaultColor.b() > 1.0) ?
                  ColorRGB(renState.defaultColor.r() / 255.,
                           renState.defaultColor.g() / 255.,
                           renState.defaultColor.b() / 255.) : renState.defaultColor;

  std::string c_type = state->getValue(ShowFieldGlyphs::VectorsColoring).toString();
  if(c_type == "Colormap Lookup")
  {
    renState.set(RenderState::USE_COLORMAP, true);
  }
  else if (c_type == "Conversion to RGB")
  {
    renState.set(RenderState::USE_COLOR_CONVERT, true);
  }
  else
  {
    renState.set(RenderState::USE_DEFAULT_COLOR, true);
  }
  renState.mColorInput = getInput(state->getValue(ShowFieldGlyphs::VectorsColoringDataInput).toString());
  renState.mSecondaryVectorParameterInput = getInput(state->getValue(ShowFieldGlyphs::SecondaryVectorParameterDataInput).toString());

  return renState;
}

RenderState GlyphBuilder::getScalarsRenderState(ModuleStateHandle state)
{
  RenderState renState;

  renState.set(RenderState::USE_NORMALS, true);

  // Transparency
  int transparency = state->getValue(ShowFieldGlyphs::ScalarsTransparency).toInt();
  if(transparency == 0)
  {
    renState.set(RenderState::USE_TRANSPARENT_NODES, false);
  }
  //TODO add input option
  else if(transparency == 1)
  {
    renState.set(RenderState::USE_TRANSPARENT_NODES, true);
  }
  else
  {
    renState.set(RenderState::USE_TRANSPARENT_NODES, true);
  }

  std::string g_type = state->getValue(ShowFieldGlyphs::ScalarsDisplayType).toString();
  if(g_type == "Lines")
    renState.mGlyphType = RenderState::GlyphType::POINT_GLYPH;
  else if(g_type == "Spheres")
    renState.mGlyphType = RenderState::GlyphType::SPHERE_GLYPH;
  else if(g_type == "Boxes")
    renState.mGlyphType = RenderState::GlyphType::BOX_GLYPH;
  else if(g_type == "Axis")
    renState.mGlyphType = RenderState::GlyphType::AXIS_GLYPH;
  else
    renState.mGlyphType = RenderState::GlyphType::POINT_GLYPH;

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldGlyphs::DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
    renState.defaultColor.g() > 1.0 ||
    renState.defaultColor.b() > 1.0) ?
    ColorRGB(
    renState.defaultColor.r() / 255.,
    renState.defaultColor.g() / 255.,
    renState.defaultColor.b() / 255.)
    : renState.defaultColor;

  std::string c_type = state->getValue(ShowFieldGlyphs::ScalarsColoring).toString();
  if(c_type == "Colormap Lookup")
  {
    renState.set(RenderState::USE_COLORMAP, true);
  }
  else if (c_type == "Conversion to RGB")
  {
    renState.set(RenderState::USE_COLOR_CONVERT, true);
  }
  else
  {
    renState.set(RenderState::USE_DEFAULT_COLOR, true);
  }
  renState.mColorInput = getInput(state->getValue(ShowFieldGlyphs::ScalarsColoringDataInput).toString());

  return renState;
}

RenderState GlyphBuilder::getTensorsRenderState(ModuleStateHandle state)
{
  RenderState renState;

  renState.set(RenderState::USE_NORMALS, true);

  // Show Tensors
  renState.set(RenderState::IS_ON, state->getValue(ShowFieldGlyphs::ShowTensors).toBool());

  // Transparency
  int transparency = state->getValue(ShowFieldGlyphs::TensorsTransparency).toInt();
  if(transparency == 0)
  {
    renState.set(RenderState::USE_TRANSPARENCY, false);
  }
  //TODO add input option
  else if(transparency == 1)
  {
    renState.set(RenderState::USE_TRANSPARENCY, true);
  }
  else
  {
    renState.set(RenderState::USE_TRANSPARENCY, true);
  }

  // Glpyh Type
  std::string glyph = state->getValue(ShowFieldGlyphs::TensorsDisplayType).toString();
  if(glyph == "Boxes")
    renState.mGlyphType = RenderState::GlyphType::BOX_GLYPH;
  else if(glyph == "Ellipsoids")
    renState.mGlyphType = RenderState::GlyphType::ELLIPSOID_GLYPH;
  else if(glyph == "Spheres")
    renState.mGlyphType = RenderState::GlyphType::SPHERE_GLYPH;
  else if(glyph == "Superquadrics"
       || glyph == "Superellipsoids") // This case matches old name in case files had it saved in state
    renState.mGlyphType = RenderState::GlyphType::SUPERQUADRIC_TENSOR_GLYPH;
  else
    renState.mGlyphType = RenderState::GlyphType::BOX_GLYPH;

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldGlyphs::DefaultMeshColor).toString());
  renState.defaultColor =
  (renState.defaultColor.r() > 1.0 || renState.defaultColor.g() > 1.0 || renState.defaultColor.b() > 1.0) ?
    ColorRGB( renState.defaultColor.r() / 255., renState.defaultColor.g() / 255., renState.defaultColor.b() / 255.) :
    renState.defaultColor;

  // Coloring
  std::string color = state->getValue(ShowFieldGlyphs::TensorsColoring).toString();
  if(color == "Colormap Lookup")
  {
    renState.set(RenderState::USE_COLORMAP, true);
  }
  else if (color == "Conversion to RGB")
  {
    renState.set(RenderState::USE_COLOR_CONVERT, true);
  }
  else
  {
    renState.set(RenderState::USE_DEFAULT_COLOR, true);
  }
  renState.mColorInput = getInput(state->getValue(ShowFieldGlyphs::TensorsColoringDataInput).toString());

  return renState;
}

const AlgorithmParameterName ShowFieldGlyphs::FieldName("FieldName");
// Mesh Color
const AlgorithmParameterName ShowFieldGlyphs::DefaultMeshColor("DefaultMeshColor");
// Vector Controls
const AlgorithmParameterName ShowFieldGlyphs::ShowVectorTab("ShowVectorTab");
const AlgorithmParameterName ShowFieldGlyphs::ShowVectors("ShowVectors");
const AlgorithmParameterName ShowFieldGlyphs::VectorsDisplayType("VectorsDisplayType");
const AlgorithmParameterName ShowFieldGlyphs::VectorsColoring("VectorsColoring");
const AlgorithmParameterName ShowFieldGlyphs::VectorsColoringDataInput("VectorsColoringDataInput");
const AlgorithmParameterName ShowFieldGlyphs::VectorsTransparency("VectorsTransparency");
const AlgorithmParameterName ShowFieldGlyphs::VectorsUniformTransparencyValue("VectorsUniformTransparencyValue");
//const AlgorithmParameterName ShowFieldGlyphs::VectorsTransparencyDataInput("VectorsTransparencyDataInput");
const AlgorithmParameterName ShowFieldGlyphs::NormalizeVectors("NormalizeVectors");
const AlgorithmParameterName ShowFieldGlyphs::VectorsScale("VectorsScale");
const AlgorithmParameterName ShowFieldGlyphs::RenderVectorsBelowThreshold("RenderVectorsBelowThreshold");
const AlgorithmParameterName ShowFieldGlyphs::VectorsThreshold("VectorsThreshold");
const AlgorithmParameterName ShowFieldGlyphs::SecondaryVectorParameterScalingType("SecondaryVectorParameterScalingType");
const AlgorithmParameterName ShowFieldGlyphs::SecondaryVectorParameterDataInput("SecondaryVectorParameterDataInput");
const AlgorithmParameterName ShowFieldGlyphs::SecondaryVectorParameterScale("SecondaryVectorParameterScale");
const AlgorithmParameterName ShowFieldGlyphs::ArrowHeadRatio("ArrowHeadRatio");
const AlgorithmParameterName ShowFieldGlyphs::RenderBidirectionaly("RenderBidirectionaly");
const AlgorithmParameterName ShowFieldGlyphs::RenderBases("RenderBases");
const AlgorithmParameterName ShowFieldGlyphs::VectorsResolution("VectorsResolution");
// Scalar Controls
const AlgorithmParameterName ShowFieldGlyphs::ShowScalarTab("ShowScalarTab");
const AlgorithmParameterName ShowFieldGlyphs::ShowScalars("ShowScalars");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsDisplayType("ScalarsDisplayType");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsColoring("ScalarsColoring");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsColoringDataInput("ScalarsColoringDataInput");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsTransparency("ScalarsTransparency");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsUniformTransparencyValue("ScalarsUniformTransparencyValue");
//const AlgorithmParameterName ShowFieldGlyphs::ScalarsTransparencyDataInput("ScalarsTransparencyDataInput");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsScale("ScalarsScale");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsThreshold("ScalarsThreshold");
const AlgorithmParameterName ShowFieldGlyphs::ScalarsResolution("ScalarsResolution");
// Tensor Controls
const AlgorithmParameterName ShowFieldGlyphs::ShowTensorTab("ShowTensorTab");
const AlgorithmParameterName ShowFieldGlyphs::ShowTensors("ShowTensors");
const AlgorithmParameterName ShowFieldGlyphs::TensorsDisplayType("TensorsDisplayType");
const AlgorithmParameterName ShowFieldGlyphs::TensorsColoring("TensorsColoring");
const AlgorithmParameterName ShowFieldGlyphs::TensorsColoringDataInput("TensorsColoringDataInput");
const AlgorithmParameterName ShowFieldGlyphs::TensorsTransparency("TensorsTransparency");
const AlgorithmParameterName ShowFieldGlyphs::TensorsUniformTransparencyValue("TensorsUniformTransparencyValue");
const AlgorithmParameterName ShowFieldGlyphs::SuperquadricEmphasis("SuperquadricEmphasis");
//const AlgorithmParameterName ShowFieldGlyphs::TensorsTransparencyDataInput("TensorsTransparencyDataInput");
const AlgorithmParameterName ShowFieldGlyphs::NormalizeTensors("NormalizeTensors");
const AlgorithmParameterName ShowFieldGlyphs::TensorsScale("TensorsScale");
const AlgorithmParameterName ShowFieldGlyphs::RenderTensorsBelowThreshold("RenderTensorsBelowThreshold");
const AlgorithmParameterName ShowFieldGlyphs::TensorsThreshold("TensorsThreshold");
const AlgorithmParameterName ShowFieldGlyphs::TensorsResolution("TensorsResolution");
