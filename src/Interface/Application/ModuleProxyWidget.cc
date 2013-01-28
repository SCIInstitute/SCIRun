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
#include <boost/range/join.hpp>
#include <iostream>
#include <Interface/Application/ModuleProxyWidget.h>
#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/PositionProvider.h>

using namespace SCIRun::Gui;

ModuleProxyWidget::ModuleProxyWidget(ModuleWidget* module, QGraphicsItem* parent/* = 0*/)
  : QGraphicsProxyWidget(parent),
  module_(module),
  grabbedByWidget_(false),
  pressedSubWidget_(0)
{
  setWidget(module);
  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
  boost::shared_ptr<PositionProvider> pp(new ProxyWidgetPosition(this));
  module_->setPositionObject(pp);
  setAcceptDrops(true);
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
    QGraphicsProxyWidget::mousePressEvent(event);
    grabbedByWidget_ = true;
  }
  else
  {
    QGraphicsItem::mousePressEvent(event);
    grabbedByWidget_ = false;
    position_ = pos();
    Q_EMIT selected();
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
      Q_EMIT widgetMoved();
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
  if (isSelected())
    module_->setStyleSheet("background-color: lightblue;");
  else
    module_->setStyleSheet("background-color: lightgray;");
}

QVariant ModuleProxyWidget::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == ItemPositionHasChanged)
  {
    module_->trackConnections();
  }
  return QGraphicsItem::itemChange(change, value);
}

void ModuleProxyWidget::createPortPositionProviders()
{
  Q_FOREACH(PortWidget* p, boost::join(module_->getInputPorts(), module_->getOutputPorts()))
  {
    boost::shared_ptr<PositionProvider> pp(new ProxyWidgetPosition(this, p->pos() - module_->pos() + QPointF(5,5)));
    p->setPositionObject(pp);
  }
}
