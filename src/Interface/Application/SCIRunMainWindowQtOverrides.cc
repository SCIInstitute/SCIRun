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


#include <es-log/trace-log.h>
#include <QtGui>
#include <functional>
#include <boost/bind.hpp>
#include <boost/assign.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <Core/Utils/Legacy/MemoryUtil.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/ShortcutsInterface.h>
#include <Interface/Application/TreeViewCollaborators.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/GuiCommands.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/NetworkExecutionProgressBar.h>
#include <Interface/Application/DialogErrorControl.h>
#include <Interface/Modules/Base/RemembersFileDialogDirectory.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h> //TODO
#include <Interface/Application/ModuleWizard/ModuleWizard.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h> //DOH! see TODO in setController
#include <Dataflow/Engine/Controller/ProvenanceManager.h>
#include <Dataflow/Network/SimpleSourceSink.h>  //TODO: encapsulate!!!
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Core/Thread/Parallel.h>
#include <Core/Application/Version.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Core/Utils/CurrentFileName.h>

#ifdef BUILD_WITH_PYTHON
#include <Interface/Application/PythonConsoleWidget.h>
#include <Core/Python/PythonInterpreter.h>
#endif
#include <Dataflow/Serialization/Network/XMLSerializer.h>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;

void SCIRunMainWindow::resizeEvent(QResizeEvent* event)
{
  dockSpace_ = size().height();
  QMainWindow::resizeEvent(event);
}

void SCIRunMainWindow::exitApplication(int code)
{
  if (Application::Instance().parameters()->saveViewSceneScreenshotsOnQuit())
  {
    networkEditor_->saveImages();
  }
  close();
  returnCode_ = code;
  qApp->exit(code);
}

void SCIRunMainWindow::quit()
{
  exitApplication(0);
}

void SCIRunMainWindow::closeEvent(QCloseEvent* event)
{
  windowState_ = saveState();
  if (okToContinue())
  {
    writeSettings();
    event->accept();
  }
  else
    event->ignore();
}

void SCIRunMainWindow::showEvent(QShowEvent* event)
{
  restoreState(windowState_);
  QMainWindow::showEvent(event);
}

void SCIRunMainWindow::hideEvent(QHideEvent * event)
{
  windowState_ = saveState();
  QMainWindow::hideEvent(event);
}

#ifdef __APPLE__
static const Qt::Key MetadataShiftKey = Qt::Key_Meta;
#else
static const Qt::Key MetadataShiftKey = Qt::Key_CapsLock;
#endif

void SCIRunMainWindow::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Shift)
	{
		showStatusMessage("Network zoom active");
    networkEditor_->adjustExecuteButtonsToDownstream(!Preferences::Instance().moduleExecuteDownstreamOnly);
	}
  else if (event->key() == MetadataShiftKey)
  {
    networkEditor_->metadataLayer(true);
		showStatusMessage("Metadata layer active");
  }
  else if (event->key() == Qt::Key_Alt)
  {
		if (!actionToggleTagLayer_->isChecked())
		{
	 		networkEditor_->tagLayer(true, NoTag);
			showStatusMessage("Tag layer active: none");
		}
  }
	else if (event->key() == Qt::Key_A)
	{
		if (!actionToggleTagLayer_->isChecked())
		{
    	if (networkEditor_->tagLayerActive())
    	{
      	networkEditor_->tagLayer(true, AllTags);
				showStatusMessage("Tag layer active: All");
    	}
		}
	}
  else if (event->key() == Qt::Key_G && (event->modifiers() & Qt::ShiftModifier))
  {
    if (!actionToggleTagLayer_->isChecked())
    {
      if (networkEditor_->tagLayerActive())
      {
        networkEditor_->tagLayer(true, HideGroups);
        showStatusMessage("Tag layer active: Groups hidden");
      }
    }
  }
	else if (event->key() == Qt::Key_G)
	{
		if (!actionToggleTagLayer_->isChecked())
		{
    	if (networkEditor_->tagLayerActive())
    	{
      	networkEditor_->tagLayer(true, ShowGroups);
				showStatusMessage("Tag layer active: Groups shown");
    	}
		}
	}
	else if (event->key() == Qt::Key_J)
	{
		if (!actionToggleTagLayer_->isChecked())
		{
    	if (networkEditor_->tagLayerActive())
    	{
      	networkEditor_->tagLayer(true, ClearTags);
				showStatusMessage("Tag layer active: selected modules' tags cleared");
    	}
	  }
	}
  else if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9)
  {
		if (!actionToggleTagLayer_->isChecked())
		{
    	if (networkEditor_->tagLayerActive())
    	{
      	auto key = event->key() - Qt::Key_0;
      	networkEditor_->tagLayer(true, key);
				showStatusMessage("Tag layer active: " + QString::number(key));
    	}
		}
  }
  else if (event->key() == Qt::Key_Period)
	{
    switchMouseMode();
	}

  QMainWindow::keyPressEvent(event);
}

void SCIRunMainWindow::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Shift)
	{
		showStatusMessage("Network zoom inactive", 1000);
    networkEditor_->adjustExecuteButtonsToDownstream(Preferences::Instance().moduleExecuteDownstreamOnly);
	}
  else if (event->key() == MetadataShiftKey)
  {
    networkEditor_->metadataLayer(false);
		showStatusMessage("Metadata layer inactive", 1000);
  }
  else if (event->key() == Qt::Key_Alt)
  {
		if (!actionToggleTagLayer_->isChecked())
		{
    	networkEditor_->tagLayer(false, -1);
			showStatusMessage("Tag layer inactive", 1000);
		}
  }

  QMainWindow::keyPressEvent(event);
}
