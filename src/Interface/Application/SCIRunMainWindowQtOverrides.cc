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

#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Core/Utils/Legacy/MemoryUtil.h>
#include <Dataflow/Engine/Controller/ProvenanceManager.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Interface/Application/GuiCommands.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h>  //TODO
#include <QtGui>
#include <boost/algorithm/string.hpp>

#ifdef BUILD_WITH_PYTHON
#include <Core/Python/PythonInterpreter.h>
#endif

using namespace SCIRun;
using namespace Gui;
using namespace Dataflow::Engine;
using namespace Dataflow::Networks;
using namespace Dataflow::State;
using namespace Core::Commands;
using namespace Core::Logging;
using namespace Core;
using namespace Algorithms;

void SCIRunMainWindow::resizeEvent(QResizeEvent* event)
{
  dockSpace_ = size().height();
  QMainWindow::resizeEvent(event);
}

void SCIRunMainWindow::exitApplication(int code)
{
  if (Application::Instance().parameters()->saveViewSceneScreenshotsOnQuit())
  { networkEditor_->saveImages(); }
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

void SCIRunMainWindow::hideEvent(QHideEvent* event)
{
  windowState_ = saveState();
  QMainWindow::hideEvent(event);
}

#ifdef __APPLE__
static const Qt::Key MetadataShiftKey = Qt::Key_Meta;
#else
static const Qt::Key MetadataShiftKey = Qt::Key_CapsLock;
#endif

void SCIRunMainWindow::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Shift)
  {
    showStatusMessage("Network zoom active");
    networkEditor_->adjustExecuteButtonsToDownstream(
        !Preferences::Instance().moduleExecuteDownstreamOnly);
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
      networkEditor_->tagLayer(true, TagValues::NoTag);
      showStatusMessage("Tag layer active: none");
    }
  }
  else if (event->key() == Qt::Key_A)
  {
    if (!actionToggleTagLayer_->isChecked())
    {
      if (networkEditor_->tagLayerActive())
      {
        networkEditor_->tagLayer(true, TagValues::AllTags);
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
        networkEditor_->tagLayer(true, TagValues::HideGroups);
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
        networkEditor_->tagLayer(true, TagValues::ShowGroups);
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
        networkEditor_->tagLayer(true, TagValues::ClearTags);
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

void SCIRunMainWindow::keyReleaseEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Shift)
  {
    showStatusMessage("Network zoom inactive", 1000);
    networkEditor_->adjustExecuteButtonsToDownstream(
        Preferences::Instance().moduleExecuteDownstreamOnly);
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
