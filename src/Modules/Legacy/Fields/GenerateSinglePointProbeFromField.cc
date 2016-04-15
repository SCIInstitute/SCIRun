/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Modules/Legacy/Fields/GenerateSinglePointProbeFromField.h>
#include <Core/Datatypes/Color.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Graphics/Glyphs/GlyphGeom.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Scalar.h>
#include <Core/Datatypes/DenseMatrix.h>

//
//#include <Core/Thread/CrowdMonitor.h>
//
//#include <Dataflow/Widgets/PointWidget.h>

using namespace SCIRun;
using namespace Core;
using namespace Core::Algorithms;
using namespace Datatypes;
using namespace Dataflow::Networks;
using namespace Modules::Fields;
using namespace Geometry;
using namespace Algorithms::Fields;
using namespace Graphics::Datatypes;

const ModuleLookupInfo GenerateSinglePointProbeFromField::staticInfo_("GenerateSinglePointProbeFromField", "NewField", "SCIRun");
ALGORITHM_PARAMETER_DEF(Fields, XLocation);
ALGORITHM_PARAMETER_DEF(Fields, YLocation);
ALGORITHM_PARAMETER_DEF(Fields, ZLocation);
ALGORITHM_PARAMETER_DEF(Fields, MoveMethod);
ALGORITHM_PARAMETER_DEF(Fields, DisplayValue);
ALGORITHM_PARAMETER_DEF(Fields, DisplayNode);
ALGORITHM_PARAMETER_DEF(Fields, DisplayElem);
ALGORITHM_PARAMETER_DEF(Fields, FieldValue);
ALGORITHM_PARAMETER_DEF(Fields, FieldNode);
ALGORITHM_PARAMETER_DEF(Fields, FieldElem);
ALGORITHM_PARAMETER_DEF(Fields, ProbeSize);
ALGORITHM_PARAMETER_DEF(Fields, ProbeLabel);
ALGORITHM_PARAMETER_DEF(Fields, ProbeColor);
ALGORITHM_PARAMETER_DEF(Fields, WidgetMoved);

namespace SCIRun
{
  namespace Modules
  {
    namespace Fields
    {
      PointWidgetStub::PointWidgetStub() : scale_(1) {}
      Point PointWidgetStub::position() const { return pos_; }
      void PointWidgetStub::setPosition(const Point& p) { pos_ = p; }

      class GenerateSinglePointProbeFromFieldImpl
      {
      public:
        GenerateSinglePointProbeFromFieldImpl() :
          widget_(new PointWidgetStub),
          widgetid_(0), l2norm_(0), color_changed_(false) {}
        PointWidgetPtr widget_;
        //CrowdMonitor widget_lock_;
        //int  last_input_generation_;
        BBox last_bounds_;
        int widgetid_;
        double l2norm_;
        bool color_changed_;
        GeometryHandle buildWidgetObject(FieldHandle field, ModuleStateHandle state, const GeometryIDGenerator& idGenerator);
        RenderState getWidgetRenderState(ModuleStateHandle state);
      };
    }}}

GenerateSinglePointProbeFromField::GenerateSinglePointProbeFromField()
  : GeometryGeneratingModule(staticInfo_), impl_(new GenerateSinglePointProbeFromFieldImpl)
{
  counter_ = -1;
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(GeneratedWidget);
  INITIALIZE_PORT(GeneratedPoint);
  INITIALIZE_PORT(ElementIndex);
}

void GenerateSinglePointProbeFromField::processWidgetFeedback(const ModuleFeedback& var)
{
  auto xyTr = any_cast_or_default_<Variable>(var);
  DenseMatrixHandle transformHandle(new DenseMatrix(4, 4));
  int row = 0; 
  int col = 0;
  int i = 0;
  int counter;
  for (const auto& subVar : xyTr.toVector())
  {
    if (i == 0)
    {
      counter = subVar.toInt();
      if (counter_ != counter)
        counter_ = counter;
      else
        return;
    }
    else
    {
      if (col > 3)
      {
        col = 0;
        ++row;
      }
      (*transformHandle)(row, col) = subVar.toDouble();
      ++col;
    }
    ++i;
  }
  
  //std::cout << "in probe: " << (*transformHandle) << std::endl;
  adjustPositionFromTransform(transformHandle);
}


void GenerateSinglePointProbeFromField::adjustPositionFromTransform(const DenseMatrixHandle& transformMatrix)
{
  //std::cout << "GenerateSinglePointProbeFromField::adjustPositionFromTransform\n";
  DenseMatrixHandle centerHandle(new DenseMatrix(4, 1));
  (*centerHandle) << currentLocation().x(), currentLocation().y(), currentLocation().z(), 1;
  //(*centerHandle) << 0, 0, 0, 1;
  DenseMatrix newTransform((*transformMatrix) * (*centerHandle));

  Point newLocation(newTransform.get(0, 0) / newTransform.get(3, 0),
                    newTransform.get(1, 0) / newTransform.get(3, 0),
                    newTransform.get(2, 0) / newTransform.get(3, 0));

  auto state = get_state();
  using namespace Parameters;
  state->setValue(XLocation, newLocation.x());
  state->setValue(YLocation, newLocation.y());
  state->setValue(ZLocation, newLocation.z());
  std::string oldMoveMethod = state->getValue(MoveMethod).toString();
  state->setValue(MoveMethod, std::string("Location"));
  //TODO: Communicate with dialog to Q_EMIT executeActionTriggered();
  state->setValue(WidgetMoved, true);
  state->setValue(MoveMethod, std::string(oldMoveMethod));
 
}

void GenerateSinglePointProbeFromField::setStateDefaults()
{
  auto state = get_state();
  using namespace Parameters;
  state->setValue(XLocation, 0.0);
  state->setValue(YLocation, 0.0);
  state->setValue(ZLocation, 0.0);
  state->setValue(MoveMethod, std::string("Location"));
  state->setValue(DisplayValue, true);
  state->setValue(DisplayNode, true);
  state->setValue(DisplayElem, true);
  state->setValue(FieldValue, std::string());
  state->setValue(FieldNode, 0);
  state->setValue(FieldElem, 0);
  state->setValue(ProbeSize, 1.0);
  state->setValue(ProbeLabel, std::string());
  state->setValue(ProbeColor, ColorRGB(1, 1, 1).toString());
  state->setValue(WidgetMoved, false);

  getOutputPort(GeneratedWidget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

#if 0

GenerateSinglePointProbeFromField::GenerateSinglePointProbeFromField(GuiContext* ctx)
  : Module("GenerateSinglePointProbeFromField", ctx, Filter, "NewField", "SCIRun"),
    widget_lock_("GenerateSinglePointProbeFromField widget lock"),
    last_input_generation_(0),
    widgetid_(0),
    color_changed_(false)
{
  widget_ = new PointWidget(this, &widget_lock_, 1.0);
  GeometryOPortHandle ogport;
  get_oport_handle("GenerateSinglePointProbeFromField Widget",ogport);
  widget_->Connect(ogport.get_rep());
}


GenerateSinglePointProbeFromField::~GenerateSinglePointProbeFromField()
{
  delete widget_;
}
#endif

Point GenerateSinglePointProbeFromField::currentLocation() const
{
  auto state = get_state();
  using namespace Parameters;
  return Point(state->getValue(XLocation).toDouble(), state->getValue(YLocation).toDouble(), state->getValue(ZLocation).toDouble());
}

void GenerateSinglePointProbeFromField::execute()
{
  auto ifieldOption = getOptionalInput(InputField);
  if (needToExecute())
  {
    FieldHandle field = GenerateOutputField(ifieldOption);
    sendOutput(GeneratedPoint, field);

    index_type index = GenerateIndex();
    sendOutput(ElementIndex, boost::make_shared<Int32>(static_cast<int>(index)));

    auto geom = impl_->buildWidgetObject(field, get_state(), *this);
    sendOutput(GeneratedWidget, geom);
  }
}

FieldHandle GenerateSinglePointProbeFromField::GenerateOutputField(boost::optional<FieldHandle> ifieldOption)
{
  FieldHandle ifield;

  update_state(Executing);

  const double THRESHOLD = 1e-6;
  auto state = get_state();
  using namespace Parameters;

  //std::cout << "Size: " << state->getValue(ProbeSize).toInt() << std::endl;
  //std::cout << "executing" << std::endl;

  // Maybe update the widget.
  BBox bbox;
  if (ifieldOption && *ifieldOption)
  {
    ifield = *ifieldOption;
    bbox = ifield->vmesh()->get_bounding_box();
  }
  else
  {
    bbox.extend(Point(-1.0, -1.0, -1.0));
    bbox.extend(Point(1.0, 1.0, 1.0));
  }

  if (!bbox.is_similar_to(impl_->last_bounds_))
  {
    Point bmin = bbox.get_min();
    Point bmax = bbox.get_max();

    // Fix degenerate boxes.
    const double size_estimate = std::max((bmax - bmin).length() * 0.01, 1.0e-5);
    if (fabs(bmax.x() - bmin.x()) < THRESHOLD)
    {
      bmin.x(bmin.x() - size_estimate);
      bmax.x(bmax.x() + size_estimate);
    }
    if (fabs(bmax.y() - bmin.y()) < THRESHOLD)
    {
      bmin.y(bmin.y() - size_estimate);
      bmax.y(bmax.y() + size_estimate);
    }
    if (fabs(bmax.z() - bmin.z()) < THRESHOLD)
    {
      bmin.z(bmin.z() - size_estimate);
      bmax.z(bmax.z() + size_estimate);
    }

    Point center = bmin + Vector(bmax - bmin) * 0.5;
    impl_->l2norm_ = (bmax - bmin).length();

    // If the current location looks reasonable, use that instead
    // of the center.
    Point curloc = currentLocation();

    // Invalidate current position if it's outside of our field.
    // Leave it alone if there was no field, as our bbox is arbitrary anyway.
    if (!ifieldOption ||
      (curloc.x() >= bmin.x() && curloc.x() <= bmax.x() &&
      curloc.y() >= bmin.y() && curloc.y() <= bmax.y() &&
      curloc.z() >= bmin.z() && curloc.z() <= bmax.z()))
    {
      center = curloc;
    }

    impl_->widget_->setPosition(center);

#if SCIRUN4_TO_BE_ENABLED_LATER
    GeomGroup *widget_group = new GeomGroup;
    widget_group->add(widget_->GetWidget());

    GeometryOPortHandle ogport;
    get_oport_handle("GenerateSinglePointProbeFromField Widget", ogport);
    widgetid_ = ogport->addObj(widget_group, "GenerateSinglePointProbeFromField Selection Widget",
      &widget_lock_);
    ogport->flushViews();
#endif
    impl_->last_bounds_ = bbox;
  }

#if SCIRUN4_TO_BE_ENABLED_LATER
  widget_->SetScale(gui_probe_scale_.get() * l2norm_ * 0.003);
  widget_->SetColor(Color(gui_color_r_.get(), gui_color_g_.get(), gui_color_b_.get()));
  widget_->SetLabel(gui_label_.get());
#endif

  const std::string moveto = state->getValue(MoveMethod).toString();
  bool moved_p = false;
  if (moveto == "Location")
  {
    const Point newloc = currentLocation();
    impl_->widget_->setPosition(newloc);
    moved_p = true;
  }
  else if (moveto == "Center")
  {
    Point bmin = bbox.get_min();
    Point bmax = bbox.get_max();

    // Fix degenerate boxes.
    const double size_estimate = std::max((bmax - bmin).length() * 0.01, 1.0e-5);
    if (fabs(bmax.x() - bmin.x()) < THRESHOLD)
    {
      bmin.x(bmin.x() - size_estimate);
      bmax.x(bmax.x() + size_estimate);
    }
    if (fabs(bmax.y() - bmin.y()) < THRESHOLD)
    {
      bmin.y(bmin.y() - size_estimate);
      bmax.y(bmax.y() + size_estimate);
    }
    if (fabs(bmax.z() - bmin.z()) < THRESHOLD)
    {
      bmin.z(bmin.z() - size_estimate);
      bmax.z(bmax.z() + size_estimate);
    }

    Point center = bmin + Vector(bmax - bmin) * 0.5;

#if SCIRUN4_TO_BE_ENABLED_LATER
    widget_->SetColor(Color(gui_color_r_.get(), gui_color_g_.get(), gui_color_b_.get()));
    widget_->SetLabel(gui_label_.get());
#endif

    impl_->widget_->setPosition(center);
    moved_p = true;
  }
  else if (!moveto.empty() && ifieldOption)
  {
    if (moveto == "Node")
    {
      VMesh::index_type idx = state->getValue(FieldNode).toInt();
      if (idx >= 0 && idx < ifield->vmesh()->num_nodes())
      {
        Point p;
        ifield->vmesh()->get_center(p, VMesh::Node::index_type(idx));
        impl_->widget_->setPosition(p);
        moved_p = true;
      }
    }
    else if (moveto == "Element")
    {
      VMesh::index_type idx = state->getValue(FieldElem).toInt();
      if (idx >= 0 && idx < ifield->vmesh()->num_elems())
      {
        Point p;
        ifield->vmesh()->get_center(p, VMesh::Elem::index_type(idx));
        impl_->widget_->setPosition(p);
        moved_p = true;
      }
    }
  }
  if (moved_p)
  {
#if SCIRUN4_TO_BE_ENABLED_LATER
    GeometryOPortHandle ogport;
    get_oport_handle("GenerateSinglePointProbeFromField Widget", ogport);
    ogport->flushViews();
    gui_moveto_.set("");
#endif
  }

  const Point location = impl_->widget_->position();

  FieldInformation fi("PointCloudMesh", 0, "double");
  MeshHandle mesh = CreateMesh(fi);
  mesh->vmesh()->add_point(location);

  FieldHandle ofield;

  if (ifieldOption)
  {
    if (state->getValue(DisplayNode).toBool())
    {
      ifield->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);
      Point r;
      VMesh::Node::index_type idx;
      ifield->vmesh()->find_closest_node(r, idx, location);
      state->setValue(FieldNode, static_cast<int>(idx));
    }

    if (state->getValue(DisplayElem).toBool())
    {
      ifield->vmesh()->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
      Point r;
      VMesh::Elem::index_type idx;
      ifield->vmesh()->find_closest_elem(r, idx, location);
      state->setValue(FieldElem, static_cast<int>(idx));
    }
  }

  std::ostringstream valstr;
  VField* vfield = 0;
  VMesh* vmesh = 0;
  if (ifield)
  {
    vfield = ifield->vfield();
    vmesh = ifield->vmesh();
  }

  if (!ifieldOption || ifield->basis_order() == -1 || !state->getValue(DisplayValue).toBool())
  {
    fi.make_double();
    ofield = CreateField(fi, mesh);
    ofield->vfield()->resize_values();
    valstr << 0;
    ofield->vfield()->set_value(0.0, VMesh::index_type(0));
  }
  else if (vfield->is_scalar())
  {
    double result = 0.0;
    if (!vfield->interpolate(result, location))
    {
      Point closest;
      VMesh::Node::index_type node_idx;
      if (vmesh->find_closest_node(closest, node_idx, location))
        vfield->get_value(result, node_idx);
    }
    valstr << result;

    fi.make_double();
    ofield = CreateField(fi, mesh);
    ofield->vfield()->set_value(result, VMesh::index_type(0));
  }
  else if (vfield->is_vector())
  {
    Vector result(0.0, 0.0, 0.0);
    if (!vfield->interpolate(result, location))
    {
      Point closest;
      VMesh::Node::index_type node_idx;
      if (vmesh->find_closest_node(closest, node_idx, location))
        vfield->get_value(result, node_idx);
    }
    valstr << result;

    fi.make_vector();
    ofield = CreateField(fi, mesh);
    ofield->vfield()->set_value(result, VMesh::index_type(0));
  }
  else if (vfield->is_tensor())
  {
    Tensor result(0.0);
    if (!vfield->interpolate(result, location))
    {
      Point closest;
      VMesh::Node::index_type node_idx;
      if (vmesh->find_closest_node(closest, node_idx, location))
        vfield->get_value(result, node_idx);
    }
#if SCIRUN4_TO_BE_ENABLED_LATER
    valstr << result;
#endif

    fi.make_tensor();
    ofield = CreateField(fi, mesh);
    ofield->vfield()->set_value(result, VMesh::index_type(0));
  }

  state->setValue(XLocation, location.x());
  state->setValue(YLocation, location.y());
  state->setValue(ZLocation, location.z());
  if (state->getValue(DisplayValue).toBool())
  {
    state->setValue(FieldValue, valstr.str());
  }

  return ofield;
}

index_type GenerateSinglePointProbeFromField::GenerateIndex()
{
  auto ifieldOption = getOptionalInput(InputField);
  index_type index = 0;

  auto state = get_state();
  using namespace Parameters;
  if (ifieldOption && *ifieldOption)
  {
    if (state->getValue(DisplayNode).toBool())
    {
      index = state->getValue(FieldNode).toInt();
    }
    else if (state->getValue(DisplayElem).toBool())
    {
      index = state->getValue(FieldElem).toInt();
    }
  }

  return index;
}

GeometryHandle GenerateSinglePointProbeFromFieldImpl::buildWidgetObject(FieldHandle field, ModuleStateHandle state, const GeometryIDGenerator& idGenerator)
{
  GeometryHandle geom(new GeometryObjectSpire(idGenerator, "EntireSinglePointProbeFromField", true));

  VMesh* mesh = field->vmesh();

  ColorScheme colorScheme = COLOR_UNIFORM;
  ColorRGB node_color;

  mesh->synchronize(Mesh::NODES_E);

  VMesh::Node::iterator eiter, eiter_end;
  mesh->begin(eiter);
  mesh->end(eiter_end);

  using namespace Parameters;
  double radius = state->getValue(ProbeSize).toDouble();
  double num_strips = 10;
  if (radius < 0) radius = 1.;
  if (num_strips < 0) num_strips = 10.;
  std::stringstream ss;
  ss << radius << num_strips << colorScheme;

  std::string uniqueNodeID = geom->uniqueID() + "widget" + ss.str();

  SpireIBO::PRIMITIVE primIn = SpireIBO::TRIANGLES;

  Graphics::GlyphGeom glyphs;
  while (eiter != eiter_end)
  {
    //checkForInterruption();

    Point p;
    mesh->get_point(p, *eiter);

    glyphs.addSphere(p, radius, num_strips, node_color);

    ++eiter;
  }

  RenderState renState = getWidgetRenderState(state);

  glyphs.buildObject(geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, primIn, mesh->get_bounding_box());

  return geom;
}

RenderState GenerateSinglePointProbeFromFieldImpl::getWidgetRenderState(ModuleStateHandle state)
{
  RenderState renState;

  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENCY, false);

  renState.defaultColor = ColorRGB(state->getValue(Parameters::ProbeColor).toString());
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
    renState.defaultColor.g() > 1.0 ||
    renState.defaultColor.b() > 1.0) ?
    ColorRGB(
    renState.defaultColor.r() / 255.,
    renState.defaultColor.g() / 255.,
    renState.defaultColor.b() / 255.)
    : renState.defaultColor;

  renState.set(RenderState::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_WIDGET, true);

  return renState;
}

#if 0

void
GenerateSinglePointProbeFromField::widget_moved(bool last, BaseWidget*)
{
  if (last)
  {
    want_to_execute();
  }
}



void
GenerateSinglePointProbeFromField::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("ShowString needs a minor command");
    return;
  }

  if (args[1] == "color_change")
  {
    color_changed_ = true;
  }
  else
  {
    Module::tcl_command(args, userdata);
  }
}
#endif
