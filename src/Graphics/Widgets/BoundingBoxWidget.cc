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
#include "Graphics/Widgets/Widget.h"
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
  for(int d = 0; d < DIMENSIONS_; ++d)
    scaledEigvecs_.emplace_back(eigvecs_[d] * eigvals_[d]);

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
  addBox(idGenerator, widgetNum, widgetIter);
  addCornerSpheres(idGenerator, widgetNum, widgetIter);
  addFaceSphere(idGenerator, widgetNum, widgetIter);

  glm::mat4 scaleTrans = glm::mat4(1.0);
  for(int i = 0; i < DIMENSIONS_; ++i)
    for(int j = 0; j < DIMENSIONS_; ++j)
      scaleTrans[i][j] = eigvecs_[i][j];

  addFaceCylinder(idGenerator, scaleTrans, widgetNum, widgetIter);
}

void BoundingBoxWidget::initWidgetCreation(const GeometryIDGenerator& idGenerator, int widgetNum,
                                           int widgetIter)
{
  smallestEigval_ = std::min(eigvals_[0], std::min(eigvals_[1], eigvals_[2]));
  getCorners();
  getFacesStart();
  getFacesEnd();
  bbox_ = BBox(corners_);

  createWidgets(idGenerator, widgetNum, widgetIter);

  getTranslateIds();
  getRotateIds();
  getScaleIds();
  getScaleAxisIds();


  scaleAxisMaps_.resize(DIMENSIONS_, std::vector<std::vector<std::pair<WidgetMovement, std::vector<std::string>>>>());
  for(auto &v : scaleAxisMaps_)
    v.resize(2, std::vector<std::pair<WidgetMovement, std::vector<std::string>>>());
  for(int i = 0; i < DIMENSIONS_; ++i)
    for(int sign = 0; sign < 2; ++sign)
      for(int j = 0; j < DIMENSIONS_; ++j)
      {
        scaleAxisMaps_[i][sign].push_back(std::make_pair(WidgetMovement::SCALE_AXIS,
                                                         translateIdsBySide_[i]));
        if(i == j)
        {
          WidgetMovement plusMove, minusMove;
          if(sign == 0)
          {
            plusMove = WidgetMovement::TRANSLATE_AXIS;
            minusMove = WidgetMovement::TRANSLATE_AXIS_REVERSE;
          }
          else
          {
            plusMove = WidgetMovement::TRANSLATE_AXIS_REVERSE;
            minusMove = WidgetMovement::TRANSLATE_AXIS;
          }
          scaleAxisMaps_[i][sign].push_back(std::make_pair(plusMove, translateIdsByFace_[j][0]));
          scaleAxisMaps_[i][sign].push_back(std::make_pair(minusMove, translateIdsByFace_[j][1]));
          scaleAxisMaps_[i][sign].push_back(std::make_pair(plusMove, scaleAxisIds_[j][0]));
          scaleAxisMaps_[i][sign].push_back(std::make_pair(minusMove, scaleAxisIds_[j][1]));
          scaleAxisMaps_[i][sign].push_back(std::make_pair(plusMove, rotateIdsByFace_[j][0]));
          scaleAxisMaps_[i][sign].push_back(std::make_pair(minusMove, rotateIdsByFace_[j][1]));
          scaleAxisMaps_[i][sign].push_back(std::make_pair(plusMove, scaleIdsByFace_[j][0]));
          scaleAxisMaps_[i][sign].push_back(std::make_pair(minusMove, scaleIdsByFace_[j][1]));
        }
      }

  scaleAxisUnidirectionalMaps_.resize(DIMENSIONS_, std::vector<std::vector<std::pair<WidgetMovement, std::vector<std::string>>>>());
  for(auto &v : scaleAxisUnidirectionalMaps_)
    v.resize(2, std::vector<std::pair<WidgetMovement, std::vector<std::string>>>());
  for(int i = 0; i < DIMENSIONS_; ++i)
    for(int sign = 0; sign < 2; ++sign)
    {
      scaleAxisUnidirectionalMaps_[i][sign].push_back(std::make_pair(WidgetMovement::SCALE_AXIS_UNIDIRECTIONAL, translateIdsBySide_[i]));
      scaleAxisUnidirectionalMaps_[i][sign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS, scaleAxisIds_[i][sign]));
      scaleAxisUnidirectionalMaps_[i][sign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS, translateIdsByFace_[i][sign]));
      scaleAxisUnidirectionalMaps_[i][sign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS, rotateIdsByFace_[i][sign]));
      scaleAxisUnidirectionalMaps_[i][sign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS, scaleIdsByFace_[i][sign]));
      for(int j = 0; j < DIMENSIONS_; ++j)
        if(i != j)
          for(int l = 0; l < 2; ++l)
          {
            scaleAxisUnidirectionalMaps_[i][sign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS_HALF, scaleAxisIds_[j][l]));
            scaleAxisUnidirectionalMaps_[i][sign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS_HALF, rotateIdsByFace_[j][l]));
          }
    }

  for(auto &w : translateWidgets_)
  {
    w->addMovementMap(WidgetMovement::TRANSLATE,
                      std::make_pair(WidgetMovement::TRANSLATE, allIds_));
    w->addMovementMap(WidgetMovement::TRANSLATE_AXIS,
                      std::make_pair(WidgetMovement::TRANSLATE_AXIS, allIds_));
  }

  for(auto& w : rotateWidgets_)
    w->addMovementMap(WidgetMovement::ROTATE, std::make_pair(WidgetMovement::ROTATE, allIds_));

  for(auto& w : scaleWidgets_)
  {
    w->addMovementMap(WidgetMovement::SCALE, std::make_pair(WidgetMovement::SCALE, allIds_));
    w->addMovementMap(WidgetMovement::SCALE_UNIDIRECTIONAL,
                      std::make_pair(WidgetMovement::SCALE_UNIDIRECTIONAL, allIds_));
  }

  for(int d = 0; d < DIMENSIONS_; ++d)
    for(int sign = 0; sign < 2; ++sign)
      for(auto &w : scaleAxisWidgets_[d][sign])
      {
        for (auto &m : scaleAxisMaps_[d][sign])
          w->addMovementMap(WidgetMovement::SCALE_AXIS, m);
        for (auto &m : scaleAxisUnidirectionalMaps_[d][sign])
          w->addMovementMap(WidgetMovement::SCALE_AXIS_UNIDIRECTIONAL, m);
      }
}

void BoundingBoxWidget::getEigenValuesAndEigenVectors()
{
  eigvals_.reserve(DIMENSIONS_);
  eigvecs_.reserve(DIMENSIONS_);
  for(int d = 0; d < DIMENSIONS_; ++d)
  {
    eigvals_.emplace_back(scaledEigvecs_[d].length());
    eigvecs_.emplace_back(scaledEigvecs_[d] / eigvals_[d]);
  }
}

void BoundingBoxWidget::getCorners()
{
  corners_.reserve(CORNERS_);
  double sign0, sign1, sign2;
  sign0 = sign1 = sign2 = 1.0;
  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 2; ++j)
    {
      for(int k = 0; k < 2; ++k)
      {
        corners_.emplace_back(center_ + sign0 * scaledEigvecs_[0]
                                      + sign1 * scaledEigvecs_[1]
                                      + sign2 * scaledEigvecs_[2]);
        sign2 = -sign2;
      }
      sign1 = -sign1;
    }
    sign0 = -sign0;
  }
}

void BoundingBoxWidget::getFacesStart()
{
  facesStart_.reserve(FACES_);
  for(int d = 0; d < DIMENSIONS_; ++d)
  {
    facesStart_.emplace_back(center_ + scaledEigvecs_[d]);
    facesStart_.emplace_back(center_ - scaledEigvecs_[d]);
  }
}

void BoundingBoxWidget::getFacesEnd()
{
  facesEnd_.reserve(FACES_);
  for(int i = 0; i < FACES_; i+=2)
  {
    auto dist = eigvecs_[i/2] * smallestEigval_ * diskWidth_;
    facesEnd_.emplace_back(facesStart_[i]   + dist);
    facesEnd_.emplace_back(facesStart_[i+1] - dist);
  }
}

void BoundingBoxWidget::addBox(const GeometryIDGenerator& idGenerator, int widgetNum, int widgetIter)
{
  const static std::vector<uint32_t> point_indicies = {
    0, 1,  0, 2,  2, 3,  1, 3,
    4, 5,  4, 6,  6, 7,  5, 7,
    0, 4,  1, 5,  2, 6,  3, 7};

  std::vector<std::vector<std::vector<uint32_t>>> face_indices =
    {{{0, 1, 2, 3},
      {4, 5, 6, 7}},
     {{0, 4, 8, 9},
      {2, 6, 10, 11}},
     {{1, 5, 8, 10},
      {3, 7, 9, 11}}};
  std::vector<std::vector<uint32_t>> side_indices =
    {{8, 9, 10, 11},
     {1, 3, 5, 7},
     {0, 2, 4, 6}};

  for (auto i = 0; i < point_indicies.size(); i += 2)
  {
    std::string cylinderName = widgetName(BoundingBoxWidgetSection::BOX, widgetNum, widgetsIndex_);
    mWidgets.push_back(WidgetFactory::createCylinder(idGenerator, cylinderName,
        smallestEigval_ * scale_ * boxScale_, ColorRGB().toString(),
        corners_[point_indicies[i]], corners_[point_indicies[i + 1]], center_,
        bbox_, resolution_));
    mWidgets[++widgetsIndex_]->setToTranslate(MouseButton::LEFT);
    mWidgets[widgetsIndex_]->setToTranslationAxis(MouseButton::RIGHT,
                               (corners_[point_indicies[i]] - corners_[point_indicies[i + 1]]).normal());
    translateWidgets_.push_back(mWidgets[widgetsIndex_]);
  }
  translateIdsByFace_.resize(DIMENSIONS_, std::vector<std::vector<std::string>>());
  translateIdsBySide_.resize(DIMENSIONS_, std::vector<std::string>());
  for(auto& axis : translateIdsByFace_)
    axis.resize(2, std::vector<std::string>());

  for(int d = 0; d < DIMENSIONS_; ++d)
  {
    for (int sign = 0; sign < 2; ++sign)
      for(auto i : face_indices[d][sign])
        translateIdsByFace_[d][sign].push_back(translateWidgets_[i]->uniqueID());
    for (auto i : side_indices[d])
      translateIdsBySide_[d].push_back(translateWidgets_[i]->uniqueID());
  }
}

void BoundingBoxWidget::addCornerSpheres(const Core::GeometryIDGenerator& idGenerator, int widgetNum, int widgetIter)
{
  std::vector<std::vector<std::vector<uint32_t>>> corner_indices_by_axis =
    {{{0, 1, 2, 3},
      {4, 5, 6, 7}},
     {{0, 1, 4, 5},
      {2, 3, 6, 7}},
     {{0, 2, 4, 6},
      {1, 3, 5, 7}}};

  for(int i = 0; i < CORNERS_; ++i)
  {
    std::string name = widgetName(BoundingBoxWidgetSection::CORNER_SCALE, widgetNum, widgetsIndex_);
    mWidgets.push_back(WidgetFactory::createSphere(idGenerator, name,
                                                   smallestEigval_ * scale_ * resizeSphereScale_,
                                                   resizeCol_, corners_[i], center_, bbox_, resolution_));
    Vector centerToCornerAxis = (corners_[i] - center_).normal();
    mWidgets[++widgetsIndex_]->setToScale(MouseButton::LEFT);
    mWidgets[widgetsIndex_]->setToScaleUnidirectional(MouseButton::RIGHT, centerToCornerAxis);
    scaleWidgets_.push_back(mWidgets[widgetsIndex_]);
  }
  scaleIdsByFace_.resize(DIMENSIONS_, std::vector<std::vector<std::string>>());
  for(auto& axis : scaleIdsByFace_)
    axis.resize(2, std::vector<std::string>());

  for(int d = 0; d < DIMENSIONS_; ++d)
    for(int sign = 0; sign < 2; ++sign)
      for(auto i : corner_indices_by_axis[d][sign])
        scaleIdsByFace_[d][sign].push_back(scaleWidgets_[i]->uniqueID());
}
void BoundingBoxWidget::addFaceSphere(const Core::GeometryIDGenerator& idGenerator,
                                      int widgetNum, int widgetIter)
{
  for (int i = 0; i < FACES_; ++i)
  {
    std::string name = widgetName(BoundingBoxWidgetSection::FACE_ROTATE, widgetNum, widgetsIndex_);
    mWidgets.push_back(WidgetFactory::createSphere(idGenerator, name,
                                                   smallestEigval_ * scale_ * rotSphereScale_,
                                                   deflCol_, facesStart_[i], center_, bbox_, resolution_));
    mWidgets[++widgetsIndex_]->setToRotate(MouseButton::LEFT);
    rotateWidgets_.push_back(mWidgets[widgetsIndex_]);
  }
  rotateIdsByFace_.resize(DIMENSIONS_, std::vector<std::vector<std::string>>());
  for(auto& axis : rotateIdsByFace_)
    axis.resize(2, std::vector<std::string>());

  for(int d = 0; d < DIMENSIONS_; ++d)
    for(int sign = 0; sign < 2; ++sign)
      rotateIdsByFace_[d][sign].push_back(rotateWidgets_[2*d + sign]->uniqueID());
}

void BoundingBoxWidget::addFaceCylinder(const Core::GeometryIDGenerator& idGenerator,
                                        glm::mat4& scaleTrans, int widgetNum, int widgetIter)
{
  scaleAxisWidgets_.resize(DIMENSIONS_, std::vector<std::vector<WidgetHandle>>());
  for(auto &v : scaleAxisWidgets_)
    v.resize(2, std::vector<WidgetHandle>());

  for (int d = 0; d < DIMENSIONS_; ++d)
    for (int sign = 0; sign < 2; ++sign)
    {
      float signMultiplier = (sign == 0) ? 1.0 : -1.0;
      std::string name = widgetName(BoundingBoxWidgetSection::FACE_SCALE, widgetNum, widgetsIndex_);
      mWidgets.push_back(WidgetFactory::createDisk(idGenerator, name,
                                                   smallestEigval_ * scale_ * diskRadius_, resizeCol_,
                                                   facesStart_[2*d + sign], facesEnd_[2*d + sign],
                                                   center_, bbox_, resolution_));
      mWidgets[++widgetsIndex_]->setToScaleAxis(MouseButton::LEFT, signMultiplier * scaledEigvecs_[d],
                                                scaleTrans, d);
      mWidgets[widgetsIndex_]->setToScaleAxisUnidirectional(MouseButton::RIGHT,
                                                            signMultiplier * scaledEigvecs_[d],
                                                            scaleTrans, d);
      scaleAxisWidgets_[d][sign].push_back(mWidgets[widgetsIndex_]);
    }
}

void BoundingBoxWidget::getTranslateIds()
{
  translateIds_.clear();
  for(auto& w : translateWidgets_)
  {
    translateIds_.push_back(w->uniqueID());
    allIds_.push_back(w->uniqueID());
  }
}

void BoundingBoxWidget::getRotateIds()
{
  rotateIds_.clear();
  for(auto& w : rotateWidgets_)
  {
    rotateIds_.push_back(w->uniqueID());
    allIds_.push_back(w->uniqueID());
  }
}

void BoundingBoxWidget::getScaleIds()
{
  scaleIds_.clear();
  for(auto& w : scaleWidgets_)
  {
    scaleIds_.push_back(w->uniqueID());
    allIds_.push_back(w->uniqueID());
  }
}

void BoundingBoxWidget::getScaleAxisIds()
{
  scaleAxisIds_.clear();
  scaleAxisIds_.resize(DIMENSIONS_, std::vector<std::vector<std::string>>());
  for(auto &v : scaleAxisIds_)
    v.resize(2, std::vector<std::string>());
  for(int d = 0; d < DIMENSIONS_; ++d)
    for(int sign = 0; sign < 2; ++sign)
      for(auto& w : scaleAxisWidgets_[d][sign])
      {
        scaleAxisIds_[d][sign].push_back(w->uniqueID());
        allIds_.push_back(w->uniqueID());
      }
}

std::string BoundingBoxWidget::widgetName(size_t i, size_t id, size_t iter)
{
  return "BoundingBoxWidget(" + std::to_string(i) + ")" + "(" + std::to_string(id) + ")" +
         "(" + std::to_string(iter) + ")";
}
