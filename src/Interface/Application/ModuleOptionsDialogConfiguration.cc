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


#include <Interface/qt_include.h>
#include <Interface/Application/ModuleOptionsDialogConfiguration.h>
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Dataflow/Network/Connection.h>
#include <Interface/Application/ClosestPortFinder.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/ModuleOptionsDialogConfiguration.h>
#include <Interface/Modules/Base/ModuleLogWindow.h>
#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/PortWidgetManager.h>
#include <Interface/Application/Utility.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Interface/Modules/Factory/ModuleDialogFactory.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Gui;

ModuleOptionsDialogConfiguration::ModuleOptionsDialogConfiguration(ModuleWidget* widget) : moduleWidget_(widget) {}

ModuleDialogDockWidget* ModuleOptionsDialogConfiguration::config(ModuleDialogGeneric* options)
{
  addWidgetToExecutionDisableList(options->getExecuteAction());
  QObject::connect(options, &ModuleDialogGeneric::executeActionTriggered, moduleWidget_, &ModuleWidget::executeButtonPushed);
  QObject::connect(options, &ModuleDialogGeneric::executeActionTriggeredViaStateChange, moduleWidget_, &ModuleWidget::executeTriggeredViaStateChange);
  QObject::connect(moduleWidget_, &ModuleWidget::moduleExecuted, options, &ModuleDialogGeneric::moduleExecuted);
  QObject::connect(moduleWidget_, &ModuleWidget::moduleSelected, options, &ModuleDialogGeneric::moduleSelected);

  QObject::connect(options, &ModuleDialogGeneric::setStartupNote, moduleWidget_, &ModuleWidget::setStartupNote);
  QObject::connect(options, &ModuleDialogGeneric::fatalError, moduleWidget_, &ModuleWidget::handleDialogFatalError);
  QObject::connect(options, &ModuleDialogGeneric::executionLoopStarted, moduleWidget_, &ModuleWidget::disableWidgetDisabling);
  QObject::connect(options, &ModuleDialogGeneric::executionLoopHalted, moduleWidget_, &ModuleWidget::reenableWidgetDisabling);
  QObject::connect(options, &ModuleDialogGeneric::closeButtonClicked, moduleWidget_, &ModuleWidget::toggleOptionsDialog);
  QObject::connect(options, &ModuleDialogGeneric::helpButtonClicked, moduleWidget_, &ModuleWidget::launchDocumentation);
  QObject::connect(options, &ModuleDialogGeneric::findButtonClicked, moduleWidget_, &ModuleWidget::findInNetwork);

  auto dockable = configDockable(options);

  if (!moduleWidget_->isViewScene_)
  {
    options->setupButtonBar();
  }

  if (ModuleWidget::highResolutionExpandFactor_ > 1 && !moduleWidget_->isViewScene_)
  {
    options->setFixedHeight(options->size().height() * ModuleWidget::highResolutionExpandFactor_);
    options->setFixedWidth(options->size().width() * (((ModuleWidget::highResolutionExpandFactor_ - 1) * 0.5) + 1));
  }

  if (ModuleWidget::highResolutionExpandFactor_ > 1 && moduleWidget_->isViewScene_)
    options->adjustToolbar(ModuleWidget::highResolutionExpandFactor_);

  options->pull();

  return dockable;
}


ModuleDialogDockWidget* ModuleOptionsDialogConfiguration::configDockable(ModuleDialogGeneric* options)
{
  auto dockable = new ModuleDialogDockWidget(QString::fromStdString(moduleWidget_->moduleId_), nullptr);
  dockable->setObjectName(options->windowTitle());
  dockable->setWidget(options);
  options->setDockable(dockable);
  QObject::connect(dockable, &ModuleDialogDockWidget::movedToFullScreen, options, &ModuleDialogGeneric::adaptToFullScreenView);

  dockable->setMinimumSize(options->minimumSize());
  dockable->setAllowedAreas(moduleWidget_->allowedDockArea());
  dockable->setAutoFillBackground(true);
  mainWindowWidget()->addDockWidget(Qt::RightDockWidgetArea, dockable);
  dockable->setFloating(true);
  dockable->hide();
  QObject::connect(dockable, &QDockWidget::visibilityChanged, moduleWidget_, &ModuleWidget::colorOptionsButton);
  QObject::connect(dockable, &QDockWidget::topLevelChanged, moduleWidget_, &ModuleWidget::updateDockWidgetProperties);

  if (moduleWidget_->isViewScene_ && Application::Instance().parameters()->isRegressionMode())
  {
    dockable->show();
    dockable->setFloating(true);
  }
  return dockable;
}
