/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Logging/Log.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Widgets/SphereWidget.h>
#include <Graphics/Widgets/ConeWidget.h>
#include <Graphics/Widgets/CylinderWidget.h>
#include <Modules/Legacy/Visualization/ShowAndEditDipoles.h>
#include <math.h>

using namespace SCIRun;
using namespace Core;
using namespace Logging;
using namespace Datatypes;
using namespace Algorithms;
using namespace Geometry;
using namespace Graphics;
using namespace Modules::Visualization;
using namespace Dataflow::Networks;
using namespace Graphics::Datatypes;

MODULE_INFO_DEF(ShowAndEditDipoles, Visualization, SCIRun)

enum WidgetSection {
  SPHERE,
  CYLINDER,
  CONE,
  DISK,
  LINE
};

ShowAndEditDipoles::ShowAndEditDipoles()
  : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(DipoleInputField);
  INITIALIZE_PORT(DipoleOutputField);
  INITIALIZE_PORT(DipoleWidget);

  reset_ = true;
  lastVectorShown_ = false;

  deflPointCol_ = ColorRGB(0.54, 0.6, 1.0);
  deflCol_ = ColorRGB(0.5, 0.5, 0.5);
  greenCol_ = ColorRGB(0.2, 0.8, 0.2);
  resizeCol_ = ColorRGB(0.54, 1.0, 0.60);
  lineCol_ = ColorRGB(0.8, 0.8, 0.2);

  sphereRadius_ = 0.25;
  cylinderRadius_ = 0.12;
  coneRadius_ = 0.25;
  diskRadius_ = 0.25;
  diskDistFromCenter_ = 0.85;
  diskWidth_ = 0.05;
  widgetIter_ = 0;
}

void ShowAndEditDipoles::setStateDefaults()
{
  auto state = get_state();
  state->setValue(FieldName, std::string());
  state->setValue(WidgetSize, 0.23);
  state->setValue(Sizing, 0);
  state->setValue(ShowLastAsVector, false);
  state->setValue(ShowLines, false);
  state->setValue(PointPositions, VariableList());

  getOutputPort(DipoleWidget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void ShowAndEditDipoles::execute()
{
  FieldHandle fh = getRequiredInput(DipoleInputField);
  FieldInformation fi(fh);
  auto state = get_state();

  if (!(fi.is_pointcloudmesh()))
  {
    error("Input field was not a valid point cloud.");
    return;
  }

  // Get new data upstream
  if(inputsChanged())
  {
    // Garbage collect
    for(auto dip : pointWidgets_)
    {
      dip->erase(dip->begin(), dip->end());
    }
    pointWidgets_.erase(pointWidgets_.begin(), pointWidgets_.end());

    ReceiveInputField();
    GenerateOutputGeom();
  }

  // Only run if Show Last as Vector is toggled
  if(lastVectorShown_ != state->getValue(ShowLastAsVector).toBool())
  {
    auto bbox = fh->vmesh()->get_bounding_box();
    int last_id = pos_.size() - 1;
    // Destroy last dipole
    pointWidgets_[last_id]->erase(pointWidgets_[last_id]->begin(), pointWidgets_[last_id]->end());
    createDipoleWidget(bbox, pos_[last_id], direction_[last_id] * state->getValue(WidgetSize).toDouble(), last_id, state->getValue(ShowLastAsVector).toBool());
    lastVectorShown_ = state->getValue(ShowLastAsVector).toBool();
  }

  // Recreate geom list
  geoms_.resize(0);

  // Rewrite all existing geom
  for(int d = 0; d < pointWidgets_.size(); d++)
  {
    for(int w = 0; w < pointWidgets_[d]->size(); w++)
      geoms_.push_back((*pointWidgets_[d])[w]);
  }
  if(state->getValue(ShowLines).toBool())
    geoms_.push_back(addLines());

  sendOutput(DipoleOutputField, makePointCloud());

  std::string idName = "SAEDField";
  idName += GeometryObject::delimiter +
    state->getValue(ShowAndEditDipoles::FieldName).toString() +
    " (from " + id().id_ +")" +
    "(" + std::to_string(widgetIter_) + ")";

  auto comp_geo = createGeomComposite(*this, "dipoles", geoms_.begin(), geoms_.end());
  sendOutput(DipoleWidget, comp_geo);
  widgetIter_++;
}

void ShowAndEditDipoles::processWidgetFeedback(const ModuleFeedback& var)
{
  try
  {
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    if (vsf.matchesWithModuleId(id()))
    {
      int widgetType = -1;
      int widgetID = -1;
      try
      {
      // Check if correct module
        static boost::regex r(".*Widget::SAED((.+)).+");
        boost::smatch what;
        regex_match(vsf.selectionName, what, r);

        // Get widget index and id
        static boost::regex ind_r("\\([0-9]*\\)");
        boost::smatch match;
        std::string::const_iterator searchStart(vsf.selectionName.cbegin());
        std::vector<std::string> matches;

        // Find all matches
        while(regex_search(searchStart, vsf.selectionName.cend(), match, ind_r))
        {
          matches.push_back(match[0]);
          searchStart = match.suffix().first;
        }

        // Remove parantheses
        for(int i = 0; i < matches.size(); i++)
        {
          matches[i] = matches[i].substr(1, matches[i].length()-2);
        }
        // Cast to int
        widgetType = boost::lexical_cast<int>(matches[0]);
        widgetID = boost::lexical_cast<int>(matches[1]);
      }
      catch (...)
      {
        logWarning("Failure parsing widget id");
      }
        adjustPositionFromTransform(vsf.transform, widgetType, widgetID);
        enqueueExecuteAgain(false);
    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
}

void ShowAndEditDipoles::adjustPositionFromTransform(const Transform& transformMatrix, int type, int id)
{
  DenseMatrix center(4, 1);

  auto currLoc = (*pointWidgets_[id])[type]->position();
  center << currLoc.x(), currLoc.y(), currLoc.z(), 1.0;
  DenseMatrix newTransform(DenseMatrix(transformMatrix) * center);

  Point newPoint(newTransform(0, 0) / newTransform(3, 0),
                 newTransform(1, 0) / newTransform(3, 0),
                 newTransform(2, 0) / newTransform(3, 0));

  switch (type)
  {
  // Sphere and Cylinder reposition dipole
  case WidgetSection::SPHERE:
    pos_[id] = newPoint;
    break;
  case WidgetSection::CYLINDER:
  {
    double scale = get_state()->getValue(WidgetSize).toDouble();
    // Shift direction back because newPos is the center of the cylinder
    pos_[id] = newPoint - 1.375 * direction_[id] * scale * sphereRadius_;
    break;
  }
  // Cone rotates dipole
  case WidgetSection::CONE:
    direction_[id] = (newPoint - pos_[id]).normal() * direction_[id].length();
    break;
  // Disk resizes dipole
  case WidgetSection::DISK:
  {
    Vector newVec(newPoint-pos_[id]);
    newVec /= diskDistFromCenter_;
    direction_[id] = Dot(newVec, direction_[id].normal()) * direction_[id].normal();

    double scale = get_state()->getValue(WidgetSize).toDouble();
    direction_[id] /= scale;
    break;
  }
  }
  FieldHandle fh = getRequiredInput(DipoleInputField);
  auto state = get_state();
  auto bbox = fh->vmesh()->get_bounding_box();
  bool is_vector = (pointWidgets_[id]->size() == 4);
  createDipoleWidget(bbox, pos_[id], direction_[id] * state->getValue(WidgetSize).toDouble(), id, is_vector);
}

void ShowAndEditDipoles::ReceiveInputField()
{
  FieldHandle fh = getRequiredInput(DipoleInputField);
  VField* vf = fh->vfield();
  Point p;
  Vector v;
  direction_.clear();
  pos_.clear();
  for(const auto& node : fh->mesh()->getFacade()->nodes())
  {
    int index = node.index();
    vf->get_center(p, index);
    vf->get_value(v, index);
    pos_.push_back(p);
    direction_.push_back(v);
  }
  reset_ = false;
}

void ShowAndEditDipoles::GenerateOutputGeom()
{
  FieldHandle fh = getRequiredInput(DipoleInputField);
  auto state = get_state();
  auto bbox = fh->vmesh()->get_bounding_box();

  last_bounds_ = bbox;
  pointWidgets_.resize(pos_.size());

  // Create all but last dipole as vector
  for(int i = 0; i < pos_.size() - 1; i++)
  {
    createDipoleWidget(bbox, pos_[i], direction_[i] * state->getValue(WidgetSize).toDouble(), i, true);
  }

  // Create last dipoles separately to check if shown as vector
  int last_id = pos_.size() - 1;
  createDipoleWidget(bbox, pos_[last_id], direction_[last_id] * state->getValue(WidgetSize).toDouble(), last_id, state->getValue(ShowLastAsVector).toBool());
  lastVectorShown_ = state->getValue(ShowLastAsVector).toBool();
}

void ShowAndEditDipoles::createDipoleWidget(BBox& bbox, Point& pos, Vector scaled_dir, int widget_num, bool show_as_vector)
{
  auto widgetName = [](int i, int id, int iter) {
                      return "SAED(" + std::to_string(i) + ")" +
                        "(" + std::to_string(id) + ")" +
                        "(" + std::to_string(iter) + ")"; };

  Point bmin = pos;
  Point bmax = pos + scaled_dir;

  // Fix degenerate boxes.
  const double size_estimate = std::max((bmax - bmin).length() * 0.01, 1.0e-5);
  if (fabs(bmax.x() - bmin.x()) < 1.0e-6)
  {
    bmin.x(bmin.x() - size_estimate);
    bmax.x(bmax.x() + size_estimate);
  }
  if (fabs(bmax.y() - bmin.y()) < 1.0e-6)
  {
    bmin.y(bmin.y() - size_estimate);
    bmax.y(bmax.y() + size_estimate);
  }
  if (fabs(bmax.z() - bmin.z()) < 1.0e-6)
  {
    bmin.z(bmin.z() - size_estimate);
    bmax.z(bmax.z() + size_estimate);
  }

  Point center = bmin + scaled_dir/2.0;

  pointWidgets_[widget_num] = new std::vector<WidgetHandle>(1 + show_as_vector * 3);

  ColorRGB sphereCol = (show_as_vector) ? deflPointCol_ : resizeCol_;

  // Create glyphs
  (*pointWidgets_[widget_num])[0] = (boost::dynamic_pointer_cast<WidgetBase>
                                      (WidgetFactory::createSphere(
                                        *this,
                                        widgetName(WidgetSection::SPHERE, widget_num, widgetIter_),
                                        sphereRadius_ * scaled_dir.length(),
                                        sphereCol.toString(),
                                        bmin,
                                        bbox)));

  if(show_as_vector)
  {
    // Starts the cylinder position closer to the surface of the sphere
    Point cylinderStart = bmin + 0.75 * (scaled_dir * sphereRadius_);

    (*pointWidgets_[widget_num])[1] = (boost::dynamic_pointer_cast<WidgetBase>
                                                (WidgetFactory::createCylinder(
                                                  *this,
                                                  widgetName(WidgetSection::CYLINDER, widget_num, widgetIter_),
                                                  cylinderRadius_ * scaled_dir.length(),
                                                  deflCol_.toString(),
                                                  cylinderStart,
                                                  center,
                                                  bbox)));
    (*pointWidgets_[widget_num])[2] = (boost::dynamic_pointer_cast<WidgetBase>
                                                (WidgetFactory::createCone(
                                                  *this,
                                                  widgetName(WidgetSection::CONE, widget_num, widgetIter_),
                                                  coneRadius_ * scaled_dir.length(),
                                                  deflCol_.toString(),
                                                  center,
                                                  bmax,
                                                  bbox,
                                                  true)));

    Point diskPos = bmin + scaled_dir * diskDistFromCenter_;
    Point dp1 = diskPos - diskWidth_ * scaled_dir;
    Point dp2 = diskPos + diskWidth_ * scaled_dir;

    (*pointWidgets_[widget_num])[3] = (boost::dynamic_pointer_cast<WidgetBase>
                                                (WidgetFactory::createDisk(
                                                  *this,
                                                  widgetName(WidgetSection::DISK, widget_num, widgetIter_),
                                                  diskRadius_ * scaled_dir.length(),
                                                  resizeCol_.toString(),
                                                  dp1,
                                                  dp2,
                                                  bbox)));
  }
}

GeometryHandle ShowAndEditDipoles::addLines()
{
  FieldHandle fh = getRequiredInput(DipoleInputField);
  auto state = get_state();
  auto bbox = fh->vmesh()->get_bounding_box();

  SpireIBO::PRIMITIVE primIn = SpireIBO::PRIMITIVE::LINES;
  std::string idName = "SAEDField" +
    GeometryObject::delimiter +
    state->getValue(ShowAndEditDipoles::FieldName).toString()
    + " (from " + id().id_ +")" +
    "(" + std::to_string(widgetIter_) + ")";

  auto geom(boost::make_shared<GeometryObjectSpire>(*this, idName, true));
  GlyphGeom glyphs;

  RenderState renState;
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENT_EDGES, false);
  renState.mGlyphType = RenderState::GlyphType::LINE_GLYPH;
  renState.defaultColor = lineCol_;
  renState.set(RenderState::USE_DEFAULT_COLOR, true);

  // Create lines between every point
  for(int a = 0; a < pos_.size(); a++)
  {
    for(int b = 0; b < pos_.size(); b++)
    {
      glyphs.addLine(pos_[a], pos_[b], lineCol_, lineCol_);
    }
  }

  glyphs.buildObject(*geom, idName, false, 0.5, ColorScheme::COLOR_UNIFORM, renState, primIn, bbox);
  return geom;
}

FieldHandle ShowAndEditDipoles::makePointCloud()
{
  FieldInformation fi("PointCloudMesh", 0, "Vector");
  auto ofield = CreateField(fi);
  auto mesh = ofield->vmesh();
  auto field = ofield->vfield();

  for (int i = 0; i < pointWidgets_.size(); i++)
  {
    VMesh::Node::index_type pcindex = mesh->add_point(pos_[i]);
    field->resize_fdata();
    field->set_value(static_cast<Vector>(direction_[i]), pcindex);
  }
  return ofield;
}

const AlgorithmParameterName ShowAndEditDipoles::FieldName("FieldName");
const AlgorithmParameterName ShowAndEditDipoles::WidgetSize("WidgetSize");
const AlgorithmParameterName ShowAndEditDipoles::Sizing("Sizing");
const AlgorithmParameterName ShowAndEditDipoles::ShowLastAsVector("ShowLastAsVector");
const AlgorithmParameterName ShowAndEditDipoles::ShowLines("ShowLines");
const AlgorithmParameterName ShowAndEditDipoles::PointPositions("PointPositions");





#if 0
#include <Dataflow/Network/Module.h>
#include <Core/Util/StringUtil.h>
#include <Dataflow/Network/Ports/GeometryPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Widgets/ArrowWidget.h>
#include <Dataflow/Network/Scheduler.h>

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Geom/GeomLine.h>
#include <Core/Geom/GeomSwitch.h>

#include <Core/Math/MiscMath.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Thread/CrowdMonitor.h>

#include <iostream>
#include <vector>


namespace BioPSE {

using namespace SCIRun;

class ShowAndEditDipoles : public Module {
  public:
    ShowAndEditDipoles(GuiContext *context);
    virtual ~ShowAndEditDipoles();

    virtual void execute();
    virtual void widget_moved(bool last, BaseWidget*);
    virtual void tcl_command(GuiArgs& args, void* userdata);
  private:
    void new_input_data(VField *in);
    void scale_changed();
    void scale_mode_changed();
    bool generate_output_field();
    void draw_lines();
    void load_gui();
    void last_as_vec();

    virtual void set_context(Network* network);
    static bool maybe_resize_widget(void*);

    CrowdMonitor             widget_lock_;
    std::vector<int>              widget_id_;
    std::vector<ArrowWidget*>     widget_;
    int                      gidx_;
    GuiInt                   num_dipoles_;

    MaterialHandle           greenMatl_;
    MaterialHandle           deflMatl_;

    FieldHandle              dipoleFldH_;
    GuiDouble                widgetSizeGui_;
    GuiString                scaleModeGui_;
    GuiInt                   showLastVecGui_;
    GuiInt                   showLinesGui_;
    int                      lastGen_;

    std::string                   execMsg_;
    std::vector<GeometryHandle>       widget_switch_;
    std::vector<GuiPoint*>        new_positions_;
    std::vector<GuiVector*>       new_directions_;
    std::vector<GuiDouble*>       new_magnitudes_;
    GuiDouble                max_len_;
    bool                     output_dirty_;
    bool                     been_executed_;
    bool                     reset_;
    double                   last_scale_;
    std::string                   last_scale_mode_;
    GuiInt                   force_field_reset_;
};


DECLARE_MAKER(ShowAndEditDipoles)


ShowAndEditDipoles::ShowAndEditDipoles(GuiContext *context) :
  Module("ShowAndEditDipoles", context, Filter, "Visualization", "BioPSE"),
  widget_lock_("ShowAndEditDipoles widget lock"),
  num_dipoles_(context->subVar("num-dipoles")),
  widgetSizeGui_(context->subVar("widgetSizeGui_")),
  scaleModeGui_(context->subVar("scaleModeGui_")),
  showLastVecGui_(context->subVar("showLastVecGui_")),
  showLinesGui_(context->subVar("showLinesGui_")),
  max_len_(context->subVar("max-len")),
  output_dirty_(true),
  been_executed_(false),
  reset_(false),
  last_scale_(0.0),
  last_scale_mode_("fixed"),
  force_field_reset_(context->subVar("force-field-reset", false))
{
  lastGen_=-1;
  greenMatl_ = new Material(Color(0.2, 0.8, 0.2));
  gidx_=0;
}

ShowAndEditDipoles::~ShowAndEditDipoles()
{
  for (size_t j=0; j<widget_.size();j++)
  {
    if (widget_[j]) delete widget_[j];
  }

  widget_.clear();
}

void
ShowAndEditDipoles::set_context(Network* network)
{
  Module::set_context(network);
  if (sci_getenv_p("SCIRUN_USE_DEFAULT_SETTINGS"))
  {
    sched_->add_callback(maybe_resize_widget, this);
  }
}

bool
ShowAndEditDipoles::maybe_resize_widget(void *ptr)
{
  if (sci_getenv_p("SCIRUN_USE_DEFAULT_SETTINGS"))
  {
    ShowAndEditDipoles* me = (ShowAndEditDipoles*)ptr;
    GeometryOPortHandle ogeom;
    me->get_oport_handle("Geometry",ogeom);
    BBox b;
    if (ogeom->get_view_bounds(b))
    {
      double sc = b.diagonal().length() * 0.02;
      for (int i = 0; i < me->num_dipoles_.get(); i++)
      {
        me->widget_[i]->SetScale(sc);
        me->widget_[i]->SetLength(2 * sc);
      }
    }
    else
    {
      me->warning("Could not create default widget size");
    }
  }
  return true;
}

void
ShowAndEditDipoles::execute()
{
  GeometryOPortHandle ogeom;
  get_oport_handle("Geometry",ogeom);

  // if this is the first execution then try loading all the values
  // from saved GuiVars, load the widgets from these values
  if (!been_executed_)
  {
    load_gui();
    been_executed_ = true;
  }

  FieldHandle fieldH;
  get_input_handle("dipoleFld", fieldH,true);

  FieldInformation fi(fieldH);

  if (!(fi.is_pointcloudmesh()))
  {
    error("Input field was not a valid point cloud.");
    return;
  }

  VField* field     = fieldH->vfield();

  int gen = fieldH->generation;

  reset_vars();
  if (needToExecute() || (gen != lastGen_))
  {
    lastGen_ = gen;
    if (needToExecute() || (field->num_values() != static_cast<VMesh::size_type>(num_dipoles_.get())))
    {
      new_input_data(field);
    }
    if (force_field_reset_.get())
    {
      new_input_data(field);
      output_dirty_ = true;
    }
  }

  //widget_moved(true);
  last_as_vec();
  draw_lines();
  generate_output_field();
  ogeom->flushViews();
  send_output_handle("dipoleFld", dipoleFldH_, true);
}

void
ShowAndEditDipoles::load_gui()
{
  num_dipoles_.reset();
  new_positions_.resize(num_dipoles_.get(), 0);
  new_directions_.resize(num_dipoles_.get(), 0);
  new_magnitudes_.resize(num_dipoles_.get(), 0);
  for (int i = 0; i < num_dipoles_.get(); i++) {
    if (!new_positions_[i]) {
      std::ostringstream str;
      str << "newpos" << i;
      new_positions_[i] = new GuiPoint(get_ctx()->subVar(str.str()));
      new_positions_[i]->reset();
    }
    if (!new_directions_[i]) {
      std::ostringstream str;
      str << "newdir" << i;
      new_directions_[i] = new GuiVector(get_ctx()->subVar(str.str()));
      new_directions_[i]->reset();
    }
    if (!new_magnitudes_[i]) {
      std::ostringstream str;
      str << "newmag" << i;
      new_magnitudes_[i] = new GuiDouble(get_ctx()->subVar(str.str()));
      new_magnitudes_[i]->reset();
    }

    // it is possible that these were created already, dont do it twice.
    if ((int)widget_id_.size() != num_dipoles_.get())
    {
      GeometryOPortHandle ogeom;
      get_oport_handle("Geometry",ogeom);

      ArrowWidget *a = new ArrowWidget(this, &widget_lock_,
					  widgetSizeGui_.get());
      a->Connect(ogeom.get_rep());
      a->SetCurrentMode(1);
      widget_.push_back(a);
      deflMatl_ = widget_[0]->GetMaterial(0);
      widget_switch_.push_back(widget_[i]->GetWidget());
      ((GeomSwitch *)(widget_switch_[i].get_rep()))->set_state(1);
      widget_id_.push_back(ogeom->addObj(widget_switch_[i],
					  "Dipole" + to_string((int)i),
					  &widget_lock_));

      widget_[i]->SetPosition(new_positions_[i]->get());
      double str = new_magnitudes_[i]->get();
      widget_[i]->SetDirection(new_directions_[i]->get());
      double sc = widgetSizeGui_.get();
      std::string scaleMode = scaleModeGui_.get();
      if (scaleMode == "normalize") sc *= (str / max_len_.get());
      else if (scaleMode == "scale") sc *= str;
      widget_[i]->SetScale(sc);
      widget_[i]->SetLength(2 * sc);
    }
  }
  last_scale_ = widgetSizeGui_.get();
}

void
ShowAndEditDipoles::new_input_data(VField *field)
{
  num_dipoles_.reset();
  widgetSizeGui_.reset();
  showLastVecGui_.reset();
  reset_ = false;

  if (widget_switch_.size())
  {
    widget_[num_dipoles_.get() - 1]->SetCurrentMode(1);
    widget_[num_dipoles_.get() - 1]->SetMaterial(0, deflMatl_);
  }
  // turn off any extra arrow widgets we might have.
  VField::size_type num_values = field->num_values();
  if (num_values < static_cast<size_type>(num_dipoles_.get()))
  {
    for (int i = static_cast<int>(num_values); i < num_dipoles_.get(); i++)
      ((GeomSwitch *)(widget_switch_[i].get_rep()))->set_state(0);
    num_dipoles_.set(num_values);
    num_dipoles_.reset();
  }
  else
  {
    GeometryOPortHandle ogeom;
    get_oport_handle("Geometry",ogeom);

    VField::index_type i;
    for (i = num_dipoles_.get(); i < static_cast<VField::size_type>(widget_switch_.size()); i++)
      ((GeomSwitch *)(widget_switch_[i].get_rep()))->set_state(1);

    VField::size_type num_values = field->num_values();
    for (; i < num_values; i++)
    {
      ArrowWidget *a = new ArrowWidget(this, &widget_lock_,
					  widgetSizeGui_.get());
      a->Connect(ogeom.get_rep());
      a->SetCurrentMode(1);
      widget_.push_back(a);
      deflMatl_ = widget_[0]->GetMaterial(0);
      widget_switch_.push_back(widget_[i]->GetWidget());
      ((GeomSwitch *)(widget_switch_[i].get_rep()))->set_state(1);
      widget_id_.push_back(ogeom->addObj(widget_switch_[i],
					  "Dipole" + to_string((int)i),
					  &widget_lock_));

    }
    num_dipoles_.set(static_cast<int>(num_values));
    num_dipoles_.reset();
    load_gui();
  }

  if (showLastVecGui_.get())
  {
    widget_[num_dipoles_.get() - 1]->SetCurrentMode(1);
    widget_[num_dipoles_.get() - 1]->SetMaterial(0, deflMatl_);
  }
  else
  {
    widget_[num_dipoles_.get() - 1]->SetCurrentMode(2);
    widget_[num_dipoles_.get() - 1]->SetMaterial(0, greenMatl_);
  }

  for (VField::index_type i = 0; i < static_cast<VField::size_type>(num_dipoles_.get()); i++)
  {
    Vector v;
    field->get_value(v,i);
    double dv = v.length();
    if (i == 0 || dv > max_len_.get())
    {
      max_len_.set(dv);
      max_len_.reset();
    }
  }

  std::string scaleMode = scaleModeGui_.get();
  for (VMesh::Node::index_type i = 0; i < static_cast<VMesh::size_type>(num_dipoles_.get()); i++)
  {

    VMesh* field_mesh = field->vmesh();
    Point p;
    field_mesh->get_point(p,i);
    new_positions_[i]->set(p);
    widget_[i]->SetPosition(p);
    Vector v;
    field->get_value(v,i);
    double str = v.length();
    new_magnitudes_[i]->set(str);
    if (str < 0.0000001) v.z(1);
    v.safe_normalize();
    widget_[i]->SetDirection(v);
    new_directions_[i]->set(v);
    double sc = widgetSizeGui_.get();

    if (scaleMode == "normalize") sc *= (str / max_len_.get());
    else if (scaleMode == "scale") sc *= str;
    widget_[i]->SetScale(sc);
    widget_[i]->SetLength(2 * sc);
  }
  last_scale_ = widgetSizeGui_.get();
}

void
ShowAndEditDipoles::last_as_vec()
{
  showLastVecGui_.reset();
  bool slv = showLastVecGui_.get();
  if (!num_dipoles_.get()) return;

  if (slv)
  {
    widget_[num_dipoles_.get() - 1]->SetCurrentMode(1);
    widget_[num_dipoles_.get() - 1]->SetMaterial(0, deflMatl_);
  }
  else
  {
    widget_[num_dipoles_.get() - 1]->SetCurrentMode(2);
    widget_[num_dipoles_.get() - 1]->SetMaterial(0, greenMatl_);
  }

  GeometryOPortHandle ogeom;
  get_oport_handle("Geometry",ogeom);
  ogeom->flushViews();
}


void
ShowAndEditDipoles::scale_mode_changed()
{
  scaleModeGui_.reset();
  std::string scaleMode = scaleModeGui_.get();
  if (scaleMode == last_scale_mode_) return;
  widgetSizeGui_.reset();
  max_len_.reset();
  double max = max_len_.get();
  for (int i = 0; i < num_dipoles_.get(); i++)
  {
    double sc = widgetSizeGui_.get();

    if (scaleMode == "normalize")
    {
      if (last_scale_mode_ == "scale")
      {
      sc *= (widget_[i]->GetScale() / last_scale_) / max;
      }
      else
      {
        sc *= widget_[i]->GetScale() / max;
      }
    }
    else if (scaleMode == "scale")
    {
      if (last_scale_mode_ == "normalize")
      {
        sc *= (widget_[i]->GetScale() * max) / last_scale_;
      }
      else
      {
        sc *= widget_[i]->GetScale();
      }
    }
    new_magnitudes_[i]->set(sc);
    widget_[i]->SetScale(sc);
    widget_[i]->SetLength(2*sc);
  }
  last_scale_ = widgetSizeGui_.get();
  last_scale_mode_ = scaleMode;

  GeometryOPortHandle ogeom;
  get_oport_handle("Geometry",ogeom);
  ogeom->flushViews();
}

void
ShowAndEditDipoles::scale_changed()
{
  scaleModeGui_.reset();
  std::string scaleMode = scaleModeGui_.get();
  widgetSizeGui_.reset();
  max_len_.reset();

  for (int i = 0; i < num_dipoles_.get(); i++)
  {
    double sc = widgetSizeGui_.get();
    if (scaleMode != "fixed")
    {
      sc *= widget_[i]->GetScale() / last_scale_;
    }
    new_magnitudes_[i]->set(sc);
    widget_[i]->SetScale(sc);
    widget_[i]->SetLength(2*sc);
  }
  last_scale_ = widgetSizeGui_.get();

  GeometryOPortHandle ogeom;
  get_oport_handle("Geometry",ogeom);
  ogeom->flushViews();
}


bool
ShowAndEditDipoles::generate_output_field()
{
  if (output_dirty_)
  {
    output_dirty_ = false;
    FieldInformation fi("PointCloudMesh",0,"Vector");
    dipoleFldH_ = CreateField(fi);

    VMesh *msh = dipoleFldH_->vmesh();
    VField *out =  dipoleFldH_->vfield();

    for (int i = 0; i < num_dipoles_.get(); i++)
    {
      msh->add_node(new_positions_[i]->get());
    }

    out->resize_values();

    scaleModeGui_.reset();
    std::string scaleMode = scaleModeGui_.get();
    double max = max_len_.get();
    for (VField::index_type i = 0; i < static_cast<VField::size_type>(num_dipoles_.get()); i++)
    {
      Vector d = new_directions_[i]->get();
      double sc = 1.0f;

      if (scaleMode == "normalize")
      {
        sc = widgetSizeGui_.get() / max;
      }
      else if (scaleMode == "scale")
      {
        sc = widgetSizeGui_.get();
      }

      d *= widget_[i]->GetScale() / sc;
      out->set_value(d,i);
    }
    return true;
  }
  return false;
}


void
ShowAndEditDipoles::widget_moved(bool release, BaseWidget*)
{
  if (release)
  {
    output_dirty_ = true;
    scaleModeGui_.reset();
    std::string scaleMode = scaleModeGui_.get();
    double max = max_len_.get();

    // dont know which widget moved so update all of them
    for (int i = 0; i < num_dipoles_.get(); i++)
    {
      new_positions_[i]->set(widget_[i]->GetPosition());
      new_directions_[i]->set(widget_[i]->GetDirection());

      double sc = 1.0f;

      if (scaleMode == "normalize")
      {
        // need to renormalize...
        double old = widget_[i]->GetScale();
        // undo the old normalize
      //	double vec = (old * old_max) / widgetSizeGui_.get();
        double vec = (old * max) / widgetSizeGui_.get();
        sc = widgetSizeGui_.get() * (vec / max);
        widget_[i]->SetScale(sc);
        new_magnitudes_[i]->set(sc);
      }
      else
      {
        new_magnitudes_[i]->set(widget_[i]->GetScale());
      }

      new_positions_[i]->reset();
      new_directions_[i]->reset();
      new_magnitudes_[i]->reset();
    }
    last_scale_ = widgetSizeGui_.get();
    draw_lines();


    GeometryOPortHandle ogeom;
    get_oport_handle("Geometry",ogeom);
    ogeom->flushViews();
  }
  if (release) want_to_execute();
}

void
ShowAndEditDipoles::draw_lines()
{
  GeometryOPortHandle ogeom;
  get_oport_handle("Geometry",ogeom);

  showLinesGui_.reset();
  if (gidx_)
  {
    ogeom->delObj(gidx_);
    gidx_=0;
  }
  if (showLinesGui_.get() && new_positions_.size())
  {
    GeomLines *g = new GeomLines;
    for (unsigned i = 0; i < new_positions_.size() - 1; i++)
      for (unsigned j = i+1; j < new_positions_.size(); j++)
        g->add(new_positions_[i]->get(), new_positions_[j]->get());

    GeomMaterial *gm = new GeomMaterial(g, new Material(Color(.8,.8,.2)));
    gidx_ = ogeom->addObj(gm, std::string("ShowDipole Lines"));
  }
  ogeom->flushViews();
}

void
ShowAndEditDipoles::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("ShowAndEditDipoles needs a minor command");
    return;
  }

  if (args[1] == "widget_scale")
  {
    scale_changed();
  }
  else if (args[1] == "scale_mode")
  {
    scale_mode_changed();
  }
  else if (args[1] == "show_last_vec")
  {
    last_as_vec();
  }
  else if (args[1] == "show_lines")
  {
    draw_lines();
  }
  else if (args[1] == "reset")
  {
    reset_ = true;
    want_to_execute();
  }
  else
  {
    Module::tcl_command(args, userdata);
  }
}


} // End namespace BioPSE
#endif
