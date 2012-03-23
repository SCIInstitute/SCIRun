#include <QtGui>
#include "Connection.h"
#include "Node.h"

Link::Link(Node* fromNode, Node* toNode)
{
  myFromNode_ = fromNode;
  myToNode_ = toNode;

  myFromNode_->addLink(this);
  myToNode_->addLink(this);

  setFlags(QGraphicsItem::ItemIsSelectable);
  setZValue(-1);

  setColor(Qt::darkRed);
  trackNodes();
}

Link::~Link()
{
  myFromNode_->removeLink(this);
  myToNode_->removeLink(this);
}

void Link::setColor(const QColor& color)
{
  setPen(QPen(color, 5.0));
}

QColor Link::color() const
{
  return pen().color();
}

void Link::trackNodes()
{
  setLine(QLineF(myFromNode_->pos(), myToNode_->pos()));
}