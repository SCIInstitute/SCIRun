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
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h> //TODO
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>


using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;


void SCIRunMainWindow::executeAll()
{
	if (Application::Instance().parameters()->isRegressionMode())
	{
		auto timeout = Application::Instance().parameters()->developerParameters()->regressionTimeoutSeconds();
		QTimer::singleShot(1000 * *timeout, this, &SCIRunMainWindow::networkTimedOut);
	}
  writeSettings();
  networkEditor_->executeAll();
}

void SCIRunMainWindow::setConnectionPipelineType(int type)
{
	networkEditor_->setConnectionPipelineType(type);
  switch (ConnectionDrawType(type))
	{
  case ConnectionDrawType::MANHATTAN:
		prefsWindow_->manhattanPipesRadioButton_->setChecked(true);
		break;
  case ConnectionDrawType::CUBIC:
		prefsWindow_->cubicPipesRadioButton_->setChecked(true);
		break;
  case ConnectionDrawType::EUCLIDEAN:
		prefsWindow_->euclideanPipesRadioButton_->setChecked(true);
		break;
	}

  {
    QSettings settings;
    settings.setValue("connectionPipeType", networkEditor_->connectionPipelineType());
  }
}

void SCIRunMainWindow::setSaveBeforeExecute(int state)
{
  prefsWindow_->setSaveBeforeExecute(state != 0);
}

void SCIRunMainWindow::showZoomStatusMessage(int zoomLevel)
{
  statusBar()->showMessage(tr("Zoom: %1%").arg(zoomLevel), 2000);
}

void SCIRunMainWindow::setDataDirectoryFromGUI()
{
  auto dir = QFileDialog::getExistingDirectory(this, tr("Choose Data Directory"), QString::fromStdString(Core::Preferences::Instance().dataDirectory().parent_path().string()));
  setDataDirectory(dir);

  {
    QSettings settings;
    settings.setValue("dataDirectory", QString::fromStdString(Preferences::Instance().dataDirectory().string()));
  }
}

void SCIRunMainWindow::setScreenshotDirectoryFromGUI()
{
	auto dir = QFileDialog::getExistingDirectory(this, tr("Choose Screenshot Directory"), QString::fromStdString(Core::Preferences::Instance().screenshotDirectory().parent_path().string()));
	setScreenshotDirectory(dir);

	{
		QSettings settings;
		settings.setValue("screenshotDirectory", QString::fromStdString(Preferences::Instance().screenshotDirectory().string()));
	}

}
