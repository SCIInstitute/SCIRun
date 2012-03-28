#ifndef CONNECTION_H
#define CONNECTION_H

#include <QGraphicsLineItem>

namespace SCIRun {
namespace Gui {

class Module;

class Connection : public QGraphicsLineItem
{
public:
	Connection(Module* fromNode, Module* toNode);
  ~Connection();

  Module* fromModule() const;
  Module* toModule() const;

  void setColor(const QColor& color);
  QColor color() const;

  void trackNodes();

private:
  Module* fromModule_;
  Module* toModule_;
};

}
}
#endif