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

using namespace SCIRun::Core::Algorithms::Fields;

EditMeshBoundingBoxDialog::EditMeshBoundingBoxDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent), doubleUpScale_(1.5625), upScale_(1.25), downScale_(0.8), doubleDownScale_(0.64)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addCheckableButtonManager(noTranslationRadioButton_, Parameters::NoTranslation);
  addCheckableButtonManager(xyzTranslationRadioButton_, Parameters::XYZTranslation);
  addCheckableButtonManager(rdiTranslationRadioButton_, Parameters::RDITranslation);
  addDoubleSpinBoxManager(outputCenterXSpinBox_, Parameters::OutputCenterX);
  addDoubleSpinBoxManager(outputCenterYSpinBox_, Parameters::OutputCenterY);
  addDoubleSpinBoxManager(outputCenterZSpinBox_, Parameters::OutputCenterZ);
  addDoubleSpinBoxManager(outputSizeXSpinBox_, Parameters::OutputSizeX);
  addDoubleSpinBoxManager(outputSizeYSpinBox_, Parameters::OutputSizeY);
  addDoubleSpinBoxManager(outputSizeZSpinBox_, Parameters::OutputSizeZ);
  addDynamicLabelManager(inputCenterXLabel_, Parameters::InputCenterX);
  addDynamicLabelManager(inputCenterYLabel_, Parameters::InputCenterY);
  addDynamicLabelManager(inputCenterZLabel_, Parameters::InputCenterZ);
  addDynamicLabelManager(inputSizeXLabel_, Parameters::InputSizeX);
  addDynamicLabelManager(inputSizeYLabel_, Parameters::InputSizeY);
  addDynamicLabelManager(inputSizeZLabel_, Parameters::InputSizeZ);

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
  auto scale = state_->getValue(Parameters::Scale).toDouble();
  scale *= scaleFactor;
  state_->setValue(Parameters::Scale, scale);
  state_->setTransientValue(Parameters::ScaleChanged, true);
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
    state_->setTransientValue(Parameters::SetOutputCenter, true);
  }
  else if (button.startsWith("resetCenter"))
  {
    state_->setTransientValue(Parameters::ResetCenter, true);
  }
  else if (button.startsWith("setSize"))
  {
    state_->setTransientValue(Parameters::SetOutputSize, true);
  }
  else if (button.startsWith("resetSize"))
  {
    state_->setTransientValue(Parameters::ResetSize, true);
  }
  else if (button.startsWith("resetAll"))
  {
    state_->setTransientValue(Parameters::ResetToInput, true);
  }
}
