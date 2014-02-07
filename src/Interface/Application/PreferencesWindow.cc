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

#include <QtGui>
#include <iostream>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Core/Application/Preferences.h>
//#include <Core/Logging/Log.h>

using namespace SCIRun::Gui;
//using namespace SCIRun::Core::Logging;

PreferencesWindow::PreferencesWindow(NetworkEditor* editor, QWidget* parent /* = 0 */) : QDialog(parent), networkEditor_(editor),
  regressionMode_(false), saveBeforeExecute_(false)
{
  setupUi(this);
  connect(regressionTestDataButton_, SIGNAL(clicked()), this, SLOT(updateRegressionTestDataDir()));
  connect(moduleErrorDialogDisableCheckbox_, SIGNAL(stateChanged(int)), this, SLOT(updateModuleErrorDialogOption(int)));
  connect(saveBeforeExecuteCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(updateSaveBeforeExecuteOption(int)));
}

void PreferencesWindow::updateRegressionTestDataDir()
{
  auto newDir = QFileDialog::getExistingDirectory(this, "Select regression data directory", ".");
  if (!newDir.isEmpty())
  {
    regressionTestDataDir_ = newDir;
    setRegressionTestDataDir();
  }
}

void PreferencesWindow::setRegressionTestDataDir()
{
  regressionTestDataDirLineEdit_->setText(regressionTestDataDir_);
  networkEditor_->setRegressionTestDataDir(regressionTestDataDir_);
}

void PreferencesWindow::updateModuleErrorDialogOption(int state)
{
  SCIRun::Core::Preferences::Instance().moduleErrorDialogState = (state == 0);
}

void PreferencesWindow::updateSaveBeforeExecuteOption(int state)
{
  SCIRun::Core::Preferences::Instance().saveBeforeExecute = (state != 0);
  //Log::get() << DEBUG_LOG << "saveBeforeExecute is " << (state != 0) << std::endl;
}