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

#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Color.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

void BoxPosition::setPosition(const Point& center, const Point& right, const Point& down, const Point& in)
{
  center_ = center;
  right_ = right;
  down_ = down;
  in_ = in;
}
void BoxPosition::getPosition(Point& center, Point& right, Point& down, Point& in) const
{
  center = center_;
  right = right_;
  down = down_;
  in = in_;
}

BoundingBoxWidget::BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator,
  double scale, const BoxPosition& pos, const BBox& bbox)
  : WidgetBase(idGenerator, "BoundingBox", true)
{
  auto colorScheme(ColorScheme::COLOR_UNIFORM);
  //get all the bbox edges
  Point c,r,d,b;
  pos.getPosition(c,r,d,b);
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
    glyphs.addCylinder(points[point_indicies[edge]], points[point_indicies[edge + 1]], scale, num_strips, ColorRGB(), ColorRGB());
  }
  //generate triangles for the spheres
  for (const auto& a : points)
  {
    glyphs.addSphere(a, scale, num_strips, ColorRGB(1, 0, 0));
  }

  std::stringstream ss;
  ss << scale;
  for (const auto& a : points) ss << a.x() << a.y() << a.z();

  auto uniqueNodeID = "bounding_box_cylinders" + ss.str();

  RenderState renState;

  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENCY, false);

  renState.defaultColor = ColorRGB(1, 1, 1);
  renState.set(RenderState::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_WIDGET, true);

  glyphs.buildObject(*this, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, bbox);
}
