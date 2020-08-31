/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

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


#include <Interface/qt_include.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Interface/Application/ModuleProxyWidget.h>
#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/PositionProvider.h>
#include <Interface/Application/PortWidgetManager.h>
#include <Interface/Application/NetworkEditor.h>
#include <Core/Logging/Log.h>
#include <Core/Math/MiscMath2.h>
#include <Core/Application/Preferences/Preferences.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core;

namespace SCIRun
{
  namespace Gui
  {
    class ModuleWidgetNoteDisplayStrategy : public NoteDisplayStrategy
    {
      mutable QString previousTooltip_;
    public:
      QPointF relativeNotePosition(QGraphicsItem* item, const QGraphicsTextItem* note, NotePosition position) const override
      {
        const int noteMargin = 2;
        auto noteRect = note->boundingRect();
        auto thisRect = item->boundingRect();

        switch (position)
        {
          case NotePosition::None:
          {
            break;
          }
          case NotePosition::Top:
          {
            auto noteBottomMidpoint = (noteRect.bottomRight() + noteRect.bottomLeft()) / 2;
            auto noteBottomMidpointShift = noteRect.topLeft() - noteBottomMidpoint;
            auto moduleTopHalfLength = thisRect.width() / 2;
            noteBottomMidpointShift.rx() += moduleTopHalfLength;
            noteBottomMidpointShift.ry() -= noteMargin;
            if (!previousTooltip_.isEmpty())
              item->setToolTip(previousTooltip_);
            return noteBottomMidpointShift;
          }
          case NotePosition::Bottom:
          {
            auto noteTopMidpoint = (noteRect.topRight() + noteRect.topLeft()) / 2;
            auto noteTopMidpointShift = noteRect.topLeft() - noteTopMidpoint;
            auto moduleTopHalfLength = thisRect.width() / 2;
            noteTopMidpointShift.rx() += moduleTopHalfLength;
            noteTopMidpointShift.ry() += thisRect.height() + noteMargin;
            if (!previousTooltip_.isEmpty())
              item->setToolTip(previousTooltip_);
            return noteTopMidpointShift;
          }
          case NotePosition::Left:
          {
            auto noteRightMidpoint = (noteRect.topRight() + noteRect.bottomRight()) / 2;
            auto noteRightMidpointShift = noteRect.topLeft() - noteRightMidpoint;
            auto moduleSideHalfLength = thisRect.height() / 2;
            noteRightMidpointShift.rx() -= noteMargin;
            noteRightMidpointShift.ry() += moduleSideHalfLength;
            if (!previousTooltip_.isEmpty())
              item->setToolTip(previousTooltip_);
            return noteRightMidpointShift;
          }
          case NotePosition::Right:
          {
            auto noteLeftMidpoint = (noteRect.topLeft() + noteRect.bottomLeft()) / 2;
            auto noteLeftMidpointShift = noteRect.topLeft() - noteLeftMidpoint;
            auto moduleSideHalfLength = thisRect.height() / 2;
            noteLeftMidpointShift.rx() += thisRect.width() + noteMargin;
            noteLeftMidpointShift.ry() += moduleSideHalfLength;
            if (!previousTooltip_.isEmpty())
              item->setToolTip(previousTooltip_);
            return noteLeftMidpointShift;
          }
          case NotePosition::Tooltip:
            previousTooltip_ = item->toolTip();
            item->setToolTip(note->toHtml());
            break;
          case NotePosition::Default:
            break;
        }
        return QPointF();
      }
    };
  }
}

ModuleProxyWidget::ModuleProxyWidget(ModuleWidget* module, QGraphicsItem* parent/* = 0*/)
  : QGraphicsProxyWidget(parent),
  NoteDisplayHelper(boost::make_shared<ModuleWidgetNoteDisplayStrategy>(), this),
  module_(module),
  grabbedByWidget_(false),
  isSelected_(false),
  pressedSubWidget_(nullptr),
  doHighlight_(false),
  timeLine_(nullptr)
{
  setWidget(module);
  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
  setAcceptDrops(true);
  setData(TagDataKey, NoTag);

  connect(module, SIGNAL(noteUpdated(const Note&)), this, SLOT(updateNote(const Note&)));
  connect(module, SIGNAL(requestModuleVisible()), this, SLOT(ensureThisVisible()));
  connect(module, SIGNAL(deleteMeLater()), this, SLOT(deleteLater()));
  connect(module, SIGNAL(executionDisabled(bool)), this, SLOT(disableModuleGUI(bool)));
  connect(module, SIGNAL(findInNetwork()), this, SLOT(findInNetwork()));

  stackDepth_ = 0;

  originalSize_ = size();

  module_->setupPortSceneCollaborator(this);

  if (module_->getModule()->isImplementationDisabled())
  {
    setOpacity(0.5);
    auto colorize = new QGraphicsColorizeEffect;
    colorize->setColor(QColor(150,0,0));
    previousEffect_ = colorize;
    setGraphicsEffect(previousEffect_);
    QMessageBox::warning(nullptr, "Disabled module",
      tr("Module %1 is disabled; you might need a different build of SCIRun.").arg(QString::fromStdString(module_->getModuleId())));
  }

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "ctor" << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  setToolTip("Description: " +  QString::fromStdString(module_->getModule()->description()));

  auto oldName = module_->getModule()->legacyModuleName();
  if (module_->getModule()->name() != oldName)
    setToolTip(toolTip() + "\nConverted version of module " + QString::fromStdString(oldName));
}

ModuleProxyWidget::~ModuleProxyWidget()
{
  delete backgroundShape_;
#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~dtor" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::showAndColor(const QColor& color)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  showAndColorImpl(color, 4000);

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::showAndColorImpl(const QColor& color, int milliseconds)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  if (timeLine_)
    return;

  animateColor_ = color;
  timeLine_ = new QTimeLine(milliseconds, this);
  connect(timeLine_, SIGNAL(valueChanged(qreal)), this, SLOT(colorAnimate(qreal)));
  timeLine_->start();
  ensureThisVisible();

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::findInNetwork()
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  showAndColorImpl(Qt::white, 2000);

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::loadAnimate(qreal val)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  setOpacity(val);
  setScale(val);

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::colorAnimate(qreal val)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  if (val < 1)
  {
    auto effect = graphicsEffect();
    if (!effect)
    {
      auto colorize = new QGraphicsColorizeEffect;
      colorize->setColor(animateColor_);
      setGraphicsEffect(colorize);
    }
    else if (auto c = dynamic_cast<QGraphicsColorizeEffect*>(effect))
    {
      auto newColor = c->color();
      newColor.setAlphaF(1 - val);
      c->setColor(newColor);
    }
  }
  else // 1 = done coloring
  {
    setGraphicsEffect(nullptr);
    delete timeLine_;
    timeLine_ = nullptr;
  }
#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::adjustHeight(int delta)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  auto p = pos();
  module_->setFixedHeight(originalSize_.height() + delta);
  setMaximumHeight(originalSize_.height() + delta);

#ifdef MODULE_POSITION_LOGGING
  logCritical("{} module proxy {} SETPOS to {},{}", __LINE__, module_->getModuleId(), p.x(), p.y());
#endif

  setPos(p);

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::adjustWidth(int delta)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  auto p = pos();
  module_->setFixedWidth(originalSize_.width() + delta);
  setMaximumWidth(originalSize_.width() + delta);

#ifdef MODULE_POSITION_LOGGING
  logCritical("{} module proxy {} SETPOS to {},{}", __LINE__, module_->getModuleId(), p.x(), p.y());
#endif

  setPos(p);

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::createStartupNote()
{
  module_->createStartupNote();
}

void ModuleProxyWidget::ensureThisVisible()
{
  return; //disable for now
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  ensureItemVisible(this);

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::ensureItemVisible(QGraphicsItem* item)
{
  auto views = item->scene()->views();
  if (!views.isEmpty())
  {
    auto netEd = qobject_cast<NetworkEditor*>(views[0]);
    if (netEd && netEd->currentZoomPercentage() > 200)
    {
      return; // the call below led to a crash when too zoomed in to fit a module.
    }
    views[0]->ensureVisible(item);
  }
}

void ModuleProxyWidget::updatePressedSubWidget(QGraphicsSceneMouseEvent* event)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  pressedSubWidget_ = widget()->childAt(event->pos().toPoint());

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

ModuleWidget* ModuleProxyWidget::getModuleWidget()
{
  return module_;
}

void ModuleProxyWidget::disableModuleGUI(bool disabled)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  if (disabled)
    setGraphicsEffect(blurEffect(3));
  else
    setGraphicsEffect(nullptr);

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  clearNoteCursor();
  auto taggingOn = data(TagLayerKey).toBool();
  auto currentTag = data(CurrentTagKey).toInt();
  if (taggingOn && currentTag > NoTag)
  {
    auto thisTag = data(TagDataKey).toInt();
    auto newTag = currentTag == thisTag ? NoTag : currentTag;
    setData(TagDataKey, newTag);
    Q_EMIT tagChanged(newTag);
    return;
  }

  updatePressedSubWidget(event);

  if (auto p = qobject_cast<PortWidget*>(pressedSubWidget_))
  {
    p->doMousePress(event->button(), mapToScene(event->pos()));
    return;
  }

  if (isSubwidget(pressedSubWidget_))
  {
    QGraphicsItem::mousePressEvent(event);
    Q_EMIT selected();
    QGraphicsProxyWidget::mousePressEvent(event);
    grabbedByWidget_ = true;
  }
  else
  {
    QGraphicsItem::mousePressEvent(event);
    Q_EMIT selected();
    grabbedByWidget_ = false;
    position_ = pos();
  }
#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

static int snapTo(int oldPos)
{
  using namespace Math;
  const int strip = 76; // size of new background grid png
  const int shift = oldPos % strip;

  return oldPos - shift + (std::abs(shift) < strip/2 ? 0 : sgn(shift)*strip);
}

void ModuleProxyWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  stackDepth_ = 0;
  auto taggingOn = data(TagLayerKey).toBool();
  if (taggingOn)
    return;

  if (auto p = qobject_cast<PortWidget*>(pressedSubWidget_))
  {
    p->doMouseRelease(event->button(), mapToScene(event->pos()), event->modifiers());
    return;
  }
  if (grabbedByWidget_)
  {
    QGraphicsProxyWidget::mouseReleaseEvent(event);
  }
  else
  {
    if (position_ != pos())
    {
      snapToGrid();
      Q_EMIT widgetMoved(ModuleId(module_->getModuleId()), pos().x(), pos().y());
    }
    QGraphicsItem::mouseReleaseEvent(event);
  }
  grabbedByWidget_ = false;

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::snapToGrid()
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif
  if (Preferences::Instance().modulesSnapToGrid)
  {
    setPos(snapTo(pos().x()), snapTo(pos().y()));
  }

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  stackDepth_++;
  if (auto p = qobject_cast<PortWidget*>(pressedSubWidget_))
  {
    auto conn = p->doMouseMove(event->buttons(), mapToScene(event->pos()));
    if (conn)
    {
      if (stackDepth_ > 1)
        return;
      ensureItemVisible(conn);
    }
    stackDepth_ = 0;
    return;
  }
  if (grabbedByWidget_)
  {
    return;
  }
  if (stackDepth_ > 1)
    return;

  if (stackDepth_ == 1)
    ensureThisVisible();

  if (backgroundShape_)
    backgroundShape_->mouseMoveEventPublic(event);

  QGraphicsItem::mouseMoveEvent(event);
  stackDepth_ = 0;

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

bool ModuleProxyWidget::isSubwidget(QWidget* alienWidget) const
{
  return qobject_cast<QPushButton*>(alienWidget) ||
    qobject_cast<QToolButton*>(alienWidget) ||
    qobject_cast<QProgressBar*>(alienWidget);
}

void ModuleProxyWidget::highlightIfSelected()
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif
  if (!isSelected_ && isSelected())
  {
    module_->setColorSelected();
    isSelected_ = true;
  }
  else if (isSelected_ && !isSelected())
  {
    module_->setColorUnselected();
    isSelected_ = false;
  }
#ifdef MODULE_POSITION_LOGGING
  logCritical("{} module proxy {} when is pos set back to 0,0 {},{}", __LINE__, module_->getModuleId(), pos().x(), pos().y());
#endif

  createPortPositionProviders();

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

QVariant ModuleProxyWidget::itemChange(GraphicsItemChange change, const QVariant& value)
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
  qDebug() << "     " << change << " ?= " << ItemPositionHasChanged << "value=" << value;
#endif
  if (pos() != QPointF(0,0))
  {
    cachedPosition_ = pos();
#ifdef MODULE_POSITION_LOGGING
    logCritical("module proxy {} caching position to {},{}", module_->getModuleId(),
      cachedPosition_.x(), cachedPosition_.y());
#endif
  }
  if (change == ItemPositionHasChanged)
  {
#ifdef MODULE_POSITION_LOGGING
    qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

    module_->trackConnections();

#ifdef MODULE_POSITION_LOGGING
    qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

    updateNotePosition();

#ifdef MODULE_POSITION_LOGGING
    qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif
  }
#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  auto ret = QGraphicsItem::itemChange(change, value);

#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  return ret;
}

//TODO
LoopDiamondPolygon::LoopDiamondPolygon(QGraphicsItem* parent) : QGraphicsPolygonItem(parent)
{
  shape_ << QPointF(-100, 0) << QPointF(0, 70)
                          << QPointF(100, 0) << QPointF(0, -70)
                          << QPointF(-100, 0);
  setPolygon(shape_);

  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, false);
  //setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
  setFillRule(Qt::WindingFill);
  setPen(QPen(Qt::darkGray, 5, Qt::SolidLine));
  setBrush(Qt::darkGray);
}

void ModuleProxyWidget::setBackgroundPolygon(LoopDiamondPolygon* p)
{
  backgroundShape_ = p;
}

void ModuleProxyWidget::createPortPositionProviders()
{
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
  logCritical("module proxy {} when is pos set back to 0,0 {},{}", module_->getModuleId(),
    pos().x(), pos().y());
#endif

  const int firstPortXPos = 5;
  Q_FOREACH(PortWidget* p, module_->ports().getAllPorts())
  {
#ifdef MODULE_POSITION_LOGGING
    qDebug() << "Setting position provider for port " << QString::fromStdString(p->realId().toString()) << " at index " << p->getIndex() << " to " << firstPortXPos + (static_cast<int>(p->getIndex()) * (p->properWidth() + getModuleWidget()->portSpacing())) << "," << p->pos().y();
    qDebug() << firstPortXPos << static_cast<int>(p->getIndex()) << p->properWidth() << getModuleWidget()->portSpacing();
    qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

    QPoint realPosition(firstPortXPos + (static_cast<int>(p->getIndex()) * (p->properWidth() + getModuleWidget()->portSpacing())), p->pos().y());

    int extraPadding = p->isHighlighted() ? 4 : 0;
    auto pp(boost::make_shared<ProxyWidgetPosition>(this, realPosition + QPointF(p->properWidth() / 2 + extraPadding, 5)));

#ifdef MODULE_POSITION_LOGGING
    qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
    qDebug() << "PWP real " << realPosition + QPointF(p->properWidth() / 2 + extraPadding, 5);
#endif

    p->setPositionObject(pp);

#ifdef MODULE_POSITION_LOGGING
    qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif
  }
#ifdef MODULE_POSITION_LOGGING
  qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif

  if (pos() == QPointF(0, 0) && cachedPosition_ != pos())
  {
#ifdef MODULE_POSITION_LOGGING
    logCritical("module proxy {} setPos to cachedPosition_ {},{}", module_->getModuleId(),
      cachedPosition_.x(), cachedPosition_.y());
#endif

    setPos(cachedPosition_);

#ifdef MODULE_POSITION_LOGGING
    qDebug() << __FILE__ << __LINE__ << pos() << scenePos();
#endif
  }
#ifdef MODULE_POSITION_LOGGING
  qDebug() << "~" << __FILE__ << __LINE__ << pos() << scenePos();
#endif
}

void ModuleProxyWidget::updateNote(const Note& note)
{
  updateNoteImpl(note);
}

PassThroughPositioner::PassThroughPositioner(const QGraphicsProxyWidget* widget) : widget_(widget) {}

QPointF PassThroughPositioner::currentPosition() const
{
  return widget_->pos();
}

void ModuleProxyWidget::setDefaultNotePosition(NotePosition position)
{
  setDefaultNotePositionImpl(position);
}

void ModuleProxyWidget::setDefaultNoteSize(int size)
{
  setDefaultNoteSizeImpl(size);
  module_->setDefaultNoteFontSize(size);
}

void ModuleProxyWidget::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
#ifdef MODULE_POSITION_LOGGING
  logCritical("{} module {} hover at proxy pos {},{} scenePos {},{}", __LINE__,
    module_->getModuleId(),
    pos().x(), pos().y(), scenePos().x(), scenePos().y());
#endif

  if (doHighlight_)
  {
    module_->highlightPorts();

#ifdef MODULE_POSITION_LOGGING
    logCritical("{} module proxy {} when is pos set back to 0,0 {},{}", __LINE__, module_->getModuleId(),
       pos().x(), pos().y());
#endif

    createPortPositionProviders();

#ifdef MODULE_POSITION_LOGGING
    logCritical("{} module {} hover at proxy pos {},{} scenePos {},{}", __LINE__,
      module_->getModuleId(),
      pos().x(), pos().y(), scenePos().x(), scenePos().y());
#endif
  }

  {
    auto dropShadow = new QGraphicsDropShadowEffect;
    dropShadow->setColor(Qt::darkGray);
    dropShadow->setOffset(5, 5);
    dropShadow->setBlurRadius(30);
    {
      auto prev = graphicsEffect();
      if (auto blur = qobject_cast<QGraphicsBlurEffect*>(prev))
      {
        auto newBlur = new QGraphicsBlurEffect;
        newBlur->setBlurRadius(blur->blurRadius());
        previousEffect_ = newBlur;
      }
      else if (auto colorize = qobject_cast<QGraphicsColorizeEffect*>(prev))
      {
        auto newColorize = new QGraphicsColorizeEffect;
        newColorize->setColor(colorize->color());
        previousEffect_ = newColorize;
      }
      else
        previousEffect_ = nullptr;
    }
    setGraphicsEffect(dropShadow);
  }

  QGraphicsProxyWidget::hoverEnterEvent(event);

#ifdef MODULE_POSITION_LOGGING
  logCritical("{} module {} hover at proxy pos {},{} scenePos {},{}", __LINE__,
    module_->getModuleId(),
    pos().x(), pos().y(), scenePos().x(), scenePos().y());
#endif
}

void ModuleProxyWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
#ifdef MODULE_POSITION_LOGGING
  logCritical("{} module {} hoverLeave at proxy pos {},{} scenePos {},{}", __LINE__,
    module_->getModuleId(),
    pos().x(), pos().y(), scenePos().x(), scenePos().y());
#endif
  if (doHighlight_)
  {
    module_->unhighlightPorts();

#ifdef MODULE_POSITION_LOGGING
    logCritical("{} module proxy {} when is pos set back to 0,0 {},{}", __LINE__, module_->getModuleId(),
      pos().x(), pos().y());
#endif

    createPortPositionProviders();
  }

#ifdef MODULE_POSITION_LOGGING
  logCritical("{} module {} hoverLeave at proxy pos {},{} scenePos {},{}", __LINE__,
    module_->getModuleId(),
    pos().x(), pos().y(), scenePos().x(), scenePos().y());
#endif

  setGraphicsEffect(previousEffect_);

#ifdef MODULE_POSITION_LOGGING
  logCritical("{} module {} hoverLeave at proxy pos {},{} scenePos {},{}", __LINE__,
    module_->getModuleId(),
    pos().x(), pos().y(), scenePos().x(), scenePos().y());
#endif

  QGraphicsProxyWidget::hoverLeaveEvent(event);

#ifdef MODULE_POSITION_LOGGING
  logCritical("{} module {} hoverLeave at proxy pos {},{} scenePos {},{}", __LINE__,
    module_->getModuleId(),
    pos().x(), pos().y(), scenePos().x(), scenePos().y());
#endif
}

void ModuleProxyWidget::highlightPorts(int state)
{
  doHighlight_ = state != 0;
}

ProxyWidgetPosition::ProxyWidgetPosition(QGraphicsProxyWidget* widget, const QPointF& offset/* = QPointF()*/) : widget_(widget), offset_(offset)
{
}

QPointF ProxyWidgetPosition::currentPosition() const
{
  return widget_->pos() + offset_;
}

SubnetPortsBridgeProxyWidget::SubnetPortsBridgeProxyWidget(SubnetPortsBridgeWidget* ports, QGraphicsItem* parent) : QGraphicsProxyWidget(parent), ports_(ports)
{
}

void SubnetPortsBridgeProxyWidget::updateConnections()
{
  for (auto& port : ports_->ports())
  {
    port->trackConnections();
  }
}
