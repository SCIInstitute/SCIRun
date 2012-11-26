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

class ConnectionDrawStrategy
{
public:
  virtual ~ConnectionDrawStrategy() {}
  virtual void draw(class ConnectionGraphicsItem* item, const QPointF& from, const QPointF& to) = 0;
};

typedef boost::shared_ptr<ConnectionDrawStrategy> ConnectionDrawStrategyPtr;

class ConnectionGraphicsItem : virtual public QGraphicsLineItem, virtual public QGraphicsPathItem
{
public:
  ConnectionGraphicsItem();
  void setColor(const QColor& color);
  QColor color() const;
  

  ConnectionDrawStrategyPtr getDrawStrategy() const { return drawer_; }
  void setDrawStrategy(ConnectionDrawStrategyPtr cds) { drawer_ = cds; }

protected:
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
  ConnectionDrawStrategyPtr drawer_;
  virtual void destroy() = 0;
};

class ConnectionLine : public QObject, public ConnectionGraphicsItem
{
  Q_OBJECT

public:
  ConnectionLine(PortWidget* fromPort, PortWidget* toPort, const SCIRun::Dataflow::Networks::ConnectionId& id);
  ~ConnectionLine();
  void trackNodes();

Q_SIGNALS:
  void deleted(const SCIRun::Dataflow::Networks::ConnectionId& id);

private:
  PortWidget* fromPort_;
  PortWidget* toPort_;
  SCIRun::Dataflow::Networks::ConnectionId id_;
  virtual void destroy();
  bool destroyed_;
};

struct InvalidConnection : virtual Core::ExceptionBase {};

class ConnectionInProgress
{
public:
  virtual ~ConnectionInProgress() {}
  virtual void update(const QPointF& end) = 0;
};

template <class Base>
class ConnectionInProgressGraphicsItem : public Base, public ConnectionInProgress
{
public:
  explicit ConnectionInProgressGraphicsItem(PortWidget* port) : fromPort_(port)
  {
    setZValue(1000); //TODO
    setColor(port->color());
  }

  void setColor(const QColor& color)
  {
    setPen(QPen(color, 5.0, Qt::DashLine));
  }

  QColor color() const
  {
    return pen().color();
  }

protected:
  PortWidget* fromPort_;
};

class ConnectionInProgressStraight : public ConnectionInProgressGraphicsItem<QGraphicsLineItem>
{
public:
  explicit ConnectionInProgressStraight(PortWidget* port);
  virtual void update(const QPointF& end);
};

class ConnectionInProgressCurved : public ConnectionInProgressGraphicsItem<QGraphicsPathItem>
{
public:
  explicit ConnectionInProgressCurved(PortWidget* port);
  virtual void update(const QPointF& end);
};

class ConnectionFactory
{
public:
  enum Type { MANHATTAN, EUCLIDEAN, CUBIC };
  explicit ConnectionFactory(QGraphicsScene* scene);
  void setType(Type type);
  ConnectionInProgress* makeConnectionInProgress(PortWidget* port) const;
private:
  Type currentType_;
  void activate(QGraphicsItem* item) const;
  QGraphicsScene* scene_;
};

}
}
#endif