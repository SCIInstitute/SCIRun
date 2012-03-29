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

#include <QtGui>
#include "Module.h"
#include "Connection.h"

using namespace SCIRun::Gui;

QPointF ProxyWidgetPosition::currentPosition() const
{
  return widget_->pos();
}

Module::Module(const QString& name, QWidget* parent /* = 0 */)
  : QFrame(parent), positionProvider_(0)
{
  setupUi(this);
  titleLabel_->setText(name);
  progressBar_->setMaximum(100);
  progressBar_->setMinimum(0);
  progressBar_->setValue(0);
}

Module::~Module()
{
  foreach (Connection* c, connections_)
    delete c;
}

void Module::addConnection(Connection* c)
{
  connections_.insert(c);
}

void Module::removeConnection(Connection* c)
{
  connections_.erase(c);
}

void Module::trackConnections()
{
  foreach (Connection* c, connections_)
    c->trackNodes();
}

QPointF Module::inputPortPosition() const 
{
  if (positionProvider_)
    return positionProvider_->currentPosition() + QPointF(20,10);
  return pos();
}

QPointF Module::outputPortPosition() const 
{
  if (positionProvider_)
    return positionProvider_->currentPosition() + QPointF(20, height() - 10);
  return pos();
}

double Module::percentComplete() const
{
  return progressBar_->value() / 100.0;
}

void Module::setPercentComplete(double p)
{
  if (0 <= p && p <= 1)
  {
    progressBar_->setValue(100 * p);
  }
}

void Module::incrementProgressFake()
{
  setPercentComplete(percentComplete() + 0.1);
}