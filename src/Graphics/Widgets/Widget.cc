/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Graphics/Widgets/SphereWidget.h>
#include <Graphics/Widgets/CylinderWidget.h>
#include <Graphics/Widgets/DiskWidget.h>
#include <Graphics/Widgets/ConeWidget.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;

WidgetBase::WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable)
  : GeometryObjectSpire(idGenerator, tag, isClippable)
{
}

WidgetBase::WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable, const Point& pos)
  : GeometryObjectSpire(idGenerator, tag, isClippable),
    position_(pos)
{
}

WidgetBase::WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable, const Vector& pos)
  : GeometryObjectSpire(idGenerator, tag, isClippable),
    position_(pos)
{
}

Point WidgetBase::position() const
{
  return position_;
}

void WidgetBase::setPosition(const Point& p)
{
  position_ = p;
}

WidgetHandle WidgetFactory::createBox(const Core::GeometryIDGenerator& idGenerator, double scale,
  const BoxPosition& pos, const BBox& bbox)
{
  return boost::make_shared<BoundingBoxWidget>(idGenerator, scale, pos, bbox);
}

WidgetHandle WidgetFactory::createSphere(const Core::GeometryIDGenerator& idGenerator,
  const std::string& name,
  double scale,
  const std::string& defaultColor, const Point& point, const BBox& bbox)
{
  return boost::make_shared<SphereWidget>(idGenerator, name, scale, defaultColor, point, bbox);
}

WidgetHandle WidgetFactory::createCylinder(const Core::GeometryIDGenerator& idGenerator,
                                           const std::string& name,
                                           double scale,
                                           const std::string& defaultColor,
                                           const Point& p1,
                                           const Point& p2,
                                           const BBox& bbox)
{
  return boost::make_shared<CylinderWidget>(idGenerator, name, scale, defaultColor, p1, p2, bbox);
}

WidgetHandle WidgetFactory::createCone(const Core::GeometryIDGenerator& idGenerator,
                                       const std::string& name,
                                       double scale,
                                       const std::string& defaultColor,
                                       const Point& p1,
                                       const Point& p2,
                                       const BBox& bbox,
                                       bool renderBase)
{
  return boost::make_shared<ConeWidget>(idGenerator, name, scale, defaultColor, p1, p2, bbox, renderBase);
}

WidgetHandle WidgetFactory::createDisk(const Core::GeometryIDGenerator& idGenerator,
                                       const std::string& name,
                                       double scale,
                                       const std::string& defaultColor,
                                       const Point& p1,
                                       const Point& p2,
                                       const BBox& bbox)
{
  return boost::make_shared<DiskWidget>(idGenerator, name, scale, defaultColor, p1, p2, bbox);
}

void CompositeWidget::addToList(GeometryBaseHandle handle, GeomList& list)
{
  if (handle.get() == this)
  {
    list.insert(widgets_.begin(), widgets_.end());
  }
}

CompositeWidget::~CompositeWidget()
{
}
