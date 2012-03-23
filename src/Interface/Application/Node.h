#ifndef DIAGRAMNODE_H
#define DIAGRAMNODE_H

#include <QGraphicsItem>

class Link;

class Node : public QGraphicsItem
{
	Q_DECLARE_TR_FUNCTIONS(Node)
	
public:
  Node();
  ~Node();
  void setText(const QString& text);
  QString text() const;
  void setTextColor(const QColor& color);
  QColor textColor() const;
  void setOutlineColor(const QColor& color);
  QColor outlineColor() const;
  void setBackgroundColor(const QColor& color);
  QColor backgroundColor() const;

  void addLink(Link* link);
  void removeLink(Link* link);

  QRectF boundingRect() const;
  QPainterPath shape() const;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

protected:
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
  QVariant itemChange(GraphicsItemChange change, const QVariant& value);

private:
  QRectF outlineRect() const;
  int roundness(double size) const;

  QSet<Link*> links_;
  QString text_;
  QColor textColor_;
  QColor backgroundColor_;
  QColor outlineColor_;
};

#endif