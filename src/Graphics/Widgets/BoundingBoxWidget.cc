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
#include "Graphics/Widgets/Widget.h"
#include "glm/gtx/string_cast.hpp"

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

// Index diagram for corners and edges:
// NOTE: This ordering is based on the order corners are generated.
// If you change order of corners, update this diagram.
//
//          (0)          1            (2)
//           .+-----------------------+
//         .' |                     .'|
//       8.'  |                 10.'  |
//      .'    |     5           .'    |
//  (4)+------+----------------+(6)   |
//     |      |                |      |
//     |      |                |      |
//     |    0 |                |      | 2
//     |      |                |      |
//    4|      |               6|      |
//     |      |                |      |
//     |      |                |      |
//     |   (1)+----------------+------+(3)
//     |    .'          3      |    .'
//     |  .' 9                 |  .' 11
//     |.'                     |.'
//     +-----------------------+
//   (5)            7          (7)
//
//  Legend: (#) - corner index
//           #  - edge index

BoundingBoxWidget::BoundingBoxWidget(const GeometryIDGenerator& idGenerator, const std::string& name,
                                     double scale, const Point& center,
                                     const std::vector<Vector>& eigvecs,
                                     const std::vector<double>& eigvals, int widgetNum)
  : CompositeWidget(idGenerator, name), scale_(scale), center_(center), eigvecs_(eigvecs),
    eigvals_(eigvals), widgetsIndex_(-1)
{
  scaledEigvecs_.reserve(DIMENSIONS_);
  for(int iDim = 0; iDim < DIMENSIONS_; ++iDim)
    scaledEigvecs_.emplace_back(eigvecs_[iDim] * eigvals_[iDim]);

  initWidgetCreation(idGenerator, widgetNum);
}

BoundingBoxWidget::BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator,
                                     const std::string& name, double scale, const Transform& trans,
                                     const Point& center, int widgetNum)
  : CompositeWidget(idGenerator, name), scale_(scale), widgetsIndex_(-1)
{
  center_ = trans.get_translation_point();
  scaledEigvecs_ = trans.get_rotation_vectors();
  getEigenValuesAndEigenVectors();
  initWidgetCreation(idGenerator, widgetNum);
}

BoundingBoxWidget::BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator,
                                     const std::string& name, double scale, const BoxPosition& pos,
                                     const Point& center, int widgetNum, const BBox& bbox)
  : CompositeWidget(idGenerator, name), scale_(scale), center_(center), bbox_(bbox), widgetsIndex_(-1)
{
  Point c,r,d,b;
  pos.getPosition(center_, r, d, b);
  auto centerVector = Vector(center_);
  scaledEigvecs_ = { Vector(r), Vector(d), Vector(b) };
  for(auto& iVec : scaledEigvecs_)
    iVec -= centerVector;

  getEigenValuesAndEigenVectors();
  initWidgetCreation(idGenerator, widgetNum);
}

void BoundingBoxWidget::createWidgets(const GeometryIDGenerator& idGenerator, int widgetNum)
{
  addBox(idGenerator, widgetNum);
  addCornerSpheres(idGenerator, widgetNum);
  addFaceSphere(idGenerator, widgetNum);

  auto scaleTrans = glm::mat4(1.0);
  for(int iCol = 0; iCol < DIMENSIONS_; ++iCol)
    for(int iRow = 0; iRow < DIMENSIONS_; ++iRow)
      scaleTrans[iCol][iRow] = eigvecs_[iCol][iRow];

  addFaceCylinder(idGenerator, scaleTrans, widgetNum);
}

void BoundingBoxWidget::initWidgetCreation(const GeometryIDGenerator& idGenerator, int widgetNum)
{
  smallestEigval_ = std::min(eigvals_[0], std::min(eigvals_[1], eigvals_[2]));
  generateWidgetPoints();
  createWidgets(idGenerator, widgetNum);
  assignIds();
  assignMoveMaps();
}

void BoundingBoxWidget::generateWidgetPoints()
{
  getCorners();
  getFacesStart();
  getFacesEnd();
  bbox_ = BBox(corners_);
}

void BoundingBoxWidget::assignIds()
{
  getTranslateIds();
  getRotateIds();
  getScaleIds();
  getScaleAxisIds();
}

void BoundingBoxWidget::assignMoveMaps()
{
  assignTranslateMaps();
  assignRotateMaps();
  assignScaleMaps();
  assignScaleAxisMaps();
  assignScaleAxisUnidirectionalMaps();
}

void BoundingBoxWidget::assignTranslateMaps()
{
  for(auto &iWidget : translateWidgets_)
    {
      iWidget->addMovementMap(WidgetMovement::TRANSLATE,
                              std::make_pair(WidgetMovement::TRANSLATE, allIds_));
      iWidget->addMovementMap(WidgetMovement::TRANSLATE_AXIS,
                              std::make_pair(WidgetMovement::TRANSLATE_AXIS, allIds_));
    }
}

void BoundingBoxWidget::assignRotateMaps()
{
  for(auto& iWidget : rotateWidgets_)
    iWidget->addMovementMap(WidgetMovement::ROTATE, std::make_pair(WidgetMovement::ROTATE, allIds_));
}

void BoundingBoxWidget::assignScaleMaps()
{
  for(auto& iWidget : scaleWidgets_)
    {
      iWidget->addMovementMap(WidgetMovement::SCALE, std::make_pair(WidgetMovement::SCALE, allIds_));
      iWidget->addMovementMap(WidgetMovement::SCALE_UNIDIRECTIONAL,
                              std::make_pair(WidgetMovement::SCALE_UNIDIRECTIONAL, allIds_));
    }
}

void BoundingBoxWidget::assignScaleAxisMaps()
{
  std::vector<std::vector<std::vector<std::pair<WidgetMovement, std::vector<std::string>>>>>
    maps(DIMENSIONS_, std::vector<std::vector<std::pair<WidgetMovement, std::vector<std::string>>>>());
  for(auto &iMapVec : maps)
    iMapVec.resize(2, std::vector<std::pair<WidgetMovement, std::vector<std::string>>>());
  for(int iDim = 0; iDim < DIMENSIONS_; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      for(int iNestedDim = 0; iNestedDim < DIMENSIONS_; ++iNestedDim)
      {
        // Scales the cylinders parallel to the eigenvector.
        maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::SCALE_AXIS,
                                                            translateIdsBySide_[iDim]));
        if(iDim == iNestedDim)
        {
          // Positive and negative directions along an eigenvector will translate these widgets in
          // opposite directions. Ignores the stationary widgets.
          std::vector<WidgetMovement> plusAndMinusMove(2);
          if(iSign == 0)
            plusAndMinusMove = {WidgetMovement::TRANSLATE_AXIS,
                                WidgetMovement::TRANSLATE_AXIS_REVERSE};
          else
            plusAndMinusMove = {WidgetMovement::TRANSLATE_AXIS_REVERSE,
                                WidgetMovement::TRANSLATE_AXIS};
          for(int iNestedSign = 0; iNestedSign < 2; ++iNestedSign)
          {
            maps[iDim][iSign].push_back(std::make_pair(plusAndMinusMove[iNestedSign],
                                                       translateIdsByFace_[iNestedDim][iNestedSign]));
            maps[iDim][iSign].push_back(std::make_pair(plusAndMinusMove[iNestedSign],
                                                       scaleAxisIds_[iNestedDim][iNestedSign]));
            maps[iDim][iSign].push_back(std::make_pair(plusAndMinusMove[iNestedSign],
                                                       rotateIdsByFace_[iNestedDim][iNestedSign]));
            maps[iDim][iSign].push_back(std::make_pair(plusAndMinusMove[iNestedSign],
                                                       scaleIdsByFace_[iNestedDim][iNestedSign]));
          }
        }
      }
  for(int iDim = 0; iDim < DIMENSIONS_; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      for(auto &iWidget : scaleAxisWidgets_[iDim][iSign])
        for (auto &iMap : maps[iDim][iSign])
          iWidget->addMovementMap(WidgetMovement::SCALE_AXIS, iMap);
}

void BoundingBoxWidget::assignScaleAxisUnidirectionalMaps()
{
  std::vector<std::vector<std::vector<std::pair<WidgetMovement, std::vector<std::string>>>>>
    maps(DIMENSIONS_, std::vector<std::vector<std::pair<WidgetMovement, std::vector<std::string>>>>());
  for(auto &iMapVec : maps)
    iMapVec.resize(2, std::vector<std::pair<WidgetMovement, std::vector<std::string>>>());
  for(int iDim = 0; iDim < DIMENSIONS_; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
    {
      // Scales the cylinders parallel to the eigenvector in only one direction.
      maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::SCALE_AXIS_UNIDIRECTIONAL,
                                                 translateIdsBySide_[iDim]));

      // Only need to translate widgets in positive direction along eigenvector.
      // Ignores the stationary widgets
      maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS,
                                                 scaleAxisIds_[iDim][iSign]));
      maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS,
                                                 translateIdsByFace_[iDim][iSign]));
      maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS,
                                                 rotateIdsByFace_[iDim][iSign]));
      maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS,
                                                 scaleIdsByFace_[iDim][iSign]));

      // Unlike the regular scale axis, the widgets in the center of the box need to move.
      // Here we need to translate them at half distance to stay in the center of the box.
      for(int iNestedDim = 0; iNestedDim < DIMENSIONS_; ++iNestedDim)
        if(iDim != iNestedDim)
          for(int iNestedSign = 0; iNestedSign < 2; ++iNestedSign)
          {
            maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS_HALF,
                                                       scaleAxisIds_[iNestedDim][iNestedSign]));
            maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS_HALF,
                                                       rotateIdsByFace_[iNestedDim][iNestedSign]));
          }
    }
  for(int iDim = 0; iDim < DIMENSIONS_; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      for(auto &iWidget : scaleAxisWidgets_[iDim][iSign])
        for (auto &iMap : maps[iDim][iSign])
          iWidget->addMovementMap(WidgetMovement::SCALE_AXIS_UNIDIRECTIONAL, iMap);
}

void BoundingBoxWidget::getEigenValuesAndEigenVectors()
{
  eigvals_.reserve(DIMENSIONS_);
  eigvecs_.reserve(DIMENSIONS_);
  for(int iDim = 0; iDim < DIMENSIONS_; ++iDim)
  {
    eigvals_.emplace_back(scaledEigvecs_[iDim].length());
    eigvecs_.emplace_back(scaledEigvecs_[iDim] / eigvals_[iDim]);
  }
}

void BoundingBoxWidget::getCorners()
{
  // NOTE: If you change the order which corners are generated, update the diagram at the top of the file
  corners_.reserve(CORNERS_);
  double sign0, sign1, sign2;
  sign0 = sign1 = sign2 = 1.0;
  for(int iEig0Sign = 0; iEig0Sign < 2; ++iEig0Sign)
  {
    for(int iEig1Sign = 0; iEig1Sign < 2; ++iEig1Sign)
    {
      for(int iEig2Sign = 0; iEig2Sign < 2; ++iEig2Sign)
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
  for(int iDim = 0; iDim < DIMENSIONS_; ++iDim)
  {
    facesStart_.emplace_back(center_ + scaledEigvecs_[iDim]);
    facesStart_.emplace_back(center_ - scaledEigvecs_[iDim]);
  }
}

void BoundingBoxWidget::getFacesEnd()
{
  facesEnd_.reserve(FACES_);
  for(int iFace = 0; iFace < FACES_; iFace+=2)
  {
    auto dist = eigvecs_[iFace/2] * smallestEigval_ * DISK_WIDTH_ * scale_;
    facesEnd_.emplace_back(facesStart_[iFace]   + dist);
    facesEnd_.emplace_back(facesStart_[iFace+1] - dist);
  }
}

void BoundingBoxWidget::addBox(const GeometryIDGenerator& idGenerator, int widgetNum)
{
  // NOTE: See index diagram at the top of the file

  // For accessing the list of corner points.
  // These points are grouped by pairs to make the edges.
  const static std::vector<uint32_t> pointIndices =
    {0, 1,  0, 2,  2, 3,  1, 3,
     4, 5,  4, 6,  6, 7,  5, 7,
     0, 4,  1, 5,  2, 6,  3, 7};

  // These indices access the edges to get the cylinders that are orthogonal to the eigenvector.
  // Each index matches to the point pairs in pointIndices.
  const static std::vector<std::vector<std::vector<uint32_t>>> faceIndices =
    {{{0, 1, 2, 3},
      {4, 5, 6, 7}},
     {{0, 4, 8, 9},
      {2, 6, 10, 11}},
     {{1, 5, 8, 10},
      {3, 7, 9, 11}}};

  // These indices access the edges to get the cylinders that are parallel to the eigenvector.
  // Each index matches to the point pairs in pointIndices.
  const static std::vector<std::vector<uint32_t>> sideIndices =
    {{8, 9, 10, 11},
     {1, 3, 5, 7},
     {0, 2, 4, 6}};

  for (auto iPoint = 0; iPoint < pointIndices.size(); iPoint += 2)
  {
    std::string cylinderName = widgetName(BoundingBoxWidgetSection::BOX, widgetNum, ++widgetsIndex_);
    widgets_.push_back(WidgetFactory::createCylinder(idGenerator, cylinderName,
        smallestEigval_ * scale_ * BOX_SCALE_, ColorRGB().toString(),
        corners_[pointIndices[iPoint]], corners_[pointIndices[iPoint + 1]], center_,
        bbox_, RESOLUTION_));
    widgets_[widgetsIndex_]->setToTranslate(MouseButton::LEFT);
    widgets_[widgetsIndex_]->setToTranslationAxis(MouseButton::RIGHT,
                               (corners_[pointIndices[iPoint]] - corners_[pointIndices[iPoint + 1]]).normal());
    translateWidgets_.push_back(widgets_[widgetsIndex_]);
  }
  translateIdsByFace_.resize(DIMENSIONS_, std::vector<std::vector<std::string>>());
  translateIdsBySide_.resize(DIMENSIONS_, std::vector<std::string>());
  for(auto& iIdVec : translateIdsByFace_)
    iIdVec.resize(2, std::vector<std::string>());

  for(int iDim = 0; iDim < DIMENSIONS_; ++iDim)
  {
    for (int iSign = 0; iSign < 2; ++iSign)
      for(auto iFace : faceIndices[iDim][iSign])
        translateIdsByFace_[iDim][iSign].push_back(translateWidgets_[iFace]->uniqueID());
    for (auto iSide : sideIndices[iDim])
      translateIdsBySide_[iDim].push_back(translateWidgets_[iSide]->uniqueID());
  }
}

void BoundingBoxWidget::addCornerSpheres(const Core::GeometryIDGenerator& idGenerator, int widgetNum)
{
  // See index diagram at the top of the file

  // For accessing the list of corner points.
  // This groups corner points based on eigenvector.
  const static std::vector<std::vector<std::vector<uint32_t>>> cornerIndicesByAxis =
    {{{0, 1, 2, 3},
      {4, 5, 6, 7}},
     {{0, 1, 4, 5},
      {2, 3, 6, 7}},
     {{0, 2, 4, 6},
      {1, 3, 5, 7}}};

  for(int iCorner = 0; iCorner < CORNERS_; ++iCorner)
  {
    std::string name = widgetName(BoundingBoxWidgetSection::CORNER_SCALE, widgetNum, ++widgetsIndex_);
    widgets_.push_back(WidgetFactory::createSphere(idGenerator, name,
                                                   smallestEigval_ * scale_ * RESIZE_SPHERE_SCALE_,
                                                   RESIZE_COL_, corners_[iCorner], center_, bbox_, RESOLUTION_));
    Vector centerToCornerAxis = (corners_[iCorner] - center_).normal();
    widgets_[widgetsIndex_]->setToScale(MouseButton::LEFT, false);
    widgets_[widgetsIndex_]->setToScaleUnidirectional(MouseButton::RIGHT, centerToCornerAxis, false);
    scaleWidgets_.push_back(widgets_[widgetsIndex_]);
  }
  scaleIdsByFace_.resize(DIMENSIONS_, std::vector<std::vector<std::string>>());
  for(auto& iIdVec : scaleIdsByFace_)
    iIdVec.resize(2, std::vector<std::string>());

  for(int iDim = 0; iDim < DIMENSIONS_; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      for(auto iCornerAxis : cornerIndicesByAxis[iDim][iSign])
        scaleIdsByFace_[iDim][iSign].push_back(scaleWidgets_[iCornerAxis]->uniqueID());
}
void BoundingBoxWidget::addFaceSphere(const Core::GeometryIDGenerator& idGenerator,
                                      int widgetNum)
{
  for (int iFace = 0; iFace < FACES_; ++iFace)
  {
    std::string name = widgetName(BoundingBoxWidgetSection::FACE_ROTATE, widgetNum, ++widgetsIndex_);
    widgets_.push_back(WidgetFactory::createSphere(idGenerator, name,
                                                   smallestEigval_ * scale_ * ROT_SPHERE_SCALE_,
                                                   DEFL_COL_, facesStart_[iFace], center_, bbox_, RESOLUTION_));
    widgets_[widgetsIndex_]->setToRotate(MouseButton::LEFT);
    widgets_[widgetsIndex_]->setToRotate(MouseButton::RIGHT);
    rotateWidgets_.push_back(widgets_[widgetsIndex_]);
  }
  rotateIdsByFace_.resize(DIMENSIONS_, std::vector<std::vector<std::string>>());
  for(auto& iIdVec : rotateIdsByFace_)
    iIdVec.resize(2, std::vector<std::string>());

  for(int iDim = 0; iDim < DIMENSIONS_; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      rotateIdsByFace_[iDim][iSign].push_back(rotateWidgets_[2*iDim + iSign]->uniqueID());
}

void BoundingBoxWidget::addFaceCylinder(const Core::GeometryIDGenerator& idGenerator,
                                        glm::mat4& scaleTrans, int widgetNum)
{
  scaleAxisWidgets_.resize(DIMENSIONS_, std::vector<std::vector<WidgetHandle>>());
  for(auto &iWidgetVec : scaleAxisWidgets_)
    iWidgetVec.resize(2, std::vector<WidgetHandle>());

  for (int iDim = 0; iDim < DIMENSIONS_; ++iDim)
    for (int iSign = 0; iSign < 2; ++iSign)
    {
      float signMultiplier = (iSign == 0) ? 1.0 : -1.0;
      std::string name = widgetName(BoundingBoxWidgetSection::FACE_SCALE, widgetNum, ++widgetsIndex_);
      widgets_.push_back(WidgetFactory::createDisk(idGenerator, name,
                                                   smallestEigval_ * scale_ * DISK_RADIUS_, DISK_COL_,
                                                   facesStart_[2*iDim + iSign], facesEnd_[2*iDim + iSign],
                                                   center_, bbox_, RESOLUTION_));
      widgets_[widgetsIndex_]->setToScaleAxis(MouseButton::LEFT, signMultiplier * scaledEigvecs_[iDim],
                                              scaleTrans, iDim, false);
      widgets_[widgetsIndex_]->setToScaleAxisUnidirectional(MouseButton::RIGHT,
                                                            signMultiplier * scaledEigvecs_[iDim],
                                                            scaleTrans, iDim, false);
      scaleAxisWidgets_[iDim][iSign].push_back(widgets_[widgetsIndex_]);
    }
}

void BoundingBoxWidget::getTranslateIds()
{
  translateIds_.clear();
  for(auto& iWidget : translateWidgets_)
  {
    translateIds_.push_back(iWidget->uniqueID());
    allIds_.push_back(iWidget->uniqueID());
  }
}

void BoundingBoxWidget::getRotateIds()
{
  rotateIds_.clear();
  for(auto& iWidget : rotateWidgets_)
  {
    rotateIds_.push_back(iWidget->uniqueID());
    allIds_.push_back(iWidget->uniqueID());
  }
}

void BoundingBoxWidget::getScaleIds()
{
  scaleIds_.clear();
  for(auto& iWidget : scaleWidgets_)
  {
    scaleIds_.push_back(iWidget->uniqueID());
    allIds_.push_back(iWidget->uniqueID());
  }
}

void BoundingBoxWidget::getScaleAxisIds()
{
  scaleAxisIds_.clear();
  scaleAxisIds_.resize(DIMENSIONS_, std::vector<std::vector<std::string>>());
  for(auto &iIdVec : scaleAxisIds_)
    iIdVec.resize(2, std::vector<std::string>());
  for(int iDim = 0; iDim < DIMENSIONS_; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      for(auto& iWidget : scaleAxisWidgets_[iDim][iSign])
      {
        scaleAxisIds_[iDim][iSign].push_back(iWidget->uniqueID());
        allIds_.push_back(iWidget->uniqueID());
      }
}

std::string BoundingBoxWidget::widgetName(size_t i, size_t id, size_t iter)
{
  return "BoundingBoxWidget(" + std::to_string(i) + ")" + "(" + std::to_string(id) + ")" +
         "(" + std::to_string(iter) + ")";
}
