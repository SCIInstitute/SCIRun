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

#include <iostream>
#include <QtGui>
#include <boost/range/join.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/PositionProvider.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/ModuleLogWindow.h>
#include <Interface/Modules/Factory/ModuleDialogFactory.h>

//TODO: BAD, or will we have some sort of Application global anyway?
#include <Interface/Application/SCIRunMainWindow.h>

#include <Dataflow/Network/Module.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;

QPointF ProxyWidgetPosition::currentPosition() const
{
  return widget_->pos() + offset_;
}

namespace {
//TODO move to separate header
QColor to_color(const std::string& str)
{
  if (str == "red")
    return Qt::red;
  if (str == "blue")
    return Qt::blue;
  if (str == "darkGreen")
    return Qt::darkGreen;
  if (str == "cyan")
    return Qt::cyan;
  if (str == "magenta")
    return Qt::magenta;
  if (str == "white")
    return Qt::white;
  if (str == "yellow")
    return Qt::yellow;
  if (str == "darkYellow")
    return Qt::darkYellow;
  else
    return Qt::black;
}
}

ModuleWidget::ModuleWidget(const QString& name, SCIRun::Dataflow::Networks::ModuleHandle theModule, QWidget* parent /* = 0 */)
  : QFrame(parent),
  moduleId_(theModule->get_id()),
  executionTime_(0),
  theModule_(theModule)
{
  setupUi(this);
  titleLabel_->setText("<b><h2>" + name + "</h2></b>");
  progressBar_->setMaximum(100);
  progressBar_->setMinimum(0);
  progressBar_->setValue(0);
  
  addPortLayouts();
  addPorts(*theModule);

  //TODO: this code should be used to set the correct sizes.
  //int pixelWidth = titleLabel_->fontMetrics().width(titleLabel_->text());
  //titleLabel_->setMinimumWidth(pixelWidth + 10);

  //setMinimumSize(pixelWidth + 10, this->minimumHeight());

  connect(optionsButton_, SIGNAL(clicked()), this, SLOT(showOptionsDialog()));
  makeOptionsDialog();

  logWindow_ = new ModuleLogWindow(QString::fromStdString(moduleId_), SCIRunMainWindow::Instance());
  connect(logButton2_, SIGNAL(clicked()), logWindow_, SLOT(show()));
  connect(logWindow_, SIGNAL(messageReceived(const QColor&)), this, SLOT(setLogButtonColor(const QColor&)));
  LoggerHandle logger(new ModuleLogger(logWindow_));
  theModule_->setLogger(logger);
}

void ModuleWidget::setLogButtonColor(const QColor& color)
{
  logButton2_->setStyleSheet(
    QString("* { background-color: rgb(%1,%2,%3) }")
    .arg(color.red())
    .arg(color.green())
    .arg(color.blue()));
}

void ModuleWidget::addPortLayouts()
{
  outputPortLayout_ = new QHBoxLayout;
  outputPortLayout_->setSpacing(3);
  
  QHBoxLayout* outputRowLayout = new QHBoxLayout;
  outputRowLayout->setAlignment(Qt::AlignLeft);
  outputRowLayout->addLayout(outputPortLayout_);
  verticalLayout_2->insertLayout(-1, outputRowLayout);

  inputPortLayout_ = new QHBoxLayout;
  inputPortLayout_->setSpacing(3);
  QHBoxLayout* inputRowLayout = new QHBoxLayout;
  inputRowLayout->setAlignment(Qt::AlignLeft);
  inputRowLayout->addLayout(inputPortLayout_);
  verticalLayout_2->insertLayout(0, inputRowLayout);
}

void ModuleWidget::addPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider)
{
  const std::string moduleId = moduleInfoProvider.get_id();
  for (size_t i = 0; i < moduleInfoProvider.num_input_ports(); ++i)
  {
    InputPortHandle port = moduleInfoProvider.get_input_port(i);
    InputPortWidget* w = new InputPortWidget(QString::fromStdString(port->get_portname()), to_color(port->get_colorname()), QString::fromStdString(moduleId), i, this);
    hookUpSignals(w);
    connect(this, SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)), w, SLOT(MakeTheConnection(const SCIRun::Dataflow::Networks::ConnectionDescription&)));
    addPort(w);
  }
  for (size_t i = 0; i < moduleInfoProvider.num_output_ports(); ++i)
  {
    OutputPortHandle port = moduleInfoProvider.get_output_port(i);
    OutputPortWidget* w = new OutputPortWidget(QString::fromStdString(port->get_portname()), to_color(port->get_colorname()), QString::fromStdString(moduleId), i, this);
    hookUpSignals(w);
    addPort(w);
  }
  optionsButton_->setVisible(moduleInfoProvider.has_ui());
}

void ModuleWidget::hookUpSignals(PortWidget* port) const
{
  connect(port, SIGNAL(needConnection(const SCIRun::Dataflow::Networks::ConnectionDescription&)), 
    this, SIGNAL(needConnection(const SCIRun::Dataflow::Networks::ConnectionDescription&)));
  connect(port, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)), 
    this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)));
}

void ModuleWidget::addPort(OutputPortWidget* port)
{
  outputPortLayout_->addWidget(port);
  outputPorts_.push_back(port);
}

void ModuleWidget::addPort(InputPortWidget* port)
{
  inputPortLayout_->addWidget(port);
  inputPorts_.push_back(port);
}

ModuleWidget::~ModuleWidget()
{
  Q_FOREACH (PortWidget* p, boost::join(inputPorts_, outputPorts_))
    p->deleteConnections();
  GuiLogger::Instance().log("Module deleted.");
  dialog_.reset();
  theModule_->setLogger(LoggerHandle());
  Q_EMIT removeModule(moduleId_);
}

void ModuleWidget::trackConnections()
{
  Q_FOREACH (PortWidget* p, boost::join(inputPorts_, outputPorts_))
    p->trackConnections();
}

QPointF ModuleWidget::inputPortPosition() const 
{
  if (positionProvider_)
    return positionProvider_->currentPosition() + QPointF(20,10);
  return pos();
}

QPointF ModuleWidget::outputPortPosition() const 
{
  if (positionProvider_)
    return positionProvider_->currentPosition() + QPointF(20, height() - 10);
  return pos();
}

double ModuleWidget::percentComplete() const
{
  return progressBar_->value() / 100.0;
}

void ModuleWidget::setPercentComplete(double p)
{
  if (0 <= p && p <= 1)
  {
    progressBar_->setValue(100 * p);
  }
}

void ModuleWidget::ModuleExecutionRunner::operator()()
{
  const int numIncrements = 20;
  const int increment = module_->executionTime_ / numIncrements;
  
  module_->theModule_->do_execute();

  for (int i = 0; i < numIncrements; ++i)
  {
     boost::this_thread::sleep(boost::posix_time::milliseconds(increment));
     module_->setPercentComplete(i / (double)numIncrements);
  }
  module_->setPercentComplete(1);
}

void ModuleWidget::execute()
{
  {
    ModuleExecutionRunner runner(this);
    runner();
  }
  Q_EMIT moduleExecuted();
}

void ModuleWidget::setExecutionTime(int milliseconds) 
{ 
  executionTime_ = milliseconds; 
  setPercentComplete(0);
}

boost::shared_ptr<ModuleDialogFactory> ModuleWidget::dialogFactory_;

void ModuleWidget::makeOptionsDialog()
{
  if (!dialog_)
  {
    if (!dialogFactory_)
      dialogFactory_.reset(new ModuleDialogFactory(SCIRunMainWindow::Instance()));

    dialog_.reset(dialogFactory_->makeDialog(moduleId_, theModule_->get_state(), executionTime_));
    dialog_->pull();
    connect(dialog_.get(), SIGNAL(executionTimeChanged(int)), this, SLOT(setExecutionTime(int)));
    connect(dialog_.get(), SIGNAL(executeButtonPressed()), this, SLOT(execute()));
    connect(this, SIGNAL(moduleExecuted()), dialog_.get(), SLOT(moduleExecuted()));
  }
}

void ModuleWidget::showOptionsDialog()
{
  makeOptionsDialog();
  dialog_->show();
}

