#ifndef MODULEPROXY_H
#define MODULEPROXY_H

#include <QGraphicsProxyWidget>

namespace SCIRun
{
  namespace Gui
  {

    class Module;

class ModuleProxyWidget : public QGraphicsProxyWidget
{
	Q_OBJECT
	
public:
  explicit ModuleProxyWidget(Module* module, QGraphicsItem* parent = 0);
public slots:
  void highlightIfSelected();
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  QVariant itemChange(GraphicsItemChange change, const QVariant& value);
private:
  bool isSubwidget(QWidget* alienWidget) const;
  Module* module_;
  bool grabbedByWidget_;
};

  }
}

#endif