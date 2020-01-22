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

#include <Core/Datatypes/MatrixFwd.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Graphics/Widgets/BoundingBoxWidget.h>
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
  widgetAxesRotated_ = false;
}

void EditMeshBoundingBox::processWidgetFeedback(const ModuleFeedback& var)
{
  try
  {
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    if (vsf.matchesWithModuleId(id()))
    {
      widgetMoved_ = true;
      adjustGeometryFromTransform(vsf.buttonClicked, vsf.movementType, vsf.transform);
      enqueueExecuteAgain(true);
    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
}

void EditMeshBoundingBox::adjustGeometryFromTransform(MouseButton btn, WidgetMovement move, const Transform& feedbackTrans)
{
  if(btn == MouseButton::RIGHT && move == WidgetMovement::ROTATE)
  {
    widgetAxesRotated_ = true;
  }
  else
  {
    fieldTrans_ = feedbackTrans * fieldTrans_;
    trans_ = feedbackTrans * trans_;
    trans_.orthogonalize();
  }
  if(move == WidgetMovement::ROTATE)
  {
    widgetAxes_ = feedbackTrans * widgetAxes_;
    widgetAxes_.orthogonalize();
  }
}

void EditMeshBoundingBox::clearVals()
{
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
  clearVals();
  auto state = get_state();
  state->setValue(TransformMatrix, VariableList());
  state->setValue(FieldTransformMatrix, VariableList());
  state->setValue(DataSaved, false);
  state->setValue(Scale, 1.0);
  state->setValue(OldScale, 1.0);
  state->setValue(ScaleChanged, false);
  state->setValue(ResetToInput, true);
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

  getOutputPort(Transformation_Widget)->connectConnectionFeedbackListener([this]
    (const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void EditMeshBoundingBox::execute()
{
  auto field = getRequiredInput(InputField);
  if (needToExecute() || widgetMoved_)
  {
    updateState(field);
    sendOutputPorts();
    saveToParameters();
  }
}

void EditMeshBoundingBox::updateState(FieldHandle field)
{
  outputField_ = FieldHandle(field->deep_clone());
  auto state = get_state();
  widgetMoved_ = false;
  bool inputResetRequested = transient_value_cast<bool>(state->getTransientValue(ResetToInput));

  if(inputsChanged() || inputResetRequested)
    resetToInputField();
  if (widgetAxesRotated_)
    changeAxesOrientation(field);
  if (firstRun_ && state->getValue(DataSaved).toBool())
  {
    firstRun_ = false;
    loadFromParameters();
    updateInputFieldAttributes();
  }

  if(transient_value_cast<bool>(state->getTransientValue(SetOutputCenter)))
    setOutputCenter();
  else if (transient_value_cast<bool>(state->getTransientValue(ResetCenter)))
    resetOutputCenter();
}

void EditMeshBoundingBox::sendOutputPorts()
{
  sendOutput(Transformation_Matrix, MatrixHandle(new DenseMatrix(trans_)));
  outputField_->vmesh()->transform(fieldTrans_);
  sendOutput(OutputField, outputField_);

  generateGeomsList();
  auto geo = createGeomComposite(*this, "bbox", geoms_.begin(), geoms_.end());
  sendOutput(Transformation_Widget, geo);
}

void EditMeshBoundingBox::resetToInputField()
{
  trans_ = Transform();
  fieldTrans_ = Transform();
  widgetAxes_ = Transform();
  eigvecs_.resize(mDIMENSIONS);
  eigvecs_[0] = Vector(1,0,0);
  eigvecs_[1] = Vector(0,1,0);
  eigvecs_[2] = Vector(0,0,1);
  get_state()->setTransientValue(ResetToInput, false);
  computeWidgetBox(outputField_->vmesh()->get_oriented_bounding_box(eigvecs_[0], eigvecs_[1], eigvecs_[2]));
  ogPos_ = pos_;
  trans_ = Transform(pos_, eigvecs_[0]*eigvals_[0], eigvecs_[1]*eigvals_[1], eigvecs_[2]*eigvals_[2]);
  updateInputFieldAttributes();
}

void EditMeshBoundingBox::changeAxesOrientation(FieldHandle field)
{
  for(auto &eigvec : eigvecs_)
    eigvec = widgetAxes_ * eigvec;
  widgetAxes_ = Transform();

  auto fh = FieldHandle(field->deep_clone());
  fh->vmesh()->transform(fieldTrans_);
  computeWidgetBox(fh->vmesh()->get_oriented_bounding_box(eigvecs_[0], eigvecs_[1], eigvecs_[2]));
  trans_ = Transform(trans_.get_translation_point(), eigvecs_[0]*eigvals_[0], eigvecs_[1]*eigvals_[1], eigvecs_[2]*eigvals_[2]);
  widgetAxesRotated_ = false;
}

// Sets the translation vector in the homogeneous matrices
void EditMeshBoundingBox::setOutputCenter()
{
  auto state = get_state();
  state->setTransientValue(SetOutputCenter, false);

  trans_.set_mat_val(0, 3, state->getValue(OutputCenterX).toDouble());
  trans_.set_mat_val(1, 3, state->getValue(OutputCenterY).toDouble());
  trans_.set_mat_val(2, 3, state->getValue(OutputCenterZ).toDouble());

  // Vector multiplication with Transform only does rotation, not translation
  auto rotatedPositionVec = fieldTrans_ * Vector(ogPos_);
  fieldTrans_.set_mat_val(0, 3, state->getValue(OutputCenterX).toDouble() - rotatedPositionVec[0]);
  fieldTrans_.set_mat_val(1, 3, state->getValue(OutputCenterY).toDouble() - rotatedPositionVec[1]);
  fieldTrans_.set_mat_val(2, 3, state->getValue(OutputCenterZ).toDouble() - rotatedPositionVec[2]);
}

void EditMeshBoundingBox::resetOutputCenter()
{
  get_state()->setTransientValue(ResetCenter, false);

  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
  {
    trans_.set_mat_val(iDim, 3, ogPos_[iDim]);
    fieldTrans_.set_mat_val(iDim, 3, 0);
  }
}

std::string EditMeshBoundingBox::convertForLabel(double coord)
{
  return str(boost::format("%8.4f") % coord);
}


void EditMeshBoundingBox::computeWidgetBox(const OrientedBBox& box)
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

  eigvals_.resize(mDIMENSIONS);
  pos_ = bbox.center();
  eigvals_[0] = diag.x() * 0.5;
  eigvals_[1] = diag.y() * 0.5;
  eigvals_[2] = diag.z() * 0.5;
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
  static int widgetNum = 0;
  const auto bboxWidget = WidgetFactory::createBoundingBox(*this, "EMBB",
    get_state()->getValue(Scale).toDouble(), trans_, trans_.get_translation_point(), widgetNum++);

  geoms_.clear();
  for (const auto& widget : bboxWidget->widgets_)
    geoms_.push_back(widget);
}

void EditMeshBoundingBox::loadFromParameters()
{
  auto state = get_state();
  trans_ = transformFromString(state->getValue(TransformMatrix).toString());
  fieldTrans_ = transformFromString(state->getValue(FieldTransformMatrix).toString());
}

void EditMeshBoundingBox::updateInputFieldAttributes()
{
  auto state = get_state();
  state->setValue(InputCenterX, convertForLabel(ogPos_[0]));
  state->setValue(InputCenterY, convertForLabel(ogPos_[1]));
  state->setValue(InputCenterZ, convertForLabel(ogPos_[2]));
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
const AlgorithmParameterName EditMeshBoundingBox::ResetToInput("ResetToInput");
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
