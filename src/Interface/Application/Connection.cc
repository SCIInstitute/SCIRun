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
#include "Connection.h"
#include "Module.h"
#include "Utility.h"
#include "Port.h"

using namespace SCIRun::Gui;

Connection::Connection(Module* fromModule, Module* toModule)
  : fromModule_(fromModule), toModule_(toModule),
  fromPort_(0), toPort_(0)
{
  fromModule_->addConnection(this);
  toModule_->addConnection(this);

  setFlags(QGraphicsItem::ItemIsSelectable);
  setZValue(-1);

  setColor(Qt::darkRed);
  trackNodes();
}

Connection::Connection(Port* fromPort, Port* toPort)
  //TODO: yuck, replace with sensible factory
  : fromPort_(dynamic_cast<OutputPort*>(fromPort)), toPort_(dynamic_cast<InputPort*>(toPort)),
  fromModule_(0), toModule_(0)
{
  if (fromPort_)
    fromPort_->addConnection(this);
  if (toPort_)
    toPort_->addConnection(this);

  setFlags(QGraphicsItem::ItemIsSelectable);
  setZValue(-1);

  if (fromPort_ && toPort_)
    setColor(Qt::darkRed);
  else // "in-progress" pipe
    setColor(Qt::red);

  trackNodes();
}

Connection::~Connection()
{
  if (fromModule_ && toModule_)
  {
    fromModule_->removeConnection(this);
    toModule_->removeConnection(this);
  }
  else if (fromPort_ && toPort_)
  {
    fromPort_->removeConnection(this);
    toPort_->removeConnection(this);
  }
}

void Connection::setColor(const QColor& color)
{
  setPen(QPen(color, 5.0));
}

QColor Connection::color() const
{
  return pen().color();
}

void Connection::trackNodes()
{
  if (fromModule_ && toModule_)
  {
    std::cout << "Drawing line from " << to_string(fromModule_->outputPortPosition()) << " to " << to_string(toModule_->inputPortPosition()) << std::endl;
    setLine(QLineF(fromModule_->outputPortPosition(), toModule_->inputPortPosition()));
  }
  else if (fromPort_ && toPort_)
  {
    std::cout << "Drawing line from " << to_string(fromPort_->pos()) << " to " << to_string(toPort_->pos()) << std::endl;
    setLine(QLineF(fromPort_->position(), toPort_->position()));
  }
  else if (fromPort_)
  {
    std::cout << "Drawing line from " << to_string(fromPort_->pos()) << " to " << to_string(pos()) << std::endl;
    setLine(QLineF(fromPort_->position(), pos()));
  }
  else if (toPort_)
  {
    std::cout << "Drawing line from " << to_string(pos()) << " to " << to_string(toPort_->pos()) << std::endl;
    setLine(QLineF(pos(), toPort_->position()));
  }
  else
    throw std::logic_error("no from/to set for Connection");
}

ConnectionInProgress::ConnectionInProgress(Port* port)
  : fromPort_(port)
{
  //if (fromPort_)
  //  fromPort_->addConnection(this);

  setZValue(-1);

  setColor(Qt::red);
}

void ConnectionInProgress::setColor(const QColor& color)
{
  setPen(QPen(color, 5.0));
}

QColor ConnectionInProgress::color() const
{
  return pen().color();
}

void ConnectionInProgress::update(const QPoint& end)
{
  setLine(QLineF(fromPort_->position(), mapToScene(end)));
}
