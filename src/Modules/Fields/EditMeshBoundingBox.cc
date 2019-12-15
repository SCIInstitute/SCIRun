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

#include "Core/Datatypes/MatrixFwd.h"
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

EditMeshBoundingBox::EditMeshBoundingBox()
: GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(Transformation_Widget);
  INITIALIZE_PORT(Transformation_Matrix);
  firstRun_ = true;
  widgetMoved_ = true;
}

void EditMeshBoundingBox::processWidgetFeedback(const ModuleFeedback& var)
{
  try
  {
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    if (vsf.matchesWithModuleId(id()))
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

void EditMeshBoundingBox::adjustGeometryFromTransform(const Transform& feedbackTrans)
{
  fieldTrans_ = feedbackTrans * fieldTrans_;
  trans_ = feedbackTrans * trans_;
  trans_.orthogonalize();
}

void EditMeshBoundingBox::clear_vals() {
  auto state = get_state();
  const std::string cleared("---");
  state->setValue(InputCenterX, cleared);
  state->setValue(InputCenterY, cleared);
  state->setValue(InputCenterZ, cleared);
  state->setValue(InputSizeX, cleared);
  state->setValue(InputSizeY, cleared);
  state->setValue(InputSizeZ, cleared);
}

void EditMeshBoundingBox::setStateDefaults()
{
  clear_vals();
  auto state = get_state();
  state->setValue(TransformMatrix, VariableList());
  state->setValue(FieldTransformMatrix, VariableList());
  state->setValue(DataSaved, false);
  state->setValue(Scale, 1.0);
  state->setValue(OldScale, 1.0);
  state->setValue(ScaleChanged, false);
  state->setValue(RefreshGeom, true);
  state->setValue(SetOutputCenter, false);
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
  if (needToExecute() || widgetMoved_)
  {
    widgetMoved_ = false;
    outputField_ = FieldHandle(field->deep_clone());
    auto state = get_state();

    if (firstRun_ || inputsChanged() || state->getValue(RefreshGeom).toBool())
      computeWidgetBox(field->vmesh()->get_bounding_box());
    if (firstRun_ && state->getValue(DataSaved).toBool())
    {
      firstRun_ = false;
      loadFromParameters();
      updateInputFieldAttributes();
    }
    else if (inputsChanged() || state->getValue(RefreshGeom).toBool())
    {
      state->setValue(RefreshGeom, false);
      fieldTrans_ = Transform(); // Identity matrix
      trans_ = Transform(pos_, eigvecs_[0] * eigvals_[0], eigvecs_[1] * eigvals_[1], eigvecs_[2] * eigvals_[2]);
      updateInputFieldAttributes();
    }
    // applyScaling();
    updateState();

    sendOutput(Transformation_Matrix, MatrixHandle(new DenseMatrix(trans_)));
    outputField_->vmesh()->transform(fieldTrans_);
    sendOutput(OutputField, outputField_);

    generateGeomsList();
    auto comp_geo = createGeomComposite(*this, "bbox", geoms_.begin(), geoms_.end());
    sendOutput(Transformation_Widget, comp_geo);
    saveToParameters();
  }
}

void EditMeshBoundingBox::updateState()
{
  auto state = get_state();
  if(transient_value_cast<bool>(state->getTransientValue(SetOutputCenter)))
  {
    state->setTransientValue(SetOutputCenter, false);
    fieldTrans_.set_mat_val(0, 3, state->getValue(OutputCenterX).toDouble());
    fieldTrans_.set_mat_val(1, 3, state->getValue(OutputCenterY).toDouble());
    fieldTrans_.set_mat_val(2, 3, state->getValue(OutputCenterZ).toDouble());
    trans_.set_mat_val(0, 3, pos_[0] + state->getValue(OutputCenterX).toDouble());
    trans_.set_mat_val(1, 3, pos_[1] + state->getValue(OutputCenterY).toDouble());
    trans_.set_mat_val(2, 3, pos_[2] + state->getValue(OutputCenterZ).toDouble());
  }
  else if (transient_value_cast<bool>(state->getTransientValue(ResetCenter)))
  {
    state->setTransientValue(ResetCenter, false);
    for(int i = 0; i < 3; ++i)
    {
      trans_.set_mat_val(i, 3, pos_[i]);
      fieldTrans_.set_mat_val(i, 3, 0);
    }
  }
}

void EditMeshBoundingBox::refreshGeometry(const FieldHandle field)
{
  auto state = get_state();
  computeWidgetBox(field->vmesh()->get_bounding_box());
  trans_ = Transform(pos_, eigvecs_[0]*eigvals_[0], eigvecs_[1]*eigvals_[1], eigvecs_[2]*eigvals_[2]);
}

void EditMeshBoundingBox::applyScaling()
{
  auto state = get_state();

  if (state->getValue(ScaleChanged).toBool())
  {
    double scale = (state->getValue(Scale).toDouble());
    for (auto &eigval : eigvals_)
      eigval *= scale;
    trans_.pre_scale(scale);
  }

  // We have to undo previous scaling if there is no new geometry
  if (!state->getValue(RefreshGeom).toBool() &&
       state->getValue(ScaleChanged).toBool())
  {
    double oldScale = state->getValue(OldScale).toDouble();
    for (auto &eigval : eigvals_)
      eigval /= oldScale;
    state->setValue(OldScale, state->getValue(Scale).toDouble());
    trans_.pre_scale(oldScale);
  }

  state->setValue(DataSaved, false);
}

std::string EditMeshBoundingBox::convertForLabel(double coord)
{
  return str(boost::format("%8.4f") % coord);
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
  static int i = 0;
  const auto bboxWidget = WidgetFactory::createBoundingBox(*this, "EMBB",
    get_state()->getValue(Scale).toDouble(), trans_, trans_.get_translation_point(), i++,0);

  geoms_.clear();
  for (const auto& widget : bboxWidget->widgets_)
    geoms_.push_back(widget);
}

void EditMeshBoundingBox::loadFromParameters()
{
  trans_ = transformFromString(get_state()->getValue(TransformMatrix).toString());
  fieldTrans_ = transformFromString(get_state()->getValue(FieldTransformMatrix).toString());
}

void EditMeshBoundingBox::updateInputFieldAttributes()
{
  auto state = get_state();
  state->setValue(InputCenterX, convertForLabel(pos_[0]));
  state->setValue(InputCenterY, convertForLabel(pos_[1]));
  state->setValue(InputCenterZ, convertForLabel(pos_[2]));
  state->setValue(InputSizeX, convertForLabel(eigvals_[0]));
  state->setValue(InputSizeY, convertForLabel(eigvals_[1]));
  state->setValue(InputSizeZ, convertForLabel(eigvals_[2]));
}

void EditMeshBoundingBox::saveToParameters()
{
  auto state = get_state();
  state->setValue(TransformMatrix, trans_.get_string());
  state->setValue(FieldTransformMatrix, fieldTrans_.get_string());
  state->setValue(DataSaved, true);
}

const AlgorithmParameterName EditMeshBoundingBox::TransformMatrix("TransformMatrix");
const AlgorithmParameterName EditMeshBoundingBox::FieldTransformMatrix("FieldTransformMatrix");
const AlgorithmParameterName EditMeshBoundingBox::RefreshGeom("RefreshGeom");
const AlgorithmParameterName EditMeshBoundingBox::ResetCenter("ResetCenter");
const AlgorithmParameterName EditMeshBoundingBox::ResetSize("ResetSize");
const AlgorithmParameterName EditMeshBoundingBox::DataSaved("DataSaved");

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
