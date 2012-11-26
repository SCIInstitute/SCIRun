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
#include <Interface/Application/GuiLogger.h>

using namespace SCIRun::Gui;

ConnectionGraphicsItem::ConnectionGraphicsItem()
{
  QGraphicsItem::setFlags(QGraphicsItem::ItemIsSelectable);
  //TODO: need dynamic zValue
  QGraphicsItem::setZValue(100); 
  QGraphicsItem::setToolTip("Left - Highlight*\nDouble-Left - Menu");
}

class EuclideanDrawStrategy : public ConnectionDrawStrategy
{
public:
  void draw(ConnectionGraphicsItem* item, const QPointF& from, const QPointF& to)
  {
    item->setLine(QLineF(from, to));
  }
};

class CubicBezierDrawStrategy : public ConnectionDrawStrategy
{
public:
  void draw(ConnectionGraphicsItem* item, const QPointF& from, const QPointF& to)
  {
    QPainterPath path;
    QPointF start = from;

    path.moveTo(start);
    auto mid = (to - start) / 2 + start;

    QPointF qDir(-(to-start).y() / ((double)(to-start).x()) , 1);
    double qFactor = std::min(std::abs(100.0 / qDir.x()), 80.0);
    //TODO: scale down when start close to end. need a unit test at this point.
    //qFactor /= (end-start).manhattanDistance()

    auto q1 = (static_cast<QGraphicsLineItem*>(item))->mapToScene(mid + qFactor * qDir);
    auto q2 = (static_cast<QGraphicsLineItem*>(item))->mapToScene(mid - qFactor * qDir);
    path.cubicTo(q1, q2, to);  
    item->setPath(path);
  }
};

ConnectionLine::ConnectionLine(PortWidget* fromPort, PortWidget* toPort, const SCIRun::Dataflow::Networks::ConnectionId& id)
  : fromPort_(fromPort), toPort_(toPort), id_(id), destroyed_(false)
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

  if (fromPort_ && toPort_)
    setColor(fromPort_->color());
 
  ConnectionDrawStrategyPtr p(new EuclideanDrawStrategy);
  setDrawStrategy(p);

  trackNodes();
  GuiLogger::Instance().log("Connection made.");
}

ConnectionLine::~ConnectionLine()
{
  if (!destroyed_)
    destroy();
}

void ConnectionLine::destroy() 
{
  if (fromPort_ && toPort_)
  {
    fromPort_->removeConnection(this);
    fromPort_->turn_off_light();
    toPort_->removeConnection(this);
    toPort_->turn_off_light();
  }
  Q_EMIT deleted(id_);
  GuiLogger::Instance().log("Connection deleted.");
  destroyed_ = true;
}

void ConnectionGraphicsItem::setColor(const QColor& color)
{
  QGraphicsLineItem::setPen(QPen(color, 5.0));
  QGraphicsPathItem::setPen(QPen(color, 5.0));
}

QColor ConnectionGraphicsItem::color() const
{
  return QGraphicsLineItem::pen().color();
}

void ConnectionLine::trackNodes()
{
  if (fromPort_ && toPort_)
  {
    drawer_->draw(this, fromPort_->position(), toPort_->position());
  }
  else
    BOOST_THROW_EXCEPTION(InvalidConnection() << Core::ErrorMessage("no from/to set for Connection"));
}

namespace
{
  const QString deleteAction("Delete");
  const QString insertModuleAction("Insert Module->*");
  const QString disableEnableAction("Disable*");
  const QString editNotesAction("Edit Notes...*");
}

class ConnectionMenu : public QMenu
{
public:
  ConnectionMenu()
  {
    addAction(deleteAction);
    addAction(insertModuleAction)->setDisabled(true);
    addAction(disableEnableAction)->setDisabled(true);
    addAction(editNotesAction)->setDisabled(true);
  }
};

void ConnectionGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  ConnectionMenu menu;
  auto a = menu.exec(event->screenPos());
  if (a && a->text() == deleteAction)
  {
    (static_cast<QGraphicsLineItem*>(this))->scene()->removeItem((static_cast<QGraphicsLineItem*>(this)));
    destroy();
  }
}

ConnectionInProgressStraight::ConnectionInProgressStraight(PortWidget* port)
  : ConnectionInProgressGraphicsItem<QGraphicsLineItem>(port)
{
}

void ConnectionInProgressStraight::update(const QPointF& end)
{
  setLine(QLineF(fromPort_->position(), end));
}

ConnectionInProgressCurved::ConnectionInProgressCurved(PortWidget* port)
  : ConnectionInProgressGraphicsItem<QGraphicsPathItem>(port)
{
}

void ConnectionInProgressCurved::update(const QPointF& end)
{
  QPainterPath path;
  QPointF start = fromPort_->position();

  path.moveTo(start);
  auto mid = (end - start) / 2 + start;
  
  QPointF qDir(-(end-start).y() / ((double)(end-start).x()) , 1);
  double qFactor = std::min(std::abs(100.0 / qDir.x()), 80.0);
  //TODO: scale down when start close to end. need a unit test at this point.
  //qFactor /= (end-start).manhattanDistance()
  
  auto q1 = mapToScene(mid + qFactor * qDir);
  auto q2 = mapToScene(mid - qFactor * qDir);
  path.cubicTo(q1, q2, end);  
  setPath(path);
}

ConnectionFactory::ConnectionFactory(QGraphicsScene* scene) : currentType_(EUCLIDEAN), scene_(scene) {}

ConnectionInProgress* ConnectionFactory::makeConnectionInProgress(PortWidget* port) const
{
  switch (currentType_)
  {
  case EUCLIDEAN:
  {
    auto c = new ConnectionInProgressStraight(port);
    activate(c);
    return c;
  }
  case CUBIC:
  {
    auto c = new ConnectionInProgressCurved(port);
    activate(c);
    return c;
  }
  case MANHATTAN:
    std::cout << "Manhattan connections not implemented yet." << std::endl;
    return 0; //TODO
  default:
    std::cerr << "Unknown connection type." << std::endl;
    return 0;
  }
}

void ConnectionFactory::activate(QGraphicsItem* item) const
{
  if (item)
  {
    if (scene_)
      scene_->addItem(item);
    item->setVisible(true);
  }
}

void ConnectionFactory::setType(Type type)
{
  currentType_ = type;
}