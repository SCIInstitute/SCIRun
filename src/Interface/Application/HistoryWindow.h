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

#ifndef INTERFACE_APPLICATION_HISTORYWINDOW_H
#define INTERFACE_APPLICATION_HISTORYWINDOW_H

#include "ui_HistoryWindow.h"

#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Controller/ControllerInterfaces.h>
#include <Dataflow/Engine/Controller/HistoryItem.h>

namespace SCIRun {
namespace Gui {

  class NetworkEditor;

class HistoryWindow : public QDockWidget, public Ui::HistoryWindow
{
	Q_OBJECT
	
public:
  explicit HistoryWindow(SCIRun::Dataflow::Engine::HistoryManagerHandle historyManager, QWidget* parent = 0);
public Q_SLOTS:
  void showFile(const QString& path);
  void clear();
  void addHistoryItem(SCIRun::Dataflow::Engine::HistoryItemHandle item);
private Q_SLOTS:
  void displayInfo(QListWidgetItem* item);
  void undo();
  void redo();
Q_SIGNALS:

private:
  SCIRun::Dataflow::Engine::HistoryManagerHandle historyManager_;
  int lastUndoRow_;
  bool modifyingNetwork_;
};

//TODO: will become several classes
class GuiActionCommandHistoryConverter : public QObject
{
  Q_OBJECT
public:
  explicit GuiActionCommandHistoryConverter(NetworkEditor* editor);
public Q_SLOTS:
  void moduleAdded(const std::string& name, SCIRun::Dataflow::Networks::ModuleHandle module);
  void moduleRemoved(const std::string& name);
Q_SIGNALS:
  void historyItemCreated(SCIRun::Dataflow::Engine::HistoryItemHandle item);
private:
  NetworkEditor* editor_;
};

}
}

#endif
