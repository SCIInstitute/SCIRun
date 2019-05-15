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

#include <Modules/Legacy/Visualization/ShowAndEditDipoles.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Graphics/Widgets/SphereWidget.h>
#include <Graphics/Widgets/ConeWidget.h>
#include <Graphics/Widgets/CylinderWidget.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace Core;
using namespace Logging;
using namespace Datatypes;
using namespace Algorithms;
using namespace Geometry;
using namespace Modules::Visualization;
using namespace Dataflow::Networks;
using namespace Graphics::Datatypes;

MODULE_INFO_DEF(ShowAndEditDipoles, Visualization, SCIRun)

/*
namespace SCIRun
{
  namespace Modules
  {
    namespace Visualization
    {
      class ShowAndEditDipolesImpl
      {
      public:
        ShowAndEditDipolesImpl() :
          widgetid_(0), l2norm_(0), color_changed_(false) {}
        BBox last_bounds_;
        Point widgetLocation_;
        int widgetid_;
        double l2norm_;
        bool color_changed_;
        GeometryHandle buildWidgetObject(FieldHandle field, ModuleStateHandle state, const GeometryIDGenerator& idGenerator);
        RenderState getWidgetRenderState(ModuleStateHandle state);
        Transform previousTransform_;
      };
    }}}

ShowAndEditDipoles::ShowAndEditDipoles():
  GeometryGeneratingModule(staticInfo_), impl_(new ShowAndEditDipolesImpl)
{
  INITIALIZE_PORT(DipoleInputField);
  INITIALIZE_PORT(DipoleOutputField);
  INITIALIZE_PORT(DipoleWidget);
}

void ShowAndEditDipoles::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Sizing, 0);
  state->setValue(ShowLastAsVector, false);
  state->setValue(ShowLines, false);
  state->setValue(XLocation, 0.0);
  state->setValue(YLocation, 0.0);
  state->setValue(ZLocation, 0.0);
  state->setValue(MoveMethod, std::string("Location"));
  state->setValue(FieldNode, 0);
  state->setValue(FieldElem, 0);
  state->setValue(ProbeSize, 1.0);
  state->setValue(ProbeColor, ColorRGB(1, 1, 1).toString());
  state->setValue(SnapToNode, false);
  state->setValue(SnapToElement, false);

  getOutputPort(DipoleWidget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void ShowAndEditDipoles::execute()
{
  auto field = getRequiredInput(DipoleInputField);
  if(needToExecute())
  {
    FieldInformation finfo(field);
    if(finfo.is_pointcloudmesh())
      {
        auto out_field = GenerateOutputField(field);
        sendOutput(DipoleOutputField, out_field);

        auto geom = impl_->buildWidgetObject(field, get_state(), *this);
        sendOutput(DipoleWidget, geom);
      }
    else
      {
        error("Input field was not a valid point cloud.");
      }
  }
}

GeometryHandle ShowAndEditDipolesImpl::buildWidgetObject(FieldHandle field, ModuleStateHandle state, const GeometryIDGenerator& idGenerator)
{
  double radius = state->getValue(ShowAndEditDipoles::ProbeSize).toDouble();
  auto mesh = field->vmesh();
  mesh->synchronize(Mesh::NODES_E);

  // todo: quicker way to get a single point
  VMesh::Node::iterator eiter;
  mesh->begin(eiter);
  Point point;
  mesh->get_point(point, *eiter);

  return WidgetFactory::createSphere(idGenerator,
                                     "SAED",
                                     radius,
                                     state->getValue(ShowAndEditDipoles::ProbeColor).toString(),
                                     point,
                                     mesh->get_bounding_box());
}

void ShowAndEditDipoles::processWidgetFeedback(const ModuleFeedback& var)
{
  try
    {
      auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
      if (vsf.selectionName.find(get_id()) != std::string::npos &&
          impl_->previousTransform_ != vsf.transform)
        {
          adjustPositionFromTransform(vsf.transform);
          enqueueExecuteAgain(false);
        }
    }
  catch (std::bad_cast&)
    {
      //ignore
    }
}

void ShowAndEditDipoles::adjustPositionFromTransform(const Transform& transformMatrix)
{
  DenseMatrix center(4, 1);
  auto currLoc = currentLocation();
  center << currLoc.x(), currLoc.y(), currLoc.z(), 1.0;
  DenseMatrix newTransform(DenseMatrix(transformMatrix) * center);

  Point newLocation(newTransform(0, 0) / newTransform(3, 0),
                    newTransform(1, 0) / newTransform(3, 0),
                    newTransform(2, 0) / newTransform(3, 0));

  auto state = get_state();

  state->setValue(XLocation, newLocation.x());
  state->setValue(YLocation, newLocation.y());
  state->setValue(ZLocation, newLocation.z());

  if (get_state()->getValue(MoveMethod).toString() == "Node" &&
      get_state()->getValue(SnapToNode).toBool())
  {
    setNearestNode(newLocation);
  }

  if (get_state()->getValue(MoveMethod).toString() == "Element" &&
      get_state()->getValue(SnapToElement).toBool())
  {
    setNearestElement(newLocation);
  }

  auto oldMoveMethod = state->getValue(MoveMethod).toString();
  state->setValue(MoveMethod, std::string("Location"));
  state->setValue(MoveMethod, oldMoveMethod);
  impl_->previousTransform_ = transformMatrix;
}

Point ShowAndEditDipoles::currentLocation() const
{
  auto state = cstate();
  return Point(state->getValue(XLocation).toDouble(), state->getValue(YLocation).toDouble(), state->getValue(ZLocation).toDouble());
}

void ShowAndEditDipoles::setNearestNode(const Point& location)
{
  auto fieldOpt = getOptionalInput(DipoleInputField);
  if (fieldOpt && *fieldOpt)
    {
      auto ifield = *fieldOpt;
      ifield->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);
      Point r;
      VMesh::Node::index_type idx;
      ifield->vmesh()->find_closest_node(r, idx, location);
      get_state()->setValue(FieldNode, static_cast<int>(idx));
    }
}

void ShowAndEditDipoles::setNearestElement(const Point& location)
{
  auto fieldOpt = getOptionalInput(DipoleInputField);
  if (fieldOpt && *fieldOpt)
    {
      auto ifield = *fieldOpt;
      ifield->vmesh()->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
      Point r;
      VMesh::Elem::index_type idx;
      ifield->vmesh()->find_closest_elem(r, idx, location);
      get_state()->setValue(FieldElem, static_cast<int>(idx));
    }
}
FieldHandle ShowAndEditDipoles::GenerateOutputField(boost::optional<FieldHandle> ifieldOption)
{
  FieldHandle ifield;
  const double THRESHOLD = 1e-6;
  auto state = get_state();

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
    auto bmin = bbox.get_min();
    auto bmax = bbox.get_max();

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

    auto center = bmin + Vector(bmax - bmin) * 0.5;
    impl_->l2norm_ = (bmax - bmin).length();

    // If the current location looks reasonable, use that instead
    // of the center.
    auto curloc = currentLocation();

    // Invalidate current position if it's outside of our field.
    // Leave it alone if there was no field, as our bbox is arbitrary anyway.
    if (!ifieldOption ||
      (curloc.x() >= bmin.x() && curloc.x() <= bmax.x() &&
      curloc.y() >= bmin.y() && curloc.y() <= bmax.y() &&
      curloc.z() >= bmin.z() && curloc.z() <= bmax.z()))
    {
      center = curloc;
    }

    impl_->widgetLocation_ = center;

    impl_->last_bounds_ = bbox;
  }

  const auto moveto = state->getValue(MoveMethod).toString();
  bool moved_p = false;

  if (moveto == "Location")
  {
    const auto newloc = currentLocation();
    impl_->widgetLocation_ = newloc;
    moved_p = true;
  }
  else if (moveto == "Center")
  {
    auto bmin = bbox.get_min();
    auto bmax = bbox.get_max();

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

    auto center = bmin + Vector(bmax - bmin) * 0.5;

    impl_->widgetLocation_ = center;
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
        impl_->widgetLocation_ = p;
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
        impl_->widgetLocation_ = p;
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

  const auto location = impl_->widgetLocation_;

  FieldInformation fi("PointCloudMesh", 0, "double");
  auto mesh = CreateMesh(fi);
  mesh->vmesh()->add_point(location);

  FieldHandle ofield;

  if (ifieldOption)
  {
    setNearestNode(location);
    setNearestElement(location);
  }


  std::ostringstream valstr;
  VField* vfield = nullptr;
  VMesh* vmesh = nullptr;
  if (ifield)
  {
    vfield = ifield->vfield();
    vmesh = ifield->vmesh();
  }

  if (!ifieldOption || ifield->basis_order() == -1)
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

    fi.make_tensor();
    ofield = CreateField(fi, mesh);
    ofield->vfield()->set_value(result, VMesh::index_type(0));
  }

  state->setValue(XLocation, location.x());
  state->setValue(YLocation, location.y());
  state->setValue(ZLocation, location.z());
  state->setValue(FieldValue, valstr.str());

  return ofield;
}
*/

namespace SCIRun
{
  namespace Modules
  {
    namespace Visualization
    {
      class ShowAndEditDipolesImpl
      {
      public:
        BBox last_bounds_;
        std::vector<CylinderWidgetHandle> pointWidgets_;
        std::vector<Transform> previousTransforms_;
        double l2norm_;

        FieldHandle makePointCloud()
        {
          FieldInformation fi("PointCloudMesh", 1, "double");
          auto ofield = CreateField(fi);
          auto mesh = ofield->vmesh();
          auto field = ofield->vfield();

          for (int i = 0; i < pointWidgets_.size(); i++)
          {
            const Point location = pointWidgets_[i]->position();

            VMesh::Node::index_type pcindex = mesh->add_point(location);
            field->resize_fdata();
            field->set_value(static_cast<double>(i), pcindex);
          }
          return ofield;
        }
      };
    }
  }
}


ShowAndEditDipoles::ShowAndEditDipoles()
  : GeometryGeneratingModule(staticInfo_), impl_(new ShowAndEditDipolesImpl)
{
  INITIALIZE_PORT(DipoleInputField);
  INITIALIZE_PORT(DipoleOutputField);
  INITIALIZE_PORT(DipoleWidget);
}

void ShowAndEditDipoles::setStateDefaults()
{
  auto state = get_state();
  state->setValue(NumSeeds, 1);
  state->setValue(ProbeScale, 0.23);
  state->setValue(PointPositions, VariableList());
  getOutputPort(DipoleWidget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void ShowAndEditDipoles::execute()
{
  sendOutput(DipoleOutputField, GenerateOutputField());

  auto geom = WidgetFactory::createLinkedComposite(*this, "dipoles", impl_->pointWidgets_.begin(), impl_->pointWidgets_.end());
  sendOutput(DipoleWidget, geom);
}

void ShowAndEditDipoles::processWidgetFeedback(const ModuleFeedback& var)
{
  try
  {
    std::cout << "SAED1\n";
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    std::cout << "SAED2\n";
    if (vsf.selectionName.find(get_id()) != std::string::npos)
    {
      int widgetIndex = -1;
      try
      {
        static boost::regex r("CylinderWidget::SAED\\((.+)\\).+");
        boost::smatch what;
        std::cout << "SAED3\n";
        regex_match(vsf.selectionName, what, r);
        std::cout << "SAED4\n";
        widgetIndex = boost::lexical_cast<int>(what[1]);
        std::cout << "SAED5\n";
      }
      catch (...)
      {
        logWarning("Failure parsing widget id");
      }
      std::cout << "SAED6\n";
      if (impl_->previousTransforms_[widgetIndex] != vsf.transform)
      {
        std::cout << "SAED7\n";
        adjustPositionFromTransform(vsf.transform, widgetIndex);
        std::cout << "SAED8\n";
        enqueueExecuteAgain(false);
        std::cout << "SAED9\n";
      }
    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
}

void ShowAndEditDipoles::adjustPositionFromTransform(const Transform& transformMatrix, int index)
{
  DenseMatrix center(4, 1);

  auto currLoc = impl_->pointWidgets_[index]->position();
  center << currLoc.x(), currLoc.y(), currLoc.z(), 1.0;
  DenseMatrix newTransform(DenseMatrix(transformMatrix) * center);

  Point newLocation(newTransform(0, 0) / newTransform(3, 0),
    newTransform(1, 0) / newTransform(3, 0),
    newTransform(2, 0) / newTransform(3, 0));

  impl_->pointWidgets_[index]->setPosition(newLocation);
  impl_->previousTransforms_[index] = transformMatrix;
}

FieldHandle ShowAndEditDipoles::GenerateOutputField()
{
  auto ifieldhandle = getRequiredInput(DipoleInputField);

  auto bbox = ifieldhandle->vmesh()->get_bounding_box();

  Point center;
  Point bmin = bbox.get_min();
  Point bmax = bbox.get_max();

  if (!bbox.is_similar_to(impl_->last_bounds_))
  {
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

    center = bmin + Vector(bmax - bmin) * 0.5;
    impl_->l2norm_ = (bmax - bmin).length();
    impl_->last_bounds_ = bbox;
  }

  auto state = get_state();
  auto numSeeds = 3;
  auto scale = state->getValue(ProbeScale).toDouble();
  auto widgetName = [](int i) { return "SAED(" + std::to_string(i) + ")"; };
  if (impl_->pointWidgets_.size() != numSeeds)
  {
    if (numSeeds < impl_->pointWidgets_.size())
    {
      impl_->pointWidgets_.resize(numSeeds);
    }
    else
    {
      auto positions = state->getValue(PointPositions).toVector();
      for (size_t i = impl_->pointWidgets_.size(); i < numSeeds; i++)
      {
        auto location = center + Vector(i, i, i);
        Point p2(1,1,1);
        p2 += location;
        if (i < positions.size())
          location = pointFromString(positions[i].toString());

        p2 += center;
        auto seed = boost::dynamic_pointer_cast<CylinderWidget>(WidgetFactory::createCylinder(
          *this,
          widgetName(i),
          scale,
          "Color(0.5,0.5,0.5)",
          location,
          p2,
          bbox));
        impl_->pointWidgets_.push_back(seed);
      }
    }
    impl_->previousTransforms_.resize(impl_->pointWidgets_.size());
  }
  else
  {
    std::vector<CylinderWidgetHandle> newWidgets;
    int counter = 0;
    moveCount_++;
    for (const auto& oldWidget : impl_->pointWidgets_)
    {
      Point p2(1,1,1);
      p2 += oldWidget->position();
      auto seed = boost::dynamic_pointer_cast<CylinderWidget>(WidgetFactory::createCylinder(
        *this,
        widgetName(counter++) + std::string(moveCount_, ' '),
        scale,
        "Color(0.5,0.5,0.5)",
        oldWidget->position(),
        p2,
        bbox));
      newWidgets.push_back(seed);
    }
    impl_->pointWidgets_ = newWidgets;
  }

  VariableList positions;
  for (const auto& widget : impl_->pointWidgets_)
  {
    positions.push_back(makeVariable("widget_i", widget->position().get_string()));
  }
  state->setValue(PointPositions, positions);

  return impl_->makePointCloud();
}




















const AlgorithmParameterName ShowAndEditDipoles::Sizing("Sizing");
const AlgorithmParameterName ShowAndEditDipoles::ShowLastAsVector("ShowLastAsVector");
const AlgorithmParameterName ShowAndEditDipoles::ShowLines("ShowLines");
//const AlgorithmParameterName ShowAndEditDipoles::XLocation("XLocation");
//const AlgorithmParameterName ShowAndEditDipoles::YLocation("YLocation");
//const AlgorithmParameterName ShowAndEditDipoles::ZLocation("ZLocation");
//const AlgorithmParameterName ShowAndEditDipoles::FieldValue("FieldValue");
//const AlgorithmParameterName ShowAndEditDipoles::MoveMethod("MoveMethod");
//const AlgorithmParameterName ShowAndEditDipoles::SnapToElement("SnapToElement");
//const AlgorithmParameterName ShowAndEditDipoles::SnapToNode("SnapToNode");
//const AlgorithmParameterName ShowAndEditDipoles::FieldNode("FieldNode");
//const AlgorithmParameterName ShowAndEditDipoles::FieldElem("FieldElem");
//const AlgorithmParameterName ShowAndEditDipoles::ProbeSize("ProbeSize");
//const AlgorithmParameterName ShowAndEditDipoles::ProbeColor("ProbeColor");
const AlgorithmParameterName ShowAndEditDipoles::NumSeeds("NumSeeds");
const AlgorithmParameterName ShowAndEditDipoles::ProbeScale("ProbeScale");
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
    std::vector<GeomHandle>       widget_switch_;
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
  if (reset_ || (gen != lastGen_))
  {
    lastGen_ = gen;
    if (reset_ || (field->num_values() != static_cast<VMesh::size_type>(num_dipoles_.get())))
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
