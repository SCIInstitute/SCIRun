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

#include "Dataflow/Network/ModuleStateInterface.h"
#include "Graphics/Widgets/BoundingBoxWidget.h"
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/WidgetFactory.h>
#include <Modules/Fields/EditMeshBoundingBox.h>
#include <boost/format.hpp>

using namespace SCIRun;
using namespace Modules::Fields;
using namespace Core::Datatypes;
using namespace Dataflow::Networks;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Graphics;
using namespace Datatypes;

MODULE_INFO_DEF(EditMeshBoundingBox, ChangeMesh, SCIRun)

namespace SCIRun
{
  namespace Modules
  {
    namespace Fields
    {
      class EditMeshBoundingBoxImpl
      {
      public:
        Transform userWidgetTransform_;
        Transform field_initial_transform_;
        BBox box_initial_bounds_;
        BBox bbox_;
        BoxPosition position_;

        Core::Geometry::Point input_pos_;
        Core::Geometry::Vector input_eigvecs_[3];
        double input_eigvals_[3];
      };
    }
  }
}

EditMeshBoundingBox::EditMeshBoundingBox()
: GeometryGeneratingModule(staticInfo_),
impl_(new EditMeshBoundingBoxImpl), widgetMoved_(false)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(Transformation_Widget);
  INITIALIZE_PORT(Transformation_Matrix);
}

void EditMeshBoundingBox::processWidgetFeedback(const ModuleFeedback& var)
{
  try
  {
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    if (vsf.matchesWithModuleId(id()) && impl_->userWidgetTransform_ != vsf.transform)
    {
      widgetMoved_ = true;
      adjustGeometryFromTransform(vsf.transform);
      enqueueExecuteAgain(true);
    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
}

void EditMeshBoundingBox::adjustGeometryFromTransform(const Transform& transform)
{
  for(auto& eigvec : eigvecs_)
    eigvec = transform * eigvec;
  pos_ = transform * pos_;
  outputField_->vmesh()->transform(transform);
}

void EditMeshBoundingBox::setStateDefaults()
{
  // clear_vals();
  auto state = get_state();
  state->setValue(RefreshGeom, true);
  state->setValue(RestrictX, false);
  state->setValue(RestrictY, false);
  state->setValue(RestrictZ, false);
  state->setValue(RestrictR, false);
  state->setValue(RestrictD, false);
  state->setValue(RestrictI, false);
  state->setValue(OutputCenterX, 0.0);
  state->setValue(OutputCenterY, 0.0);
  state->setValue(OutputCenterZ, 0.0);
  state->setValue(OutputSizeX, 1.0);
  state->setValue(OutputSizeY, 1.0);
  state->setValue(OutputSizeZ, 1.0);
  state->setValue(Scale, 0.1);
  state->setValue(NoTranslation, true);
  state->setValue(XYZTranslation, false);
  state->setValue(RDITranslation, false);
  state->setValue(BoxRealScale, 0.0);
  state->setValue(BoxMode, 0);

  getOutputPort(Transformation_Widget)->connectConnectionFeedbackListener([this](const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void EditMeshBoundingBox::execute()
{
  auto field = getRequiredInput(InputField);

  if (needToExecute())
  {
    auto state = get_state();

    Transform trans;
    trans.load_identity();
    if(state->getValue(RefreshGeom).toBool())
      refreshGeometry(field, trans);

    applyScaling(trans);
    outputField_->vmesh()->transform(trans);

    MatrixHandle mh(new DenseMatrix(trans));
    sendOutput(Transformation_Matrix, mh);

    // state->setTransientValue(ScaleChanged, false);
    // state->setTransientValue(SetOutputCenter, false);
    // state->setTransientValue(ResetCenter, false);
    // state->setTransientValue(SetOutputSize, false);
    // state->setTransientValue(ResetSize, false);

    sendOutput(OutputField, outputField_);

    generateGeomsList();
    auto comp_geo = createGeomComposite(*this, "bbox", geoms_.begin(), geoms_.end());
    sendOutput(Transformation_Widget, comp_geo);
  }
}

void EditMeshBoundingBox::refreshGeometry(const FieldHandle field, Transform& trans)
{
  computeWidgetBox(field->vmesh()->get_bounding_box());
  outputField_ = field;
  widgetMoved_ = false;
  trans = Transform(pos_, eigvecs_[0]*eigvals_[0], eigvecs_[1]*eigvals_[1], eigvecs_[2]*eigvals_[2]);
}

void EditMeshBoundingBox::applyScaling(Transform& trans)
{
  auto state = get_state();

  if (state->getValue(ScaleChanged).toBool()) {
    double scale = (state->getValue(Scale).toDouble());
    for (auto &eigval : eigvals_)
      eigval *= scale;
    trans.pre_scale(scale);
  }

  // We have to undo previous scaling if there is no new geometry
  if (!state->getValue(RefreshGeom).toBool() &&
      state->getValue(ScaleChanged).toBool()) {
    double oldScale = state->getValue(OldScale).toDouble();
    for (auto &eigval : eigvals_)
      eigval /= oldScale;
    state->setValue(OldScale, state->getValue(Scale).toDouble());
    trans.pre_scale(oldScale);
  }
}

namespace
{
  std::string convertForLabel(double coord)
  {
    return str(boost::format("%8.4f") % coord);
  }
}

// void EditMeshBoundingBox::update_input_attributes(FieldHandle f)
// {
  // impl_->bbox_ = f->vmesh()->get_bounding_box();

  // if (!impl_->bbox_.valid())
  // {
    // warning("Input field is empty -- using unit cube.");
    // impl_->bbox_.extend(Point(0, 0, 0));
    // impl_->bbox_.extend(Point(1, 1, 1));
  // }

  // impl_->input_pos_ = impl_->bbox_.center();
  // auto diag = impl_->bbox_.diagonal();
  // impl_->input_eigvals_[0] = diag.x();
  // impl_->input_eigvals_[1] = diag.y();
  // impl_->input_eigvals_[2] = diag.z();

  // Start with axis aligned
  // impl_->input_eigvecs_[0] = Vector(1, 0, 0);
  // impl_->input_eigvecs_[1] = Vector(0, 1, 0);
  // impl_->input_eigvecs_[2] = Vector(0, 0, 1);
// }

void EditMeshBoundingBox::buildGeometryObject()
{
  // return WidgetFactory::createBoundingBox(*this, "EMBB", get_state()->getValue(Scale).toDouble(),
                                          // impl_->position_, impl_->position_.center_, 0, 0, impl_->bbox_);
}

void EditMeshBoundingBox::computeWidgetBox(const BBox& box)
{
  auto bbox(box);
  if (!bbox.valid())
  {
    warning("Input field is empty -- using unit cube.");
    bbox.extend(Point(0, 0, 0));
    bbox.extend(Point(1, 1, 1));
  }

  // build a widget identical to the BBox
  auto diag = bbox.diagonal();
  const auto SMALL = 1e-4;
  if (fabs(diag.x())<SMALL)
  {
    diag.x(2 * SMALL);
    bbox.extend(bbox.get_min() - Vector(SMALL, 0.0, 0.0));
    bbox.extend(bbox.get_max() + Vector(SMALL, 0.0, 0.0));
  }
  if (fabs(diag.y())<SMALL)
  {
    diag.y(2 * SMALL);
    bbox.extend(bbox.get_min() - Vector(0.0, SMALL, 0.0));
    bbox.extend(bbox.get_max() + Vector(0.0, SMALL, 0.0));
  }
  if (fabs(diag.z())<SMALL)
  {
    diag.z(2 * SMALL);
    bbox.extend(bbox.get_min() - Vector(0.0, 0.0, SMALL));
    bbox.extend(bbox.get_max() + Vector(0.0, 0.0, SMALL));
  }

  eigvals_.resize(3);
  eigvecs_.resize(3);
  pos_ = bbox.center();
  eigvals_[0] = diag.x() * 0.5;
  eigvals_[1] = diag.y() * 0.5;
  eigvals_[2] = diag.z() * 0.5;
  eigvecs_[0] = Vector(1,0,0);
  eigvecs_[1] = Vector(0,1,0);
  eigvecs_[2] = Vector(0,0,1);
}

namespace
{
  void checkForVerySmall(double& size)
  {
    const auto VERY_SMALL = 1e-12;
    if (size < VERY_SMALL) size = 1.0;
  }
}

void EditMeshBoundingBox::generateGeomsList()
{
  const auto bboxWidget = WidgetFactory::createBoundingBox(
    *this, "EMBB", get_state()->getValue(Scale).toDouble(), impl_->position_,
    pos_, 0, 0, impl_->bbox_);

  // Rewrite all existing geom
  geoms_.clear();
  for (const auto& widget : bboxWidget->widgets_)
    geoms_.push_back(widget);
}

const AlgorithmParameterName EditMeshBoundingBox::RefreshGeom("RefreshGeom");
const AlgorithmParameterName EditMeshBoundingBox::ResetCenter("ResetCenter");
const AlgorithmParameterName EditMeshBoundingBox::ResetSize("ResetSize");

const AlgorithmParameterName EditMeshBoundingBox::InputCenterX("InputCenterX");
const AlgorithmParameterName EditMeshBoundingBox::InputCenterY("InputCenterY");
const AlgorithmParameterName EditMeshBoundingBox::InputCenterZ("InputCenterZ");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeX("InputSizeX");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeY("InputSizeY");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeZ("InputSizeZ");
 //Output Field Atributes
const AlgorithmParameterName EditMeshBoundingBox::SetOutputCenter("SetOutputCenter");
const AlgorithmParameterName EditMeshBoundingBox::SetOutputSize("SetOutputSize");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterX("OutputCenterX");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterY("OutputCenterY");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterZ("OutputCenterZ");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeX("OutputSizeX");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeY("OutputSizeY");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeZ("OutputSizeZ");
//Widget Scale/Mode
const AlgorithmParameterName EditMeshBoundingBox::Scale("Scale");
const AlgorithmParameterName EditMeshBoundingBox::OldScale("OldScale");
const AlgorithmParameterName EditMeshBoundingBox::ScaleChanged("ScaleChanged");
const AlgorithmParameterName EditMeshBoundingBox::NoTranslation("NoTranslation");
const AlgorithmParameterName EditMeshBoundingBox::XYZTranslation("XYZTranslation");
const AlgorithmParameterName EditMeshBoundingBox::RDITranslation("RDITranslation");
const AlgorithmParameterName EditMeshBoundingBox::RestrictX("RestrictX");
const AlgorithmParameterName EditMeshBoundingBox::RestrictY("RestrictY");
const AlgorithmParameterName EditMeshBoundingBox::RestrictZ("RestrictZ");
const AlgorithmParameterName EditMeshBoundingBox::RestrictR("RestrictR");
const AlgorithmParameterName EditMeshBoundingBox::RestrictD("RestrictD");
const AlgorithmParameterName EditMeshBoundingBox::RestrictI("RestrictI");

const AlgorithmParameterName EditMeshBoundingBox::BoxMode("BoxMode");
const AlgorithmParameterName EditMeshBoundingBox::BoxRealScale("BoxRealScale");
