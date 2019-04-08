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
          boost::optional<ColorMapHandle> colorMap,
          boost::optional<ColorMapHandle> scolorMap,
          boost::optional<ColorMapHandle> tcolorMap,
          Interruptible* interruptible,
          ModuleStateHandle state,
          const GeometryIDGenerator& idgen,
          const Module* module);
        GeometryHandle buildGeometryObject(
          FieldHandle field,
          boost::optional<ColorMapHandle> colorMap,
          Interruptible* interruptible,
          ModuleStateHandle state,
          const GeometryIDGenerator& idgen,
          Module* module_);

        void renderVectors(
          ShowFieldGlyphsPortHandler& portHandler,
                           //FieldHandle pfield,
                           //boost::optional<FieldHandle> sfield,
                           //boost::optional<FieldHandle> tfield,
                           //boost::optional<ColorMapHandle> pcolorMap,
                           //boost::optional<ColorMapHandle> scolorMap,
                           //boost::optional<ColorMapHandle> tcolorMap,
          ModuleStateHandle state,
          Interruptible* interruptible,
          const RenderState& renState,
          GeometryHandle geom,
          const std::string& id,
          const Module* module_);

        void renderScalars(
          FieldHandle pfield,
          boost::optional<FieldHandle> sfield,
          boost::optional<FieldHandle> tfield,
          boost::optional<ColorMapHandle> pcolorMap,
          boost::optional<ColorMapHandle> scolorMap,
          boost::optional<ColorMapHandle> tcolorMap,
          ModuleStateHandle state,
          Interruptible* interruptible,
          const RenderState& renState,
          GeometryHandle geom,
          const std::string& id);

        void renderTensors(
          FieldHandle field,
          boost::optional<FieldHandle> sfield,
          boost::optional<FieldHandle> tfield,
          boost::optional<ColorMapHandle> colorMap,
          ModuleStateHandle state,
          Interruptible* interruptible,
          const RenderState& renState,
          GeometryHandle geom,
          const std::string& id,
          const Module* module_);

        RenderState getVectorsRenderState(
          ModuleStateHandle state,
          boost::optional<FieldHandle> sfield,
          boost::optional<FieldHandle> tfield,
          boost::optional<ColorMapHandle> colorMap,
          boost::optional<ColorMapHandle> scolorMap,
          boost::optional<ColorMapHandle> tcolorMap);

        RenderState getScalarsRenderState(
          ModuleStateHandle state,
          boost::optional<ColorMapHandle> colorMap);

        RenderState getTensorsRenderState(
          ModuleStateHandle state,
          boost::optional<ColorMapHandle> colorMap);

      private:
        std::string moduleId_;
        RenderState::InputPort getInput(std::string&& port_name);
        void addGlyph(
          GlyphGeom& glyphs,
          int glyph_type,
          Point& p1,
          Point& p2,
          double radius,
          double ratio,
          double resolution,
          ColorRGB& node_color,
          bool use_lines);

        ColorScheme getColoringType(const RenderState& renState, VField* fld);

        ColorRGB set_tensor_color(Tensor& t, boost::optional<ColorMapHandle> colorMap, ColorScheme colorScheme);
      };
    }
  }
}

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
  Point& p2,
  double radius,
  double ratio,
  double resolution,
  ColorRGB& node_color,
  bool use_lines)
{
  switch (glyph_type)
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
      glyphs.addArrow(p1, p2, radius, ratio, resolution, node_color, node_color);
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
      if (use_lines)
        glyphs.addLine(p1, p2, node_color, node_color);
      else
        glyphs.addArrow(p1, p2, radius, ratio, resolution, node_color, node_color);
  }
}


ShowFieldGlyphs::ShowFieldGlyphs() : GeometryGeneratingModule(staticInfo_), builder_(new GlyphBuilder(get_id().id_))
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
  state->setValue(VectorsRadiusWidthDataInput, std::string("Primary"));
  state->setValue(VectorsRadiusWidthScale, 0.25);
  state->setValue(NormalizeVectors, false);
  state->setValue(VectorsScale, 1.0);
  state->setValue(RenderVectorsBelowThreshold, true);
  state->setValue(VectorsThreshold, 0.0);
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
  //  state->setValue(TensorsTransparencyDataInput, std::string("Primary"));
  state->setValue(NormalizeTensors, false);
  state->setValue(TensorsScale, 1.0);
  state->setValue(RenderTensorsBelowThreshold, true);
  state->setValue(TensorsThreshold, 0.0);
  state->setValue(TensorsResolution, 10);

  //state->setValue(TensorsScale, 0.1);
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
    auto geom = builder_->buildGeometryObject(pfield, sfield, tfield, pcolorMap, scolorMap, tcolorMap, this, get_state(), *this, this);
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

RenderState::InputPort GlyphBuilder::getInput(std::string&& port_name)
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
  boost::optional<ColorMapHandle> pcolorMap,
  boost::optional<ColorMapHandle> scolorMap,
  boost::optional<ColorMapHandle> tcolorMap,
  Interruptible* interruptible,
  ModuleStateHandle state,
  const GeometryIDGenerator& idgen,
  const Module* module)
{
  // Function for reporting progress.
  //SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc progressFunc = getUpdaterFunc();

  bool showVectors = state->getValue(ShowFieldGlyphs::ShowVectors).toBool();
  bool showScalars = state->getValue(ShowFieldGlyphs::ShowScalars).toBool();
  bool showTensors = state->getValue(ShowFieldGlyphs::ShowTensors).toBool();

  std::string idname = "EntireGlyphField";
  if(!state->getValue(ShowFieldGlyphs::FieldName).toString().empty()){
    idname += GeometryObject::delimiter + state->getValue(ShowFieldGlyphs::FieldName).toString() + " (from " + moduleId_ +")";
  }

  auto geom(boost::make_shared<GeometryObjectSpire>(idgen, idname, true));

  FieldInformation finfo(pfield);
  boost::optional<FieldInformation> sfinfo(sfield.get());
  boost::optional<FieldInformation> tfinfo(tfield.get());

  if (finfo.is_vector())
  {
    state->setValue(ShowFieldGlyphs::ShowVectorTab, true);
    if (showVectors)
    {
      RenderState renState = getVectorsRenderState(state, sfield, tfield, pcolorMap, scolorMap, tcolorMap);
      ShowFieldGlyphsPortHandler portHandler(module, state, renState, pfield, sfield, tfield, finfo, sfinfo,
                                             tfinfo, pcolorMap, scolorMap, tcolorMap);
      renderVectors(portHandler, state, interruptible, renState, geom, geom->uniqueID(), module);
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
      renderScalars(pfield, sfield, tfield, pcolorMap, scolorMap, tcolorMap, state, interruptible, getScalarsRenderState(state, pcolorMap), geom, geom->uniqueID());
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
      renderTensors(pfield, sfield, tfield, pcolorMap, state, interruptible, getTensorsRenderState(state, pcolorMap), geom, geom->uniqueID(), module);
    }
  }
  else
  {
    state->setValue(ShowFieldGlyphs::ShowTensorTab, false);
  }

  return geom;
}

void GlyphBuilder::renderVectors(
  ShowFieldGlyphsPortHandler& portHandler,
  ModuleStateHandle state,
  Interruptible* interruptible,
  const RenderState& renState,
  GeometryHandle geom,
  const std::string& id,
  const Module* module_)
{
  VMesh* mesh = portHandler.getMesh();
  mesh->synchronize(Mesh::EDGES_E);
  FieldInformation pfinfo = portHandler.getPrimaryFieldInfo();

  bool useLines = renState.mGlyphType == RenderState::GlyphType::LINE_GLYPH || renState.mGlyphType == RenderState::GlyphType::NEEDLE_GLYPH;

  SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::TRIANGLES;
  // Use Lines
  if (useLines)
  {
    primIn = SpireIBO::PRIMITIVE::LINES;
  }

  double scale = state->getValue(ShowFieldGlyphs::VectorsScale).toDouble();
  double radiusWidthScale = state->getValue(ShowFieldGlyphs::VectorsRadiusWidthScale).toDouble();
  double resolution = state->getValue(ShowFieldGlyphs::VectorsResolution).toInt();
  double arrowHeadRatio = state->getValue(ShowFieldGlyphs::ArrowHeadRatio).toDouble();

  bool normalizeGlyphs = state->getValue(ShowFieldGlyphs::NormalizeVectors).toBool();
  bool renderBidirectionaly = state->getValue(ShowFieldGlyphs::RenderBidirectionaly).toBool();
  bool renderGlphysBelowThreshold = state->getValue(ShowFieldGlyphs::RenderVectorsBelowThreshold).toBool();
  float threshold = state->getValue(ShowFieldGlyphs::VectorsThreshold).toDouble();

  // Make sure scale and resolution are not below minimum values
  if (scale < 0) scale = 1.0;
  if (resolution < 3) resolution = 5;
  if(arrowHeadRatio < 0) arrowHeadRatio = 0;
  if(arrowHeadRatio > 1) arrowHeadRatio = 1;

 //sets field location for constant field data 1: node centered 2: edge centered 3: face centered 4: cell centered
  int fieldLocation = pfinfo.is_point()*1 + pfinfo.is_line()*2 + pfinfo.is_surface()*3 + pfinfo.is_volume()*4;
  //sets field location to 0 for linear data regardless of location
  fieldLocation = fieldLocation * !pfinfo.is_linear();

  std::vector<int> indices;
  std::vector<Point> points;
  GlyphGeom glyphs;
  switch(fieldLocation)
  {
  case 0: //linear data falls through to node data handling routine
  case 1: //node centered constant data
    for (const auto& node : portHandler.getPrimaryFacade()->nodes())
      {
        indices.push_back(node.index());
        Point p;
        mesh->get_center(p, node.index());
        points.push_back(p);
      }
    break;
  case 2: //edge centered constant data
    for (const auto& edge : portHandler.getPrimaryFacade()->edges())
      {
        indices.push_back(edge.index());
        Point p;
        mesh->get_center(p, edge.index());
        points.push_back(p);
      }
    break;
  case 3: //face centered constant data
    for (const auto& face : portHandler.getPrimaryFacade()->faces())
      {
        indices.push_back(face.index());
        Point p;
        mesh->get_center(p, face.index());
        points.push_back(p);
      }
    break;
  case 4: //cell centered constant data
    for (const auto& cell : portHandler.getPrimaryFacade()->cells())
      {
        indices.push_back(cell.index());
        Point p;
        mesh->get_center(p, cell.index());
        points.push_back(p);
      }
    break;
  }
  for(int i = 0; i < indices.size(); i++)
    {
      interruptible->checkForInterruption();
      Vector v, pinputVector; Point p2, p3; double radius, height;

      //fld->get_value(pinputVector, node.index());
      pinputVector = portHandler.getPrimaryVector(indices[i]);
      //mesh->get_center(p1, indices[i]);

      // Normalize/Scale
      if(normalizeGlyphs)
        v = pinputVector.normal() * scale;
      else
        v = pinputVector * scale;

      // Calculate points
      p2 = points[i] + v;
      p3 = points[i] - v;

      // Get radius
      try
        {
          radius = portHandler.getSecondaryVectorParameter(indices[i]);
        } catch(const std::invalid_argument& e)
        {
          module_->error(e.what());
          return;
        }
      radius *= radiusWidthScale;

      ColorRGB node_color;
      try
        {
          node_color = portHandler.getNodeColor(indices[i]);
        } catch(const std::invalid_argument& e)
        {
          module_->error(e.what());
          return;
        }

      if(renderGlphysBelowThreshold || pinputVector.length() >= threshold)
        {
          addGlyph(glyphs, renState.mGlyphType, points[i], p2, radius, arrowHeadRatio, resolution, node_color, useLines);
          if(renderBidirectionaly)
            addGlyph(glyphs, renState.mGlyphType, points[i], p3, radius, arrowHeadRatio, resolution, node_color, useLines);
        }
    }

  // Old loop for reference. TODO delete
      /**  case 2: //edge centered constant data
       for (const auto& edge : pfacade->edges())
      {
        interruptible->checkForInterruption();
        Vector v, pinputVector; Point p1, p2, p3; double radius, height;

        fld->get_value(pinputVector, edge.index());
        mesh->get_center(p1, edge.index());

        if(normalizeGlyphs)
          v = pinputVector.normal() * scale;
        else
          v = pinputVector * scale;

        Tensor sinputTensor, tinputTensor;
        Vector sinputVector, tinputVector;
        double sinputScalar, tinputScalar;
        if(sfield)
          {
            if (sf_ptr->is_scalar())
              sfld->get_value(sinputScalar, edge.index());
            else if (sf_ptr->is_vector())
              sfld->get_value(sinputVector, edge.index());
            else
              sfld->get_value(sinputTensor, edge.index());
          }
        if(tfield)
          {
            if (tf_ptr->is_scalar())
              tfld->get_value(tinputScalar, edge.index());
            else if (tf_ptr->is_vector())
              tfld->get_value(tinputVector, edge.index());
            else
              tfld->get_value(tinputTensor, edge.index());
          }

        p2 = p1 + v;
        p3 = p1 - v;

        Vector colorVector;
        float colorMapVal;
        switch(renState.mColorInput)
          {
          case RenderState::InputPort::PRIMARY_PORT:
            if(colorScheme == ColorScheme::COLOR_IN_SITU)
              {
                colorVector = pinputVector;
              }
            else if(colorScheme == ColorScheme::COLOR_MAP)
              {
                colorMapVal = pinputVector.length();
              }
            break;
          case RenderState::InputPort::SECONDARY_PORT:
            if(colorScheme == ColorScheme::COLOR_IN_SITU)
              {
                if(sf_ptr->is_scalar())
                  {
                    module_->error("Secondary Field input cannot be a scalar for RGB Conversion.");
                    return;
                  }
                else if(sf_ptr->is_vector())
                  {
                    colorVector = sinputVector;
                  }
                else
                  {
                    Vector eigvec1, eigvec2, eigvec3;
                    sinputTensor.get_eigenvectors(eigvec1, eigvec2, eigvec3);
                    colorVector = eigvec1;
                  }
              }
            else if(colorScheme == ColorScheme::COLOR_MAP)
              {
                if(sf_ptr->is_scalar())
                  {
                    colorMapVal = sinputScalar;
                  }
                else if(sf_ptr->is_vector())
                  {
                    colorMapVal = sinputVector.length();
                  }
                else
                  {
                    colorMapVal = sinputTensor.magnitude();
                  }
              }
            break;
          case RenderState::InputPort::TERTIARY_PORT:
            if(colorScheme == ColorScheme::COLOR_IN_SITU)
              {
                if(tf_ptr->is_scalar())
                  {
                    module_->error("Tertiary Field input cannot be a scalar for RGB Conversion.");
                    return;
                  }
                else if(tf_ptr->is_vector())
                  {
                    colorVector = tinputVector;
                  }
                else
                  {
                    Vector eigvec1, eigvec2, eigvec3;
                    sinputTensor.get_eigenvectors(eigvec1, eigvec2, eigvec3);
                    colorVector = eigvec1;
                  }
              }
            else if(colorScheme == ColorScheme::COLOR_MAP)
              {
                if(tf_ptr->is_scalar())
                  {
                    colorMapVal = tinputScalar;
                  }
                else if(tf_ptr->is_vector())
                  {
                    colorMapVal = tinputVector.length();
                  }
                else
                  {
                    colorMapVal = tinputTensor.magnitude();
                  }
              }
            break;
        }

        switch(renState.mVectorRadiusWidthInput)
          {
          case RenderState::InputPort::PRIMARY_PORT:
            radius = pinputVector.length();
            break;
          case RenderState::InputPort::SECONDARY_PORT:
            if(sfield)
              {
                if(sf_ptr->is_scalar())
                  {
                    radius = sinputScalar;
                  }
                else if(sf_ptr->is_vector())
                  {
                    radius = sinputVector.length();
                  }
                else
                  {
                    radius = sinputTensor.magnitude();
                  }
              }
            else
              {
                module_->error("Secondary Field input is required for Secondary Vector Parameter.");
                return;
              }
            break;
          case RenderState::InputPort::TERTIARY_PORT:
            if(tfield)
              {
                if(tf_ptr->is_scalar())
                  {
                    radius = tinputScalar;
                  }
                else if(tf_ptr->is_vector())
                  {
                    radius = tinputVector.length();
                  }
                else
                  {
                    radius = tinputTensor.magnitude();
                  }
              }
            else
              {
                module_->error("Tertiary Field input is required for Secondary Vector Parameter.");
                return;
              }
            break;
         }
        radius *= radiusWidthScale;

        if (colorScheme == ColorScheme::COLOR_UNIFORM)
        {
          node_color = renState.defaultColor;
        }
        else if (colorScheme == ColorScheme::COLOR_MAP)
        {
          ColorMapHandle map = colorMap.get();
          node_color = map->valueToColor(colorMapVal);
        }
        else if (colorScheme == ColorScheme::COLOR_IN_SITU)
        {
          colorVector = colorVector.normal();
          node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
        }

        if(renderGlphysBelowThreshold || pinputVector.length() >= threshold)
        {
          addGlyph(glyphs, renState.mGlyphType, p1, p2, radius, arrowHeadRatio, resolution, node_color, useLines);
          if(renderBidirectionaly)
            addGlyph(glyphs, renState.mGlyphType, p1, p3, radius, arrowHeadRatio, resolution, node_color, useLines);
        }
      }
      break;
**/

  std::stringstream ss;
  try
    {
      ss << renState.mGlyphType << resolution << scale << static_cast<int>(portHandler.getColorScheme());
    } catch(const std::invalid_argument& e)
    {
      module_->error(e.what());
      return;
    }

  std::string uniqueNodeID = id + "vector_glyphs" + ss.str();

  glyphs.buildObject(*geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENT_EDGES),
                     state->getValue(ShowFieldGlyphs::VectorsUniformTransparencyValue).toDouble(), portHandler.getColorScheme(), renState, primIn, mesh->get_bounding_box());
}

void GlyphBuilder::renderScalars(
  FieldHandle pfield,
  boost::optional<FieldHandle> sfield,
  boost::optional<FieldHandle> tfield,
  boost::optional<ColorMapHandle> pcolorMap,
  boost::optional<ColorMapHandle> scolorMap,
  boost::optional<ColorMapHandle> tcolorMap,
  ModuleStateHandle state,
  Interruptible* interruptible,
  const RenderState& renState,
  GeometryHandle geom,
  const std::string& id)
{
  FieldInformation finfo(pfield);

  VField* fld = pfield->vfield();
  VMesh*  mesh = pfield->vmesh();

  boost::optional<ColorMapHandle> colorMap;
  ColorScheme colorScheme;
  colorScheme= getColoringType(renState, fld);
  if(colorScheme == ColorScheme::COLOR_MAP)
    {
      switch(renState.mColorInput)
        {
        case RenderState::InputPort::PRIMARY_PORT:
          if(pfield && pcolorMap)
            colorMap = pcolorMap;
          else
            {
              // TODO throw error
            }
          break;
        case RenderState::InputPort::SECONDARY_PORT:
          if(sfield && scolorMap)
            colorMap = scolorMap;
          else
            {
              // TODO throw error
            }
          break;
        case RenderState::InputPort::TERTIARY_PORT:
          if(tfield && tcolorMap)
            colorMap = tcolorMap;
          else
            {
              // TODO throw error
            }
          break;
        }
    }

  mesh->synchronize(Mesh::NODES_E);

  double scale = state->getValue(ShowFieldGlyphs::ScalarsScale).toDouble();
  double resolution = state->getValue(ShowFieldGlyphs::ScalarsResolution).toInt();
  if (scale < 0) scale = 1.0;
  if (resolution < 3) resolution = 5;

  bool usePoints = renState.mGlyphType == RenderState::GlyphType::POINT_GLYPH;

  SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::TRIANGLES;;
  // Use Points
  if (usePoints)
  {
    primIn = SpireIBO::PRIMITIVE::POINTS;
  }

  GlyphGeom glyphs;
  auto facade(pfield->mesh()->getFacade());

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

      ColorRGB node_color;
      if (colorScheme != ColorScheme::COLOR_UNIFORM)
      {
        if (colorScheme == ColorScheme::COLOR_MAP)
        {
          ColorMapHandle map = colorMap.get();
          node_color = map->valueToColor(v);
        }
        if (colorScheme == ColorScheme::COLOR_IN_SITU)
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
      colorScheme = ColorScheme::COLOR_UNIFORM;
    }

    for (const auto& node : facade->nodes())
    {
      interruptible->checkForInterruption();
      double v;
      fld->get_value(v, node.index());
      Point p = node.point();
      double radius = std::abs(v) * scale;

      ColorRGB node_color;
      if (colorScheme != ColorScheme::COLOR_UNIFORM)
      {
        if (colorScheme == ColorScheme::COLOR_MAP)
        {
          ColorMapHandle map = colorMap.get();
          node_color = map->valueToColor(v);
        }
        if (colorScheme == ColorScheme::COLOR_IN_SITU)
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
  ss << renState.mGlyphType << resolution << scale << static_cast<int>(colorScheme);

  std::string uniqueNodeID = id + "scalar_glyphs" + ss.str();

  glyphs.buildObject(*geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENT_NODES),
                     state->getValue(ShowFieldGlyphs::ScalarsUniformTransparencyValue).toDouble(), colorScheme, renState, primIn, mesh->get_bounding_box());
}

void GlyphBuilder::renderTensors(
        FieldHandle field,
        boost::optional<FieldHandle> sfield,
        boost::optional<FieldHandle> tfield,
        boost::optional<ColorMapHandle> colorMap,
        ModuleStateHandle state,
        Interruptible* interruptible,
        const RenderState& renState,
        GeometryHandle geom,
        const std::string& id,
        const Module* module_)
{
    FieldInformation finfo(field);

    VField* fld = field->vfield();
    VMesh*  mesh = field->vmesh();

    ColorScheme colorScheme = ColorScheme::COLOR_UNIFORM;
    ColorRGB node_color;
    // std::cout << "basis order: " << fld->basis_order() << " dimensionality: " << mesh->dimensionality() << " def col: " << renState.get(RenderState::USE_DEFAULT_COLOR) << std::endl;;

    if (fld->basis_order() < 0 || renState.get(RenderState::USE_DEFAULT_COLOR))
    {
        colorScheme = ColorScheme::COLOR_UNIFORM;
    }
    else if (renState.get(RenderState::USE_COLORMAP))
    {
        colorScheme = ColorScheme::COLOR_MAP;
    }
    else
    {
        colorScheme = ColorScheme::COLOR_IN_SITU;
    }

    mesh->synchronize(Mesh::NODES_E);

    double scale = state->getValue(ShowFieldGlyphs::TensorsScale).toDouble();
    double resolution = state->getValue(ShowFieldGlyphs::TensorsResolution).toInt();
    bool normalizeGlyphs = state->getValue(ShowFieldGlyphs::NormalizeTensors).toBool();
    bool renderGlyphsBelowThreshold = state->getValue(ShowFieldGlyphs::RenderTensorsBelowThreshold).toBool();
    float threshold = state->getValue(ShowFieldGlyphs::TensorsThreshold).toDouble();
    if (resolution < 3) resolution = 5;

    std::stringstream ss;
    ss << renState.mGlyphType << resolution << scale << static_cast<int>(colorScheme);

    std::string uniqueNodeID = id + "tensor_glyphs" + ss.str();
    std::string uniqueLineID = id + "tensor_line_glyphs" + ss.str();
    std::string uniquePointID = id + "tensor_point_glyphs" + ss.str();

    SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::TRIANGLES;

    GlyphGeom glyphs;
    GlyphGeom tensor_line_glyphs;
    GlyphGeom point_glyphs;
    auto facade(field->mesh()->getFacade());

    int neg_eigval_count = 0;

    //sets feild location for consant feild data 1: node centered 2: edge centered 3: face centered 4: cell centered
    int fieldLocation = finfo.is_point() * 1 + finfo.is_line() * 2 +
      finfo.is_surface() * 3 + finfo.is_volume() * 4;

    //sets feild location to 0 for linear data regardless of location
    fieldLocation *= !finfo.is_linear();
    int tensorcount = 0;
    double epsilon = pow(2, -52);

    switch(fieldLocation){
      case 0: //linear data falls through to node data handling routine
      case 1: //node centered constant data
        for (const auto& node : facade->nodes())
        {
            interruptible->checkForInterruption();
            Tensor t;
            fld->get_value(t, node.index());

            Point p = node.point();
            double eigen1, eigen2, eigen3;
            t.get_eigenvalues(eigen1, eigen2, eigen3);

            Vector eigvals(fabs(eigen1), fabs(eigen2), fabs(eigen3));

            bool eig_x_0 = eigvals.x() < epsilon && eigvals.x() > -epsilon;
            bool eig_y_0 = eigvals.y() < epsilon && eigvals.y() > -epsilon;
            bool eig_z_0 = eigvals.z() < epsilon && eigvals.z() > -epsilon;

            bool neg_eigval = (eigen1 < -epsilon || eigen2 < -epsilon || eigen3 < -epsilon);
            if(neg_eigval)
                neg_eigval_count++;

            node_color = set_tensor_color(t, colorMap, colorScheme);

            double magnitude = t.magnitude();
            // If normalize checkbox selected
            if(normalizeGlyphs){
              eigvals.x(eigen1 / magnitude);
              eigvals.y(eigen2 / magnitude);
              eigvals.z(eigen3 / magnitude);
            }

            // Do not render tensors that are too small - because surfaces
            // are not renderd at least two of the scales must be non zero.
            if(!renderGlyphsBelowThreshold){
              if(t.magnitude() < threshold)
                continue;
            }

            if((eig_x_0 + eig_y_0 + eig_z_0) <= 1)
              {
                eigvals *= scale;

                switch (renState.mGlyphType)
                  {
                  case RenderState::GlyphType::BOX_GLYPH:
                    glyphs.addBox(p, t, scale);
                    break;
                  case RenderState::GlyphType::ELLIPSOID_GLYPH:
                    glyphs.addEllipsoid(p, t, eigvals, resolution, node_color);
                    tensorcount++;
                    break;
                  case RenderState::GlyphType::SPHERE_GLYPH:
                    glyphs.addSphere(p, eigvals.x(), resolution, node_color);
                  default:
                    break;
                  }
              }
            // Tensor as line
            else if((eig_x_0 + eig_y_0 + eig_z_0) == 2)
              {
                eigvals *= scale;

                Vector eigvec1, eigvec2, eigvec3;
                t.get_eigenvectors(eigvec1, eigvec2, eigvec3);
                Transform trans(p, eigvec1, eigvec2, eigvec3);
                Point p1 = p + trans *  eigvals/2;
                Point p2 = p + trans * -eigvals/2;
                addGlyph(tensor_line_glyphs, RenderState::GlyphType::LINE_GLYPH, p1, p2, scale, scale, resolution, node_color, true);
              }
            // Too small: render as point
            else
              {
                point_glyphs.addPoint(p, node_color);
              }
        }
        break;

      case 2: //edge centered constant data
        for(const auto& edge : facade->edges()){
            interruptible->checkForInterruption();
            Tensor t;
            fld->get_value(t, edge.index());

            Point p;
            mesh->get_center(p, edge.index());

            double eigen1, eigen2, eigen3;
            t.get_eigenvalues(eigen1, eigen2, eigen3);

            Vector eigvals(fabs(eigen1), fabs(eigen2), fabs(eigen3));

            bool eig_x_0 = eigvals.x() < epsilon && eigvals.x() > -epsilon;
            bool eig_y_0 = eigvals.y() < epsilon && eigvals.y() > -epsilon;
            bool eig_z_0 = eigvals.z() < epsilon && eigvals.z() > -epsilon;

            bool neg_eigval = (eigen1 < -epsilon || eigen2 < -epsilon || eigen3 < -epsilon);
            if(neg_eigval)
                neg_eigval_count++;

            node_color = set_tensor_color(t, colorMap, colorScheme);

            double magnitude = t.magnitude();
            // If normalize checkbox selected
            if(normalizeGlyphs){
              eigvals.x(eigen1 / magnitude);
              eigvals.y(eigen2 / magnitude);
              eigvals.z(eigen3 / magnitude);
            }

            // Do not render tensors that are too small - because surfaces
            // are not renderd at least two of the scales must be non zero.
            if(!renderGlyphsBelowThreshold){
              if(t.magnitude() < threshold)
                continue;
            }

            if((eig_x_0 + eig_y_0 + eig_z_0) <= 1)
              {
                eigvals *= scale;

                switch (renState.mGlyphType)
                  {
                  case RenderState::GlyphType::BOX_GLYPH:
                    glyphs.addBox(p, t, scale);
                    break;
                  case RenderState::GlyphType::ELLIPSOID_GLYPH:
                    glyphs.addEllipsoid(p, t, eigvals, resolution, node_color);
                    tensorcount++;
                    break;
                  case RenderState::GlyphType::SPHERE_GLYPH:
                    glyphs.addSphere(p, eigvals.x(), resolution, node_color);
                  default:
                    break;
                  }
              }
            // Tensor as line
            else if((eig_x_0 + eig_y_0 + eig_z_0) == 2)
              {
                eigvals *= scale;

                Vector eigvec1, eigvec2, eigvec3;
                t.get_eigenvectors(eigvec1, eigvec2, eigvec3);
                Transform trans(p, eigvec1, eigvec2, eigvec3);
                Point p1 = p + trans *  eigvals/2;
                Point p2 = p + trans * -eigvals/2;
                addGlyph(tensor_line_glyphs, RenderState::GlyphType::LINE_GLYPH, p1, p2, scale, scale, resolution, node_color, true);
              }
            // Too small: render as point
            else
              {
                point_glyphs.addPoint(p, node_color);
              }
        }
        break;

      case 3: //face centered constant data
        for (const auto& face : facade->faces())
        {
            interruptible->checkForInterruption();
            Tensor t;
            fld->get_value(t, face.index());

            Point p;
            mesh->get_center(p,face.index());
            double eigen1, eigen2, eigen3;
            t.get_eigenvalues(eigen1, eigen2, eigen3);

            Vector eigvals(fabs(eigen1), fabs(eigen2), fabs(eigen3));

            bool eig_x_0 = eigvals.x() < epsilon && eigvals.x() > -epsilon;
            bool eig_y_0 = eigvals.y() < epsilon && eigvals.y() > -epsilon;
            bool eig_z_0 = eigvals.z() < epsilon && eigvals.z() > -epsilon;

            bool neg_eigval = (eigen1 < -epsilon || eigen2 < -epsilon || eigen3 < -epsilon);
            if(neg_eigval)
              neg_eigval_count++;

            node_color = set_tensor_color(t, colorMap, colorScheme);

            double magnitude = t.magnitude();
            // If normalize checkbox selected
            if(normalizeGlyphs){
              eigvals.x(eigen1 / magnitude);
              eigvals.y(eigen2 / magnitude);
              eigvals.z(eigen3 / magnitude);
            }

            // Do not render tensors that are too small - because surfaces
            // are not renderd at least two of the scales must be non zero.
            if(!renderGlyphsBelowThreshold){
              if(t.magnitude() < threshold)
                continue;
            }

            if((eig_x_0 + eig_y_0 + eig_z_0) <= 1)
              {
                eigvals *= scale;

                switch (renState.mGlyphType)
                  {
                  case RenderState::GlyphType::BOX_GLYPH:
                    glyphs.addBox(p, t, scale);
                    break;
                  case RenderState::GlyphType::ELLIPSOID_GLYPH:
                    glyphs.addEllipsoid(p, t, eigvals, resolution, node_color);
                    tensorcount++;
                    break;
                  case RenderState::GlyphType::SPHERE_GLYPH:
                    glyphs.addSphere(p, eigvals.x(), resolution, node_color);
                  default:
                    break;
                  }
              }
               
            // Tensor as line
            else if((eig_x_0 + eig_y_0 + eig_z_0) == 2)
              {
                eigvals *= scale;

                Vector eigvec1, eigvec2, eigvec3;
                t.get_eigenvectors(eigvec1, eigvec2, eigvec3);
                Transform trans(p, eigvec1, eigvec2, eigvec3);
                Point p1 = p + trans *  eigvals/2;
                Point p2 = p + trans * -eigvals/2;
                addGlyph(tensor_line_glyphs, RenderState::GlyphType::LINE_GLYPH, p1, p2, scale, scale, resolution, node_color, true);
              }
            // Too small: render as point
            else
              {
                point_glyphs.addPoint(p, node_color);
              }
        }
        break;
      case 4: //cell centerd constant data
        for (const auto& cell : facade->cells())
        {
            interruptible->checkForInterruption();
            Tensor t;
            fld->get_value(t, cell.index());
            Point p = cell.center();
            double eigen1, eigen2, eigen3;
            t.get_eigenvalues(eigen1, eigen2, eigen3);

            Vector eigvals(fabs(eigen1), fabs(eigen2), fabs(eigen3));

            bool eig_x_0 = eigvals.x() < epsilon && eigvals.x() > -epsilon;
            bool eig_y_0 = eigvals.y() < epsilon && eigvals.y() > -epsilon;
            bool eig_z_0 = eigvals.z() < epsilon && eigvals.z() > -epsilon;

            bool neg_eigval = (eigen1 < -epsilon || eigen2 < -epsilon || eigen3 < -epsilon);
            if(neg_eigval)
              neg_eigval_count++;

            node_color = set_tensor_color(t, colorMap, colorScheme);

            double magnitude = t.magnitude();
            // If normalize checkbox selected
            if(normalizeGlyphs){
              eigvals.x(eigen1 / magnitude);
              eigvals.y(eigen2 / magnitude);
              eigvals.z(eigen3 / magnitude);
            }

            // Do not render tensors that are too small - because surfaces
            // are not renderd at least two of the scales must be non zero.
            if(!renderGlyphsBelowThreshold){
              if(t.magnitude() < threshold)
                continue;
            }

            if((eig_x_0 + eig_y_0 + eig_z_0) <= 1)
              {
                eigvals *= scale;

                switch (renState.mGlyphType)
                  {
                  case RenderState::GlyphType::BOX_GLYPH:
                    glyphs.addBox(p, t, scale);
                    break;
                  case RenderState::GlyphType::ELLIPSOID_GLYPH:
                    glyphs.addEllipsoid(p, t, eigvals, resolution, node_color);
                    tensorcount++;
                    break;
                  case RenderState::GlyphType::SPHERE_GLYPH:
                    glyphs.addSphere(p, eigvals.x(), resolution, node_color);
                  default:
                    break;
                  }
              }
            // Tensor as line
            else if((eig_x_0 + eig_y_0 + eig_z_0) == 2)
              {
                eigvals *= scale;

                Vector eigvec1, eigvec2, eigvec3;
                t.get_eigenvectors(eigvec1, eigvec2, eigvec3);
                Transform trans(p, eigvec1, eigvec2, eigvec3);
                Point p1 = p + trans *  eigvals/2;
                Point p2 = p + trans * -eigvals/2;
                addGlyph(tensor_line_glyphs, RenderState::GlyphType::LINE_GLYPH, p1, p2, scale, scale, resolution, node_color, true);
              }
            // Too small: render as point
            else
              {
                point_glyphs.addPoint(p, node_color);
              }
        }
        break;
    }


    if(neg_eigval_count > 0) {
        module_->warning(std::to_string(neg_eigval_count) + " negative eigen values in data.");
    }

    glyphs.buildObject(*geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENCY),
                       state->getValue(ShowFieldGlyphs::TensorsUniformTransparencyValue).toDouble(), colorScheme, renState, primIn, mesh->get_bounding_box());

    // Render lines(2 eigenvalues equalling 0)
    RenderState lineRenState = getVectorsRenderState(state, sfield, tfield, colorMap, colorMap, colorMap);
    tensor_line_glyphs.buildObject(*geom, uniqueLineID, lineRenState.get(RenderState::USE_TRANSPARENT_EDGES),
                       state->getValue(ShowFieldGlyphs::TensorsUniformTransparencyValue).toDouble(), colorScheme, lineRenState, SpireIBO::PRIMITIVE::LINES, mesh->get_bounding_box());
    // Render scalars(3 eigenvalues equalling 0)
    RenderState pointRenState = getScalarsRenderState(state, colorMap);
    point_glyphs.buildObject(*geom, uniquePointID, pointRenState.get(RenderState::USE_TRANSPARENT_NODES),
                       state->getValue(ShowFieldGlyphs::TensorsUniformTransparencyValue).toDouble(), colorScheme, pointRenState, SpireIBO::PRIMITIVE::POINTS, mesh->get_bounding_box());
}

ColorRGB GlyphBuilder::set_tensor_color(Tensor& t, boost::optional<ColorMapHandle> colorMap, ColorScheme colorScheme){
  ColorRGB node_color;
  if (colorScheme != ColorScheme::COLOR_UNIFORM)
  {
      if (colorScheme == ColorScheme::COLOR_MAP)
      {
          ColorMapHandle map = colorMap.get();
          node_color = map->valueToColor(t);
      }
      if (colorScheme == ColorScheme::COLOR_IN_SITU)
      {
          Vector colorVector;
          double eigval1, eigval2, eigval3;
          t.get_eigenvalues(eigval1, eigval2, eigval3);

          if(eigval1 == eigval2 && eigval1 != eigval3){
            Vector eigvec3_norm = t.get_eigenvector3().normal();
            Vector xCross = Cross(eigvec3_norm, Vector(1,0,0));
            Vector yCross = Cross(eigvec3_norm, Vector(0,1,0));
            Vector zCross = Cross(eigvec3_norm, Vector(0,0,1));
            xCross.normalize();
            yCross.normalize();
            zCross.normalize();

            double epsilon = pow(2, -52);
            if(std::abs(Dot(xCross, yCross)) > (1-epsilon)){
              colorVector = xCross;
            }
            else if(std::abs(Dot(yCross, zCross)) > (1-epsilon)){
              colorVector = yCross;
            }
            else if(std::abs(Dot(xCross, zCross)) > (1-epsilon)){
              colorVector = zCross;
            }
            else{
              colorVector = t.get_eigenvector1();
            }

          } else{
            colorVector = t.get_eigenvector1();
          }
          colorVector = Abs(colorVector);
          colorVector.normalize();

          node_color = ColorRGB(std::abs(colorVector.x()), std::abs(colorVector.y()), std::abs(colorVector.z()));
      }
  }
  return node_color;
}

RenderState GlyphBuilder::getVectorsRenderState(
  ModuleStateHandle state,
  boost::optional<FieldHandle> sfield,
  boost::optional<FieldHandle> tfield,
  boost::optional<ColorMapHandle> colorMap,
  boost::optional<ColorMapHandle> scolorMap,
  boost::optional<ColorMapHandle> tcolorMap)
{
  RenderState renState;

  bool useColorMap = state->getValue(ShowFieldGlyphs::VectorsColoring).toInt() == 1;
  bool rgbConversion = state->getValue(ShowFieldGlyphs::VectorsColoring).toInt() == 2;

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
  if(c_type == "Colormap Lookup" && colorMap)
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
  renState.mVectorRadiusWidthInput = getInput(state->getValue(ShowFieldGlyphs::VectorsRadiusWidthDataInput).toString());

  return renState;
}

RenderState GlyphBuilder::getScalarsRenderState(
  ModuleStateHandle state,
  boost::optional<ColorMapHandle> colorMap)
{
  RenderState renState;

  renState.set(RenderState::USE_NORMALS, true);

  //renState.set(RenderState::IS_ON, state->getValue(ShowFieldGlyphs::ShowScalars).toBool());
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

  //renState.mTransparencyInput = getInput(state->getValue(ShowFieldGlyphs::ScalarsTransparencyDataInput).toString());
  //renState.mTransparencyInput = state->getValue(ShowFieldGlyphs::ScalarsTransparenycDataInput).toString();

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
  if(c_type == "Colormap Lookup" && colorMap)
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
  //renState.mColorInput = state->getValue(ShowFieldGlyphs::ScalarsColorDataInput).toString();

  return renState;
}

RenderState GlyphBuilder::getTensorsRenderState(
  ModuleStateHandle state,
  boost::optional<ColorMapHandle> colorMap)
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

  //renState.mTransparencyInput = getInput(state->getValue(ShowFieldGlyphs::TensorsTransparencyDataInput).toString());
  /**  std::string trans_input = state->getValue(ShowFieldGlyphs::TensorsTransparenycDataInput).toString();
  if(trans_input == "Primary")
    {
      renState.mTransparencyInput = PRIMARY_PORT;
    }
  else if(trans_input == "Secondary")
    {
      renState.mTransparencyInput = SECONDARY_PORT;
    }
  else if(trans_input == "Tertiary")
    {
      renState.mTransparencyInput = TERTIARY_PORT;
      }**/

  // Glpyh Type
  std::string glyph = state->getValue(ShowFieldGlyphs::TensorsDisplayType).toString();
  if(glyph == "Boxes")
    renState.mGlyphType = RenderState::GlyphType::BOX_GLYPH;
  else if(glyph == "Ellipsoids")
    renState.mGlyphType = RenderState::GlyphType::ELLIPSOID_GLYPH;
  else if(glyph == "Spheres")
    renState.mGlyphType = RenderState::GlyphType::SPHERE_GLYPH;
  else
    renState.mGlyphType = RenderState::GlyphType::BOX_GLYPH;

  renState.defaultColor = ColorRGB(state->getValue(ShowFieldGlyphs::DefaultMeshColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
    renState.defaultColor.g() > 1.0 ||
    renState.defaultColor.b() > 1.0) ?
    ColorRGB(
    renState.defaultColor.r() / 255.,
    renState.defaultColor.g() / 255.,
    renState.defaultColor.b() / 255.)
    : renState.defaultColor;

  // Coloring
  std::string color = state->getValue(ShowFieldGlyphs::TensorsColoring).toString();
  if(color == "Colormap Lookup" && colorMap)
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
  //  renState.mColorInput = state->getValue(ShowFieldGlyphs::TensorsColorDataInput).toString();

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
const AlgorithmParameterName ShowFieldGlyphs::VectorsRadiusWidthDataInput("VectorsRadiusWidthDataInput");
const AlgorithmParameterName ShowFieldGlyphs::VectorsRadiusWidthScale("VectorsRadiusWidthScale");
const AlgorithmParameterName ShowFieldGlyphs::ArrowHeadRatio("ArrowHeadRatio");
const AlgorithmParameterName ShowFieldGlyphs::RenderBidirectionaly("RenderBidirectionaly");
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
//const AlgorithmParameterName ShowFieldGlyphs::TensorsTransparencyDataInput("TensorsTransparencyDataInput");
const AlgorithmParameterName ShowFieldGlyphs::NormalizeTensors("NormalizeTensors");
const AlgorithmParameterName ShowFieldGlyphs::TensorsScale("TensorsScale");
const AlgorithmParameterName ShowFieldGlyphs::RenderTensorsBelowThreshold("RenderTensorsBelowThreshold");
const AlgorithmParameterName ShowFieldGlyphs::TensorsThreshold("TensorsThreshold");
const AlgorithmParameterName ShowFieldGlyphs::TensorsResolution("TensorsResolution");
