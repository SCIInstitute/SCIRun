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
{
  setWidget(module);
  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
  module_->setPositionObject(new ModuleProxyWidgetPosition(this));
  //module_->setStyleSheet("background-color: lightgray;");
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
    //std::cout << module_->styleSheet().toStdString() << std::endl;
    //module_->setStyleSheet("background-color: cyan;");
    grabbedByWidget_ = false;
  }
}

void ModuleProxyWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if (grabbedByWidget_)
    QGraphicsProxyWidget::mouseReleaseEvent(event);
  else
    QGraphicsItem::mouseReleaseEvent(event);
  //module_->setStyleSheet("background-color: lightgray;");
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
    //foreach (Link* link, links_)
    //  link->trackNodes();
  }
  return QGraphicsItem::itemChange(change, value);
}