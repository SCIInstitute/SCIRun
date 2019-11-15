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
#include <Graphics/Widgets/BoxWidget.h>
#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Graphics/Widgets/WidgetFactory.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

BoundingBoxWidget::BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator,
                                     const std::string& name, double scale, const Transform& trans,
                                     const Point& origin, int widget_num, int widget_iter)
  : CompositeWidget(idGenerator, name)
{
  widgets_.clear();
  int widgetsIndex = -1;
  auto colorScheme = ColorScheme::COLOR_UNIFORM;
  std::stringstream ss;
  auto uniqueNodeID = uniqueID() + "widget" + ss.str();
  ColorRGB node_color;
  // auto renState = getWidgetRenderState(defaultColor);

  auto eigvecs = trans.get_rotation_vectors();
  auto c = trans.get_translation_point();
  std::vector<Point> corners = {
    c + eigvecs[0] + eigvecs[1] + eigvecs[2],
    c + eigvecs[0] + eigvecs[1] - eigvecs[2],
    c + eigvecs[0] - eigvecs[1] + eigvecs[2],
    c + eigvecs[0] - eigvecs[1] - eigvecs[2],
    c - eigvecs[0] + eigvecs[1] + eigvecs[2],
    c - eigvecs[0] + eigvecs[1] - eigvecs[2],
    c - eigvecs[0] - eigvecs[1] + eigvecs[2],
    c - eigvecs[0] - eigvecs[1] - eigvecs[2]};
  std::vector<Point> facesStart = {
    c + eigvecs[0], c - eigvecs[0],
    c + eigvecs[1], c - eigvecs[1],
    c + eigvecs[2], c - eigvecs[2]};
  std::vector<Point> facesEnd = {
    c + eigvecs[0]*diskWidth_, c - eigvecs[0]*diskWidth_,
    c + eigvecs[1]*diskWidth_, c - eigvecs[1]*diskWidth_,
    c + eigvecs[2]*diskWidth_, c - eigvecs[2]*diskWidth_};

  double diagonalLength = Vector(eigvecs[0].length(), eigvecs[1].length(), eigvecs[2].length()).length();

  auto bbox = BBox(corners);
  // Create glyphs
  std::string boxName = widgetName(BoundingBoxWidgetSection::BOX, widget_num, widget_iter);
  widgets_.push_back(WidgetFactory::createBox(idGenerator, diagonalLength*scale*boxScale_, origin, corners, bbox));
  widgets_[++widgetsIndex]->setToTranslate();

  for (auto corner : corners)
  {
    std::string cornerName = widgetName(BoundingBoxWidgetSection::CORNER_SCALE, widget_num, widgetsIndex);
    widgets_.push_back(WidgetFactory::createSphere(idGenerator, cornerName, diagonalLength*scale*resizeSphereScale_, resizeCol_,
                                                   corner, c, bbox, resolution_));
    widgets_[++widgetsIndex]->setToScale(eigvecs[0]);
  }

  for (const auto& face : facesStart)
  {
    std::string faceName = widgetName(BoundingBoxWidgetSection::FACE_ROTATE, widget_num, widgetsIndex);
    widgets_.push_back(WidgetFactory::createSphere(idGenerator, faceName, diagonalLength*scale*rotSphereScale_, deflCol_,
                                                   face, c, bbox, resolution_));
    widgets_[++widgetsIndex]->setToRotate();
  }

  for (int i = 0; i < 6; i++)
  {
    std::string faceName = widgetName(BoundingBoxWidgetSection::FACE_SCALE, widget_num, widgetsIndex);
    widgets_.push_back(WidgetFactory::createDisk(idGenerator, faceName, diagonalLength*scale*diskRadius_, resizeCol_,
                                                 facesStart[i], facesEnd[i], c, bbox, resolution_));
    widgets_[++widgetsIndex]->setToScale(eigvecs[0]);
  }

  std::vector<std::string> geom_ids(widgets_.size());
  for (int i = 0; i < widgets_.size(); i++)
    geom_ids[i] = (widgets_[i]->uniqueID());

  for (int i = 0; i < widgets_.size(); i++)
    widgets_[i]->connectedIds_ = geom_ids;
}

BoundingBoxWidget::BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator, const std::string& name,
                                     double scale, const BoxPosition& pos, const Point& origin,
                                     int widget_num, int widget_iter, const BBox& bbox)
    : CompositeWidget(idGenerator, name)
{
  widgets_.clear();
  int widgetsIndex = -1;
  auto colorScheme = ColorScheme::COLOR_UNIFORM;
  std::stringstream ss;

  auto uniqueNodeID = uniqueID() + "widget" + ss.str();

  // Graphics::GlyphGeom glyphs;
  ColorRGB node_color;

  // auto renState = getWidgetRenderState(defaultColor);

  Point c,r,d,b;
  pos.getPosition(c,r,d,b);
  auto x = r - c, y = d - c, z = b - c;
  std::vector<Point> corners = {
    c + x + y + z,
    c + x + y - z,
    c + x - y + z,
    c + x - y - z,
    c - x + y + z,
    c - x + y - z,
    c - x - y + z,
    c - x - y - z };
  std::vector<Point> facesStart = {
    c + x,
    c - x,
    c + y,
    c - y,
    c + z,
    c - z};
  std::vector<Point> facesEnd = {
    c + x*diskWidth_,
    c - x*diskWidth_,
    c + y*diskWidth_,
    c - y*diskWidth_,
    c + z*diskWidth_,
    c - z*diskWidth_};

  // Create glyphs
  std::string boxName = widgetName(BoundingBoxWidgetSection::BOX, widget_num, widget_iter);
  widgets_.push_back(WidgetFactory::createBox(idGenerator, scale, pos, origin, bbox));
  widgets_[++widgetsIndex]->setToTranslate();

  for (const auto& corner : corners)
  {
    std::string cornerName = widgetName(BoundingBoxWidgetSection::CORNER_SCALE, widget_num, widgetsIndex);
    widgets_.push_back(WidgetFactory::createSphere(idGenerator, cornerName, scale*resizeSphereScale_, resizeCol_,
                                                   corner, c, bbox, resolution_));
    widgets_[++widgetsIndex]->setToScale(x);
  }

  for (const auto& face : facesStart)
  for (int i = 0; i < 6; i++)
    {
      std::string faceName = widgetName(BoundingBoxWidgetSection::FACE_ROTATE, widget_num, widgetsIndex);
      widgets_.push_back(WidgetFactory::createSphere(idGenerator, faceName, scale*rotSphereScale_, deflCol_,
                                                     face, c, bbox, resolution_));
      widgets_[++widgetsIndex]->setToRotate();
    }

  for (int i = 0; i < 6; i++)
  {
    std::string faceName = widgetName(BoundingBoxWidgetSection::FACE_SCALE, widget_num, widgetsIndex);
    widgets_.push_back(WidgetFactory::createDisk(idGenerator, faceName, scale*diskRadius_, resizeCol_,
                                                 facesStart[i], facesEnd[i], c, bbox, resolution_));
    widgets_[++widgetsIndex]->setToScale(x);
  }

  std::vector<std::string> geom_ids(widgets_.size());
  for (int i = 0; i < widgets_.size(); i++)
    geom_ids[i] = (widgets_[i]->uniqueID());

  for (int i = 0; i < widgets_.size(); i++)
    widgets_[i]->connectedIds_ = geom_ids;
}

BoundingBoxWidget::BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator, const std::string& name,
                                     double scale, const Point& pos, const Point& origin,
                                     const std::vector<Vector>& eigvecs_, const std::vector<double>& eigvals_,
                                     int widget_num, int widget_iter)
  : CompositeWidget(idGenerator, name) {
}

std::string BoundingBoxWidget::widgetName(size_t i, size_t id, size_t iter)
{
  return "BoundingBoxWidget(" + std::to_string(i) + ")" + "(" + std::to_string(id) + ")" +
         "(" + std::to_string(iter) + ")";
}
