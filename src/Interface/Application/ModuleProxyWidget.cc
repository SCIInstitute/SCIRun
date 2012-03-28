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
#include "ModuleProxyWidget.h"
#include "Module.h"

using namespace SCIRun::Gui;

class ModuleProxyWidgetPosition : public PositionProvider
{
public:
  explicit ModuleProxyWidgetPosition(ModuleProxyWidget* widget) : widget_(widget) {}
  virtual QPointF currentPosition() const
  {
    return widget_->pos();
  }
private:
  ModuleProxyWidget* widget_;
};

ModuleProxyWidget::ModuleProxyWidget(Module* module, QGraphicsItem* parent/* = 0*/)
  : QGraphicsProxyWidget(parent),
  module_(module),
  grabbedByWidget_(false)
  //,
  //backupZ_(zValue())
{
  setWidget(module);
  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
  module_->setPositionObject(new ModuleProxyWidgetPosition(this));
}

void ModuleProxyWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QPointF pos = event->pos();
  QWidget* alienWidget = widget()->childAt(pos.toPoint());
  if (isSubwidget(alienWidget))
  {
    QGraphicsProxyWidget::mousePressEvent(event);
    grabbedByWidget_ = true;
  }
  else
  {
    QGraphicsItem::mousePressEvent(event);
    grabbedByWidget_ = false;
    emit selected();
  }
}

void ModuleProxyWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if (grabbedByWidget_)
    QGraphicsProxyWidget::mouseReleaseEvent(event);
  else
  {
    QGraphicsItem::mouseReleaseEvent(event);
    //setZValue(backupZ_);
  }
  grabbedByWidget_ = false;
}

void ModuleProxyWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  if (grabbedByWidget_)
    return;
  QGraphicsItem::mouseMoveEvent(event);
}

bool ModuleProxyWidget::isSubwidget(QWidget* alienWidget) const
{
  return qobject_cast<QPushButton*>(alienWidget) || qobject_cast<QToolButton*>(alienWidget) || qobject_cast<QProgressBar*>(alienWidget);
}

void ModuleProxyWidget::highlightIfSelected()
{
  if (isSelected())
    module_->setStyleSheet("background-color: cyan;");
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