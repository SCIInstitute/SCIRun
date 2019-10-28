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
#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Graphics/Widgets/WidgetFactory.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

BoxWidget::BoxWidget(const Core::GeometryIDGenerator& idGenerator, std::string& name, double scale,
                     pconst BoxPosition& pos, const Point& origin, const BBox& bbox)
    : CompositeWidget(idGenerator, name) {
  static const ColorRGB deflPointCol_ = ColorRGB(0.54, 0.6, 1.0);
  static const ColorRGB deflCol_ = ColorRGB(0.5, 0.5, 0.5);
  static const ColorRGB resizeCol_ = ColorRGB(0.54, 1.0, 0.60);
  ColorRGB diskCol = (show_as_vector) ? deflPointCol_ : resizeCol_;


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
  std::string box_name = widgetName(BoundingBoxWidgetSection::BOX, widget_num, widget_iter);
  widgets_.push_back(WidgetFactory::createBox(idGenerator, box_name, sphereRadius_ * scale,
                                              diskCol.toString(), bmin, bmin, bbox, resolution));
  widgets_[0]->setToTranslate();

  std::string x_plus_name = widgetName(BoundingBoxWidgetSection::X_PLUS, widget_num, widget_iter);
  widgets_.push_back(WidgetFactory::createDisk(idGenerator, x_plus_name, diskRadius_ * scale,
                                  resizeCol_.toString(), dp1, dp2, bmin, bbox, resolution));

    Vector flipVec = dir.getArbitraryTangent().normal();
    widgets_[1]->setToScale(Vector(0,1,0));
  }

  std::vector<std::string> geom_ids;
  for(int i = 0; i < 2; i++)
    geom_ids.push_back(widgets_[i]->uniqueID());

  for(int i = 0; i < 2; i++)
  {
    widgets_[i]->connectedIds_ = geom_ids;
    addToList(widgets_[i]);
  }
}

std::string BoundingBoxWidget::widgetName(size_t i, size_t id, size_t iter)
{
  return "BoundingBoxWidget(" + std::to_string(i) + ")" + "(" + std::to_string(id) + ")" +
         "(" + std::to_string(iter) + ")";
}
