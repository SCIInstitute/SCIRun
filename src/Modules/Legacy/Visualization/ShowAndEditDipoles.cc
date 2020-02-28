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


/*
 *  ShowAndEditDipoles.cc:  Builds the RHS of the FE matrix for current sources
 *
 *  Written by:
 *   David Weinstein
 *   University of Utah
 *   May 1999
 *
 */

#include <Core/Algorithms/Visualization/RenderFieldState.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Logging/Log.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/WidgetFactory.h>
#include <Modules/Legacy/Visualization/ShowAndEditDipoles.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/ArrowWidget.h>
#include <Graphics/Widgets/WidgetFactory.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/GeometryPrimitives/BBox.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Networks;
using namespace Modules::Visualization;
using namespace Graphics::Datatypes;
using namespace Core::Algorithms;
using namespace Core::Datatypes;
using namespace Core::Geometry;
using namespace Core::Algorithms::Visualization;

MODULE_INFO_DEF(ShowAndEditDipoles, Visualization, SCIRun)

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

class ShowAndEditDipolesImpl
{
public:
  ShowAndEditDipolesImpl(std::function<Dataflow::Networks::ModuleStateHandle()> s,
    GeometryGeneratingModule* module) : state_(s), module_(module) {}
  void setInput(FieldHandle input) { fieldInput_ = input; }
  void loadData(bool inputsChanged);
  bool shouldRefreshGeometry() const;
  void refreshGeometry();
  bool shouldToggleLastVectorShown() const;
  void toggleLastVectorShown();
  void generateGeomsList();
  FieldHandle makePointCloud();
  const std::vector<Graphics::Datatypes::GeometryHandle>& geoms() const { return geoms_; }
  void saveToParameters();
  void adjustPositionFromTransform(const Core::Geometry::Transform& transformMatrix, size_t index, size_t id);
private:
  std::function<Dataflow::Networks::ModuleStateHandle()> state_;
  GeometryGeneratingModule* module_;
  FieldHandle fieldInput_;
  std::vector<Core::Geometry::Point> pos_;
  std::vector<Core::Geometry::Vector> direction_;
  std::vector<double> scale_;
  Core::Geometry::BBox last_bounds_;
  std::vector<Graphics::Datatypes::WidgetHandle> arrows_;
  std::vector<Graphics::Datatypes::GeometryHandle> geoms_;
  std::vector<Core::Geometry::Transform> previousTransforms_;

  bool firstRun_ = true;
  bool getFromFile_ = false;
  bool lastVectorShown_ = false;
  SizingType previousSizing_ = SizingType::ORIGINAL;
  double sphereRadius_{0};
  double cylinderRadius_{0};
  double coneRadius_{0};
  double diskRadius_{0};
  double diskDistFromCenter_{0};
  double diskWidth_{0};
  size_t widgetIter_ = 0;
  int resolution_ = 20;
  double previousScaleFactor_ = 0.0;
  double zeroVectorRescale_ = 1.0e-3;

  Core::Datatypes::ColorRGB lineCol_ = {0.8, 0.8, 0.2};

  void receiveInputPoints();
  void receiveInputDirections();
  void receiveInputScales();
  void receiveInputField();
  void generateOutputGeom();
  void makeScalesPositive();
  void resetData();
  std::string widgetName(size_t i, size_t id, size_t iter);
  void createDipoleWidget(Core::Geometry::BBox& bbox, Core::Geometry::Point& pos, Core::Geometry::Vector dir, double scale, size_t widget_num, bool show_as_vector);
  void moveDipolesTogether(const Core::Geometry::Transform &transform);
  Graphics::Datatypes::GeometryHandle addLines();
  void loadFromParameters();
};
}}}

ShowAndEditDipoles::ShowAndEditDipoles()
  : GeometryGeneratingModule(staticInfo_), impl_(new ShowAndEditDipolesImpl([this]() { return get_state(); }, this))
{
  INITIALIZE_PORT(DipoleInputField);
  INITIALIZE_PORT(DipoleOutputField);
  INITIALIZE_PORT(DipoleWidget);
}

void ShowAndEditDipoles::setStateDefaults()
{
  auto state = get_state();
  using namespace Parameters;
  state->setValue(FieldName, std::string());
  state->setValue(WidgetScaleFactor, 1.0);
  state->setValue(Sizing, static_cast<int>(SizingType::ORIGINAL));
  state->setValue(ShowLastAsVector, false);
  state->setValue(MoveDipolesTogether, false);
  state->setValue(ShowLines, false);
  state->setValue(Reset, false);
  state->setValue(DataSaved, false);
  state->setValue(DipolePositions, VariableList());
  state->setValue(DipoleDirections, VariableList());
  state->setValue(DipoleScales, VariableList());
  state->setValue(LargestSize, 0.0);

  getOutputPort(DipoleWidget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void ShowAndEditDipoles::execute()
{
  auto input = getRequiredInput(DipoleInputField);
  FieldInformation fi(input);
  auto state = get_state();

  // Point clouds must be linear, so we only loook for node-based data
  if (!(fi.is_pointcloudmesh()))
  {
    error("Input field was not a valid point cloud.");
    return;
  }
  else if(!fi.is_vector())
  {
    error("Input field does not contain vectors.");
    return;
  }

  using namespace Parameters;
  impl_->setInput(input);
  impl_->loadData(inputsChanged());

  // Recreate all dipoles only if all were altered
  if (inputsChanged() || impl_->shouldRefreshGeometry())
  {
    impl_->refreshGeometry();
  }

  // Only run if Show Last as Vector is toggled
  if (impl_->shouldToggleLastVectorShown())
  {
    impl_->toggleLastVectorShown();
  }

  impl_->generateGeomsList();

  sendOutput(DipoleOutputField, impl_->makePointCloud());

  // Generate composite geometry object
  auto comp_geo = createGeomComposite(*this, "dipoles", impl_->geoms().begin(), impl_->geoms().end());
  sendOutput(DipoleWidget, comp_geo);

  impl_->saveToParameters();
}

bool ShowAndEditDipolesImpl::shouldRefreshGeometry() const
{
  return state_()->getValue(Parameters::Reset).toBool()
    || static_cast<int>(previousSizing_) != state_()->getValue(Parameters::Sizing).toInt()
    || previousScaleFactor_ != state_()->getValue(Parameters::WidgetScaleFactor).toDouble()
    || state_()->getValue(Parameters::MoveDipolesTogether).toBool()
    || getFromFile_;
}

bool ShowAndEditDipolesImpl::shouldToggleLastVectorShown() const
{
  return lastVectorShown_ != state_()->getValue(Parameters::ShowLastAsVector).toBool();
}

void ShowAndEditDipolesImpl::loadData(bool inputsChanged)
{
  // Get new data upstream or load from existing data
  getFromFile_ = firstRun_ && state_()->getValue(Parameters::DataSaved).toBool();

  if (getFromFile_
     && !state_()->getValue(Parameters::Reset).toBool())
  {
    loadFromParameters();
    makeScalesPositive();
    firstRun_ = false;
  }
  else
  {
    if (inputsChanged
       || state_()->getValue(Parameters::Reset).toBool())
    {
      receiveInputField();
      makeScalesPositive();
    }

    // Scaling
    if (static_cast<int>(previousSizing_) != state_()->getValue(Parameters::Sizing).toInt()
       || previousScaleFactor_ != state_()->getValue(Parameters::WidgetScaleFactor).toDouble())
    {
      receiveInputScales();
      makeScalesPositive();
    }
  }

  bool zeroVector = false;
  for(size_t i = 0; i < scale_.size(); i++)
  {
    if(scale_[i] == 0.0)
    {
      scale_[i] = zeroVectorRescale_;
      direction_[i] = Vector(1, 1, 1).normal();
      zeroVector = true;
    }
  }

  if ((static_cast<int>(previousSizing_) != state_()->getValue(Parameters::Sizing).toInt() || state_()->getValue(Parameters::Reset).toBool())
     && SizingType(state_()->getValue(Parameters::Sizing).toInt()) == SizingType::NORMALIZE_VECTOR_DATA)
  {
    scale_.clear();
    for(size_t i = 0; i < pos_.size(); i++)
    {
      scale_[i] = 1.0;
    }
  }

  if (zeroVector)
  {
    module_->warning("Input data contains zero vectors.");
  }
}

void ShowAndEditDipolesImpl::loadFromParameters()
{
  VariableList positions = state_()->getValue(Parameters::DipolePositions).toVector();
  VariableList directions = state_()->getValue(Parameters::DipoleDirections).toVector();
  VariableList scales = state_()->getValue(Parameters::DipoleScales).toVector();

  pos_.resize(positions.size());
  direction_.resize(positions.size());
  scale_.resize(positions.size());
  for(size_t i = 0; i < positions.size(); i++)
  {
    pos_[i] = pointFromString(positions[i].toString());
    direction_[i] = vectorFromString(directions[i].toString());
    scale_[i] = scales[i].toDouble();
  }
}

void ShowAndEditDipolesImpl::saveToParameters()
{
  widgetIter_++;
  VariableList positions;
  VariableList directions;
  VariableList scales;
  for(size_t i = 0; i < pos_.size(); i++)
  {
    positions.push_back(makeVariable("dip_pos", pos_[i].get_string()));
    directions.push_back(makeVariable("dip_dir", direction_[i].get_string()));
    scales.push_back(makeVariable("dip_scale", scale_[i]));
  }
  state_()->setValue(Parameters::DipolePositions, positions);
  state_()->setValue(Parameters::DipoleDirections, directions);
  state_()->setValue(Parameters::DipoleScales, scales);
  state_()->setValue(Parameters::DataSaved, true);
}

void ShowAndEditDipolesImpl::refreshGeometry()
{
  arrows_.clear();

  generateOutputGeom();
  state_()->setValue(Parameters::Reset, false);
  previousSizing_ = static_cast<SizingType>(state_()->getValue(Parameters::Sizing).toInt());
  previousScaleFactor_ = state_()->getValue(Parameters::WidgetScaleFactor).toDouble();
}

void ShowAndEditDipolesImpl::toggleLastVectorShown()
{
  auto bbox = fieldInput_->vmesh()->get_bounding_box();

  size_t last_id = pos_.size() - 1;

  double scale = scale_[last_id];
  if (static_cast<SizingType>(state_()->getValue(Parameters::Sizing).toInt()) == SizingType::NORMALIZE_BY_LARGEST_VECTOR)
    scale /= state_()->getValue(Parameters::LargestSize).toDouble();

  // Overwrite point
  arrows_[last_id] = WidgetFactory::createArrowWidget(
    {*module_, "SAED"},
    {{scale * state_()->getValue(Parameters::WidgetScaleFactor).toDouble(),
    "no-color",
    pos_[last_id],
    bbox,
    resolution_},
    pos_[last_id],
    direction_[last_id],
    lastVectorShown_, last_id, ++widgetIter_});

  lastVectorShown_ = state_()->getValue(Parameters::ShowLastAsVector).toBool();
}

void ShowAndEditDipolesImpl::generateGeomsList()
{
  // Rewrite all existing geom
  geoms_.clear();
  for (const auto& arrow : arrows_)
  {
    auto composite = boost::dynamic_pointer_cast<CompositeWidget>(arrow);
    geoms_.insert(geoms_.end(), composite->subwidgetBegin(), composite->subwidgetEnd());
  }

  if (state_()->getValue(Parameters::ShowLines).toBool())
    geoms_.push_back(addLines());
}

void ShowAndEditDipoles::processWidgetFeedback(const ModuleFeedback& var)
{
  try
  {
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    if (vsf.matchesWithModuleId(id()))
    {
      size_t widgetType;
      size_t widgetID;
      try
      {
      // Check if correct widget type
        static boost::regex r("ArrowWidget((.+)).+");
        boost::smatch what;
        regex_match(vsf.selectionName, what, r);

        // Get widget index and id
        static boost::regex ind_r("\\([0-9]*\\)");
        boost::smatch match;
        std::string::const_iterator searchStart(vsf.selectionName.cbegin());
        std::vector<std::string> matches;

        // Find all matches
        while (regex_search(searchStart, vsf.selectionName.cend(), match, ind_r))
        {
          matches.push_back(match[0]);
          searchStart = match.suffix().first;
        }

        // Remove parantheses
        for (auto& match : matches)
        {
          match = match.substr(1, match.length()-2);
        }

        // Cast to size_t
        widgetType = boost::lexical_cast<size_t>(matches[0]);
        widgetID = boost::lexical_cast<size_t>(matches[1]);
        impl_->adjustPositionFromTransform(vsf.transform, widgetType, widgetID);
      }
      catch (...)
      {
        logWarning("Failure parsing widget id");
      }
      enqueueExecuteAgain(false);
    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
}

void ShowAndEditDipolesImpl::moveDipolesTogether(const Transform& transform)
{
  for (auto& pos : pos_)
  {
    pos = transform * pos;
  }
}

void ShowAndEditDipolesImpl::adjustPositionFromTransform(const Transform& transformMatrix, size_t type, size_t id)
{
  auto bbox = fieldInput_->vmesh()->get_bounding_box();
  bool is_vector = boost::dynamic_pointer_cast<ArrowWidget>(arrows_[id])->isVector();

  if (state_()->getValue(Parameters::MoveDipolesTogether).toBool() && (type == ArrowWidgetSection::CYLINDER || type == ArrowWidgetSection::SPHERE))
  {
    moveDipolesTogether(transformMatrix);
  }
  else
  {
    pos_[id] = transformMatrix * pos_[id];
    direction_[id] = transformMatrix * direction_[id];
    scale_[id] = direction_[id].length() * scale_[id];
    direction_[id].normalize();
  }
  makeScalesPositive();

  double currentScale = scale_[id] * state_()->getValue(Parameters::WidgetScaleFactor).toDouble();
  if (state_()->getValue(Parameters::Sizing).toInt() == static_cast<int>(SizingType::NORMALIZE_BY_LARGEST_VECTOR))
    currentScale /= state_()->getValue(Parameters::LargestSize).toDouble();

  arrows_[id] = WidgetFactory::createArrowWidget(
    {*module_, "SAED"},
    {{currentScale, "no-color", pos_[id], bbox, resolution_ },
      pos_[id], direction_[id], is_vector, id, ++widgetIter_ });
}

void ShowAndEditDipolesImpl::receiveInputPoints()
{
  auto vf = fieldInput_->vfield();
  Point p;
  pos_.clear();
  for(const auto& node : fieldInput_->mesh()->getFacade()->nodes())
  {
    size_t index = node.index();
    vf->get_center(p, index);
    pos_.push_back(p);
  }
}

void ShowAndEditDipolesImpl::receiveInputDirections()
{
  auto vf = fieldInput_->vfield();
  Vector v;
  direction_.clear();
  for(const auto& node : fieldInput_->mesh()->getFacade()->nodes())
  {
    size_t index = node.index();
    vf->get_value(v, index);
    direction_.push_back(v.normal());
  }
}

void ShowAndEditDipolesImpl::receiveInputScales()
{
  auto vf = fieldInput_->vfield();
  Vector v;
  scale_.clear();
  double newLargest = 0.0;
  for (const auto& node : fieldInput_->mesh()->getFacade()->nodes())
  {
    size_t index = node.index();
    vf->get_value(v, index);
    scale_.push_back(v.length());
    newLargest = std::max(newLargest, std::abs(v.length()));
  }
  state_()->setValue(Parameters::LargestSize, newLargest);
}

void ShowAndEditDipolesImpl::makeScalesPositive()
{
  // If dipole is scaled below 0, make the scale positive and flip the direction
  for (size_t i = 0; i < scale_.size(); i++)
  {
    if (scale_[i] < 0.0)
    {
      scale_[i] = std::abs(scale_[i]);
      direction_[i] = -direction_[i];
    }
  }
}

void ShowAndEditDipolesImpl::receiveInputField()
{
  auto vf = fieldInput_->vfield();
  Point p;
  Vector v;
  direction_.clear();
  pos_.clear();
  scale_.clear();
  double newLargest = 0.0;
  for (const auto& node : fieldInput_->mesh()->getFacade()->nodes())
  {
    size_t index = node.index();
    vf->get_center(p, index);
    vf->get_value(v, index);
    pos_.push_back(p);
    direction_.push_back(v.normal());
    scale_.push_back(v.length());
    newLargest = std::max(newLargest, std::abs(v.length()));
  }
  state_()->setValue(Parameters::LargestSize, newLargest);
}

void ShowAndEditDipolesImpl::generateOutputGeom()
{
  auto bbox = fieldInput_->vmesh()->get_bounding_box();

  last_bounds_ = bbox;
  arrows_.resize(0);

  std::string name = "SAED";
  // Create all but last dipole as vector
  for (size_t i = 0; i < pos_.size() - 1; i++)
  {
    double scale = scale_[i];
    if (static_cast<SizingType>(state_()->getValue(Parameters::Sizing).toInt()) == SizingType::NORMALIZE_BY_LARGEST_VECTOR)
      scale /= state_()->getValue(Parameters::LargestSize).toDouble();

    arrows_.push_back(WidgetFactory::createArrowWidget(
        {*module_, name},
        {{scale * state_()->getValue(Parameters::WidgetScaleFactor).toDouble(),
        "no-color", pos_[i], bbox, resolution_},
        pos_[i], direction_[i],
        true, i, ++widgetIter_}));
  }

  // Create last dipoles separately to check if shown as vector
  size_t last_id = pos_.size() - 1;

  double scale = scale_[last_id];
  if (static_cast<SizingType>(state_()->getValue(Parameters::Sizing).toInt()) == SizingType::NORMALIZE_BY_LARGEST_VECTOR)
    scale /= state_()->getValue(Parameters::LargestSize).toDouble();

    arrows_.push_back(WidgetFactory::createArrowWidget(
      {*module_, name},
      {{scale * state_()->getValue(Parameters::WidgetScaleFactor).toDouble(),
        "no-color",
      pos_[last_id], bbox, resolution_},
      pos_[last_id], direction_[last_id],
      state_()->getValue(Parameters::ShowLastAsVector).toBool(),
      last_id, ++widgetIter_ }));

  lastVectorShown_ = state_()->getValue(Parameters::ShowLastAsVector).toBool();
}

GeometryHandle ShowAndEditDipolesImpl::addLines()
{
  auto bbox = fieldInput_->vmesh()->get_bounding_box();

  SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::LINES;
  std::string idName = "SAEDField" +
    GeometryObject::delimiter +
    state_()->getValue(Parameters::FieldName).toString()
    + " (from " + module_->id().id_ +")" +
    "(" + std::to_string(widgetIter_) + ")";

  auto geom(boost::make_shared<GeometryObjectSpire>(*module_, idName, true));
  Graphics::GlyphGeom glyphs;

  RenderState renState;
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENT_EDGES, false);
  renState.mGlyphType = RenderState::GlyphType::LINE_GLYPH;
  renState.defaultColor = lineCol_;
  renState.set(RenderState::USE_DEFAULT_COLOR, true);

  // Create lines between every point
  for (size_t a = 0; a < pos_.size(); a++)
  {
    for (size_t b = a + 1; b < pos_.size(); b++)
    {
      glyphs.addLine(pos_[a], pos_[b], lineCol_, lineCol_);
    }
  }

  glyphs.buildObject(*geom, idName, false, 0.5, ColorScheme::COLOR_UNIFORM, renState, primIn, bbox);
  return geom;
}

FieldHandle ShowAndEditDipolesImpl::makePointCloud()
{
  FieldInformation fi("PointCloudMesh", 0, "Vector");
  auto ofield = CreateField(fi);
  auto mesh = ofield->vmesh();
  auto field = ofield->vfield();

  for (size_t i = 0; i < arrows_.size(); i++)
  {
    auto pcindex = mesh->add_point(pos_[i]);
    field->resize_fdata();

    double scale = scale_[i];
    if (state_()->getValue(Parameters::Sizing).toInt() == static_cast<int>(SizingType::NORMALIZE_BY_LARGEST_VECTOR))
      scale /= state_()->getValue(Parameters::LargestSize).toDouble();

    field->set_value(static_cast<Vector>(direction_[i] * scale), pcindex);
  }
  return ofield;
}

ALGORITHM_PARAMETER_DEF(Visualization, FieldName);
ALGORITHM_PARAMETER_DEF(Visualization, WidgetScaleFactor);
ALGORITHM_PARAMETER_DEF(Visualization, Sizing);
ALGORITHM_PARAMETER_DEF(Visualization, ShowLastAsVector);
ALGORITHM_PARAMETER_DEF(Visualization, ShowLines);
ALGORITHM_PARAMETER_DEF(Visualization, Reset);
ALGORITHM_PARAMETER_DEF(Visualization, MoveDipolesTogether);
ALGORITHM_PARAMETER_DEF(Visualization, DipolePositions);
ALGORITHM_PARAMETER_DEF(Visualization, DipoleDirections);
ALGORITHM_PARAMETER_DEF(Visualization, DipoleScales);
ALGORITHM_PARAMETER_DEF(Visualization, DataSaved);
ALGORITHM_PARAMETER_DEF(Visualization, LargestSize);
