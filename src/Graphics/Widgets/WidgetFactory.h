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

#ifndef Graphics_Graphics_Widgets_WidgetFactory_H
#define Graphics_Graphics_Widgets_WidgetFactory_H

#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/ArrowWidget.h>
#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Graphics/Widgets/BoxWidget.h>
#include <Graphics/Widgets/ConeWidget.h>
#include <Graphics/Widgets/CylinderWidget.h>
#include <Graphics/Widgets/DiskWidget.h>
#include <Graphics/Widgets/SphereWidget.h>
#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun {
  namespace Graphics {
    namespace Datatypes {
    class SCISHARE WidgetFactory {
    public:
      static ArrowWidgetHandle createArrowWidget(
        const Core::GeometryIDGenerator &idGenerator, const std::string &name,
        double scale, const Core::Geometry::Point &pos,
        const Core::Geometry::Vector &dir, int resolution, bool show_as_vector,
        int widget_num, int widget_iter, const Core::Geometry::BBox &bbox);
      static WidgetHandle createBox(const Core::GeometryIDGenerator &idGenerator,
                                    double scale, const BoxPosition &pos,
                                    const Core::Geometry::Point &origin,
                                    const Core::Geometry::BBox &bbox);
      static WidgetHandle createBox(const Core::GeometryIDGenerator &idGenerator,
                                    double scale, const Core::Geometry::Point &origin,
                                    const std::vector<Core::Geometry::Point>& points,
                                    const Core::Geometry::BBox& bbox);
      static BoundingBoxWidgetHandle createBoundingBox(const Core::GeometryIDGenerator &idGenerator,
                                                       const std::string& name,
                                                       double scale,
                                                       const BoxPosition &pos,
                                                       const Core::Geometry::Point &origin,
                                                       int widget_num,
                                                       const Core::Geometry::BBox &bbox);

      static BoundingBoxWidgetHandle createBoundingBox(const Core::GeometryIDGenerator& idGenerator,
                                                       const std::string& name, double scale,
                                                       const Core::Geometry::Transform& trans,
                                                       const Core::Geometry::Point &origin,
                                                       int widget_num);
      static WidgetHandle createSphere(const Core::GeometryIDGenerator &idGenerator,
                                       const std::string &name, double radius,
                                       const std::string &defaultColor,
                                       const Core::Geometry::Point &point,
                                       const Core::Geometry::Point &origin,
                                       const Core::Geometry::BBox &bbox,
                                       int resolution);
      static WidgetHandle createCylinder(
              const Core::GeometryIDGenerator &idGenerator, const std::string &name,
              double scale, const std::string &defaultColor,
              const Core::Geometry::Point &p1, const Core::Geometry::Point &p2,
              const Core::Geometry::Point &origin, const Core::Geometry::BBox &bbox,
              int resolution);
      static WidgetHandle createCone(const Core::GeometryIDGenerator &idGenerator,
                                     const std::string &name, double scale,
                                     const std::string &defaultColor,
                                     const Core::Geometry::Point &p1,
                                     const Core::Geometry::Point &p2,
                                     const Core::Geometry::Point &origin,
                                     const Core::Geometry::BBox &bbox,
                                     bool renderBase, int resolution);
      static WidgetHandle createDisk(const Core::GeometryIDGenerator &idGenerator,
                                     const std::string &name, double scale,
                                     const std::string &defaultColor,
                                     const Core::Geometry::Point &p1,
                                     const Core::Geometry::Point &p2,
                                     const Core::Geometry::Point &origin,
                                     const Core::Geometry::BBox &bbox,
                                     int resolution);

      template <typename WidgetIter>
      static CompositeWidgetHandle
      createComposite(const Core::GeometryIDGenerator &idGenerator,
                      const std::string &tag, WidgetIter begin,
                      WidgetIter end) {
        return boost::make_shared<CompositeWidget>(idGenerator, tag, begin,
                                                   end);
      }
    };
    } // namespace Datatypes
  } // namespace Graphics
} // namespace SCIRun

#endif
