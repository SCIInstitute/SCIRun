#ifndef MODULE_H
#define MODULE_H

#include "ui_Module.h"
#include <QFrame>
#include <set>

namespace SCIRun {
namespace Gui {

class Connection;

class PositionProvider
{
public:
  virtual ~PositionProvider() {}
  virtual QPointF currentPosition() const = 0;
};

class Module : public QFrame, public Ui::Module
{
	Q_OBJECT
	
public:
  explicit Module(const QString& name, QWidget* parent = 0);
  ~Module();
  void addConnection(Connection* c);
  void removeConnection(Connection* c);

  void trackConnections();

  void setPositionObject(PositionProvider* provider) { positionProvider_ = provider; }

  QPointF position() const;
private:
  //TODO distinguish input/output
  std::set<Connection*> connections_;
  PositionProvider* positionProvider_;
};

}
}

#endif