/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
#include <Interface/Application/Port.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Core/Logging/Log.h>
#include <Core/Utils/Exception.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;

class EuclideanDrawStrategy : public ConnectionDrawStrategy
{
public:
  void draw(QGraphicsPathItem* item, const QPointF& from, const QPointF& to)
  {
    QPainterPath path;

		if (from.y() > to.y() - 15)
		{
				path.moveTo(from);
				path.lineTo(from.x(),from.y()+6);
				path.lineTo(to.x(), to.y()-8);
				path.lineTo(to);
				item->setPath(path);
		}
		else
		{
				path.moveTo(from);
				path.lineTo(from.x(),from.y());
				path.lineTo(to.x(), to.y());
				path.lineTo(to);
				item->setPath(path);
		}
  }
};

class CubicBezierDrawStrategy : public ConnectionDrawStrategy
{
public:
  void draw(QGraphicsPathItem* item, const QPointF& from, const QPointF& to)
  {
    QPainterPath path;
    QPointF start = from;

    path.moveTo(start);
    auto mid = (to - start) / 2 + start;

    double qDirXNum = -(to-start).y();
    double qDirXDenom = ((double)(to-start).x());
    if (0 == qDirXDenom)
      qDirXDenom = -0.0001;
    QPointF qDir(qDirXNum / qDirXDenom , 1);
    double qFactor = std::min(std::abs(100.0 / qDir.x()), 80.0);
    //TODO: scale down when start close to end. need a unit test at this point.
    //qFactor /= (end-start).manhattanDistance()

    auto q1 = item->mapToScene(mid + qFactor * qDir);
    auto q2 = item->mapToScene(mid - qFactor * qDir);
    path.cubicTo(q1, q2, to);
    item->setPath(path);
  }
};

//#define USE_IMPROVED_MANHATTAN

#ifndef USE_IMPROVED_MANHATTAN
class ManhattanDrawStrategy : public ConnectionDrawStrategy
{
public:
  void draw(QGraphicsPathItem* item, const QPointF& from, const QPointF& to)
  {
    QPainterPath path;
    path.moveTo(from);
    const int case1Threshold = 15;
    if (from.y() > to.y() - case1Threshold) // input above output
    {
      path.lineTo(from.x(), from.y() + case1Threshold);
      const int leftSideBuffer = 30;
      auto nextX = std::min(from.x() - leftSideBuffer, to.x() - leftSideBuffer); // TODO will be a function of port position
      path.lineTo(nextX, from.y() + case1Threshold); // TODO will be a function of port position
      path.lineTo(nextX, to.y() - case1Threshold);
      path.lineTo(to.x(), to.y() - case1Threshold);
      path.lineTo(to);
    }
    else // output above input
    {
      auto midY = (from.y() + to.y()) / 2;
      path.lineTo(from.x(), midY);
      path.lineTo(to.x(), midY);
      path.lineTo(to);
    }
    item->setPath(path);
  }
};

#else

class ManhattanDrawStrategy : public ConnectionDrawStrategy
{
public:
  void draw(QGraphicsPathItem* item, const QPointF& from, const QPointF& to)
  {
    QPainterPath path;
    path.moveTo(from);
    const int case1Threshold = 15;
		if (from.y() > to.y() - case1Threshold) // input above output
		{
				//option 1 uses detection collision, not perfect
				int collisions = item->collidingItems().count();
				if (collisions < 1) collisions = 1;
				path.lineTo(from.x(), from.y() + case1Threshold);
				int leftSideBuffer = collisions * 15;

				//Option 2 -> noticeably slower
				//QList<QGraphicsItem*> collidesWithConnectionLine = item->collidingItems();
				//if(!collidesWithConnectionLine.isEmpty())
				//{
				//		Q_FOREACH(QGraphicsItem* item_, collidesWithConnectionLine)
				//				if(auto w = dynamic_cast<ConnectionLine*>(item_))
				//				{
				//						leftSideBuffer = leftSideBuffer + 15;
				//				}
				//}
				if (to.x() > from.x())
						{
								leftSideBuffer = 15;
						}
				auto nextX = std::min(from.x() - leftSideBuffer, to.x() - leftSideBuffer); // TODO will be a function of port position
				path.lineTo(nextX, from.y() + case1Threshold); // TODO will be a function of port position
				path.lineTo(nextX, to.y() - case1Threshold);
				path.lineTo(to.x(), to.y() - case1Threshold);
				path.lineTo(to);
		}
    else  // output above input
    {
      auto midY = (from.y() + to.y()) / 2;
      path.lineTo(from.x(), midY);
      path.lineTo(to.x(), midY);
      path.lineTo(to);
    }
    item->setPath(path);
  }
};
#endif

namespace SCIRun
{
  namespace Gui
  {
    const QString deleteAction("Delete");
    const QString insertModuleAction("Insert Module->*");
    const QString disableEnableAction("Disable");
    const QString editNotesAction("Edit Notes...");

    class ConnectionMenu : public QMenu
    {
    public:
      ConnectionMenu(QWidget* parent = 0) : QMenu(parent)
      {
        deleteAction_ = addAction(deleteAction);
        addWidgetToExecutionDisableList(deleteAction_);
        addAction(insertModuleAction)->setDisabled(true);
        disableAction_ = addAction(disableEnableAction);
        notesAction_ = addAction(editNotesAction);
      }
      ~ConnectionMenu()
      {
        removeWidgetFromExecutionDisableList(deleteAction_);
      }
      QAction* notesAction_;
      QAction* deleteAction_;
      QAction* disableAction_;
    };
  }
}

namespace SCIRun
{
  namespace Gui
  {
    class ConnectionLineNoteDisplayStrategy : public NoteDisplayStrategy
    {
    public:
      virtual QPointF relativeNotePosition(QGraphicsItem* item, const QGraphicsTextItem* note, NotePosition position) const
      {
        return QPointF(0,0);
      }
    };
  }
}

namespace
{
  const int DEFAULT_CONNECTION_WIDTH = 3.0;
  const int HOVERED_CONNECTION_WIDTH = 5.0;
}

ConnectionLine::ConnectionLine(PortWidget* fromPort, PortWidget* toPort, const ConnectionId& id, ConnectionDrawStrategyPtr drawer)
  : HasNotes(id, false),
  NoteDisplayHelper(boost::make_shared<ConnectionLineNoteDisplayStrategy>()),
  fromPort_(fromPort), toPort_(toPort), id_(id), drawer_(drawer), destroyed_(false), menu_(nullptr), menuOpen_(0), placeHoldingWidth_(0)
{
  if (fromPort_)
  {
    fromPort_->addConnection(this);
  }
  else
    LOG_DEBUG("NULL FROM PORT: " << id_.id_ << std::endl);
  if (toPort_)
  {
    toPort_->addConnection(this);
  }
  else
    LOG_DEBUG("NULL TO PORT: " << id_.id_ << std::endl);

  if (fromPort_ && toPort_)
  {
    setColor(fromPort_->color());
	  placeHoldingColor_ = fromPort_->color();
  }

  setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges | ItemIsFocusable);

  setZValue(defaultZValue());
  setToolTip("Left - Highlight\nDouble-Left - Menu\ni - Datatype info");
  setAcceptHoverEvents(true);

  menu_ = new ConnectionMenu();
  connectNoteEditorToAction(menu_->notesAction_);
  connectUpdateNote(this);

  setPositionObject(boost::make_shared<MidpointPositionerFromPorts>(fromPort_, toPort_));

  connect(menu_->disableAction_, SIGNAL(triggered()), this, SLOT(toggleDisabled()));

  trackNodes();
  GuiLogger::Instance().logInfoStd("Connection made: " + id_.id_);
}

ConnectionLine::~ConnectionLine()
{
  destroyConnection();
}

void ConnectionLine::destroyConnection()
{
  if (!destroyed_)
  {
    delete menu_;
    if (fromPort_ && toPort_)
    {
      fromPort_->removeConnection(this);
      toPort_->removeConnection(this);
    }
    drawer_.reset();
    Q_EMIT deleted(id_);
    //GuiLogger::Instance().log("Connection deleted.");
    HasNotes::destroy();
    NoteDisplayHelper::destroy();
    destroyed_ = true;
  }
}

void ConnectionLine::toggleDisabled()
{
  setDisabled(!disabled_);
}

void ConnectionLine::setDisabled(bool disabled)
{
  disabled_ = disabled;
  qDebug() << "Disabled set to " << disabled_ << "on" << id_.id_.c_str();
  if (disabled_)
  {
    menu_->disableAction_->setText("Enable");
    setColor(Qt::gray);
    placeHoldingColor_ = Qt::gray;
    setGraphicsEffect(blurEffect(3));
  }
  else
  {
    menu_->disableAction_->setText("Disable");
    setColor(fromPort_->color());
    placeHoldingColor_ = fromPort_->color();
    setGraphicsEffect(nullptr);
  }
  toPort_->connectionDisabled(disabled_);
}

void ConnectionLine::setColor(const QColor& color)
{
  setColorAndWidth(color, DEFAULT_CONNECTION_WIDTH);
}

void ConnectionLine::setColorAndWidth(const QColor& color, int width)
{
  setPen(QPen(color, width));
}

QColor ConnectionLine::color() const
{
  return pen().color();
}

void ConnectionLine::trackNodes()
{
  if (fromPort_ && toPort_)
  {
    //qDebug() << "trackNodes";
    drawer_->draw(this, fromPort_->position(), toPort_->position());
    updateNotePosition();
    setZValue(defaultZValue());
  }
  else
    BOOST_THROW_EXCEPTION(InvalidConnection() << Core::ErrorMessage("no from/to set for Connection: " + id_.id_));
}

void ConnectionLine::setDrawStrategy(ConnectionDrawStrategyPtr cds)
{
  if (!destroyed_)
  {
    drawer_ = cds;
    trackNodes();
  }
}

double ConnectionLine::defaultZValue() const
{
  // longer the connection length, the lower the z-value should be. Just negate the length.
  return -(fromPort_->position() - toPort_->position()).manhattanLength();
}

void ConnectionLine::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  //TODO: this is a bit inconsistent, disabling for now
//  if (event->button() == Qt::MiddleButton)
//    DataInfoDialog::show(fromPort_->getPortDataDescriber(), "Connection", id_.id_);

	setColorAndWidth(placeHoldingColor_, placeHoldingWidth_);
	menuOpen_ = false;
	setZValue(defaultZValue());
  fromPort_->turn_off_light();
  toPort_->turn_off_light();
  QGraphicsPathItem::mouseReleaseEvent(event);
}

void ConnectionLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	setAcceptedMouseButtons(Qt::LeftButton);

	if (!menuOpen_)
	{
		placeHoldingColor_ = color();
    placeHoldingWidth_ = pen().width();
		setColorAndWidth(Qt::red, HOVERED_CONNECTION_WIDTH);
		setZValue(1);
    fromPort_->turn_on_light();
    toPort_->turn_on_light();
	}
  QGraphicsPathItem::mousePressEvent(event);
}

void ConnectionLine::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
  auto action = menu_->exec(event->screenPos());
  menuOpen_ = true;
  if (action && action->text() == deleteAction)
  {
    scene()->removeItem(this);
    destroyConnection(); //TODO: another place to hook up deleteLater()
  }
  else if (action && action->text() == editNotesAction)
  {
    //std::cout << "POP UP NOTES EDITOR. Done. TODO: display note." << std::endl;
  }
  QGraphicsPathItem::mouseDoubleClickEvent(event);
}

void ConnectionLine::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsPathItem::mouseMoveEvent(event);
}

QVariant ConnectionLine::itemChange(GraphicsItemChange change, const QVariant& value)
{
	//Position drag movement relative to movement in network. Otherwise CL moves relative to modules as though they are the whole scene (faster, CL moves out of modules).
	if (change == ItemPositionChange && scene())
	{
		QPointF newPos = value.toPointF();
		newPos.setX(0);
		newPos.setY(0);
		return newPos;
	}
	return QGraphicsItem::itemChange(change, value);
}

ModuleIdPair ConnectionLine::getConnectedToModuleIds() const
{
	return std::make_pair(toPort_->getUnderlyingModuleId(), fromPort_->getUnderlyingModuleId());
}

void ConnectionLine::setNoteGraphicsContext()
{
  scene_ = scene();
  item_ = this;
  positioner_ = getPositionObject();
}

void ConnectionLine::updateNote(const Note& note)
{
  updateNoteImpl(note);
}

void ConnectionLine::updateNoteFromFile(const Note& note)
{
  setCurrentNote(note, true);
  updateNote(note);
}

void ConnectionLine::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
  setColorAndWidth(color(), HOVERED_CONNECTION_WIDTH);
}

void ConnectionLine::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
  setColorAndWidth(color(), DEFAULT_CONNECTION_WIDTH);
}

namespace
{
  template <typename F>
  QString eval(F f)
  {
    return f ? QString::fromStdString(f()) : "[null info]";
  }
}

void DataInfoDialog::show(PortDataDescriber portDataDescriber, const QString& label, const std::string& id)
{
  auto info = eval(portDataDescriber);

  QMessageBox* msgBox = new QMessageBox(SCIRunMainWindow::Instance());
  msgBox->setAttribute(Qt::WA_DeleteOnClose);
  msgBox->setStandardButtons(QMessageBox::Ok);
  msgBox->setWindowTitle(label + " Data info: " + QString::fromStdString(id));
  msgBox->setText(info);
  msgBox->setModal(false);
  msgBox->show();
}

void ConnectionLine::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_I)
    DataInfoDialog::show(fromPort_->getPortDataDescriber(), "Connection", id_.id_);
}

ConnectionInProgressStraight::ConnectionInProgressStraight(PortWidget* port, ConnectionDrawStrategyPtr drawer)
  : ConnectionInProgressGraphicsItem<QGraphicsLineItem>(port, drawer)
{
}

void ConnectionInProgressStraight::update(const QPointF& end)
{
  lastEnd_ = end;
  //TODO: use strategy object. probably need to improve first parameter: templatized? or just change this case to use QGraphicsPathItem directly
  //drawStrategy_->draw(this, fromPort_->position(), end);

  setLine(QLineF(fromPort_->position(), end));
}

ConnectionInProgressCurved::ConnectionInProgressCurved(PortWidget* port, ConnectionDrawStrategyPtr drawer)
  : ConnectionInProgressGraphicsItem<QGraphicsPathItem>(port, drawer)
{
}

void ConnectionInProgressCurved::update(const QPointF& end)
{
  lastEnd_ = end;
  drawStrategy_->draw(this, fromPort_->position(), end);
}

ConnectionInProgressManhattan::ConnectionInProgressManhattan(PortWidget* port, ConnectionDrawStrategyPtr drawer)
  : ConnectionInProgressGraphicsItem<QGraphicsPathItem>(port, drawer)
{
}

void ConnectionInProgressManhattan::update(const QPointF& end)
{
  lastEnd_ = end;
  if (fromPort_->isInput())
    drawStrategy_->draw(this, end, fromPort_->position());
  else
    drawStrategy_->draw(this, fromPort_->position(), end);
}

MidpointPositionerFromPorts::MidpointPositionerFromPorts(NeedsScenePositionProvider* p1, NeedsScenePositionProvider* p2) : p1_(p1), p2_(p2)
{
  ENSURE_NOT_NULL(p1, "port1");
  ENSURE_NOT_NULL(p2, "port2");
}

QPointF MidpointPositionerFromPorts::currentPosition() const
{
  return (p1_->getPositionObject()->currentPosition() + p2_->getPositionObject()->currentPosition()) / 2;
}

ConnectionFactory::ConnectionFactory(QGraphicsScene* scene) :
  currentType_(EUCLIDEAN),
  visible_(true),
  scene_(scene),
  euclidean_(new EuclideanDrawStrategy),
  cubic_(new CubicBezierDrawStrategy),
  manhattan_(new ManhattanDrawStrategy)
{}

ConnectionInProgress* ConnectionFactory::makeConnectionInProgress(PortWidget* port) const
{
  switch (currentType_)
  {
    case EUCLIDEAN:
    {
      auto c = new ConnectionInProgressStraight(port, getCurrentDrawer());
      activate(c);
      return c;
    }
    case CUBIC:
    {
      auto c = new ConnectionInProgressCurved(port, getCurrentDrawer());
      activate(c);
      return c;
    }
    case MANHATTAN:
    {
      auto c = new ConnectionInProgressManhattan(port, getCurrentDrawer());
      activate(c);
      return c;
    }
    default:
      std::cerr << "Unknown connection type." << std::endl;
      return 0;
  }
}

ConnectionInProgress* ConnectionFactory::makePotentialConnection(PortWidget* port) const
{
  auto conn = makeConnectionInProgress(port);
  conn->makePotential();
  return conn;
}


void ConnectionFactory::activate(QGraphicsItem* item) const
{
  if (item)
  {
    if (scene_)
      scene_->addItem(item);
    item->setVisible(visible_);
  }
}

void ConnectionFactory::setType(ConnectionDrawType type)
{
  if (type != currentType_)
  {
    currentType_ = type;
    Q_EMIT typeChanged(getCurrentDrawer());
  }
}

ConnectionDrawType ConnectionFactory::getType() const
{
  return currentType_;
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
    return manhattan_;
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
  return c;
}
