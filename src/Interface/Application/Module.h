/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#ifndef MODULE_H
#define MODULE_H

#include "ui_Module.h"
#include <QFrame>
#include <set>

class QGraphicsProxyWidget;

namespace SCIRun {
namespace Gui {

class Connection;

class PositionProvider
{
public:
  virtual ~PositionProvider() {}
  virtual QPointF currentPosition() const = 0;
};

class ProxyWidgetPosition : public PositionProvider
{
public:
  explicit ProxyWidgetPosition(QGraphicsProxyWidget* widget) : widget_(widget) {}
  virtual QPointF currentPosition() const;
private:
  QGraphicsProxyWidget* widget_;
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

  QPointF inputPortPosition() const;
  QPointF outputPortPosition() const;

  double percentComplete() const;
  void setPercentComplete(double p);

  //for testing signal/slot of Execute
public slots:
  void incrementProgressFake();
private:
  //TODO distinguish input/output
  std::set<Connection*> connections_;
  PositionProvider* positionProvider_;

  void addPort();
};

}
}

#endif
