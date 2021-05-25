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


#include <Dataflow/Network/ModuleStateInterface.h>
#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Feedback.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Point.h>
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
      adjustGeometryFromTransform(vsf.transform, vsf.movementType);
      enqueueExecuteAgain(true);
    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
}

void EditMeshBoundingBox::adjustGeometryFromTransform(const Transform& feedbackTrans, const WidgetMovement& movementType)
{
  switch(movementType)
  {
    case WidgetMovement::SCALE:
    {
      // Need to strip translation from scaling
      auto temp = feedbackTrans;
      temp.set_translation(0);

      widgetScale_ = temp * widgetScale_;
      widgetScale_.orthogonalize();
      break;
    }
    case WidgetMovement::ROTATE:
    {
      // Need to strip translation from rotation
      auto temp = feedbackTrans;
      temp.set_translation(0);

      widgetRotation_ = temp * widgetRotation_;
      widgetRotation_.orthogonalize();
      break;
    }
    case WidgetMovement::TRANSLATE:
      widgetTranslation_ += feedbackTrans.get_translation();
      break;
    default:
      break;
  }

  setOutputParameters();
}

void EditMeshBoundingBox::setOutputParameters()
{
  auto state = get_state();
  state->setValue(OutputCenterX, widgetTranslation_.x());
  state->setValue(OutputCenterY, widgetTranslation_.y());
  state->setValue(OutputCenterZ, widgetTranslation_.z());
  auto vecs = widgetScale_.get_transformation_vectors();
  state->setValue(OutputSizeX, 2.0*vecs[0].norm());
  state->setValue(OutputSizeY, 2.0*vecs[1].norm());
  state->setValue(OutputSizeZ, 2.0*vecs[2].norm());
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
  state->setValue(ScaleTransformMatrix, VariableList());
  state->setValue(RotationTransformMatrix, VariableList());
  state->setValue(TranslationPoint, VariableList());
  state->setValue(InverseFieldTransformMatrix, VariableList());
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
  if (firstRun_ && state->getValue(DataSaved).toBool())
  {
    loadFromParameters();
    updateInputFieldAttributes();
  }
  firstRun_ = false;

  if(transient_value_cast<bool>(state->getTransientValue(SetOutputCenter)))
    setOutputCenter();
  else if (transient_value_cast<bool>(state->getTransientValue(ResetCenter)))
    resetOutputCenter();
  if(transient_value_cast<bool>(state->getTransientValue(SetOutputSize)))
    setOutputSize();
  else if (transient_value_cast<bool>(state->getTransientValue(ResetSize)))
    resetOutputSize();
}

void EditMeshBoundingBox::sendOutputPorts()
{
  auto widgetTrans = (widgetRotation_ * widgetScale_);
  widgetTrans.set_translation(widgetTranslation_);
  auto fieldTrans = widgetTrans * inputFieldInverse_;

  outputField_->vmesh()->transform(fieldTrans);
  sendOutput(Transformation_Matrix, MatrixHandle(new DenseMatrix(widgetTrans)));
  sendOutput(OutputField, outputField_);

  generateGeomsList();
  auto geo = createGeomComposite(*this, "bbox", geoms_.begin(), geoms_.end());
  sendOutput(Transformation_Widget, geo);
}

void EditMeshBoundingBox::resetToInputField()
{
  auto state = get_state();
  widgetRotation_ = Transform();
  widgetScale_ = Transform();
  eigvecs_.resize(mDIMENSIONS);
  eigvecs_[0] = Vector(1,0,0);
  eigvecs_[1] = Vector(0,1,0);
  eigvecs_[2] = Vector(0,0,1);

  state->setTransientValue(ResetToInput, false);
  computeWidgetBox(outputField_->vmesh()->get_bounding_box());

  ogPos_ = pos_;
  widgetTranslation_ = pos_;
  auto trans = Transform(pos_, eigvecs_[0]*eigvals_[0], eigvecs_[1]*eigvals_[1], eigvecs_[2]*eigvals_[2]);
  ogScale_ = 2.0 * Vector(eigvals_[0], eigvals_[1], eigvals_[2]);
  widgetScale_.pre_scale(Vector(eigvals_[0], eigvals_[1], eigvals_[2]));

  inputFieldInverse_ = Transform(trans);
  inputFieldInverse_.invert();

  updateInputFieldAttributes();

  state->setValue(OutputCenterX, ogPos_.x());
  state->setValue(OutputCenterY, ogPos_.y());
  state->setValue(OutputCenterZ, ogPos_.z());
  state->setValue(OutputSizeX, ogScale_.x());
  state->setValue(OutputSizeY, ogScale_.y());
  state->setValue(OutputSizeZ, ogScale_.z());
}

// Sets the translation vector in the homogeneous matrices
void EditMeshBoundingBox::setOutputCenter()
{
  auto state = get_state();
  state->setTransientValue(SetOutputCenter, false);

  widgetTranslation_ = Point(state->getValue(OutputCenterX).toDouble(),
                              state->getValue(OutputCenterY).toDouble(),
                              state->getValue(OutputCenterZ).toDouble());

}

void EditMeshBoundingBox::resetOutputCenter()
{
  auto state = get_state();
  state->setTransientValue(ResetCenter, false);

  widgetTranslation_ = ogPos_;
  state->setValue(OutputCenterX, ogPos_.x());
  state->setValue(OutputCenterY, ogPos_.y());
  state->setValue(OutputCenterZ, ogPos_.z());
}

void EditMeshBoundingBox::setOutputSize()
{
  auto state = get_state();
  state->setTransientValue(SetOutputSize, false);

  widgetScale_ = Transform();
  widgetScale_.post_scale(0.5 * Vector(state->getValue(OutputSizeX).toDouble(),
                                       state->getValue(OutputSizeY).toDouble(),
                                       state->getValue(OutputSizeZ).toDouble()));
}

void EditMeshBoundingBox::resetOutputSize()
{
  auto state = get_state();
  state->setTransientValue(ResetSize, false);

  widgetScale_ = Transform();
  widgetScale_.pre_scale(0.5 * ogScale_);
  state->setValue(OutputSizeX, ogScale_.x());
  state->setValue(OutputSizeY, ogScale_.y());
  state->setValue(OutputSizeZ, ogScale_.z());
}

std::string EditMeshBoundingBox::convertForLabel(double coord)
{
  return str(boost::format("%8.4f") % coord);
}


void EditMeshBoundingBox::computeWidgetBox(const BBox& box)
{
  bbox_ = box;
  if (!bbox_.valid())
  {
    warning("Input field is empty -- using unit cube.");
    bbox_.extend(Point(0, 0, 0));
    bbox_.extend(Point(1, 1, 1));
  }

  // build a widget identical to the BBox
  auto diag = bbox_.diagonal();
  const auto SMALL = 1e-4;
  if (fabs(diag.x())<SMALL)
  {
    diag.x(2 * SMALL);
    bbox_.extend(bbox_.get_min() - Vector(SMALL, 0.0, 0.0));
    bbox_.extend(bbox_.get_max() + Vector(SMALL, 0.0, 0.0));
  }
  if (fabs(diag.y())<SMALL)
  {
    diag.y(2 * SMALL);
    bbox_.extend(bbox_.get_min() - Vector(0.0, SMALL, 0.0));
    bbox_.extend(bbox_.get_max() + Vector(0.0, SMALL, 0.0));
  }
  if (fabs(diag.z())<SMALL)
  {
    diag.z(2 * SMALL);
    bbox_.extend(bbox_.get_min() - Vector(0.0, 0.0, SMALL));
    bbox_.extend(bbox_.get_max() + Vector(0.0, 0.0, SMALL));
  }

  eigvals_.resize(mDIMENSIONS);
  pos_ = bbox_.center();
  eigvals_[0] = diag.x() * 0.5;
  eigvals_[1] = diag.y() * 0.5;
  eigvals_[2] = diag.z() * 0.5;
}

void EditMeshBoundingBox::generateGeomsList()
{
  geoms_.clear();
  auto bbox = WidgetFactory::createBoundingBox({*this, "EMBB"},
    {{get_state()->getValue(Scale).toDouble(), "no-color", widgetTranslation_, bbox_, resolution_},
    {widgetTranslation_, (widgetRotation_ * widgetScale_).get_transformation_vectors()}});
  auto bboxWidget = boost::dynamic_pointer_cast<CompositeWidget>(bbox);
  geoms_.insert(geoms_.end(), bboxWidget->subwidgetBegin(), bboxWidget->subwidgetEnd());
}

void EditMeshBoundingBox::loadFromParameters()
{
  auto state = get_state();
  widgetScale_ = transformFromString(state->getValue(ScaleTransformMatrix).toString());
  widgetRotation_ = transformFromString(state->getValue(RotationTransformMatrix).toString());
  widgetTranslation_ =
pointFromString(state->getValue(TranslationPoint).toString());
  inputFieldInverse_ = transformFromString(state->getValue(InverseFieldTransformMatrix).toString());
  setOutputParameters();
}

void EditMeshBoundingBox::updateInputFieldAttributes()
{
  auto state = get_state();
  state->setValue(InputCenterX, convertForLabel(ogPos_[0]));
  state->setValue(InputCenterY, convertForLabel(ogPos_[1]));
  state->setValue(InputCenterZ, convertForLabel(ogPos_[2]));
  state->setValue(InputSizeX, convertForLabel(ogScale_[0]));
  state->setValue(InputSizeY, convertForLabel(ogScale_[1]));
  state->setValue(InputSizeZ, convertForLabel(ogScale_[2]));
}

void EditMeshBoundingBox::saveToParameters()
{
  auto state = get_state();
  state->setValue(ScaleTransformMatrix, widgetScale_.get_string());
  state->setValue(RotationTransformMatrix, widgetRotation_.get_string());
  state->setValue(TranslationPoint, widgetTranslation_.get_string());
  state->setValue(InverseFieldTransformMatrix, inputFieldInverse_.get_string());
  state->setValue(DataSaved, true);
}

const AlgorithmParameterName EditMeshBoundingBox::ScaleTransformMatrix("ScaleTransformMatrix");
const AlgorithmParameterName
EditMeshBoundingBox::RotationTransformMatrix("RotationTransformMatrix");
const AlgorithmParameterName
EditMeshBoundingBox::TranslationPoint("TranslationPoint");
const AlgorithmParameterName
EditMeshBoundingBox::InverseFieldTransformMatrix("InverseFieldTransformMatrix");
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
