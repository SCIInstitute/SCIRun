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

#ifndef INTERFACE_APPLICATION_MODULEWIDGET_H
#define INTERFACE_APPLICATION_MODULEWIDGET_H

#include "ui_Module.h"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>
#include <QFrame>
#include <set>
#include <deque>
#include <Interface/Application/Note.h>
#include <Interface/Application/PositionProvider.h>

#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/ExecutableObject.h>

class QGraphicsProxyWidget;

namespace SCIRun {
namespace Gui {

class PortWidget;
class InputPortWidget;
class OutputPortWidget;
class PositionProvider;
class NetworkEditor;
class PortWidgetManager;

class ModuleWidget : public QFrame, 
  public SCIRun::Dataflow::Networks::ExecutableObject, public Ui::Module, public HasNotes
{
	Q_OBJECT
	
public:
  ModuleWidget(NetworkEditor* ed, const QString& name, SCIRun::Dataflow::Networks::ModuleHandle theModule, 
    QWidget* parent = 0);
  ~ModuleWidget();

  void trackConnections();

  size_t numInputPorts() const;
  size_t numOutputPorts() const;

  const PortWidgetManager& ports() { return *ports_; }

  std::string getModuleId() const { return moduleId_; }
  SCIRun::Dataflow::Networks::ModuleHandle getModule() const { return theModule_; }

  void setDeletedFromGui(bool b) { deletedFromGui_ = b; }

  //TODO: initialize in a new class
  static boost::shared_ptr<class ConnectionFactory> connectionFactory_;
  static boost::shared_ptr<class ClosestPortFinder> closestPortFinder_;
  
  void setColorAsWaiting();
  void setColorSelected();
  void setColorUnselected();

  void printPortPositions() const;

  bool hasDynamicPorts() const;

  static const int PORT_SPACING = 3;

  virtual boost::signals2::connection connectExecuteBegins(const SCIRun::Dataflow::Networks::ExecuteBeginsSignalType::slot_type& subscriber);
  virtual boost::signals2::connection connectExecuteEnds(const SCIRun::Dataflow::Networks::ExecuteEndsSignalType::slot_type& subscriber);
  virtual boost::signals2::connection connectErrorListener(const SCIRun::Dataflow::Networks::ErrorSignalType::slot_type& subscriber);

public Q_SLOTS:
  virtual void execute();
  void showOptionsDialog();
  void setLogButtonColor(const QColor& color);
  void resetLogButtonColor();
  void resetProgressBar();
  void updateProgressBar(double percent);
  void updateModuleTime();
  void launchDocumentation();
  void updateNote(const Note& note);
  void duplicate();
  void connectNewModule(const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);
  void addDynamicPort(const SCIRun::Dataflow::Networks::ModuleId& mid, const SCIRun::Dataflow::Networks::PortId& pid);
  void removeDynamicPort(const SCIRun::Dataflow::Networks::ModuleId& mid, const SCIRun::Dataflow::Networks::PortId& pid);
Q_SIGNALS:
  void removeModule(const SCIRun::Dataflow::Networks::ModuleId& moduleId);
  void requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to);
  void connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription& desc);
  void connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId& id);
  void moduleExecuted();
  void updateProgressBarSignal(double percent);
  void cancelConnectionsInProgress();
  void noteUpdated(const Note& note);
  void duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle& module);
  void connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo, const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);
  void backgroundColorUpdated(const QString& color);
  void dynamicPortChanged();
private Q_SLOTS:
  void updateBackgroundColor(const QString& color);
private:
  boost::shared_ptr<PortWidgetManager> ports_;
  boost::timer timer_;
  bool deletedFromGui_, colorLocked_;

  SCIRun::Dataflow::Networks::ModuleHandle theModule_;

  void addPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider);
  void addInputPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider);
  void addOutputPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider);
  void hookUpGeneralPortSignals(PortWidget* port) const;
  std::string moduleId_;
  boost::scoped_ptr<class ModuleDialogGeneric> dialog_;
  void makeOptionsDialog();
  void setupModuleActions();
  void printInputPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider);

  class ModuleLogWindow* logWindow_;
  boost::scoped_ptr<class ModuleActionsMenu> actionsMenu_;

  static boost::shared_ptr<class ModuleDialogFactory> dialogFactory_;

  void addPortLayouts();
  void addInputPortsToLayout();
  void addOutputPortsToLayout();
  QHBoxLayout* inputPortLayout_;
  QHBoxLayout* outputPortLayout_;
  NetworkEditor* editor_;
  bool deleting_;
};

}
}

#endif
