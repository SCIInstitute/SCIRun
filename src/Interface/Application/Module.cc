#include <QtGui>
#include "Module.h"
#include "Connection.h"

using namespace SCIRun::Gui;

Module::Module(const QString& name, QWidget* parent /* = 0 */)
  : QFrame(parent), positionProvider_(0)
{
  setupUi(this);
  titleLabel_->setText(name);
}

Module::~Module()
{
  foreach (Connection* c, connections_)
    delete c;
}

void Module::addConnection(Connection* c)
{
  connections_.insert(c);
}

void Module::removeConnection(Connection* c)
{
  connections_.erase(c);
}

void Module::trackConnections()
{
  foreach (Connection* c, connections_)
    c->trackNodes();
}

QPointF Module::position() const 
{
  if (positionProvider_)
    return positionProvider_->currentPosition();
  return pos();
}