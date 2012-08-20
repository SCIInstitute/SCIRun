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
#include <Interface/Application/Logger.h>
#include <Interface/Modules/ModuleDialogFactory.h>

//TODO: BAD, or will we have some sort of Application global anyway?
#include <Interface/Application/SCIRunMainWindow.h>

#include <Dataflow/Network/Module.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Domain::Networks;

QPointF ProxyWidgetPosition::currentPosition() const
{
  return widget_->pos() + offset_;
}

ModuleWidget::ModuleWidget(const QString& name, SCIRun::Domain::Networks::ModuleHandle theModule, QWidget* parent /* = 0 */)
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

void ModuleWidget::addPorts(const SCIRun::Domain::Networks::ModuleInfoProvider& moduleInfoProvider)
{
  const std::string moduleId = moduleInfoProvider.get_id();
  for (size_t i = 0; i < moduleInfoProvider.num_input_ports(); ++i)
  {
    InputPortHandle port = moduleInfoProvider.get_input_port(i);
    InputPortWidget* w = new InputPortWidget(to_QString(port->get_portname()), to_color(port->get_colorname()), to_QString(moduleId), i, this);
    hookUpSignals(w);
    //std::cout << "@@@ connecting " << moduleId << " moduleWidget to port " << port->get_portname() << "\n " << w << std::endl;
    connect(this, SIGNAL(connectionAdded(const SCIRun::Domain::Networks::ConnectionDescription&)), w, SLOT(MakeTheConnection(const SCIRun::Domain::Networks::ConnectionDescription&)));
    addPort(w);
  }
  for (size_t i = 0; i < moduleInfoProvider.num_output_ports(); ++i)
  {
    OutputPortHandle port = moduleInfoProvider.get_output_port(i);
    OutputPortWidget* w = new OutputPortWidget(to_QString(port->get_portname()), to_color(port->get_colorname()), to_QString(moduleId), i, this);
    hookUpSignals(w);
    addPort(w);
  }
  optionsButton_->setVisible(moduleInfoProvider.has_ui());
}

void ModuleWidget::hookUpSignals(PortWidget* port) const
{
  connect(port, SIGNAL(needConnection(const SCIRun::Domain::Networks::ConnectionDescription&)), 
    this, SIGNAL(needConnection(const SCIRun::Domain::Networks::ConnectionDescription&)));
  connect(port, SIGNAL(connectionDeleted(const SCIRun::Domain::Networks::ConnectionId&)), 
    this, SIGNAL(connectionDeleted(const SCIRun::Domain::Networks::ConnectionId&)));
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
  Logger::Instance()->log("Module deleted.");
  dialog_.reset();
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
  
  try
  {
    module_->theModule_->execute();
  }
  catch (std::exception& e)
  {
    std::cout << "Caught exception from module execution:" << std::endl;
    std::cout << e.what() << std::endl;
  }

  for (int i = 0; i < numIncrements; ++i)
  {
     boost::this_thread::sleep(boost::posix_time::milliseconds(increment));
     module_->setPercentComplete(i / (double)numIncrements);
  }
  module_->setPercentComplete(1);
}

void ModuleWidget::execute()
{
  std::cout << "Executing Module: " << moduleId_ << std::endl;

  //std::cout << "Will sleep for " << executionTime_ << " milliseconds." << std::endl;
  {
    ModuleExecutionRunner runner(this);
    runner();
  }
  Q_EMIT moduleExecuted();
  std::cout << "Done executing." << std::endl;
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

