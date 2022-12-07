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


#include <Interface/Modules/Visualization/GeometryBufferDialog.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Modules/Visualization/GeometryBuffer.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;

GeometryBufferDialog::GeometryBufferDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addSpinBoxManager(indexSpinBox_, Parameters::GeometryIndex);
  addSpinBoxManager(indexIncrementSpinBox_, Parameters::GeometryIncrement);
  addSpinBoxManager(executionDelaySpinBox_, Parameters::PlayModeDelay);
  addCheckBoxManager(loopCheckBox_, Parameters::LoopForever);

  nextIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSkipForward));
  connect(nextIndexButton_, &QPushButton::clicked, this, &GeometryBufferDialog::incrementIndex);
  previousIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSkipBackward));
  connect(previousIndexButton_, &QPushButton::clicked, this, &GeometryBufferDialog::decrementIndex);
  firstIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSeekBackward));
  connect(firstIndexButton_, &QPushButton::clicked, this, &GeometryBufferDialog::selectFirstIndex);
  lastIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward));
  connect(lastIndexButton_, &QPushButton::clicked, this, &GeometryBufferDialog::selectLastIndex);
  connect(indexSlider_, &QSlider::sliderReleased, [this]()
  {
    state_->setValue(Parameters::SingleStep, true);
  });

  playButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
  pauseButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPause));

  //TODO: add convenience function at ModuleDialogGeneric level
  for (QToolButton* b : { nextIndexButton_, previousIndexButton_, firstIndexButton_, lastIndexButton_, playButton_, pauseButton_ })
  {
    b->setStyleSheet("QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }");
  }

  connect(playButton_, &QPushButton::clicked, [this]() { startPlay(); });
  connect(pauseButton_, &QPushButton::clicked, [this]() { stopPlay(); });
  connect(clearBufferPushButton_, &QPushButton::clicked, [this]()
    { state_->setTransientValue(Parameters::ClearFlag, true, true); });
}

void GeometryBufferDialog::pullSpecial()
{
  auto max = state_->getValue(Parameters::BufferSize).toInt();
  indexSlider_->setMaximum(max - 1);
  indexSpinBox_->setMaximum(max - 1);

  // set value again in case it was greater than the hard-coded widget max.
  auto value = state_->getValue(Parameters::GeometryIndex).toInt();
  indexSlider_->setValue(value);
  indexSpinBox_->setValue(value);

  indexSlider_->setMinimum(0);

  sizeLabel_->setText(QString::number(max));
}

void GeometryBufferDialog::incrementIndex()
{
  if (indexSlider_->value() == indexSlider_->maximum())
    return;

  for (int i = 0; i < indexIncrementSpinBox_->value(); ++i)
    indexSpinBox_->stepUp();
  state_->setValue(Parameters::SingleStep, true);
}

void GeometryBufferDialog::decrementIndex()
{
  if (indexSlider_->value() == 0)
    return;

  for (int i = 0; i < indexIncrementSpinBox_->value(); ++i)
    indexSpinBox_->stepDown();
  state_->setValue(Parameters::SingleStep, true);
}

void GeometryBufferDialog::selectFirstIndex()
{
  indexSpinBox_->setValue(0);
  state_->setValue(Parameters::SingleStep, true);
}

void GeometryBufferDialog::selectLastIndex()
{
  indexSpinBox_->setValue(indexSlider_->maximum());
  state_->setValue(Parameters::SingleStep, true);
}

void GeometryBufferDialog::startPlay()
{
  state_->setValue(Parameters::PlayModeActive, true);
}

void GeometryBufferDialog::stopPlay()
{
  state_->setValue(Parameters::PlayModeActive, false);
}
