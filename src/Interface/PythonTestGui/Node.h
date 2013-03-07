/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   
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