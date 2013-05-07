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

#include <QtGui>
#include <Interface/PythonTestGui/Node.h>
#include <Interface/PythonTestGui/Link.h>

#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>

Node::Node()
{
  textColor_ = Qt::darkGreen;
  outlineColor_ = Qt::darkBlue;
  backgroundColor_ = Qt::white;

  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);


  log4cpp::Category& root = log4cpp::Category::getRoot();
  
  // use of functions for logging messages
  root.error("root error");
  root.info("root info");
 
  // printf-style for logging variables
  root.warn("%d + %d == %s ?", 1, 1, "two");

  // use of streams for logging messages
  root << log4cpp::Priority::ERROR << "Streamed root error";
  root << log4cpp::Priority::INFO << "Streamed root info";
 
  // or this way:
  root.errorStream() << "Another streamed error";

}

Node::~Node()
{
  Q_FOREACH (Link* link, links_)
    delete link;
}

void Node::setText(const QString& text)
{
  prepareGeometryChange();
  text_ = text;
  update();
}

QString Node::text() const
{
  return text_;
}

void Node::setTextColor(const QColor& color)
{
  textColor_ = color;
  update();
}

QColor Node::textColor() const
{
  return textColor_;
}

void Node::setOutlineColor(const QColor& color)
{
  outlineColor_ = color;
  update();
}

QColor Node::outlineColor() const
{
  return outlineColor_;
}

void Node::setBackgroundColor(const QColor& color)
{
  backgroundColor_ = color;
  update();
}

QColor Node::backgroundColor() const
{
  return backgroundColor_;
}

void Node::addLink(Link* link)
{
  links_.insert(link);
}

void Node::removeLink(Link* link)
{
  links_.remove(link);
}

QRectF Node::outlineRect() const
{
  const int Padding = 8;
  QFontMetricsF metrics = qApp->font();
  QRectF rect = metrics.boundingRect(text_);
  rect.adjust(-Padding, -Padding, +Padding, +Padding);
  rect.translate(-rect.center());
  return rect;
}

QRectF Node::boundingRect() const 
{
  const int Margin = 1;
  return outlineRect().adjusted(-Margin, -Margin, +Margin, +Margin);
}

QPainterPath Node::shape() const 
{
  QRectF rect = outlineRect();
  QPainterPath path;
  path.addRoundRect(rect, roundness(rect.width()), roundness(rect.height()));
  return path;
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
  QPen pen(outlineColor_);
  if (option->state & QStyle::State_Selected)
  {
    pen.setStyle(Qt::DotLine);
    pen.setWidth(2);
  }
  painter->setPen(pen);
  painter->setBrush(backgroundColor_);

  QRectF rect = outlineRect();
  painter->drawRoundRect(rect, roundness(rect.width()), roundness(rect.height()));
  painter->setPen(textColor_);
  painter->drawText(rect, Qt::AlignCenter, text_);
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == ItemPositionHasChanged)
  {
    Q_FOREACH (Link* link, links_)
      link->trackNodes();
  }
  return QGraphicsItem::itemChange(change, value);
}

void Node::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
  QString text = QInputDialog::getText(event->widget(), tr("Edit Text"), tr("Enter new text:"), QLineEdit::Normal, text_);
  if (!text.isEmpty())
    setText(text);
}

int Node::roundness(double size) const
{
  const int Diameter = 12;
  return 100 * Diameter / int(size);
}

