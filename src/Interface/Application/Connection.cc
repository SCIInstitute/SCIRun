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
#include <boost/bind.hpp>
#include <Interface/Application/Connection.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/GuiLogger.h>

using namespace SCIRun::Gui;

//template <class Base>
//class ConnectionGraphicsItem : public Base
//{
//public:
//  ConnectionGraphicsItem();
//  ~ConnectionGraphicsItem();
//  void setColor(const QColor& color);
//  QColor color() const;
//  virtual void destroy() = 0;
//
//};

//template <class Base>
//ConnectionGraphicsItem<Base>::ConnectionGraphicsItem()
//{
//  setFlags(QGraphicsItem::ItemIsSelectable);
//  //TODO: need dynamic zValue
//  setZValue(100); 
//  setToolTip("Left - Highlight*\nDouble-Left - Menu");
//}

//template <class Base>
//ConnectionGraphicsItem<Base>::~ConnectionGraphicsItem()
//{
//  std::cout << "doh--need to call a virtual function in a dtor..." << std::endl;
//}

ConnectionDrawStrategy::~ConnectionDrawStrategy() 
{
  //std::cout << "~ConnectionDrawStrategy" << std::endl;
}

class EuclideanDrawStrategy : public ConnectionDrawStrategy
{
public:
  //explicit EuclideanDrawStrategy(QGraphicsScene* scene) : ConnectionDrawStrategy(scene)
  //{
  //  //std::cout << "ctor Euclidean" << std::endl;
  //  scene_->addItem(this);
  //  setVisible(true);
  //}

  //~EuclideanDrawStrategy()
  //{
  //  //std::cout << "~EuclideanDrawStrategy" << std::endl;
  //  scene_->removeItem(this);
  //}

  //void destroy()
  //{
  //  Q_EMIT connectionDeleted();
  //  //scene_->removeItem(this);
  //}
  
  //void setColor(const QColor& color)
  //{
  //  ConnectionGraphicsItem<QGraphicsLineItem>::setColor(color);
  //}

  void draw(QGraphicsPathItem* item, const QPointF& from, const QPointF& to)
  {
    //std::cout << "Euclidean draw: from " << from.x() << "," << from.y() << " to " << to.x() << "," << to.y() << std::endl;
    QPainterPath path;
    path.moveTo(from);
    path.lineTo(to);
    item->setPath(path);
  }
};

class CubicBezierDrawStrategy : public ConnectionDrawStrategy//, public ConnectionGraphicsItem<QGraphicsPathItem>
{
public:
  //explicit CubicBezierDrawStrategy(QGraphicsScene* scene) : ConnectionDrawStrategy(scene)
  //{
  //  //std::cout << "ctor Cubic" << std::endl;
  //  scene_->addItem(this);
  //  setVisible(true);
  //}

  void draw(QGraphicsPathItem* item, const QPointF& from, const QPointF& to)
  {
    //std::cout << "Cubic draw: from " << from.x() << "," << from.y() << " to " << to.x() << "," << to.y() << std::endl;
    QPainterPath path;
    QPointF start = from;

    path.moveTo(start);
    auto mid = (to - start) / 2 + start;

    QPointF qDir(-(to-start).y() / ((double)(to-start).x()) , 1);
    double qFactor = std::min(std::abs(100.0 / qDir.x()), 80.0);
    //TODO: scale down when start close to end. need a unit test at this point.
    //qFactor /= (end-start).manhattanDistance()

    auto q1 = item->mapToScene(mid + qFactor * qDir);
    auto q2 = item->mapToScene(mid - qFactor * qDir);
    path.cubicTo(q1, q2, to);  
    item->setPath(path);
  }
};

ConnectionLine::ConnectionLine(PortWidget* fromPort, PortWidget* toPort, const SCIRun::Dataflow::Networks::ConnectionId& id, ConnectionDrawStrategyPtr drawer)
  : fromPort_(fromPort), toPort_(toPort), id_(id), destroyed_(false), drawer_(drawer)
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

  setFlags(QGraphicsItem::ItemIsSelectable);
  //TODO: need dynamic zValue
  setZValue(100); 
  setToolTip("Left - Highlight*\nDouble-Left - Menu");

  trackNodes();
  GuiLogger::Instance().log("Connection made.");
}

ConnectionLine::~ConnectionLine()
{
  //std::cout << "~ConnectionLine" << std::endl;
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
  drawer_.reset();
  Q_EMIT deleted(id_);
  GuiLogger::Instance().log("Connection deleted.");
  destroyed_ = true;
}

void ConnectionLine::setColor(const QColor& color)
{
  setPen(QPen(color, 5.0));
}

QColor ConnectionLine::color() const
{
  return pen().color();
}

void ConnectionLine::trackNodes()
{
  if (fromPort_ && toPort_)
  {
    //std::cout << "ConnectionLine::trackNodes" << std::endl;
    drawer_->draw(this, fromPort_->position(), toPort_->position());
  }
  else
    BOOST_THROW_EXCEPTION(InvalidConnection() << Core::ErrorMessage("no from/to set for Connection"));
}

void ConnectionLine::setDrawStrategy(ConnectionDrawStrategyPtr cds)
{
  //std::cout << "ConnectionLine::setDrawStrategy" << std::endl;
  if (!destroyed_)
  {
    drawer_ = cds;
    //connect(drawer_.get(), SIGNAL(connectionDeleted()), this, SLOT(destroy()));
    //drawer_->setColor(fromPort_->color());
    trackNodes();
  }
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

void ConnectionLine::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  ConnectionMenu menu;
  auto a = menu.exec(event->screenPos());
  if (a && a->text() == deleteAction)
  {
    destroy();
  }
}

ConnectionInProgressStraight::ConnectionInProgressStraight(PortWidget* port)
  : ConnectionInProgressGraphicsItem<QGraphicsLineItem>(port)
{
}

void ConnectionInProgressStraight::update(const QPointF& end)
{
  //TODO: use strategy object. probably need to improve first parameter.
  setLine(QLineF(fromPort_->position(), end));
}

ConnectionInProgressCurved::ConnectionInProgressCurved(PortWidget* port)
  : ConnectionInProgressGraphicsItem<QGraphicsPathItem>(port)
{
}

void ConnectionInProgressCurved::update(const QPointF& end)
{
  //TODO: use strategy object. probably need to improve first parameter.
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

ConnectionFactory::ConnectionFactory(QGraphicsScene* scene) : currentType_(EUCLIDEAN), scene_(scene), euclidean_(new EuclideanDrawStrategy), cubic_(new CubicBezierDrawStrategy) {}

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

void ConnectionFactory::setType(ConnectionDrawType type)
{
  if (type != currentType_)
  {
    //std::cout << "Factory::setType " << type << std::endl;
    currentType_ = type;
    Q_EMIT typeChanged(getCurrentDrawer());
  }
}

ConnectionDrawStrategyPtr ConnectionFactory::getCurrentDrawer() const
{
  switch (currentType_)
  {
  case EUCLIDEAN:
    return euclidean_;
  case CUBIC:
    return cubic_;
  case MANHATTAN:
    std::cout << "Manhattan connections not implemented yet." << std::endl;
    return ConnectionDrawStrategyPtr();
  default:
    std::cerr << "Unknown connection type." << std::endl;
    return ConnectionDrawStrategyPtr();
  }
}

ConnectionLine* ConnectionFactory::makeFinishedConnection(PortWidget* fromPort, PortWidget* toPort, const SCIRun::Dataflow::Networks::ConnectionId& id) const
{
  auto c = new ConnectionLine(fromPort, toPort, id, getCurrentDrawer());
  activate(c);
  connect(this, SIGNAL(typeChanged(ConnectionDrawStrategyPtr)), c, SLOT(setDrawStrategy(ConnectionDrawStrategyPtr)));
  //connect(draw.get(), SIGNAL(connectionDeleted()), c, SLOT(destroy()));
  return c;
}