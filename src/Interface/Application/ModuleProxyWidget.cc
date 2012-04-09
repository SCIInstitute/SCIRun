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
#include "Port.h"
#include "Utility.h"

using namespace SCIRun::Gui;

ModuleProxyWidget::ModuleProxyWidget(Module* module, QGraphicsItem* parent/* = 0*/)
  : QGraphicsProxyWidget(parent),
  module_(module),
  grabbedByWidget_(false),
  pressedSubWidget_(0)
{
  setWidget(module);
  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
  module_->setPositionObject(new ProxyWidgetPosition(this));
  setAcceptDrops(true);
}

void ModuleProxyWidget::updatePressedSubWidget(QGraphicsSceneMouseEvent* event)
{
  pressedSubWidget_ = widget()->childAt(event->pos().toPoint());
}

Module* ModuleProxyWidget::getModule()
{
  return module_;
}

void ModuleProxyWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  updatePressedSubWidget(event);

  if (Port* p = qobject_cast<Port*>(pressedSubWidget_))
  {
    //std::cout << "! ! ! mousePressEvent for Port" << std::endl;
    //std::cout << "mousePressEvent for ModuleProxyWidget/Port: pos = " << to_string(event->pos()) << std::endl;
    //std::cout << "\t\t\t and mapToScene(pos) = " << to_string(mapToScene(event->pos())) << std::endl;
    p->doMousePress(event->button(), mapToScene(event->pos()));
    return;
  }

  if (isSubwidget(pressedSubWidget_))
  {
    //std::cout << "QGraphicsProxyWidget::mousePressEvent" << std::endl;
    QGraphicsProxyWidget::mousePressEvent(event);
    grabbedByWidget_ = true;
  }
  else
  {
    //std::cout << "QGraphicsItem::mousePressEvent" << std::endl;
    QGraphicsItem::mousePressEvent(event);
    grabbedByWidget_ = false;
    emit selected();
  }
}

void ModuleProxyWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  //updatePressedSubWidget(event);
  if (Port* p = qobject_cast<Port*>(pressedSubWidget_))
  {
    //std::cout << "! ! ! mouseReleaseEvent for Port" << std::endl;
    //std::cout << "mouseReleaseEvent for ModuleProxyWidget/Port: pos = " << to_string(event->pos()) << std::endl;
    //std::cout << "\t\t\t and mapToScene(pos) = " << to_string(mapToScene(event->pos())) << std::endl;
    p->doMouseRelease(event->button(), mapToScene(event->pos()));
    return;
  }
  if (grabbedByWidget_)
  {
    //std::cout << "QGraphicsProxyWidget::mouseReleaseEvent" << std::endl;
    QGraphicsProxyWidget::mouseReleaseEvent(event);
  }
  else
  {
    //std::cout << "QGraphicsItem::mouseReleaseEvent" << std::endl;
    QGraphicsItem::mouseReleaseEvent(event);
  }
  grabbedByWidget_ = false;
}

void ModuleProxyWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  //updatePressedSubWidget(event);
  if (Port* p = qobject_cast<Port*>(pressedSubWidget_))
  {
    //std::cout << "mouseMoveEvent for ModuleProxyWidget/Port: pos = " << to_string(event->pos()) << std::endl;
    //std::cout << "\t\t\t and mapToScene(pos) = " << to_string(mapToScene(event->pos())) << std::endl;
    p->doMouseMove(event->buttons(), mapToScene(event->pos()));
    return;
  }
  if (grabbedByWidget_)
  {
    //std::cout << "Returning from mouseMoveEvent since grabbedByWidget is true" << std::endl;
    return;
  }
  //std::cout << "QGraphicsItem::mouseMoveEvent" << std::endl;
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
  foreach(Port* p, module_->ports_)
  {
    boost::shared_ptr<PositionProvider> pp(new ProxyWidgetPosition(this, p->pos() - module_->pos() + QPointF(15,15)));
    p->setPositionObject(pp);
  }
}
