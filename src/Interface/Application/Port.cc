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
#include <boost/foreach.hpp>
#include <QtGui>
#include <Dataflow/Network/Port.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/PositionProvider.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/ClosestPortFinder.h>
#include <Core/Application/Application.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;


namespace SCIRun {
  namespace Gui {

    bool portTypeMatches(const std::string& portTypeToMatch, bool isInput, const ModuleDescription& module) 
    {
      if (isInput)
        return std::find_if(module.output_ports_.begin(), module.output_ports_.end(), [&](const OutputPortDescription& out) { return out.datatype == portTypeToMatch; }) != module.output_ports_.end();
      else
        return std::find_if(module.input_ports_.begin(), module.input_ports_.end(), [&](const InputPortDescription& in) { return in.datatype == portTypeToMatch; }) != module.input_ports_.end();
    }

    void fillMenu(QMenu* menu, const ModuleDescriptionMap& moduleMap, PortWidget* parent)
    {
      const std::string& portTypeToMatch = parent->get_typename();
      bool isInput = parent->isInput();
      BOOST_FOREACH(const ModuleDescriptionMap::value_type& package, moduleMap)
      {
        const std::string& packageName = package.first;
        auto p = new QMenu(QString::fromStdString(packageName), menu);
        menu->addMenu(p);
        BOOST_FOREACH(const ModuleDescriptionMap::value_type::second_type::value_type& category, package.second)
        {
          const std::string& categoryName = category.first;
          auto c = new QMenu(QString::fromStdString(categoryName), menu);
          
          BOOST_FOREACH(const ModuleDescriptionMap::value_type::second_type::value_type::second_type::value_type& module, category.second)
          {
            if (portTypeMatches(portTypeToMatch, isInput, module.second))
            {
              const std::string& moduleName = module.first;
              auto m = new QAction(QString::fromStdString(moduleName), menu);
              QObject::connect(m, SIGNAL(triggered()), parent, SLOT(connectNewModule()));
              c->addAction(m);
            }
          }
          if (c->actions().count() > 0)
            p->addMenu(c);
          else
            delete c;
        }
        menu->addSeparator();
      }
    }


    class PortActionsMenu : public QMenu
    {
    public:
      explicit PortActionsMenu(PortWidget* parent) : QMenu("Actions", parent)
      {
        QList<QAction*> actions;
        if (!parent->isInput())
        {
          auto pc = new QAction("Port Caching", parent);
          pc->setCheckable(true);
          connect(pc, SIGNAL(triggered(bool)), parent, SLOT(portCachingChanged(bool)));
          //TODO for now: disable
          pc->setEnabled(false);
          //TODO:
          //pc->setChecked(parent->getCached())...or something
          actions.append(pc);
          actions.append(separatorAction(parent));
        }
        addActions(actions);

        auto m = new QMenu("Connect Module", parent);
        fillMenu(m, Core::Application::Instance().controller()->getAllAvailableModuleDescriptions(), parent);
        addMenu(m);
      }
      QAction* getAction(const char* name) const
      {
        BOOST_FOREACH(QAction* action, actions())
        {
          if (action->text().contains(name))
            return action;
        }
        return 0;
      }
    };
  }}

PortWidget::PortWidgetMap PortWidget::portWidgetMap_;

PortWidget::PortWidget(const QString& name, const QColor& color, const std::string& datatype, const ModuleId& moduleId, 
  const PortId& portId, size_t index,
  bool isInput, bool isDynamic,
  boost::shared_ptr<ConnectionFactory> connectionFactory,
  boost::shared_ptr<ClosestPortFinder> closestPortFinder, QWidget* parent /* = 0 */)
  : QPushButton(parent), 
  name_(name), moduleId_(moduleId), portId_(portId), index_(index), color_(color), typename_(datatype), isInput_(isInput), isDynamic_(isDynamic), isConnected_(false), lightOn_(false), currentConnection_(0),
  connectionFactory_(connectionFactory),
  closestPortFinder_(closestPortFinder),
  menu_(new PortActionsMenu(this))
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setAcceptDrops(true);
  setToolTip(QString(name_).replace("_", " ") + "[" + QString::number(portId_.id) + "] : " + typename_.c_str());

  setMenu(menu_);

  portWidgetMap_[moduleId_.id_][isInput_][portId_] = this;
}

PortWidget::~PortWidget()
{
  portWidgetMap_[moduleId_.id_][isInput_][portId_] = 0;
}

QSize PortWidget::sizeHint() const
{
  const int width = WIDTH;
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
  QColor lightColor = isLightOn() ? Qt::cyan : Qt::black;
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
  if (buttons & Qt::LeftButton && (!isConnected() || !isInput()))
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

size_t PortWidget::getIndex() const
{
  return index_;
}

PortId PortWidget::id() const
{
  return portId_;
}

void PortWidget::setIndex(size_t index)
{
  index_ = index;
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
  if (button == Qt::LeftButton)
  {
    toggleLight();
    update();

    if (currentConnection_)
    {
      makeConnection(pos);
    }
  }
  else if (button == Qt::RightButton && (!isConnected() || !isInput()))
  {
    showMenu();
  }
}

void PortWidget::makeConnection(const QPointF& pos)
{
  DeleteCurrentConnectionAtEndOfBlock deleter(this);  //GUI concern: could go away if we got a NO-CONNECT signal from service layer

  auto port = closestPortFinder_->closestPort(pos);  //GUI concern: needs unit test
  if (port)
    tryConnectPort(pos, port);
}

void PortWidget::tryConnectPort(const QPointF& pos, PortWidget* port)
{
  int distance = (pos - port->position()).manhattanLength();     //GUI concern: needs unit test
  if (distance <= PORT_CONNECTION_THRESHOLD)                 //GUI concern: needs unit test
  {
    Q_EMIT requestConnection(this, port);
  }
}

void PortWidget::MakeTheConnection(const SCIRun::Dataflow::Networks::ConnectionDescription& cd) 
{
  if (matches(cd))
  {
    auto out = portWidgetMap_[cd.out_.moduleId_][false][cd.out_.portId_];
    auto in = portWidgetMap_[cd.in_.moduleId_][true][cd.in_.portId_];
    auto id = SCIRun::Dataflow::Networks::ConnectionId::create(cd);
    auto c = connectionFactory_->makeFinishedConnection(out, in, id);
    connect(c, SIGNAL(deleted(const SCIRun::Dataflow::Networks::ConnectionId&)), this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)));
    connect(this, SIGNAL(portMoved()), c, SLOT(trackNodes()));
    setConnected(true);
  }
}

void PortWidget::moveEvent( QMoveEvent * event )
{
  QPushButton::moveEvent(event);
  Q_EMIT portMoved();
}

bool PortWidget::matches(const SCIRun::Dataflow::Networks::ConnectionDescription& cd) const
{
  return (isInput() && cd.in_.moduleId_ == moduleId_ && cd.in_.portId_ == portId_)
    || (!isInput() && cd.out_.moduleId_ == moduleId_ && cd.out_.portId_ == portId_);
}

bool PortWidget::sharesParentModule(const PortWidget& other) const
{
  return moduleId_ == other.moduleId_;
}

bool PortWidget::isFullInputPort() const
{
  return isInput() && !connections_.empty();
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
  setConnected(true);
  connections_.insert(c);
}

void PortWidget::removeConnection(ConnectionLine* c)
{
  connections_.erase(c);
  if (connections_.empty())
    setConnected(false);
}

void PortWidget::deleteConnections()
{
  Q_FOREACH (ConnectionLine* c, connections_)
    delete c;
  connections_.clear();
  setConnected(false);
}

void PortWidget::trackConnections()
{
  Q_FOREACH (ConnectionLine* c, connections_)
    c->trackNodes();
}

QPointF PortWidget::position() const
{
  if (positionProvider_)
  {
    return positionProvider_->currentPosition();
  }
  return pos();
}

size_t PortWidget::nconnections() const
{
  return connections_.size();
}

std::string PortWidget::get_typename() const
{
  return typename_;
}

std::string PortWidget::get_portname() const
{
  return name_.toStdString();
}

ModuleId PortWidget::getUnderlyingModuleId() const
{
  return moduleId_;
}


void PortWidget::portCachingChanged(bool checked)
{
  //TODO
  std::cout << "Port " << moduleId_.id_ << "::" << name().toStdString() << " Caching turned " << (checked ? "on." : "off.") << std::endl;
}

void PortWidget::connectNewModule()
{
  QAction* action = qobject_cast<QAction*>(sender());
  QString moduleToAddName = action->text();
  Q_EMIT connectNewModule(this, moduleToAddName.toStdString());
}

InputPortWidget::InputPortWidget(const QString& name, const QColor& color, const std::string& datatype,
  const ModuleId& moduleId, const PortId& portId, size_t index, bool isDynamic,
  boost::shared_ptr<ConnectionFactory> connectionFactory, 
  boost::shared_ptr<ClosestPortFinder> closestPortFinder, 
  QWidget* parent /* = 0 */)
  : PortWidget(name, color, datatype, moduleId, portId, index, true, isDynamic, connectionFactory, closestPortFinder, parent)
{
}

OutputPortWidget::OutputPortWidget(const QString& name, const QColor& color, const std::string& datatype,
  const ModuleId& moduleId, const PortId& portId, size_t index, bool isDynamic,
  boost::shared_ptr<ConnectionFactory> connectionFactory, 
  boost::shared_ptr<ClosestPortFinder> closestPortFinder, 
  QWidget* parent /* = 0 */)
  : PortWidget(name, color, datatype, moduleId, portId, index, false, isDynamic, connectionFactory, closestPortFinder, parent)
{
}
