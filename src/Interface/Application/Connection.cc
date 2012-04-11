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
#include <stdexcept>
#include <QtGui>
#include <Interface/Application/Connection.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/Logger.h>

using namespace SCIRun::Gui;

Connection::Connection(Port* fromPort, Port* toPort)
  : fromPort_(fromPort), toPort_(toPort)
{
  if (fromPort_)
  {
    fromPort_->addConnection(this);
    fromPort_->turn_on_light();
  }
  if (toPort_)
  {
    toPort_->addConnection(this);
    toPort_->turn_on_light();
  }

  setFlags(QGraphicsItem::ItemIsSelectable);
  setZValue(100);

  if (fromPort_ && toPort_)
    setColor(fromPort_->color());
  
  trackNodes();
}

Connection::~Connection()
{
  if (fromPort_ && toPort_)
  {
    fromPort_->removeConnection(this);
    fromPort_->turn_off_light();
    toPort_->removeConnection(this);
    toPort_->turn_off_light();
  }
  Logger::Instance->log("Connection deleted.");
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
  if (fromPort_ && toPort_)
  {
    setLine(QLineF(fromPort_->position(), toPort_->position()));
  }
  else
    throw std::logic_error("no from/to set for Connection");
}

ConnectionInProgress::ConnectionInProgress(Port* port)
  : fromPort_(port)
{
  setZValue(1000);

  setColor(port->color());
}

void ConnectionInProgress::setColor(const QColor& color)
{
  setPen(QPen(color, 5.0, Qt::DashLine));
}

QColor ConnectionInProgress::color() const
{
  return pen().color();
}

void ConnectionInProgress::update(const QPointF& end)
{
  setLine(QLineF(fromPort_->position(), end));
}
