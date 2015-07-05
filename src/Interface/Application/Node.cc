//TODO: not sure if anything useful left here. need to take another look

//#include <QtGui>
//
//Node::Node()
//{
//  textColor_ = Qt::darkGreen;
//  outlineColor_ = Qt::darkBlue;
//  backgroundColor_ = Qt::white;
//
//  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
//}
//
//Node::~Node()
//{
//  //foreach (Link* link, links_)
//  //  delete link;
//}
//
//void Node::setText(const QString& text)
//{
//  prepareGeometryChange();
//  text_ = text;
//  update();
//}
//
//QString Node::text() const
//{
//  return text_;
//}
//
//void Node::setTextColor(const QColor& color)
//{
//  textColor_ = color;
//  update();
//}
//
//QColor Node::textColor() const
//{
//  return textColor_;
//}
//
//void Node::setOutlineColor(const QColor& color)
//{
//  outlineColor_ = color;
//  update();
//}
//
//QColor Node::outlineColor() const
//{
//  return outlineColor_;
//}
//
//void Node::setBackgroundColor(const QColor& color)
//{
//  backgroundColor_ = color;
//  update();
//}
//
//QColor Node::backgroundColor() const
//{
//  return backgroundColor_;
//}
//
////void Node::addLink(Link* link)
////{
////  links_.insert(link);
////}
////
////void Node::removeLink(Link* link)
////{
////  links_.remove(link);
////}
//
//QRectF Node::outlineRect() const
//{
//  const int Padding = 8;
//  QFontMetricsF metrics = qApp->font();
//  QRectF rect = metrics.boundingRect(text_);
//  rect.adjust(-Padding, -Padding, +Padding, +Padding);
//  rect.translate(-rect.center());
//  return rect;
//}
//
//QRectF Node::boundingRect() const 
//{
//  const int Margin = 1;
//  return outlineRect().adjusted(-Margin, -Margin, +Margin, +Margin);
//}
//
//QPainterPath Node::shape() const 
//{
//  QRectF rect = outlineRect();
//  QPainterPath path;
//  path.addRoundRect(rect, roundness(rect.width()), roundness(rect.height()));
//  return path;
//}
//
//void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
//{
//  QPen pen(outlineColor_);
//  if (option->state & QStyle::State_Selected)
//  {
//    pen.setStyle(Qt::DotLine);
//    pen.setWidth(2);
//  }
//  painter->setPen(pen);
//  painter->setBrush(backgroundColor_);
//
//  QRectF rect = outlineRect();
//  painter->drawRoundRect(rect, roundness(rect.width()), roundness(rect.height()));
//  painter->setPen(textColor_);
//  painter->drawText(rect, Qt::AlignCenter, text_);
//}
//
//QVariant Node::itemChange(GraphicsItemChange change, const QVariant& value)
//{
//  if (change == ItemPositionHasChanged)
//  {
//    //foreach (Link* link, links_)
//    //  link->trackNodes();
//  }
//  return QGraphicsItem::itemChange(change, value);
//}
//
//void Node::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
//{
//  QString text = QInputDialog::getText(event->widget(), tr("Edit Text"), tr("Enter new text:"), QLineEdit::Normal, text_);
//  if (!text.isEmpty())
//    setText(text);
//}
//
//int Node::roundness(double size) const
//{
//  const int Diameter = 12;
//  return 100 * Diameter / int(size);
//}
//
