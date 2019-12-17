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
#include "glm/gtx/string_cast.hpp"

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

BoundingBoxWidget::BoundingBoxWidget(const GeometryIDGenerator& idGenerator, const std::string& name,
                                     double scale, const Point& center,
                                     const std::vector<Vector>& eigvecs,
                                     const std::vector<double>& eigvals, int widgetNum,
                                     int widgetIter)
  : CompositeWidget(idGenerator, name), scale_(scale), center_(center), eigvecs_(eigvecs),
    eigvals_(eigvals), widgetsIndex_(-1)
{
  scaledEigvecs_.reserve(DIMENSIONS_);
  for(int i = 0; i < DIMENSIONS_; i++)
    scaledEigvecs_.emplace_back(eigvecs_[i] * eigvals_[i]);

  initWidgetCreation(idGenerator, widgetNum, widgetIter);
}

BoundingBoxWidget::BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator,
                                     const std::string& name, double scale, const Transform& trans,
                                     const Point& center, int widgetNum, int widgetIter)
  : CompositeWidget(idGenerator, name), scale_(scale), widgetsIndex_(-1)
{
  center_ = trans.get_translation_point();
  scaledEigvecs_ = trans.get_rotation_vectors();
  getEigenValuesAndEigenVectors();
  initWidgetCreation(idGenerator, widgetNum, widgetIter);
}

BoundingBoxWidget::BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator,
                                     const std::string& name, double scale, const BoxPosition& pos,
                                     const Point& center, int widgetNum, int widgetIter,
                                     const BBox& bbox)
  : CompositeWidget(idGenerator, name), scale_(scale), center_(center), bbox_(bbox), widgetsIndex_(-1)
{
  Point c,r,d,b;
  pos.getPosition(center_, r, d, b);
  auto centerVector = Vector(center_);
  scaledEigvecs_ = { Vector(r), Vector(d), Vector(b) };
  for(auto& vec : scaledEigvecs_)
    vec -= centerVector;

  getEigenValuesAndEigenVectors();
  initWidgetCreation(idGenerator, widgetNum, widgetIter);
}

void BoundingBoxWidget::createWidgets(const GeometryIDGenerator& idGenerator, int widgetNum,
                                      int widgetIter)
{
  std::string boxName = widgetName(BoundingBoxWidgetSection::BOX, widgetNum, widgetIter);
  widgets_.push_back(WidgetFactory::createBox(idGenerator, smallestEigval_ * scale_ * boxScale_,
                                              center_, corners_, bbox_));
  widgets_[++widgetsIndex_]->setToTranslate(MouseButton::LEFT);

  for(int i = 0; i < CORNERS_; i++)
    addCornerSphere(i, idGenerator, scaledEigvecs_[i/2], widgetNum, widgetIter);

  for (int i = 0; i < FACES_; i++)
    addFaceSphere(i, idGenerator, widgetNum, widgetIter);

  glm::mat4 scaleTrans = glm::mat4(1.0);
  for(int i = 0; i < DIMENSIONS_; i++)
    for(int j = 0; j < DIMENSIONS_; j++)
      scaleTrans[i][j] = eigvecs_[i][j];

  for (int i = 0; i < FACES_; i++)
    addFaceCylinder(i, idGenerator, scaleTrans, i/2, widgetNum, widgetIter);
}

void BoundingBoxWidget::initWidgetCreation(const GeometryIDGenerator& idGenerator, int widgetNum,
                                           int widgetIter)
{
  diagonalLength_ = Vector(eigvals_[0], eigvals_[1], eigvals_[2]).length();
  smallestEigval_ = std::min(eigvals_[0], std::min(eigvals_[1], eigvals_[2]));
  getCorners();
  getFacesStart();
  getFacesEnd();
  bbox_ = BBox(corners_);

  createWidgets(idGenerator, widgetNum, widgetIter);
  addIds();
}

void BoundingBoxWidget::getEigenValuesAndEigenVectors()
{
  eigvals_.reserve(DIMENSIONS_);
  eigvecs_.reserve(DIMENSIONS_);
  for(int i = 0; i < DIMENSIONS_; i++)
  {
    eigvals_.emplace_back(scaledEigvecs_[i].length());
    eigvecs_.emplace_back(scaledEigvecs_[i] / eigvals_[i]);
  }
}

void BoundingBoxWidget::getCorners()
{
  corners_ = {
    center_ + scaledEigvecs_[0] + scaledEigvecs_[1] + scaledEigvecs_[2],
    center_ + scaledEigvecs_[0] + scaledEigvecs_[1] - scaledEigvecs_[2],
    center_ + scaledEigvecs_[0] - scaledEigvecs_[1] + scaledEigvecs_[2],
    center_ + scaledEigvecs_[0] - scaledEigvecs_[1] - scaledEigvecs_[2],
    center_ - scaledEigvecs_[0] + scaledEigvecs_[1] + scaledEigvecs_[2],
    center_ - scaledEigvecs_[0] + scaledEigvecs_[1] - scaledEigvecs_[2],
    center_ - scaledEigvecs_[0] - scaledEigvecs_[1] + scaledEigvecs_[2],
    center_ - scaledEigvecs_[0] - scaledEigvecs_[1] - scaledEigvecs_[2] };
}

void BoundingBoxWidget::getFacesStart()
{
  facesStart_ = {
    center_ + scaledEigvecs_[0], center_ - scaledEigvecs_[0],
    center_ + scaledEigvecs_[1], center_ - scaledEigvecs_[1],
    center_ + scaledEigvecs_[2], center_ - scaledEigvecs_[2]};
}

void BoundingBoxWidget::getFacesEnd()
{
  facesEnd_ = {
    facesStart_[0] + eigvecs_[0] * smallestEigval_ * diskWidth_,
    facesStart_[1] - eigvecs_[0] * smallestEigval_ * diskWidth_,
    facesStart_[2] + eigvecs_[1] * smallestEigval_ * diskWidth_,
    facesStart_[3] - eigvecs_[1] * smallestEigval_ * diskWidth_,
    facesStart_[4] + eigvecs_[2] * smallestEigval_ * diskWidth_,
    facesStart_[5] - eigvecs_[2] * smallestEigval_ * diskWidth_};
}

void BoundingBoxWidget::addCornerSphere(int i, const Core::GeometryIDGenerator& idGenerator,
                                        const Vector& flipVec, int widgetNum, int widgetIter)
{
  std::string name = widgetName(BoundingBoxWidgetSection::CORNER_SCALE, widgetNum, widgetsIndex_);
  widgets_.push_back(WidgetFactory::createSphere(idGenerator, name,
                                                 smallestEigval_ * scale_ * resizeSphereScale_,
                                                 resizeCol_, corners_[i], center_, bbox_, resolution_));
  widgets_[++widgetsIndex_]->setToScale(MouseButton::LEFT, flipVec);
}
void BoundingBoxWidget::addFaceSphere(int i, const Core::GeometryIDGenerator& idGenerator,
                                      int widgetNum, int widgetIter)
{
  std::string name = widgetName(BoundingBoxWidgetSection::FACE_ROTATE, widgetNum, widgetsIndex_);
  widgets_.push_back(WidgetFactory::createSphere(idGenerator, name,
                                                 smallestEigval_ * scale_ * rotSphereScale_,
                                                 deflCol_, facesStart_[i], center_, bbox_, resolution_));
  widgets_[++widgetsIndex_]->setToRotate(MouseButton::LEFT);
}

void BoundingBoxWidget::addFaceCylinder(int i, const Core::GeometryIDGenerator& idGenerator,
                                        glm::mat4& scaleTrans, int axisNum, int widgetNum,
                                        int widgetIter)
{
  std::string name = widgetName(BoundingBoxWidgetSection::FACE_SCALE, widgetNum, widgetsIndex_);
  widgets_.push_back(WidgetFactory::createDisk(idGenerator, name,
                                               smallestEigval_ * scale_ * diskRadius_, resizeCol_,
                                               facesStart_[i], facesEnd_[i], center_, bbox_,
                                               resolution_));
  widgets_[++widgetsIndex_]->setToScaleAxis(MouseButton::LEFT, eigvecs_[axisNum], scaleTrans, axisNum);
  widgets_[widgetsIndex_]->setToScaleAxisHalf(MouseButton::RIGHT, eigvecs_[axisNum], scaleTrans, axisNum);
}

void BoundingBoxWidget::addIds()
{
  std::vector<std::string> geom_ids(widgets_.size());
  for (int i = 0; i < widgets_.size(); i++)
    geom_ids[i] = (widgets_[i]->uniqueID());

  for (int i = 0; i < widgets_.size(); i++)
    widgets_[i]->connectedIds_ = geom_ids;
}

std::string BoundingBoxWidget::widgetName(size_t i, size_t id, size_t iter)
{
  return "BoundingBoxWidget(" + std::to_string(i) + ")" + "(" + std::to_string(id) + ")" +
         "(" + std::to_string(iter) + ")";
}
