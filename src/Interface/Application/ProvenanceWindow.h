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


#ifndef INTERFACE_APPLICATION_PROVENANCEWINDOW_H
#define INTERFACE_APPLICATION_PROVENANCEWINDOW_H

#include "ui_ProvenanceWindow.h"

#ifndef Q_MOC_RUN
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Controller/ControllerInterfaces.h>
#include <Dataflow/Serialization/Network/ModulePositionGetter.h>
#include <Dataflow/Engine/Controller/ProvenanceManager.h>
#endif

namespace SCIRun {
namespace Gui {

  class NetworkEditor;

class ProvenanceWindow : public QDockWidget, public Ui::ProvenanceWindow
{
	Q_OBJECT

public:
  explicit ProvenanceWindow(SCIRun::Dataflow::Engine::ProvenanceManagerHandle provenanceManager, QWidget* parent = nullptr);
  void showFile(SCIRun::Dataflow::Networks::NetworkFileHandle file);
  int maxItems() const { return maxItems_; }
public Q_SLOTS:
  void clear();
  void addProvenanceItem(SCIRun::Dataflow::Engine::ProvenanceItemHandle item);
  void undo();
  void redo();
  void undoAll();
  void redoAll();
  void setMaxItems(int max);
private Q_SLOTS:
  void displayInfo(QListWidgetItem* item);
Q_SIGNALS:
  void modifyingNetwork(bool modifying);
  void undoStateChanged(bool enabled);
  void redoStateChanged(bool enabled);
  void networkModified();
private:
  SCIRun::Dataflow::Engine::ProvenanceManagerHandle provenanceManager_;
  int lastUndoRow_, maxItems_{10};
  const SCIRun::Dataflow::Engine::ProvenanceManagerHandle::element_type::IOType* networkEditor_;

  void setUndoEnabled(bool enable);
  void setRedoEnabled(bool enable);
};

//TODO: will become several classes
class GuiActionProvenanceConverter : public QObject
{
  Q_OBJECT
public:
  explicit GuiActionProvenanceConverter(NetworkEditor* editor);
public Q_SLOTS:
  void moduleAdded(const std::string& name, SCIRun::Dataflow::Networks::ModuleHandle module);
  void moduleRemoved(const SCIRun::Dataflow::Networks::ModuleId& id);
  void connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&);
  void connectionRemoved(const SCIRun::Dataflow::Networks::ConnectionId& id);
  void moduleMoved(const SCIRun::Dataflow::Networks::ModuleId& id, double newX, double newY);
  void networkBeingModifiedByProvenanceManager(bool inProgress);
Q_SIGNALS:
  void provenanceItemCreated(SCIRun::Dataflow::Engine::ProvenanceItemHandle item);
private:
  NetworkEditor* editor_;
  bool provenanceManagerModifyingNetwork_;
};

}
}

#endif
