/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Datatypes/Color.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Widgets/ArrowWidget.h>
#include <Graphics/Widgets/WidgetFactory.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

ArrowWidget::ArrowWidget(const GeometryIDGenerator &idGenerator,
                         const std::string &name, double scale,
                         const Point &pos, const Vector &dir, int resolution,
                         bool show_as_vector, size_t widget_num, size_t widget_iter,
                         const BBox &bbox)
    : CompositeWidget(idGenerator, name) {
  static const ColorRGB deflPointCol_ = ColorRGB(0.54, 0.6, 1.0);
  static const ColorRGB deflCol_ = ColorRGB(0.5, 0.5, 0.5);
  static const ColorRGB resizeCol_ = ColorRGB(0.54, 1.0, 0.60);
  ColorRGB sphereCol = (show_as_vector) ? deflPointCol_ : resizeCol_;

  static const double sphereRadius_ = 0.25;
  static const double cylinderRadius_ = 0.12;
  static const double coneRadius_ = 0.25;
  static const double diskRadius_ = 0.25;
  static const double diskDistFromCenter_ = 0.85;
  static const double diskWidth_ = 0.05;

  if (resolution < 3) resolution = 10;

  isVector_ = show_as_vector;
  auto colorScheme = ColorScheme::COLOR_UNIFORM;
  std::stringstream ss;
  ss << pos << dir << static_cast<int>(colorScheme);

  auto uniqueNodeID = uniqueID() + "widget" + ss.str();

  // Graphics::GlyphGeom glyphs;
  ColorRGB node_color;

  // auto renState = getWidgetRenderState(defaultColor);

  Point bmin = pos;
  Point bmax = pos + dir * scale;

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

  Point center = bmin + dir/2.0 * scale;

  // Create glyphs
  mWidgets.push_back(WidgetFactory::createSphere(idGenerator,
                                widgetName(ArrowWidgetSection::SPHERE, widget_num, widget_iter),
                                sphereRadius_ * scale, sphereCol.toString(), bmin, bmin, bbox,
                                resolution));
  mWidgets[0]->setToTranslate(MouseButton::LEFT);
  mWidgets[0]->setToTranslationAxis(MouseButton::RIGHT, dir);

  if(show_as_vector)
  {
    // Starts the cylinder position closer to the surface of the sphere
    Point cylinderStart = bmin + 0.75 * (dir * scale * sphereRadius_);

    mWidgets.push_back(WidgetFactory::createCylinder(idGenerator,
                                  widgetName(ArrowWidgetSection::CYLINDER, widget_num, widget_iter),
                                  cylinderRadius_ * scale, deflCol_.toString(), cylinderStart,
                                  center, bmin, bbox, resolution));
    mWidgets[1]->setToTranslate(MouseButton::LEFT);
    mWidgets[1]->setToTranslationAxis(MouseButton::RIGHT, dir);

    mWidgets.push_back(WidgetFactory::createCone(idGenerator,
                                  widgetName(ArrowWidgetSection::CONE, widget_num, widget_iter),
                                  coneRadius_ * scale, deflCol_.toString(), center, bmax, bmin, bbox,
                                  true, resolution));
    mWidgets[2]->setToRotate(MouseButton::LEFT);

    Point diskPos = bmin + dir * scale * diskDistFromCenter_;
    Point dp1 = diskPos - diskWidth_ * dir * scale;
    Point dp2 = diskPos + diskWidth_ * dir * scale;
    mWidgets.push_back(WidgetFactory::createDisk(idGenerator,
                                  widgetName(ArrowWidgetSection::DISK, widget_num, widget_iter),
                                  diskRadius_ * scale, resizeCol_.toString(), dp1, dp2, bmin, bbox,
                                  resolution));
    Vector flipVec = dir.getArbitraryTangent().normal();
    mWidgets[3]->setToScale(MouseButton::LEFT, flipVec, true);
  }

  std::vector<std::string> ids;
  for(int i = 0; i < 1 + 3*show_as_vector; i++)
    ids.push_back(mWidgets[i]->uniqueID());

  mWidgets[0]->addMovementMap(WidgetMovement::TRANSLATE,
                              std::make_pair(WidgetMovement::TRANSLATE, ids));
  mWidgets[0]->addMovementMap(WidgetMovement::TRANSLATE_AXIS,
                              std::make_pair(WidgetMovement::TRANSLATE_AXIS, ids));
  if(show_as_vector)
  {
    mWidgets[1]->addMovementMap(WidgetMovement::TRANSLATE,
                                std::make_pair(WidgetMovement::TRANSLATE, ids));
    mWidgets[1]->addMovementMap(WidgetMovement::TRANSLATE_AXIS,
                                std::make_pair(WidgetMovement::TRANSLATE_AXIS, ids));
    mWidgets[2]->addMovementMap(WidgetMovement::ROTATE, std::make_pair(WidgetMovement::ROTATE, ids));
    mWidgets[3]->addMovementMap(WidgetMovement::SCALE, std::make_pair(WidgetMovement::SCALE, ids));
  }
}

bool ArrowWidget::isVector()
{
  return isVector_;
}

std::string ArrowWidget::widgetName(size_t i, size_t id, size_t iter)
{
  return "ArrowWidget(" + std::to_string(i) + ")" + "(" + std::to_string(id) + ")" +
         "(" + std::to_string(iter) + ")";
}
