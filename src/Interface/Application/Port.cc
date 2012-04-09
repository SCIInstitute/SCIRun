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
#include "Port.h"
#include "Connection.h"
#include "Module.h" //for PositionProvider, please move
#include "Utility.h"

using namespace SCIRun::Gui;

QGraphicsScene* Port::TheScene = 0;

Port::Port(const QString& name, const QColor& color, bool isInput, PORT_PARENT* parent /* = 0 */)
  : PORT_BASE(parent), 
  name_(name), color_(color), isInput_(isInput), isConnected_(false), lightOn_(false), currentConnection_(0)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setAcceptDrops(true);
  
}

QSize Port::sizeHint() const
{
  const int width = 11;
  const int coloredHeight = isInput() ? 5 : 4;
  const int blackHeight = 2;
  return QSize(width, coloredHeight + blackHeight);
}

void Port::toggleLight()
{
  lightOn_ = !lightOn_;
}

//void Port::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
void Port::paintEvent(QPaintEvent* event)
{
  QSize size = sizeHint();
  QPainter painter(this);
  painter.fillRect(QRect(QPoint(), size), color());
  QPoint lightStart = isInput() ? QPoint(0,5) : QPoint(0,0);
  QColor lightColor = isLightOn() ? Qt::red : Qt::black;
  painter.fillRect(QRect(lightStart, QSize(size.width(), 2)), lightColor);
}

void Port::mousePressEvent(
  //QGraphicsSceneMouseEvent* 
  QMouseEvent* 
  event)
{
  doMousePress(event->button(), event->pos());
  std::cout << "Port mousePressEvent" << std::endl;
}

void Port::doMousePress(Qt::MouseButton button, const QPointF& pos)
{
  if (button == Qt::LeftButton && !isConnected())
  {
    toggleLight();
    startPos_ = pos;
    update();
  }
}

void Port::mouseMoveEvent(
  //QGraphicsSceneMouseEvent* 
  QMouseEvent* 
  event)
{
  doMouseMove(event->buttons(), event->pos());
  std::cout << "Port mouseMoveEvent" << std::endl;
}

void Port::doMouseMove(Qt::MouseButtons buttons, const QPointF& pos)
{
  if (buttons & Qt::LeftButton)
  {
    int distance = (pos - startPos_).manhattanLength();
    if (distance >= QApplication::startDragDistance())
      performDrag(pos);
  }
}

void Port::mouseReleaseEvent(
  //QGraphicsSceneMouseEvent* 
  QMouseEvent* 
  event)
{
  doMouseRelease(event->button(), event->pos());
  std::cout << "Port mouseReleaseEvent" << std::endl;
}

void Port::doMouseRelease(Qt::MouseButton button, const QPointF& pos)
{
  if (button == Qt::LeftButton && !isConnected())
  {
    toggleLight();
    update();

    if (currentConnection_)
    {
      /*
      QPointF pos = event->pos();
      QWidget* alienWidget = widget()->childAt(pos.toPoint());
      if (isSubwidget(alienWidget))
      */
      std::cout << "Released mouse with active current connection" << std::endl;
      std::cout << to_string(pos) << std::endl;
      //std::cout << TheScene-> << std::endl;
      
      delete currentConnection_;
      currentConnection_ = 0;
    }
  }
}

void Port::performDrag(const QPointF& endPos)
{
  //QDrag* drag = new QDrag(this);
  if (!currentConnection_)
  {
    //TODO: move into factory
    currentConnection_ = new ConnectionInProgress(this);
    if (TheScene)
      TheScene->addItem(currentConnection_);
    currentConnection_->setVisible(true);
  }
  if (TheScene)
    currentConnection_->update(endPos);

  std::cout << "Port: performing drag" << std::endl;
}

//#error these overrides need to be on the proxy again!!!.  take break to work on Domain layer.

void Port::dragEnterEvent(
  //QGraphicsSceneDragDropEvent* event
  QDragEnterEvent* event
  )
{
  if (currentConnection_)
    {
       event->accept();
      /*
      QPointF pos = event->pos();
      QWidget* alienWidget = widget()->childAt(pos.toPoint());
      if (isSubwidget(alienWidget))
      */
      std::cout << to_string(event->pos()) << std::endl;
      //std::cout << TheScene-> << std::endl;
    }
 
  std::cout << "@@@@@@@@@Port dragEnterEvent" << std::endl;
}

void Port::dragMoveEvent(
  //QGraphicsSceneDragDropEvent* event
  QDragMoveEvent* event
  )
{
  event->accept();
  std::cout << "@@@@@@@@@Port dragMoveEvent" << std::endl;
}

void Port::dropEvent(
  //QGraphicsSceneDragDropEvent* event
  QDropEvent* event
  )
{
  event->accept();
  std::cout << "@@@@@@@@@Port dropEvent" << std::endl;
}

#ifndef IS_NORMAL_WIDGET
void Port::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
  event->accept();
  std::cout << "@@@@@@@@@Port dragLeaveEvent" << std::endl;
}
#endif

void Port::addConnection(Connection* c)
{
  connections_.insert(c);
}

void Port::removeConnection(Connection* c)
{
  connections_.erase(c);
}

void Port::trackConnections()
{
  foreach (Connection* c, connections_)
    c->trackNodes();
}

QPointF Port::position() const
{
  if (positionProvider_)
    return positionProvider_->currentPosition();
  return pos();
}


InputPort::InputPort(const QString& name, const QColor& color, PORT_PARENT* parent /* = 0 */)
  : Port(name, color, true, parent)
{
}

OutputPort::OutputPort(const QString& name, const QColor& color, PORT_PARENT* parent /* = 0 */)
  : Port(name, color, false, parent)
{
}

