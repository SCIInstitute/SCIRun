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


#include <Core/Datatypes/Color.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Widgets/ArrowWidget.h>
#include <Graphics/Widgets/WidgetFactory.h>
#include <Graphics/Widgets/WidgetBuilders.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

namespace detail
{
  static const ColorRGB deflPointColor = ColorRGB(0.54, 0.6, 1.0);
  static const ColorRGB deflColor = ColorRGB(0.5, 0.5, 0.5);
  static const ColorRGB resizeColor = ColorRGB(0.54, 1.0, 0.60);

  static const double sphereRadius = 0.25;
  static const double cylinderRadius = 0.12;
  static const double coneRadius = 0.25;
  static const double diskRadius = 0.25;
  static const double diskDistFromCenter = 0.85;
  static const double diskWidth = 0.05;

  std::pair<Point, Point> diskPoints(const Point& bmin, const ArrowParameters& params)
  {
    Point diskPos = bmin + params.dir * params.common.scale * diskDistFromCenter;
    Point dp1 = diskPos - diskWidth * params.dir * params.common.scale;
    Point dp2 = diskPos + diskWidth * params.dir * params.common.scale;
    return { dp1, dp2 };
  }
}

namespace
{
  void fixDegenerateBoxes(Point& bmin, Point& bmax)
  {
    const double size_estimate = std::max((bmax - bmin).length() * 0.01, 1.0e-5);
    if (std::abs(bmax.x() - bmin.x()) < 1.0e-6)
    {
      bmin.x(bmin.x() - size_estimate);
      bmax.x(bmax.x() + size_estimate);
    }
    if (std::abs(bmax.y() - bmin.y()) < 1.0e-6)
    {
      bmin.y(bmin.y() - size_estimate);
      bmax.y(bmax.y() + size_estimate);
    }
    if (std::abs(bmax.z() - bmin.z()) < 1.0e-6)
    {
      bmin.z(bmin.z() - size_estimate);
      bmax.z(bmax.z() + size_estimate);
    }
  }
}

ArrowWidget::ArrowWidget(const GeneralWidgetParameters& gen, ArrowParameters params)
    : CompositeWidget(gen.base)
{
  using namespace detail;
  ColorRGB sphereCol = (params.show_as_vector) ? deflPointColor : resizeColor;

  if (params.common.resolution < 3)
    params.common.resolution = 10;

  isVector_ = params.show_as_vector;

  std::stringstream ss;
  ss << params.pos << params.dir << static_cast<int>(ColorScheme::COLOR_UNIFORM);

  name_ = uniqueID() + "widget" + ss.str();

  Point bmin = params.pos;
  Point bmax = params.pos + params.dir * params.common.scale;

  fixDegenerateBoxes(bmin, bmax);

  const Point origin = bmin;

  auto sphere = SphereWidgetBuilder(gen.base.idGenerator)
                      .tag(widgetName(ArrowWidgetSection::SPHERE, params.widget_num, params.widget_iter))
                      .scale(sphereRadius * params.common.scale)
                      .defaultColor(sphereCol.toString())
                      .origin(origin)
                      .boundingBox(params.common.bbox)
                      .resolution(params.common.resolution)
                      .centerPoint(bmin)
                      .build();
  widgets_.push_back(sphere);

  if (params.show_as_vector)
  {
    // Starts the cylinder position closer to the surface of the sphere
    Point cylinderStart = origin + 0.75 * (params.dir * params.common.scale * sphereRadius);
    Point center = origin + params.dir/2.0 * params.common.scale;

    auto cylinder = WidgetFactory::createCylinder(
                                  {gen.base.idGenerator,
                                  widgetName(ArrowWidgetSection::CYLINDER, params.widget_num, params.widget_iter),
                                  {{WidgetInteraction::CLICK, WidgetMovement::TRANSLATE}}},
                                  {{cylinderRadius * params.common.scale,
                                  deflColor.toString(),
                                  origin,
                                  params.common.bbox,
                                  params.common.resolution}, cylinderStart,
                                  center});
    widgets_.push_back(cylinder);

    auto cone = ConeWidgetBuilder(gen.base.idGenerator)
                  .tag(widgetName(ArrowWidgetSection::CONE, params.widget_num, params.widget_iter))
                  .transformMapping({{WidgetInteraction::CLICK, WidgetMovement::ROTATE}})
                  .scale(coneRadius * params.common.scale)
                  .defaultColor(deflColor.toString())
                  .origin(origin)
                  .boundingBox(params.common.bbox)
                  .resolution(params.common.resolution)
                  .diameterPoints(center, bmax)
                  .renderBase(true)
                  .build();
                  
    widgets_.push_back(cone);

    setPosition(cone->position());

    auto diskDiameterPoints = diskPoints(origin, params);

    auto disk = DiskWidgetBuilder(gen.base.idGenerator)
                        .tag(widgetName(ArrowWidgetSection::DISK, params.widget_num, params.widget_iter))
                        .transformMapping({{WidgetInteraction::CLICK, WidgetMovement::SCALE}})
                        .scale(diskRadius * params.common.scale)
                        .defaultColor(resizeColor.toString())
                        .origin(origin)
                        .boundingBox(params.common.bbox)
                        .resolution(params.common.resolution)
                        .diameterPoints(diskDiameterPoints.first, diskDiameterPoints.second)
                        .build();

    widgets_.push_back(disk);

    //TODO: create cool operator syntax for wiring these up.
    registerAllSiblingWidgetsForEvent(cylinder, WidgetMovement::TRANSLATE);  //TODO: concern #2--how transform of "root" maps to siblings
    registerAllSiblingWidgetsForEvent(cone, WidgetMovement::ROTATE);

    cone->setTransformParameters<Rotation>(origin);                        //TODO: concern #3--what data transform of "root" requires

    registerAllSiblingWidgetsForEvent(disk, WidgetMovement::SCALE);

    Vector flipVec = params.dir.getArbitraryTangent().normal();
    disk->setTransformParameters<Scaling>(origin, flipVec);
  }

  registerAllSiblingWidgetsForEvent(sphere, WidgetMovement::TRANSLATE);
}

bool ArrowWidget::isVector() const
{
  return isVector_;
}

std::string ArrowWidget::widgetName(ArrowWidgetSection s, size_t id, size_t iter)
{
  return "ArrowWidget(" + std::to_string(static_cast<int>(s)) + ")" + "(" + std::to_string(id) + ")" +
         "(" + std::to_string(iter) + ")";
}
