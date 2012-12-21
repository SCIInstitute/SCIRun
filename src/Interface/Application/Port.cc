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
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/PositionProvider.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/ClosestPortFinder.h>

using namespace SCIRun::Gui;

QGraphicsScene* PortWidget::TheScene = 0;
std::map<PortWidget::Key, PortWidget*> PortWidget::portWidgetMap_;

PortWidget::PortWidget(const QString& name, const QColor& color, const QString& moduleId, size_t index,
  bool isInput, boost::shared_ptr<ConnectionFactory> connectionFactory, QWidget* parent /* = 0 */)
  : QWidget(parent), 
  name_(name), color_(color), moduleId_(moduleId), index_(index), isInput_(isInput), isConnected_(false), lightOn_(false), currentConnection_(0),
  connectionFactory_(connectionFactory)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setAcceptDrops(true);
  setToolTip(name_);
  portWidgetMap_[boost::make_tuple(moduleId_.toStdString(), index_, isInput_)] = this;
}

PortWidget::~PortWidget()
{
  portWidgetMap_[boost::make_tuple(moduleId_.toStdString(), index_, isInput_)] = 0;
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
        p_->cancelConnectionsInProgress();
      }
      PortWidget* p_;
    };
  }
}

void PortWidget::cancelConnectionsInProgress()
{
  delete currentConnection_;
  currentConnection_ = 0;
}

void PortWidget::doMouseRelease(Qt::MouseButton button, const QPointF& pos)
{
  if (button == Qt::LeftButton && !isConnected())
  {
    toggleLight();
    update();

    if (currentConnection_)
    {
      makeConnection(pos);
    }
  }
}

void PortWidget::makeConnection(const QPointF& pos)
{
  //std::cout << "-----------makeConnection" << std::endl;
  DeleteCurrentConnectionAtEndOfBlock deleter(this);

  ClosestPortFinder finder(TheScene);
  auto port = finder.closestPort(pos);
  if (port)
    tryConnectPort(pos, port);

  //else
  //  std::cout << "null port from ClosestPortFinder" << std::endl;
}

bool PortWidget::tryConnectPort(const QPointF& pos, PortWidget* port)
{
  //std::cout << "tryConnectPort" << std::endl;
  int distance = (pos - port->position()).manhattanLength();
  //std::cout << "distance: " << distance << std::endl;
  if (distance <= PORT_CONNECTION_THRESHOLD)
  {
    if (canBeConnected(port))
    {
      PortWidget* out = this->isInput() ? port : this;
      PortWidget* in = this->isInput() ? this : port;

      SCIRun::Dataflow::Networks::ConnectionDescription cd(
        SCIRun::Dataflow::Networks::OutgoingConnectionDescription(out->moduleId_.toStdString(), out->index_), 
        SCIRun::Dataflow::Networks::IncomingConnectionDescription(in->moduleId_.toStdString(), in->index_));

      Q_EMIT needConnection(cd);

      return true;
    }
    else
    {
      GuiLogger::Instance().log("input port is full, or ports are different datatype or same i/o type: should not be connected.  this message should come from the domain layer!");
    }
  }
  //std::cout << "---tryConnectPort returns false" << std::endl;
  return false;
}

void PortWidget::MakeTheConnection(const SCIRun::Dataflow::Networks::ConnectionDescription& cd) 
{
  if (matches(cd))
  {
    PortWidget* out = portWidgetMap_[boost::make_tuple(cd.out_.moduleId_, cd.out_.port_, false)];
    PortWidget* in = portWidgetMap_[boost::make_tuple(cd.in_.moduleId_, cd.in_.port_, true)];
    SCIRun::Dataflow::Networks::ConnectionId id = SCIRun::Dataflow::Networks::ConnectionId::create(cd);
    ConnectionLine* c = connectionFactory_->makeFinishedConnection(out, in, id);
    connect(c, SIGNAL(deleted(const SCIRun::Dataflow::Networks::ConnectionId&)), this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)));
  }
}

bool PortWidget::matches(const SCIRun::Dataflow::Networks::ConnectionDescription& cd) const
{
  return (isInput() && cd.in_.moduleId_ == moduleId_.toStdString() && cd.in_.port_ == index_)
    || (!isInput() && cd.out_.moduleId_ == moduleId_.toStdString() && cd.out_.port_ == index_);
}

//TODO: extract this class as a collaborator, change PortWidget to PortDescriptionInterface, and unit test in a lower layer
class PortConnectionDeterminer
{
public:
  bool canBeConnected(const PortWidget& port1, const PortWidget& port2) const
  {
    //std::cout << "in PortConnectionDeterminer::canBeConnected" << std::endl;
    if (port1.isFullInputPort() || port2.isFullInputPort())
    {
      //std::cout << "can't connect since input ports can only take one connection" << std::endl;
      return false;
    }
    if (port1.color() != port2.color())
    {
      //std::cout << "can't connect since colors don't match" << std::endl;
      return false;
    }
    if (port1.isInput() == port2.isInput())
    {
      //std::cout << "can't connect since input/output not compatible" << std::endl;
      return false;
    }
    if (port1.sharesParentModule(port2))
    {
      //std::cout << "can't connect since it's the same module" << std::endl;
      return false;
    }
    return true;
  }
};

bool PortWidget::sharesParentModule(const PortWidget& other) const
{
  return moduleId_ == other.moduleId_;
}

bool PortWidget::isFullInputPort() const
{
  return isInput() && !connections_.empty();
}

//TODO: push this verification down to the domain layer!  make this layer as dumb as possible
bool PortWidget::canBeConnected(PortWidget* other) const
{
  //std::cout << "in PortWidget::canBeConnected" << std::endl;
  if (!other)
    return false;

  PortConnectionDeterminer q;
  return q.canBeConnected(*this, *other);
}

void PortWidget::performDrag(const QPointF& endPos)
{
  if (!currentConnection_)
  {
    currentConnection_ = connectionFactory_->makeConnectionInProgress(this);
  }
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
  qDeleteAll(connections_);
  connections_.clear();
}

void PortWidget::trackConnections()
{
  Q_FOREACH (ConnectionLine* c, connections_)
    c->trackNodes();
}

QPointF PortWidget::position() const
{
  if (positionProvider_)
    return positionProvider_->currentPosition();
  return pos();
}

InputPortWidget::InputPortWidget(const QString& name, const QColor& color, const QString& moduleId, size_t index, boost::shared_ptr<ConnectionFactory> connectionFactory, QWidget* parent /* = 0 */)
  : PortWidget(name, color, moduleId, index, true, connectionFactory, parent)
{
}

OutputPortWidget::OutputPortWidget(const QString& name, const QColor& color, const QString& moduleId, size_t index, boost::shared_ptr<ConnectionFactory> connectionFactory, QWidget* parent /* = 0 */)
  : PortWidget(name, color, moduleId, index, false, connectionFactory, parent)
{
}
