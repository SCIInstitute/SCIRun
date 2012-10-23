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
#include <QFrame>
#include <set>
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

class ModuleWidget : public QFrame, public NeedsScenePositionProvider, public SCIRun::Dataflow::Networks::ExecutableObject, public Ui::Module
{
	Q_OBJECT
	
public:
  ModuleWidget(const QString& name, SCIRun::Dataflow::Networks::ModuleHandle theModule, QWidget* parent = 0);
  ~ModuleWidget();

  void trackConnections();
  QPointF inputPortPosition() const;
  QPointF outputPortPosition() const;

  double percentComplete() const;
  void setPercentComplete(double p);

  size_t numInputPorts() const;
  size_t numOutputPorts() const;
  //TODO abstract
  typedef std::vector<PortWidget*> Ports;
  const Ports& getInputPorts() const { return inputPorts_; }
  const Ports& getOutputPorts() const { return outputPorts_; }

  std::string getModuleId() const { return moduleId_; }
  SCIRun::Dataflow::Networks::ModuleHandle getModule() const { return theModule_; }
  
public Q_SLOTS:
  virtual void execute();
  void showOptionsDialog();
  void setExecutionTime(int milliseconds);
Q_SIGNALS:
  void removeModule(const std::string& moduleId);
  void needConnection(const SCIRun::Dataflow::Networks::ConnectionDescription& desc);
  void connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription& desc);
  void connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId& id);
  void moduleExecuted();
private:
  Ports inputPorts_;
  Ports outputPorts_;

  SCIRun::Dataflow::Networks::ModuleHandle theModule_;

  void addPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider);
  void addPort(InputPortWidget* port);
  void addPort(OutputPortWidget* port);
  void hookUpSignals(PortWidget* port) const;
  std::string moduleId_;
  boost::scoped_ptr<class ModuleDialogGeneric> dialog_;
  void makeOptionsDialog();
  int executionTime_;

  class ModuleLogWindow* logWindow_;

  static boost::shared_ptr<class ModuleDialogFactory> dialogFactory_;

  class ModuleExecutionRunner
  {
  public:
    explicit ModuleExecutionRunner(ModuleWidget* module) : module_(module) {}
    void operator()();
  private:
    ModuleWidget* module_;
  };
  friend class FakeExecutionRunner;
  //
  void addPortLayouts();
  QHBoxLayout* outputPortLayout_;
  QHBoxLayout* inputPortLayout_;
};

}
}

#endif
