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

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Interface/Modules/Visualization/ShowOrientationAxesDialog.h>
#include <Modules/Visualization/ShowOrientationAxes.h>
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;

using SOA = SCIRun::Modules::Visualization::ShowOrientationAxes;

ShowOrientationAxesDialog::ShowOrientationAxesDialog(
    const std::string& name, ModuleStateHandle state, QWidget* parent /* = 0*/)
    : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addDoubleSpinBoxManager(scaleDoubleSpinBox_, SOA::Scale);
  connect(upScaleToolButton_, SIGNAL(clicked()), this, SLOT(scaleUpPush()));
  connect(doubleUpScaleToolButton_, SIGNAL(clicked()), this, SLOT(scaleDoubleUpPush()));
  connect(downScaleToolButton_, SIGNAL(clicked()), this, SLOT(scaleDownPush()));
  connect(doubleDownScaleToolButton_, SIGNAL(clicked()), this, SLOT(scaleDoubleDownPush()));

  addDoubleSpinBoxManager(xDoubleSpinBox_, SOA::X);
  addDoubleSpinBoxManager(yDoubleSpinBox_, SOA::Y);
  addDoubleSpinBoxManager(zDoubleSpinBox_, SOA::Z);
  addCheckBoxManager(scaleByFieldCheckBox_,SOA::ScaleByField);
  addCheckBoxManager(scaleFactorCheckBox_, SOA::ScaleByScaleFactor);
  addCheckBoxManager(usePositionOfFieldCheckBox_, SOA::UseFieldPosition);

  connectButtonsToExecuteSignal();
}

void ShowOrientationAxesDialog::connectButtonsToExecuteSignal()
{
  connectButtonToExecuteSignal(upScaleToolButton_);
  connectButtonToExecuteSignal(doubleUpScaleToolButton_);
  connectButtonToExecuteSignal(downScaleToolButton_);
  connectButtonToExecuteSignal(doubleDownScaleToolButton_);
  connectButtonToExecuteSignal(scaleByFieldCheckBox_);
  connectButtonToExecuteSignal(scaleFactorCheckBox_);
  connectButtonToExecuteSignal(usePositionOfFieldCheckBox_);
}

void ShowOrientationAxesDialog::adjustScale(float scaleFactor) const
{
  auto scale = state_->getValue(SOA::Scale).toDouble();
  scale *= scaleFactor;
  state_->setValue(SOA::Scale, scale);
}

namespace
{
  const float upScale_ = 1.25f;
  const float doubleUpScale_ = upScale_ * upScale_;
  const float downScale_ = 0.8f;
  const float doubleDownScale_ = downScale_ * downScale_;
}

void ShowOrientationAxesDialog::scaleUpPush() const
{
  adjustScale(upScale_);
}

void ShowOrientationAxesDialog::scaleDoubleUpPush() const
{
  adjustScale(doubleUpScale_);
}

void ShowOrientationAxesDialog::scaleDownPush() const
{
  adjustScale(downScale_);
}

void ShowOrientationAxesDialog::scaleDoubleDownPush() const
{
  adjustScale(doubleDownScale_);
}
