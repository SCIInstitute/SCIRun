/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


#ifndef POSITION_PROVIDER_H
#define POSITION_PROVIDER_H

#include <boost/shared_ptr.hpp>
#include <QPointF>

class QGraphicsProxyWidget;

namespace SCIRun {
namespace Gui {

class PositionProvider
{
public:
  virtual ~PositionProvider() {}
  virtual QPointF currentPosition() const = 0;
};

typedef boost::shared_ptr<PositionProvider> PositionProviderPtr;

class NeedsScenePositionProvider
{
public:
  virtual ~NeedsScenePositionProvider() {}
  virtual void setPositionObject(PositionProviderPtr provider)
  {
    positionProvider_ = provider;
  }
  PositionProviderPtr getPositionObject() const { return positionProvider_; }
protected:
  PositionProviderPtr positionProvider_;
};

class ProxyWidgetPosition : public PositionProvider
{
public:
  explicit ProxyWidgetPosition(QGraphicsProxyWidget* widget, const QPointF& offset = QPointF());
  virtual QPointF currentPosition() const override;
private:
  QGraphicsProxyWidget* widget_;
  QPointF offset_;
};

class MidpointPositionerFromPorts : public PositionProvider
{
public:
  MidpointPositionerFromPorts(NeedsScenePositionProvider* p1, NeedsScenePositionProvider* p2);
  virtual QPointF currentPosition() const override;
private:
  NeedsScenePositionProvider *p1_, *p2_;
};

class PassThroughPositioner : public PositionProvider
{
public:
  explicit PassThroughPositioner(const QGraphicsProxyWidget* widget);
  virtual QPointF currentPosition() const override;
private:
  const QGraphicsProxyWidget* widget_;
};

class LambdaPositionProvider : public PositionProvider
{
public:
  explicit LambdaPositionProvider(std::function<QPointF()> pointFunc) : pointFunc_(pointFunc) {}
  virtual QPointF currentPosition() const override { return pointFunc_(); }
private:
  std::function<QPointF()> pointFunc_;
};

enum class NotePosition
{
  Default,
  None,
  Tooltip,
  Top,
  Left,
  Right,
  Bottom
};

}
}

#endif
