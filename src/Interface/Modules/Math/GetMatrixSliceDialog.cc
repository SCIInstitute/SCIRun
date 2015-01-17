/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

  nextIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSkipForward));
  connect(nextIndexButton_, SIGNAL(clicked()), this, SLOT(incrementIndex()));
  previousIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSkipBackward));
  connect(previousIndexButton_, SIGNAL(clicked()), this, SLOT(decrementIndex()));
  firstIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSeekBackward));
  connect(firstIndexButton_, SIGNAL(clicked()), this, SLOT(selectFirstIndex()));
  lastIndexButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward));
  connect(lastIndexButton_, SIGNAL(clicked()), this, SLOT(selectLastIndex()));

  playButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
  connect(playButton_, SIGNAL(clicked()), this, SLOT(initiatePlay()));
}

void GetMatrixSliceDialog::pull()
{
  pull_newVersionToReplaceOld();
}

void GetMatrixSliceDialog::incrementIndex()
{
  indexSpinBox_->stepUp();
  Q_EMIT executeActionTriggered();
}

void GetMatrixSliceDialog::decrementIndex()
{
  indexSpinBox_->stepDown();
  Q_EMIT executeActionTriggered();
}

void GetMatrixSliceDialog::selectFirstIndex()
{
  indexSpinBox_->setValue(0);
  Q_EMIT executeActionTriggered();
}

void GetMatrixSliceDialog::selectLastIndex()
{
  std::cout << "end--needs requirements" << std::endl;
}

void GetMatrixSliceDialog::initiatePlay()
{
  std::cout << "play--needs requirements" << std::endl;
  state_->setValue(Parameters::PlayMode, true);
  Q_EMIT executeActionTriggered();
}
