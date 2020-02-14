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


#include <Interface/Modules/Math/GetMatrixSliceDialog.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Algorithms/Math/GetMatrixSliceAlgo.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

GetMatrixSliceDialog::GetMatrixSliceDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addSpinBoxManager(indexSpinBox_, Parameters::SliceIndex);
  addTwoChoiceBooleanComboBoxManager(rowColumnComboBox_, Parameters::IsSliceColumn);
  addSpinBoxManager(indexIncrementSpinBox_, Parameters::SliceIncrement);
  addSpinBoxManager(executionDelaySpinBox_, Parameters::PlayModeDelay);

  playModeMap_.insert(StringPair("Loop once", "looponce"));
  playModeMap_.insert(StringPair("Loop forever (EXPERIMENTAL)", "loopforever"));
  addComboBoxManager(playModeComboBox_, Parameters::PlayModeType, playModeMap_);

  nextIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSkipForward));
  connect(nextIndexButton_, SIGNAL(clicked()), this, SLOT(incrementIndex()));
  previousIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSkipBackward));
  connect(previousIndexButton_, SIGNAL(clicked()), this, SLOT(decrementIndex()));
  firstIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSeekBackward));
  connect(firstIndexButton_, SIGNAL(clicked()), this, SLOT(selectFirstIndex()));
  lastIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward));
  connect(lastIndexButton_, SIGNAL(clicked()), this, SLOT(selectLastIndex()));

  connect(indexSlider_, SIGNAL(sliderReleased()), this, SIGNAL(executeFromStateChangeTriggered()));

  playButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
  connect(playButton_, SIGNAL(clicked()), this, SLOT(startPlay()));
  pauseButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPause));
  connect(pauseButton_, SIGNAL(clicked()), this, SLOT(stopPlay()));

  //TODO: add convenience function at ModuleDialogGeneric level
  for (QToolButton* b : { nextIndexButton_, previousIndexButton_, firstIndexButton_, lastIndexButton_, playButton_, pauseButton_ })
  {
    b->setStyleSheet("QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }");
  }
}

void GetMatrixSliceDialog::pullSpecial()
{
  auto max = state_->getValue(Parameters::MaxIndex).toInt();
  indexSlider_->setMaximum(max);
  indexSpinBox_->setMaximum(max);

  // set value again in case it was greater than the hard-coded widget max.
  auto value = state_->getValue(Parameters::SliceIndex).toInt();
  indexSlider_->setValue(value);
  indexSpinBox_->setValue(value);

  indexSlider_->setMinimum(0);
}

void GetMatrixSliceDialog::incrementIndex()
{
  for (int i = 0; i < indexIncrementSpinBox_->value(); ++i)
    indexSpinBox_->stepUp();
  Q_EMIT executeFromStateChangeTriggered();
}

void GetMatrixSliceDialog::decrementIndex()
{
  for (int i = 0; i < indexIncrementSpinBox_->value(); ++i)
    indexSpinBox_->stepDown();
  Q_EMIT executeFromStateChangeTriggered();
}

void GetMatrixSliceDialog::selectFirstIndex()
{
  indexSpinBox_->setValue(0);
  Q_EMIT executeFromStateChangeTriggered();
}

void GetMatrixSliceDialog::selectLastIndex()
{
  indexSpinBox_->setValue(indexSlider_->maximum());
  Q_EMIT executeFromStateChangeTriggered();
}

void GetMatrixSliceDialog::startPlay()
{
  state_->setTransientValue(Parameters::PlayModeActive, static_cast<int>(GetMatrixSliceAlgo::PLAY));
  Q_EMIT executeFromStateChangeTriggered();
  Q_EMIT executionLoopStarted();
  //qDebug() << " execution loop started emitted ";
}

void GetMatrixSliceDialog::stopPlay()
{
  state_->setTransientValue(Parameters::PlayModeActive, static_cast<int>(GetMatrixSliceAlgo::PAUSE));
  Q_EMIT executionLoopHalted();
  //qDebug() << " execution loop halted emitted ";
}
