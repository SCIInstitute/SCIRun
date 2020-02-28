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

#include <Graphics/Widgets/GlyphFactory.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Widgets/Widget.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;

std::string RealGlyphFactory::sphere(SphereParameters params, WidgetBase& widget) const
{
  if (params.common.scale < 0) params.common.scale = 1.;
  if (params.common.resolution < 0) params.common.resolution = 10;

  auto colorScheme = ColorScheme::COLOR_UNIFORM;
  std::stringstream ss;
  ss << params.common.scale << params.common.resolution << static_cast<int>(colorScheme);

  auto name = widget.uniqueID() + "widget" + ss.str();

  Graphics::GlyphGeom glyphs;
  ColorRGB node_color;
  glyphs.addSphere(params.point, params.common.scale, params.common.resolution, node_color);

  auto renState = getSphereRenderState(params.common.defaultColor);

  glyphs.buildObject(widget, name, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, params.common.bbox);

  return name;
}

RenderState RealGlyphFactory::getSphereRenderState(const std::string& defaultColor) const
{
  RenderState renState;

  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENCY, false);

  renState.defaultColor = ColorRGB(defaultColor);
  renState.defaultColor = (renState.defaultColor.r() > 1.0 ||
    renState.defaultColor.g() > 1.0 ||
    renState.defaultColor.b() > 1.0) ?
    ColorRGB(
    renState.defaultColor.r() / 255.,
    renState.defaultColor.g() / 255.,
    renState.defaultColor.b() / 255.)
    : renState.defaultColor;

  renState.set(RenderState::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_WIDGET, true);

  return renState;
}

std::string RealGlyphFactory::disk(DiskParameters params, WidgetBase& widget) const
{
  if (params.common.scale < 0) params.common.scale = 1.;
  if (params.common.resolution < 0) params.common.resolution = 10;

  auto colorScheme = ColorScheme::COLOR_UNIFORM;
  std::stringstream ss;
  ss << params.common.scale << params.common.resolution << static_cast<int>(colorScheme);

  auto name = widget.uniqueID() + "widget" + ss.str();

  Graphics::GlyphGeom glyphs;
  ColorRGB node_color;
  glyphs.addDisk(params.p1, params.p2, params.common.scale, params.common.resolution, node_color, node_color);

  auto renState = getSphereRenderState(params.common.defaultColor);

  glyphs.buildObject(widget, name, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, params.common.bbox);
  return name;
}

std::string RealGlyphFactory::cylinder(CylinderParameters params, WidgetBase& widget) const
{
  if (params.common.scale < 0) params.common.scale = 1.;
  if (params.common.resolution < 0) params.common.resolution = 10;

  auto colorScheme = ColorScheme::COLOR_UNIFORM;
  std::stringstream ss;
  ss << params.common.scale << params.common.resolution << static_cast<int>(colorScheme);

  auto name = widget.uniqueID() + "widget" + ss.str();

  Graphics::GlyphGeom glyphs;
  ColorRGB node_color;
  glyphs.addCylinder(params.p1, params.p2, params.common.scale, params.common.resolution, node_color, node_color);

  auto renState = getSphereRenderState(params.common.defaultColor);

  glyphs.buildObject(widget, name, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, params.common.bbox);

  return name;
}

std::string RealGlyphFactory::cone(ConeParameters params, WidgetBase& widget) const
{
  if (params.cylinder.common.scale < 0) params.cylinder.common.scale = 1.;
  if (params.cylinder.common.resolution < 0) params.cylinder.common.resolution = 10;

  auto colorScheme = ColorScheme::COLOR_UNIFORM;
  std::stringstream ss;
  ss << params.cylinder.common.scale << params.cylinder.common.resolution << static_cast<int>(colorScheme);

  auto name = widget.uniqueID() + "widget" + ss.str();

  Graphics::GlyphGeom glyphs;
  ColorRGB node_color;
  glyphs.addCone(params.cylinder.p1, params.cylinder.p2, params.cylinder.common.scale, params.cylinder.common.resolution, params.renderBase, node_color, node_color);

  auto renState = getSphereRenderState(params.cylinder.common.defaultColor);

  glyphs.buildObject(widget, name, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, params.cylinder.common.bbox);

  return name;
}

std::string RealGlyphFactory::basicBox(BasicBoundingBoxParameters params, WidgetBase& widget) const
{
  auto colorScheme(ColorScheme::COLOR_UNIFORM);
  //get all the bbox edges
  Point c,r,d,b;
  params.pos.getPosition(c,r,d,b);
  auto x = r - c, y = d - c, z = b - c;
  std::vector<Point> points = {
    c + x + y + z,
    c + x + y - z,
    c + x - y + z,
    c + x - y - z,
    c - x + y + z,
    c - x + y - z,
    c - x - y + z,
    c - x - y - z };
  uint32_t point_indicies[] = {
    0, 1, 0, 2, 0, 4,
    7, 6, 7, 5, 3, 7,
    4, 5, 4, 6, 1, 5,
    3, 2, 3, 1, 2, 6
  };
  const auto num_strips = 50;
  std::vector<Vector> tri_points;
  std::vector<Vector> tri_normals;
  std::vector<uint32_t> tri_indices;
  std::vector<ColorRGB> colors;
  GlyphGeom glyphs;
  //generate triangles for the cylinders.
  for (auto edge = 0; edge < 24; edge += 2)
  {
    glyphs.addCylinder(points[point_indicies[edge]], points[point_indicies[edge + 1]], params.common.scale, num_strips, ColorRGB(), ColorRGB());
  }
  //generate triangles for the spheres
  for (const auto& a : points)
  {
    glyphs.addSphere(a, params.common.scale, num_strips, ColorRGB(1, 0, 0));
  }

  std::stringstream ss;
  ss << params.common.scale;
  for (const auto& a : points) ss << a.x() << a.y() << a.z();

  auto name = "bounding_box_cylinders" + ss.str();

  RenderState renState;

  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENCY, false);

  renState.defaultColor = ColorRGB(1, 1, 1);
  renState.set(RenderState::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_WIDGET, true);

  glyphs.buildObject(widget, name, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, params.common.bbox);

  return name;
}
