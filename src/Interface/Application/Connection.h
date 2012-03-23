#ifndef DIAGRAM_LINK_H
#define DIAGRAM_LINK_H

#include <QGraphicsLineItem>

class Node;

class Link : public QGraphicsLineItem
{
public:
	Link(Node* fromNode, Node* toNode);
  ~Link();

  Node* fromNode() const;
  Node* toNode() const;

  void setColor(const QColor& color);
  QColor color() const;

  void trackNodes();

private:
  Node* myFromNode_;
  Node* myToNode_;
};

#endif