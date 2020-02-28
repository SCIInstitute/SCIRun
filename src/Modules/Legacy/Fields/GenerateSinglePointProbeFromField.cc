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


#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Graphics/Widgets/WidgetBuilders.h>
#include <Modules/Legacy/Fields/GenerateSinglePointProbeFromField.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Scalar.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace Core;
using namespace Algorithms;
using namespace Datatypes;
using namespace Dataflow::Networks;
using namespace Modules::Fields;
using namespace Geometry;
using namespace Fields;
using namespace Graphics::Datatypes;

MODULE_INFO_DEF(GenerateSinglePointProbeFromField, NewField, SCIRun)

ALGORITHM_PARAMETER_DEF(Fields, XLocation);
ALGORITHM_PARAMETER_DEF(Fields, YLocation);
ALGORITHM_PARAMETER_DEF(Fields, ZLocation);
ALGORITHM_PARAMETER_DEF(Fields, MoveMethod);
ALGORITHM_PARAMETER_DEF(Fields, FieldValue);
ALGORITHM_PARAMETER_DEF(Fields, FieldNode);
ALGORITHM_PARAMETER_DEF(Fields, FieldElem);
ALGORITHM_PARAMETER_DEF(Fields, ProbeSize);
ALGORITHM_PARAMETER_DEF(Fields, ProbeLabel);
ALGORITHM_PARAMETER_DEF(Fields, ProbeColor);
ALGORITHM_PARAMETER_DEF(Fields, SnapToNode);
ALGORITHM_PARAMETER_DEF(Fields, SnapToElement);

namespace SCIRun
{
  namespace Modules
  {
    namespace Fields
    {
      class GenerateSinglePointProbeFromFieldImpl
      {
      public:
        GenerateSinglePointProbeFromFieldImpl() :
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

GenerateSinglePointProbeFromField::GenerateSinglePointProbeFromField()
  : GeometryGeneratingModule(staticInfo_), impl_(new GenerateSinglePointProbeFromFieldImpl)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(GeneratedWidget);
  INITIALIZE_PORT(GeneratedPoint);
  INITIALIZE_PORT(ElementIndex);
}

void GenerateSinglePointProbeFromField::processWidgetFeedback(const ModuleFeedback& var)
{
  try
  {
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    if (vsf.matchesWithModuleId(id()) && impl_->previousTransform_ != vsf.transform)
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

void GenerateSinglePointProbeFromField::adjustPositionFromTransform(const Transform& transformMatrix)
{
  using namespace Parameters;

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

void GenerateSinglePointProbeFromField::setStateDefaults()
{
  auto state = get_state();
  using namespace Parameters;
  state->setValue(XLocation, 0.0);
  state->setValue(YLocation, 0.0);
  state->setValue(ZLocation, 0.0);
  state->setValue(MoveMethod, std::string("Location"));
  state->setValue(FieldValue, std::string());
  state->setValue(FieldNode, 0);
  state->setValue(FieldElem, 0);
  state->setValue(ProbeSize, 1.0);
  state->setValue(ProbeLabel, std::string());
  state->setValue(ProbeColor, ColorRGB(1, 1, 1).toString());
  state->setValue(SnapToNode, false);
  state->setValue(SnapToElement, false);

  getOutputPort(GeneratedWidget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

Point GenerateSinglePointProbeFromField::currentLocation() const
{
  auto state = cstate();
  using namespace Parameters;
  return Point(state->getValue(XLocation).toDouble(), state->getValue(YLocation).toDouble(), state->getValue(ZLocation).toDouble());
}

void GenerateSinglePointProbeFromField::execute()
{
  auto ifieldOption = getOptionalInput(InputField);
  if (needToExecute())
  {
    auto field = GenerateOutputField(ifieldOption);
    sendOutput(GeneratedPoint, field);

    auto index = GenerateIndex();
    sendOutput(ElementIndex, boost::make_shared<Int32>(static_cast<int>(index)));

    auto geom = impl_->buildWidgetObject(field, get_state(), *this);
    sendOutput(GeneratedWidget, geom);
  }
}

FieldHandle GenerateSinglePointProbeFromField::GenerateOutputField(boost::optional<FieldHandle> ifieldOption)
{
  FieldHandle ifield;
  const double THRESHOLD = 1e-6;
  auto state = get_state();
  using namespace Parameters;

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

void GenerateSinglePointProbeFromField::setNearestNode(const Point& location)
{
  auto fieldOpt = getOptionalInput(InputField);
  if (fieldOpt && *fieldOpt)
  {
    auto ifield = *fieldOpt;
    ifield->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);
    Point r;
    VMesh::Node::index_type idx;
    ifield->vmesh()->find_closest_node(r, idx, location);
    get_state()->setValue(Parameters::FieldNode, static_cast<int>(idx));
  }
}

void GenerateSinglePointProbeFromField::setNearestElement(const Point& location)
{
  auto fieldOpt = getOptionalInput(InputField);
  if (fieldOpt && *fieldOpt)
  {
    auto ifield = *fieldOpt;
    ifield->vmesh()->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
    Point r;
    VMesh::Elem::index_type idx;
    ifield->vmesh()->find_closest_elem(r, idx, location);
    get_state()->setValue(Parameters::FieldElem, static_cast<int>(idx));
  }
}

index_type GenerateSinglePointProbeFromField::GenerateIndex()
{
  auto ifieldOption = getOptionalInput(InputField);
  index_type index = 0;

  auto state = get_state();
  using namespace Parameters;
  if (ifieldOption && *ifieldOption)
  {
    if (state->getValue(MoveMethod).toString() == "Node")
    {
      index = state->getValue(FieldNode).toInt();
    }
    else if (state->getValue(MoveMethod).toString() == "Element")
    {
      index = state->getValue(FieldElem).toInt();
    }
  }

  return index;
}

GeometryHandle GenerateSinglePointProbeFromFieldImpl::buildWidgetObject(FieldHandle field, ModuleStateHandle state, const GeometryIDGenerator& idGenerator)
{
  using namespace Parameters;
  double radius = state->getValue(ProbeSize).toDouble();
  auto mesh = field->vmesh();
  mesh->synchronize(Mesh::NODES_E);

  // todo: quicker way to get a single point
  VMesh::Node::iterator eiter;
  mesh->begin(eiter);
  Point point;
  mesh->get_point(point, *eiter);

  return SphereWidgetBuilder(idGenerator)
    .tag("GSPPFF")
    .scale(radius)
    .defaultColor(state->getValue(Parameters::ProbeColor).toString())
    .origin(point)
    .boundingBox(mesh->get_bounding_box())
    .resolution(10)
    .centerPoint(point)
    .build();
}
