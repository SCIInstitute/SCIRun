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
#include <Interface/Application/Port.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/PositionProvider.h>
#include <Interface/Application/Logger.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/ModuleProxyWidget.h>
#include <Interface/Application/Module.h>

using namespace SCIRun::Gui;

QGraphicsScene* PortWidget::TheScene = 0;

PortWidget::PortWidget(const QString& name, const QColor& color, bool isInput, QWidget* parent /* = 0 */)
  : QWidget(parent), 
  name_(name), color_(color), isInput_(isInput), isConnected_(false), lightOn_(false), currentConnection_(0),
  moduleParent_(parent)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setAcceptDrops(true);
}

QSize PortWidget::sizeHint() const
{
  const int width = 11;
  const int coloredHeight = isInput() ? 5 : 4;
  const int blackHeight = 2;
  return QSize(width, coloredHeight + blackHeight);
}

void PortWidget::toggleLight()
{
  lightOn_ = !lightOn_;
}

void PortWidget::turn_off_light()
{
  lightOn_ = false;
}

void PortWidget::turn_on_light()
{
  lightOn_ = true;
}

void PortWidget::paintEvent(QPaintEvent* event)
{
  QSize size = sizeHint();
  QPainter painter(this);
  painter.fillRect(QRect(QPoint(), size), color());
  QPoint lightStart = isInput() ? QPoint(0,5) : QPoint(0,0);
  QColor lightColor = isLightOn() ? Qt::red : Qt::black;
  painter.fillRect(QRect(lightStart, QSize(size.width(), 2)), lightColor);
}

void PortWidget::mousePressEvent(QMouseEvent* event)
{
  doMousePress(event->button(), event->pos());
}

void PortWidget::doMousePress(Qt::MouseButton button, const QPointF& pos)
{
  if (button == Qt::LeftButton && !isConnected())
  {
    toggleLight();
    startPos_ = pos;
    update();
  }
}

void PortWidget::mouseMoveEvent(QMouseEvent* event)
{
  doMouseMove(event->buttons(), event->pos());
}

void PortWidget::doMouseMove(Qt::MouseButtons buttons, const QPointF& pos)
{
  if (buttons & Qt::LeftButton)
  {
    int distance = (pos - startPos_).manhattanLength();
    if (distance >= QApplication::startDragDistance())
      performDrag(pos);
  }
}

void PortWidget::mouseReleaseEvent(QMouseEvent* event)
{
  doMouseRelease(event->button(), event->pos());
}

namespace
{
  const int PORT_CONNECTION_THRESHOLD = 12;
}

namespace SCIRun {
  namespace Gui {
    struct DeleteCurrentConnectionAtEndOfBlock
    {
      explicit DeleteCurrentConnectionAtEndOfBlock(PortWidget* p) : p_(p) {}
      ~DeleteCurrentConnectionAtEndOfBlock()
      {
        delete p_->currentConnection_;
        p_->currentConnection_ = 0;
      }
      PortWidget* p_;
    };
  }
}

void PortWidget::doMouseRelease(Qt::MouseButton button, const QPointF& pos)
{
  if (button == Qt::LeftButton && !isConnected())
  {
    toggleLight();
    update();

    if (currentConnection_)
    {
      DeleteCurrentConnectionAtEndOfBlock deleter(this);
      QList<QGraphicsItem*> items = TheScene->items(pos);
      foreach (QGraphicsItem* item, items)
      {
        if (item)
        {
          if (ModuleProxyWidget* mpw = dynamic_cast<ModuleProxyWidget*>(item))
          {
            ModuleWidget* overModule = mpw->getModule();
            if (overModule != moduleParent_)
            {
              foreach (PortWidget* port, overModule->ports_)
              {
                int distance = (pos - port->position()).manhattanLength();
                if (distance <= PORT_CONNECTION_THRESHOLD)
                {
                  if (canBeConnected(port))
                  {
                    Logger::Instance->log("Connection made.");

                    ConnectionLine* c = new ConnectionLine(this, port);
                    TheScene->addItem(c);
                    return;
                  }
                  else
                    std::cout << "ports are different datatype or same i/o type, should not be connected" << std::endl;
                }
              }
            }
            else
            {
              std::cout << "trying to connect a module with itself, let's not allow circular connections yet." << std::endl;
            }
          }
        }
      }
    }
  }
}

bool PortWidget::canBeConnected(PortWidget* other) const
{
  if (!other)
    return false;
  return color() == other->color() &&
    isInput() != other->isInput();
}

void PortWidget::performDrag(const QPointF& endPos)
{
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
}

void PortWidget::addConnection(ConnectionLine* c)
{
  connections_.insert(c);
}

void PortWidget::removeConnection(ConnectionLine* c)
{
  connections_.erase(c);
}

void PortWidget::deleteConnections()
{
  foreach (ConnectionLine* c, connections_)
    delete c;
  connections_.clear();
}

void PortWidget::trackConnections()
{
  foreach (ConnectionLine* c, connections_)
    c->trackNodes();
}

QPointF PortWidget::position() const
{
  if (positionProvider_)
    return positionProvider_->currentPosition();
  return pos();
}


InputPortWidget::InputPortWidget(const QString& name, const QColor& color, QWidget* parent /* = 0 */)
  : PortWidget(name, color, true, parent)
{
}

OutputPortWidget::OutputPortWidget(const QString& name, const QColor& color, QWidget* parent /* = 0 */)
  : PortWidget(name, color, false, parent)
{
}

