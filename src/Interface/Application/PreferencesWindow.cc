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
#include <Core/Logging/Log.h>

using namespace SCIRun::Gui;

PreferencesWindow::PreferencesWindow(NetworkEditor* editor, QWidget* parent /* = 0 */) : QDialog(parent), networkEditor_(editor),
  regressionMode_(false)
{
  setupUi(this);
  connect(regressionTestDataButton_, SIGNAL(clicked()), this, SLOT(updateRegressionTestDataDir()));
  connect(saveBeforeExecuteCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(updateSaveBeforeExecuteOption(int)));
  connect(moduleErrorDialogDisableCheckbox_, SIGNAL(stateChanged(int)), this, SLOT(updateModuleErrorDialogOption(int)));
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
  SCIRun::Core::Preferences::Instance().showModuleErrorDialogs = (state == 0);
  LOG_DEBUG("showModuleErrorDialogs is " << (state == 0));
}

void PreferencesWindow::updateSaveBeforeExecuteOption(int state)
{
  SCIRun::Core::Preferences::Instance().saveBeforeExecute = (state != 0);
  LOG_DEBUG("saveBeforeExecute is " << (state != 0));
}

void PreferencesWindow::setSaveBeforeExecute(bool mode)
{
  updateSaveBeforeExecuteOption(mode ? 1 : 0);
  saveBeforeExecuteCheckBox_->setChecked(mode);
}

void PreferencesWindow::setDisableModuleErrorDialogs(bool mode)
{
  updateModuleErrorDialogOption(mode ? 1 : 0);
  moduleErrorDialogDisableCheckbox_->setChecked(mode);
}

bool PreferencesWindow::disableModuleErrorDialogs() const
{
  return !SCIRun::Core::Preferences::Instance().showModuleErrorDialogs;
}

bool PreferencesWindow::saveBeforeExecute() const
{
  return SCIRun::Core::Preferences::Instance().saveBeforeExecute;
}