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


#include <Graphics/Widgets/WidgetFactory.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;

ArrowWidgetHandle WidgetFactory::createArrowWidget(const Core::GeometryIDGenerator &idGenerator,
                                                       const std::string& name,
                                                       double scale,
                                                       const Point &pos,
                                                       const Vector &dir,
                                                       int resolution,
                                                       bool show_as_vector,
                                                       int widget_num,
                                                       int widget_iter,
                                                       const BBox &bbox)
{
  return boost::make_shared<ArrowWidget>(idGenerator, name, scale, pos, dir,
                                         resolution, show_as_vector,
                                         widget_num, widget_iter, bbox);
}

WidgetHandle WidgetFactory::createBox(const Core::GeometryIDGenerator& idGenerator,
                                               double scale,
                                               const BoxPosition& pos,
                                               const Point& origin,
                                               const BBox& bbox)
{
  auto widget = boost::make_shared<BoundingBoxWidget>(idGenerator, scale, pos, origin, bbox);
  widget->addInitialId();
  widget->setToTranslate();
  return widget;
}

WidgetHandle WidgetFactory::createSphere(const Core::GeometryIDGenerator& idGenerator,
                                         const std::string& name,
                                         double radius,
                                         const std::string& defaultColor,
                                         const Point& point,
                                         const Core::Geometry::Point& origin,
                                         const BBox& bbox,
                                         int resolution)
{
  auto widget = boost::make_shared<SphereWidget>(idGenerator, name, radius, defaultColor, point, origin, bbox, resolution);
  widget->addInitialId();
  widget->setToTranslate();
  return widget;
}

WidgetHandle WidgetFactory::createCylinder(const Core::GeometryIDGenerator& idGenerator,
                                           const std::string& name,
                                           double radius,
                                           const std::string& defaultColor,
                                           const Point& p1,
                                           const Point& p2,
                                           const Core::Geometry::Point& origin,
                                           const BBox& bbox,
                                           int resolution)
{
  auto widget = boost::make_shared<CylinderWidget>(idGenerator, name, radius, defaultColor, p1, p2, origin, bbox, resolution);
  widget->addInitialId();
  widget->setToTranslate();
  return widget;
}

WidgetHandle WidgetFactory::createCone(const Core::GeometryIDGenerator& idGenerator,
                                       const std::string& name,
                                       double radius,
                                       const std::string& defaultColor,
                                       const Point& p1,
                                       const Point& p2,
                                       const Core::Geometry::Point& origin,
                                       const BBox& bbox,
                                       bool renderBase,
                                       int resolution)
{
  auto widget = boost::make_shared<ConeWidget>(idGenerator, name, radius, defaultColor, p1, p2, origin, bbox, renderBase, resolution);
  widget->addInitialId();
  widget->setToTranslate();
  return widget;
}

WidgetHandle WidgetFactory::createDisk(const Core::GeometryIDGenerator& idGenerator,
                                       const std::string& name,
                                       double radius,
                                       const std::string& defaultColor,
                                       const Point& p1,
                                       const Point& p2,
                                       const Core::Geometry::Point& origin,
                                       const BBox& bbox,
                                       int resolution)
{
  auto widget = boost::make_shared<DiskWidget>(idGenerator, name, radius, defaultColor, p1, p2, origin, bbox, resolution);
  widget->addInitialId();
  widget->setToTranslate();
  return widget;
}
