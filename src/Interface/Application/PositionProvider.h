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

#ifndef POSITION_PROVIDER_H
#define POSITION_PROVIDER_H

#include <QPointF>

class QGraphicsProxyWidget;

namespace SCIRun {
namespace Gui {

class PositionProvider
{
public:
  virtual ~PositionProvider() {}
  virtual QPointF currentPosition() const = 0;
  virtual QPointF mapToScene(const QPointF &point) const = 0;
  virtual QPointF mapFromScene(const QPointF &point) const = 0;
};

class NeedsScenePositionProvider
{
public:
  virtual ~NeedsScenePositionProvider() {}
  void setPositionObject(boost::shared_ptr<PositionProvider> provider) { positionProvider_ = provider; }
protected:
  boost::shared_ptr<PositionProvider> positionProvider_;
};

class ProxyWidgetPosition : public PositionProvider
{
public:
  explicit ProxyWidgetPosition(QGraphicsProxyWidget* widget, const QPointF& offset = QPointF()) : widget_(widget), offset_(offset) {}
  virtual QPointF currentPosition() const;
  virtual QPointF mapToScene(const QPointF &point) const;
  virtual QPointF mapFromScene(const QPointF &point) const;
private:
  QGraphicsProxyWidget* widget_;
  QPointF offset_;
};

}
}

#endif
