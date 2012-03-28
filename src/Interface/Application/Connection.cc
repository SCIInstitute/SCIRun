#include <iostream>
#include <QtGui>
#include "Connection.h"
#include "Module.h"
#include "Utility.h"

using namespace SCIRun::Gui;

Connection::Connection(Module* fromModule, Module* toModule)
  : fromModule_(fromModule), toModule_(toModule)
{
  fromModule_->addConnection(this);
  toModule_->addConnection(this);

  setFlags(QGraphicsItem::ItemIsSelectable);
  setZValue(-1);

  setColor(Qt::darkRed);
  trackNodes();
}

Connection::~Connection()
{
  fromModule_->removeConnection(this);
  toModule_->removeConnection(this);
}

void Connection::setColor(const QColor& color)
{
  setPen(QPen(color, 5.0));
}

QColor Connection::color() const
{
  return pen().color();
}

void Connection::trackNodes()
{
  std::cout << to_string(fromModule_->pos()) << std::endl;
  setLine(QLineF(fromModule_->position(), toModule_->position()));
}