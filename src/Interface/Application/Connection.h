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

#ifndef INTERFACE_APPLICATION_CONNECTION_H
#define INTERFACE_APPLICATION_CONNECTION_H

#include <QGraphicsLineItem>
#include <Dataflow/Network/ConnectionId.h>
#include <Core/Utils/Exception.h>

namespace SCIRun {
namespace Gui {

class PortWidget;

class ConnectionLine : public QObject, public QGraphicsLineItem
{
  Q_OBJECT

public:
  ConnectionLine(PortWidget* fromPort, PortWidget* toPort, const SCIRun::Dataflow::Networks::ConnectionId& id);
  ~ConnectionLine();

  void setColor(const QColor& color);
  QColor color() const;
  void trackNodes();
  
Q_SIGNALS:
  void deleted(const SCIRun::Dataflow::Networks::ConnectionId& id);
protected:
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
private:
  PortWidget* fromPort_;
  PortWidget* toPort_;
  SCIRun::Dataflow::Networks::ConnectionId id_;
  void destroy();
  bool destroyed_;
};

struct InvalidConnection : virtual Core::ExceptionBase {};

class ConnectionInProgress : public QGraphicsLineItem
{
public:
  explicit ConnectionInProgress(PortWidget* port);

  void setColor(const QColor& color);
  QColor color() const;

  void update(const QPointF& end);

private:
  PortWidget* fromPort_;
};

}
}
#endif