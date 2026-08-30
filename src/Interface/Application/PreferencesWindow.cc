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


#include <QtGui>
#include <iostream>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Interface/Application/GuiLogger.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Logging;

PreferencesWindow::PreferencesWindow(NetworkEditor* editor, std::function<void()> writeSettings,
  QWidget* parent /* = 0 */) : QDialog(parent), networkEditor_(editor), writeSettings_(writeSettings)
{
  setupUi(this);
  connect(saveBeforeExecuteCheckBox_, &QCheckBox::toggled, this, &PreferencesWindow::updateSaveBeforeExecuteOption);
  connect(moduleErrorDialogDisableCheckbox_, &QCheckBox::toggled, this, &PreferencesWindow::updateModuleErrorDialogOption);
  connect(autoModuleNoteCheckbox_, &QCheckBox::toggled, this, &PreferencesWindow::updateAutoNotesState);
  connect(errorGraphicItemsCheckBox_, &QCheckBox::toggled, this, &PreferencesWindow::updateModuleErrorInlineMessagesOption);
  connect(highDPIAdjustCheckBox_, &QCheckBox::toggled, this, &PreferencesWindow::updateHighDPIAdjust);
  connect(forceGridBackgroundCheckBox_, &QCheckBox::toggled, this, &PreferencesWindow::updateForceGridBackground);
  connect(autoRotateViewerOnMouseReleaseCheckbox_, &QCheckBox::toggled, this, &PreferencesWindow::updateAutoRotateViewer);
  connect(moduleExecuteDownstreamOnlyCheckBox_, &QCheckBox::toggled, this, &PreferencesWindow::updateModuleExecuteDownstream);
  connect(toolBarPopupShowDelaySpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &PreferencesWindow::updateToolBarPopupShowDelay);
  connect(toolBarPopupHideDelaySpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &PreferencesWindow::updateToolBarPopupHideDelay);
}

void PreferencesWindow::updateToolBarPopupShowDelay(int delay)
{
  SCIRun::Core::Preferences::Instance().toolBarPopupShowDelay.setValueWithSignal(delay);
  toolBarPopupShowDelaySpinBox_->setValue(delay);
}

void PreferencesWindow::updateToolBarPopupHideDelay(int delay)
{
  SCIRun::Core::Preferences::Instance().toolBarPopupHideDelay.setValueWithSignal(delay);
  toolBarPopupHideDelaySpinBox_->setValue(delay);
}

void PreferencesWindow::updateAutoRotateViewer(bool state)
{
  SCIRun::Core::Preferences::Instance().autoRotateViewerOnMouseRelease.setValue(state);
}

void PreferencesWindow::updateModuleErrorDialogOption(bool state)
{
  SCIRun::Core::Preferences::Instance().showModuleErrorDialogs.setValueWithSignal(!state);
}

void PreferencesWindow::updateSaveBeforeExecuteOption(bool state)
{
  SCIRun::Core::Preferences::Instance().saveBeforeExecute.setValue(state);
}

void PreferencesWindow::updateAutoNotesState(bool state)
{
  SCIRun::Core::Preferences::Instance().autoNotes.setValue(state);
}

void PreferencesWindow::updateHighDPIAdjust(bool state)
{
  SCIRun::Core::Preferences::Instance().highDPIAdjustment.setValue(state);
}

void PreferencesWindow::updateForceGridBackground(bool state)
{
  SCIRun::Core::Preferences::Instance().forceGridBackground.setValueWithSignal(state);
}

void PreferencesWindow::updateModuleExecuteDownstream(bool state)
{
  SCIRun::Core::Preferences::Instance().moduleExecuteDownstreamOnly.setValueWithSignal(state);
}

void PreferencesWindow::setSaveBeforeExecute(bool mode)
{
  updateSaveBeforeExecuteOption(mode);
  saveBeforeExecuteCheckBox_->setChecked(mode);
}

void PreferencesWindow::setModuleExecuteDownstreamOnly(bool mode)
{
  updateModuleExecuteDownstream(mode);
  moduleExecuteDownstreamOnlyCheckBox_->setChecked(mode);
}

void PreferencesWindow::setAutoRotateViewerOnMouseRelease(bool mode)
{
  updateAutoRotateViewer(mode);
  autoRotateViewerOnMouseReleaseCheckbox_->setChecked(mode);
}

void PreferencesWindow::setToolBarPopupShowDelay(int delay)
{
  updateToolBarPopupShowDelay(delay);
}

void PreferencesWindow::setToolBarPopupHideDelay(int delay)
{
  updateToolBarPopupHideDelay(delay);
}

void PreferencesWindow::setHighDPIAdjustment(bool highDPI)
{
  updateHighDPIAdjust(highDPI);
  highDPIAdjustCheckBox_->setChecked(highDPI);
}

void PreferencesWindow::setDisableModuleErrorDialogs(bool mode)
{
  updateModuleErrorDialogOption(mode);
  moduleErrorDialogDisableCheckbox_->setChecked(mode);
}

void PreferencesWindow::updateModuleErrorInlineMessagesOption(bool state)
{
  SCIRun::Core::Preferences::Instance().showModuleErrorInlineMessages.setValue(state);
}

void PreferencesWindow::setModuleErrorInlineMessages(bool mode)
{
  updateModuleErrorInlineMessagesOption(mode);
  errorGraphicItemsCheckBox_->setChecked(mode);
}

bool PreferencesWindow::disableModuleErrorDialogs() const
{
  return !SCIRun::Core::Preferences::Instance().showModuleErrorDialogs;
}

bool PreferencesWindow::saveBeforeExecute() const
{
  return SCIRun::Core::Preferences::Instance().saveBeforeExecute;
}

void PreferencesWindow::hideEvent(QHideEvent * event)
{
  if (writeSettings_)
    writeSettings_();
  QDialog::hideEvent(event);
}
