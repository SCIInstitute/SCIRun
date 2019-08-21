#ifndef Graphics_Graphics_Widgets_WidgetFactory_H
#define Graphics_Graphics_Widgets_WidgetFactory_H

#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/ArrowWidget.h>
#include <Graphics/Widgets/BoundingBoxWidget.h>
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
