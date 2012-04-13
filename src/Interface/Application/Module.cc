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
#include <Interface/Application/Module.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/Port.h>
//#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/PositionProvider.h>
#include <Interface/Application/Logger.h>

#include <Core/Dataflow/Network/Module.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Domain::Networks;

QPointF ProxyWidgetPosition::currentPosition() const
{
  return widget_->pos() + offset_;
}

ModuleWidget::ModuleWidget(const QString& name, const SCIRun::Domain::Networks::ModuleInfoProvider& moduleInfoProvider, QWidget* parent /* = 0 */)
  : QFrame(parent),
  moduleId_(moduleInfoProvider.get_id())
{
  setupUi(this);
  titleLabel_->setText("<b><h2>" + name + "</h2></b>");
  progressBar_->setMaximum(100);
  progressBar_->setMinimum(0);
  progressBar_->setValue(0);
  
  addPortLayouts();
  addPorts(moduleInfoProvider);
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
  for (size_t i = 0; i < moduleInfoProvider.num_input_ports(); ++i)
  {
    InputPortHandle port = moduleInfoProvider.get_input_port(i);
    addPort(new InputPortWidget(to_QString(port->get_portname()), to_color(port->get_colorname()), this));
  }
  for (size_t i = 0; i < moduleInfoProvider.num_output_ports(); ++i)
  {
    OutputPortHandle port = moduleInfoProvider.get_output_port(i);
    addPort(new OutputPortWidget(to_QString(port->get_portname()), to_color(port->get_colorname()), this));
  }
  optionsButton_->setVisible(moduleInfoProvider.has_ui());
}

void ModuleWidget::addPort(OutputPortWidget* port)
{
  outputPortLayout_->addWidget(port);
  ports_.push_back(port);
}

void ModuleWidget::addPort(InputPortWidget* port)
{
  inputPortLayout_->addWidget(port);
  ports_.push_back(port);
}

ModuleWidget::~ModuleWidget()
{
  foreach (PortWidget* p, ports_)
    p->deleteConnections();
  Logger::Instance->log("Module deleted.");
  emit removeModule(moduleId_);
}

void ModuleWidget::trackConnections()
{
  foreach (PortWidget* p, ports_)
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

void ModuleWidget::incrementProgressFake()
{
  setPercentComplete(percentComplete() + 0.1);
}
