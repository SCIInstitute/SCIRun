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


#include <Interface/Modules/Fields/EditMeshBoundingBoxDialog.h>
#include <Modules/Fields/EditMeshBoundingBox.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;

typedef SCIRun::Modules::Fields::EditMeshBoundingBox EditMeshBoundingBoxModule;

EditMeshBoundingBoxDialog::EditMeshBoundingBoxDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent), doubleUpScale_(1.5625), upScale_(1.25), downScale_(0.8), doubleDownScale_(0.64)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addCheckableButtonManager(noTranslationRadioButton_, EditMeshBoundingBoxModule::NoTranslation);
  addCheckableButtonManager(xyzTranslationRadioButton_, EditMeshBoundingBoxModule::XYZTranslation);
  addCheckableButtonManager(rdiTranslationRadioButton_, EditMeshBoundingBoxModule::RDITranslation);
  addDoubleSpinBoxManager(outputCenterXSpinBox_, EditMeshBoundingBoxModule::OutputCenterX);
  addDoubleSpinBoxManager(outputCenterYSpinBox_, EditMeshBoundingBoxModule::OutputCenterY);
  addDoubleSpinBoxManager(outputCenterZSpinBox_, EditMeshBoundingBoxModule::OutputCenterZ);
  addDoubleSpinBoxManager(outputSizeXSpinBox_, EditMeshBoundingBoxModule::OutputSizeX);
  addDoubleSpinBoxManager(outputSizeYSpinBox_, EditMeshBoundingBoxModule::OutputSizeY);
  addDoubleSpinBoxManager(outputSizeZSpinBox_, EditMeshBoundingBoxModule::OutputSizeZ);
  addDynamicLabelManager(inputCenterXLabel_, EditMeshBoundingBoxModule::InputCenterX);
  addDynamicLabelManager(inputCenterYLabel_, EditMeshBoundingBoxModule::InputCenterY);
  addDynamicLabelManager(inputCenterZLabel_, EditMeshBoundingBoxModule::InputCenterZ);
  addDynamicLabelManager(inputSizeXLabel_, EditMeshBoundingBoxModule::InputSizeX);
  addDynamicLabelManager(inputSizeYLabel_, EditMeshBoundingBoxModule::InputSizeY);
  addDynamicLabelManager(inputSizeZLabel_, EditMeshBoundingBoxModule::InputSizeZ);

  connect(upScaleToolButton_, &QPushButton::clicked, this, &EditMeshBoundingBoxDialog::scaleUpPush);
  connect(doubleUpScaleToolButton_, &QPushButton::clicked, this, &EditMeshBoundingBoxDialog::scaleDoubleUpPush);
  connect(downScaleToolButton_, &QPushButton::clicked, this, &EditMeshBoundingBoxDialog::scaleDownPush);
  connect(doubleDownScaleToolButton_, &QPushButton::clicked, this, &EditMeshBoundingBoxDialog::scaleDoubleDownPush);

  setScaleButtonsEnabled(false);

  connect(resetSizePushButton_, &QPushButton::clicked, this, &EditMeshBoundingBoxDialog::userSetWidget);
  connect(resetCenterPushButton_, &QPushButton::clicked, this, &EditMeshBoundingBoxDialog::userSetWidget);
  connect(setCenterPushButton_, &QPushButton::clicked, this, &EditMeshBoundingBoxDialog::userSetWidget);
  connect(setSizePushButton_, &QPushButton::clicked, this, &EditMeshBoundingBoxDialog::userSetWidget);
  connect(resetAllPushButton_, &QPushButton::clicked, this, &EditMeshBoundingBoxDialog::userSetWidget);

  connectButtonsToExecuteSignal({ upScaleToolButton_, doubleUpScaleToolButton_, downScaleToolButton_, doubleDownScaleToolButton_,
    setCenterPushButton_, setSizePushButton_, resetSizePushButton_, resetCenterPushButton_, resetAllPushButton_ });

  createExecuteInteractivelyToggleAction();
}

void EditMeshBoundingBoxDialog::setScaleButtonsEnabled(bool enable)
{
  upScaleToolButton_->setEnabled(enable);
  doubleUpScaleToolButton_->setEnabled(enable);
  downScaleToolButton_->setEnabled(enable);
  doubleDownScaleToolButton_->setEnabled(enable);
}

void EditMeshBoundingBoxDialog::moduleExecuted()
{
  setScaleButtonsEnabled(true);
}

void EditMeshBoundingBoxDialog::adjustScale(float scaleFactor)
{
  auto scale = state_->getValue(EditMeshBoundingBoxModule::Scale).toDouble();
  scale *= scaleFactor;
  state_->setValue(EditMeshBoundingBoxModule::Scale, scale);
  state_->setTransientValue(EditMeshBoundingBoxModule::ScaleChanged, true);
}

void EditMeshBoundingBoxDialog::scaleUpPush(){ adjustScale(upScale_); }
void EditMeshBoundingBoxDialog::scaleDoubleUpPush(){ adjustScale(doubleUpScale_); }
void EditMeshBoundingBoxDialog::scaleDownPush(){ adjustScale(downScale_); }
void EditMeshBoundingBoxDialog::scaleDoubleDownPush(){ adjustScale(doubleDownScale_); }

void EditMeshBoundingBoxDialog::userSetWidget()
{
  auto button = sender()->objectName();
  if (button.startsWith("setCenter"))
  {
    state_->setTransientValue(EditMeshBoundingBoxModule::SetOutputCenter, true);
  }
  else if (button.startsWith("resetCenter"))
  {
    state_->setTransientValue(EditMeshBoundingBoxModule::ResetCenter, true);
  }
  else if (button.startsWith("setSize"))
  {
    state_->setTransientValue(EditMeshBoundingBoxModule::SetOutputSize, true);
  }
  else if (button.startsWith("resetSize"))
  {
    state_->setTransientValue(EditMeshBoundingBoxModule::ResetSize, true);
  }
  else if (button.startsWith("resetAll"))
  {
    state_->setTransientValue(EditMeshBoundingBoxModule::ResetToInput, true);
  }
}
