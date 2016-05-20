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

#include <QtGui>
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
    public:
      virtual QPointF relativeNotePosition(QGraphicsItem* item, const QGraphicsTextItem* note, NotePosition position) const override
      {
        const int noteMargin = 2;
        auto noteRect = note->boundingRect();
        auto thisRect = item->boundingRect();

        switch (position)
        {
        case None:
          {
            break;
          }
        case Top:
          {
            auto noteBottomMidpoint = (noteRect.bottomRight() + noteRect.bottomLeft()) / 2;
            auto noteBottomMidpointShift = noteRect.topLeft() - noteBottomMidpoint;
            auto moduleTopHalfLength = thisRect.width() / 2;
            noteBottomMidpointShift.rx() += moduleTopHalfLength;
            noteBottomMidpointShift.ry() -= noteMargin;
            return noteBottomMidpointShift;
          }
        case Bottom:
          {
            auto noteTopMidpoint = (noteRect.topRight() + noteRect.topLeft()) / 2;
            auto noteTopMidpointShift = noteRect.topLeft() - noteTopMidpoint;
            auto moduleTopHalfLength = thisRect.width() / 2;
            noteTopMidpointShift.rx() += moduleTopHalfLength;
            noteTopMidpointShift.ry() += thisRect.height() + noteMargin;
            return noteTopMidpointShift;
          }
        case Left:
          {
            auto noteRightMidpoint = (noteRect.topRight() + noteRect.bottomRight()) / 2;
            auto noteRightMidpointShift = noteRect.topLeft() - noteRightMidpoint;
            auto moduleSideHalfLength = thisRect.height() / 2;
            noteRightMidpointShift.rx() -= noteMargin;
            noteRightMidpointShift.ry() += moduleSideHalfLength;
            return noteRightMidpointShift;
          }
        case Right:
          {
            auto noteLeftMidpoint = (noteRect.topLeft() + noteRect.bottomLeft()) / 2;
            auto noteLeftMidpointShift = noteRect.topLeft() - noteLeftMidpoint;
            auto moduleSideHalfLength = thisRect.height() / 2;
            noteLeftMidpointShift.rx() += thisRect.width() + noteMargin;
            noteLeftMidpointShift.ry() += moduleSideHalfLength;
            return noteLeftMidpointShift;
          }
        case Tooltip:
          item->setToolTip(note->toHtml());
          break;
        case Default:
          break;
        }
        return QPointF();
      }
    };
  }
}

ModuleProxyWidget::ModuleProxyWidget(ModuleWidget* module, QGraphicsItem* parent/* = 0*/)
  : QGraphicsProxyWidget(parent),
  NoteDisplayHelper(boost::make_shared<ModuleWidgetNoteDisplayStrategy>()),
  module_(module),
  grabbedByWidget_(false),
  isSelected_(false),
  pressedSubWidget_(nullptr),
  doHighlight_(false)
{
  setWidget(module);
  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
  setAcceptDrops(true);
  setData(TagDataKey, NoTag);

  connect(module, SIGNAL(noteUpdated(const Note&)), this, SLOT(updateNote(const Note&)));
  connect(module, SIGNAL(requestModuleVisible()), this, SLOT(ensureThisVisible()));
  connect(module, SIGNAL(deleteMeLater()), this, SLOT(deleteLater()));
  connect(module, SIGNAL(executionDisabled(bool)), this, SLOT(disableModuleGUI(bool)));

  stackDepth_ = 0;

  originalSize_ = size();
}

ModuleProxyWidget::~ModuleProxyWidget()
{
}

void ModuleProxyWidget::adjustHeight(int delta)
{
  auto p = pos();
  module_->setFixedHeight(originalSize_.height() + delta);
  setMaximumHeight(originalSize_.height() + delta);
  setPos(p);
}

void ModuleProxyWidget::adjustWidth(int delta)
{
  auto p = pos();
  module_->setFixedWidth(originalSize_.width() + delta);
  setMaximumWidth(originalSize_.width() + delta);
  setPos(p);
}

void ModuleProxyWidget::createStartupNote()
{
  module_->createStartupNote();
}

void ModuleProxyWidget::ensureThisVisible()
{
  ensureItemVisible(this);
}

void ModuleProxyWidget::ensureItemVisible(QGraphicsItem* item)
{
  auto views = scene()->views();
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
  pressedSubWidget_ = widget()->childAt(event->pos().toPoint());
}

ModuleWidget* ModuleProxyWidget::getModuleWidget()
{
  return module_;
}

void ModuleProxyWidget::disableModuleGUI(bool disabled)
{
  if (disabled)
    setGraphicsEffect(blurEffect(3));
  else
    setGraphicsEffect(nullptr);
}

void ModuleProxyWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
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
}

void ModuleProxyWidget::snapToGrid()
{
  if (Preferences::Instance().modulesSnapToGrid)
    setPos(snapTo(pos().x()), snapTo(pos().y()));
}

void ModuleProxyWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
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
  if (stackDepth_ == 0)
    ensureThisVisible();
  QGraphicsItem::mouseMoveEvent(event);
  stackDepth_ = 0;
}

bool ModuleProxyWidget::isSubwidget(QWidget* alienWidget) const
{
  return qobject_cast<QPushButton*>(alienWidget) ||
    qobject_cast<QToolButton*>(alienWidget) ||
    qobject_cast<QProgressBar*>(alienWidget);
}

void ModuleProxyWidget::highlightIfSelected()
{
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
  createPortPositionProviders();
}

QVariant ModuleProxyWidget::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (pos() != QPointF(0,0))
    cachedPosition_ = pos();

  if (change == ItemPositionHasChanged)
  {
    module_->trackConnections();
    updateNotePosition();
  }
  return QGraphicsItem::itemChange(change, value);
}

void ModuleProxyWidget::createPortPositionProviders()
{
  const int firstPortXPos = 5;
  Q_FOREACH(PortWidget* p, module_->ports().getAllPorts())
  {
    //qDebug() << "Setting position provider for port " << QString::fromStdString(p->id().toString()) << " at index " << p->getIndex() << " to " << firstPortXPos + (static_cast<int>(p->getIndex()) * (p->properWidth() + getModuleWidget()->portSpacing())) << "," << p->pos().y();
    //qDebug() << firstPortXPos << static_cast<int>(p->getIndex()) << p->properWidth() << getModuleWidget()->portSpacing();

    QPoint realPosition(firstPortXPos + (static_cast<int>(p->getIndex()) * (p->properWidth() + getModuleWidget()->portSpacing())), p->pos().y());

    int extraPadding = p->isHighlighted() ? 4 : 0;
    boost::shared_ptr<PositionProvider> pp(new ProxyWidgetPosition(this, realPosition + QPointF(p->properWidth() / 2 + extraPadding, 5)));
    //qDebug() << "PWP real " << realPosition + QPointF(p->properWidth() / 2 + extraPadding, 5);
    p->setPositionObject(pp);
  }
  if (pos() == QPointF(0, 0) && cachedPosition_ != pos())
  {
    setPos(cachedPosition_);
  }
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

void ModuleProxyWidget::setNoteGraphicsContext()
{
  scene_ = scene();
  item_ = this;
  positioner_ = boost::make_shared<PassThroughPositioner>(this);
}

void ModuleProxyWidget::setDefaultNotePosition(NotePosition position)
{
  setDefaultNotePositionImpl(position);
}

void ModuleProxyWidget::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  if (doHighlight_)
  {
    module_->highlightPorts();
    createPortPositionProviders();
  }
  QGraphicsProxyWidget::hoverEnterEvent(event);
}

void ModuleProxyWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  if (doHighlight_)
  {
    module_->unhighlightPorts();
    createPortPositionProviders();
  }
  QGraphicsProxyWidget::hoverLeaveEvent(event);
}

void ModuleProxyWidget::highlightPorts(int state)
{
  doHighlight_ = state != 0;
}
