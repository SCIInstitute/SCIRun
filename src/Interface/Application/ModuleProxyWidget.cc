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

#include <QtGui>
#include <iostream>
#include <Dataflow/Network/ModuleDescription.h>
#include <Interface/Application/ModuleProxyWidget.h>
#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/PositionProvider.h>
#include <Interface/Application/PortWidgetManager.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;

namespace SCIRun
{
  namespace Gui
  {
    class ModuleWidgetNoteDisplayStrategy : public NoteDisplayStrategy
    {
    public:
      virtual QPointF relativeNotePosition(QGraphicsItem* item, const QGraphicsTextItem* note, NotePosition position) const
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
  pressedSubWidget_(0)
{
  setWidget(module);
  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
  setAcceptDrops(true);

  connect(module, SIGNAL(noteUpdated(const Note&)), this, SLOT(updateNote(const Note&)));
}

ModuleProxyWidget::~ModuleProxyWidget()
{
}

void ModuleProxyWidget::updatePressedSubWidget(QGraphicsSceneMouseEvent* event)
{
  pressedSubWidget_ = widget()->childAt(event->pos().toPoint());
}

ModuleWidget* ModuleProxyWidget::getModuleWidget()
{
  return module_;
}

void ModuleProxyWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  updatePressedSubWidget(event);

  if (PortWidget* p = qobject_cast<PortWidget*>(pressedSubWidget_))
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

void ModuleProxyWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if (PortWidget* p = qobject_cast<PortWidget*>(pressedSubWidget_))
  {
    p->doMouseRelease(event->button(), mapToScene(event->pos()));
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
      Q_EMIT widgetMoved(ModuleId(module_->getModuleId()), pos().x(), pos().y());
    }
    QGraphicsItem::mouseReleaseEvent(event);
  }
  grabbedByWidget_ = false;
}

void ModuleProxyWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  if (PortWidget* p = qobject_cast<PortWidget*>(pressedSubWidget_))
  {
    p->doMouseMove(event->buttons(), mapToScene(event->pos()));
    return;
  }
  if (grabbedByWidget_)
  {
    return;
  }
  QGraphicsItem::mouseMoveEvent(event);
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
}

void ModuleProxyWidget::setAsWaiting()
{
  module_->setColorAsWaiting();
}

QVariant ModuleProxyWidget::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == ItemPositionHasChanged)
  {
    module_->trackConnections();
    updateNotePosition();
  }
  return QGraphicsItem::itemChange(change, value);
}
void ModuleProxyWidget::createPortPositionProviders()
{
  //std::cout << "create PPPs" << std::endl;
  const int firstPortXPos = 5;
  Q_FOREACH(PortWidget* p, module_->ports().getAllPorts())
  {
    //std::cout << "Setting position provider for port " << p->id() << " at index " << p->getIndex() << " to " << firstPortXPos + (static_cast<int>(p->getIndex()) * (PortWidget::WIDTH + ModuleWidget::PORT_SPACING)) << "," << p->pos().y() << std::endl;
    QPoint realPosition(firstPortXPos + (static_cast<int>(p->getIndex()) * (PortWidget::WIDTH + ModuleWidget::PORT_SPACING)), p->pos().y());
    
    boost::shared_ptr<PositionProvider> pp(new ProxyWidgetPosition(this, realPosition + QPointF(5,5)));
    p->setPositionObject(pp);
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
