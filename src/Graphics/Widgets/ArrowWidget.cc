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

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

namespace detail
{
  static const ColorRGB deflPointCol_ = ColorRGB(0.54, 0.6, 1.0);
  static const ColorRGB deflCol_ = ColorRGB(0.5, 0.5, 0.5);
  static const ColorRGB resizeCol_ = ColorRGB(0.54, 1.0, 0.60);

  static const double sphereRadius_ = 0.25;
  static const double cylinderRadius_ = 0.12;
  static const double coneRadius_ = 0.25;
  static const double diskRadius_ = 0.25;
  static const double diskDistFromCenter_ = 0.85;
  static const double diskWidth_ = 0.05;
}

ArrowWidget::ArrowWidget(const GeneralWidgetParameters& gen, ArrowParameters params)
    : CompositeWidget(gen.base)
{
  using namespace detail;
  ColorRGB sphereCol = (params.show_as_vector) ? deflPointCol_ : resizeCol_;

  if (params.common.resolution < 3) params.common.resolution = 10;

  isVector_ = params.show_as_vector;
  auto colorScheme = ColorScheme::COLOR_UNIFORM;
  std::stringstream ss;
  ss << params.pos << params.dir << static_cast<int>(colorScheme);

  name_ = uniqueID() + "widget" + ss.str();

  ColorRGB node_color;

  Point bmin = params.pos;
  Point bmax = params.pos + params.dir * params.common.scale;

  // Fix degenerate boxes.
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

  Point center = bmin + params.dir/2.0 * params.common.scale;

  // Create glyphs
  widgets_.push_back(WidgetFactory::createSphere(
                                {gen.base.idGenerator,
                                widgetName(ArrowWidgetSection::SPHERE, params.widget_num, params.widget_iter),
                                {{WidgetInteraction::CLICK, WidgetMovement::TRANSLATE}}
                                },
                                {{
                                sphereRadius_ * params.common.scale,
                                sphereCol.toString(),
                                bmin,
                                params.common.bbox,
                                params.common.resolution}, bmin}));

  if (params.show_as_vector)
  {
    // Starts the cylinder position closer to the surface of the sphere
    Point cylinderStart = bmin + 0.75 * (params.dir * params.common.scale * sphereRadius_);

    widgets_.push_back(WidgetFactory::createCylinder(
                                  {gen.base.idGenerator,
                                  widgetName(ArrowWidgetSection::CYLINDER, params.widget_num, params.widget_iter),
                                  {{WidgetInteraction::CLICK, WidgetMovement::TRANSLATE}}},
                                  {{cylinderRadius_ * params.common.scale,
                                  deflCol_.toString(),
                                  bmin,
                                  params.common.bbox,
                                  params.common.resolution}, cylinderStart,
                                  center}));

    widgets_.push_back(WidgetFactory::createCone(
                                  {gen.base.idGenerator,
                                  widgetName(ArrowWidgetSection::CONE, params.widget_num, params.widget_iter),
                                  {{WidgetInteraction::CLICK, WidgetMovement::ROTATE}}},
                                  {{{coneRadius_ * params.common.scale,
                                  deflCol_.toString(),
                                  bmin,
                                  params.common.bbox,
                                  params.common.resolution}, center,
                                  bmax}, true}));

    setPosition(widgets_.back()->position());

    Point diskPos = bmin + params.dir * params.common.scale * diskDistFromCenter_;
    Point dp1 = diskPos - diskWidth_ * params.dir * params.common.scale;
    Point dp2 = diskPos + diskWidth_ * params.dir * params.common.scale;
    widgets_.push_back(WidgetFactory::createDisk(
                                  {gen.base.idGenerator,
                                  widgetName(ArrowWidgetSection::DISK, params.widget_num, params.widget_iter),
                                  {{WidgetInteraction::CLICK, WidgetMovement::SCALE}}},  //TODO: concern #1--how user interaction maps to transform type
                                  {{diskRadius_ * params.common.scale,
                                  resizeCol_.toString(),
                                  bmin,
                                  params.common.bbox,
                                  params.common.resolution}, dp1, dp2 }));

    //TODO: create cool operator syntax for wiring these up.
    registerAllSiblingWidgetsForEvent(widgets_[1], WidgetMovement::TRANSLATE);  //TODO: concern #2--how transform of "root" maps to siblings
    registerAllSiblingWidgetsForEvent(widgets_[2], WidgetMovement::ROTATE);
    widgets_[2]->setTransformParameters<Rotation>(bmin);                        //TODO: concern #3--what data transform of "root" requires
    registerAllSiblingWidgetsForEvent(widgets_[3], WidgetMovement::SCALE);
    Vector flipVec = params.dir.getArbitraryTangent().normal();
    widgets_[3]->setTransformParameters<Scaling>(bmin, flipVec);
  }
  registerAllSiblingWidgetsForEvent(widgets_[0], WidgetMovement::TRANSLATE);
}

bool ArrowWidget::isVector() const
{
  return isVector_;
}

std::string ArrowWidget::widgetName(size_t i, size_t id, size_t iter)
{
  return "ArrowWidget(" + std::to_string(i) + ")" + "(" + std::to_string(id) + ")" +
         "(" + std::to_string(iter) + ")";
}
