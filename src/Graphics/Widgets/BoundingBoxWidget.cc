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
  : CompositeWidget(idGenerator, name), mScale(scale), mCenter(center), mEigvecs(eigvecs),
    mEigvals(eigvals), mWidgetsIndex(-1)
{
  mScaledEigvecs.reserve(mDIMENSIONS);
  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
    mScaledEigvecs.emplace_back(mEigvecs[iDim] * mEigvals[iDim]);

  initWidgetCreation(idGenerator, widgetNum);
}

BoundingBoxWidget::BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator,
                                     const std::string& name, double scale, const Transform& trans,
                                     const Point& center, int widgetNum)
  : CompositeWidget(idGenerator, name), mScale(scale), mWidgetsIndex(-1)
{
  mCenter = trans.get_translation_point();
  mScaledEigvecs = trans.get_rotation_vectors();
  getEigenValuesAndEigenVectors();
  initWidgetCreation(idGenerator, widgetNum);
}

BoundingBoxWidget::BoundingBoxWidget(const Core::GeometryIDGenerator& idGenerator,
                                     const std::string& name, double scale, const BoxPosition& pos,
                                     const Point& center, int widgetNum, const BBox& bbox)
  : CompositeWidget(idGenerator, name), mScale(scale), mCenter(center), mBbox(bbox), mWidgetsIndex(-1)
{
  Point c,r,d,b;
  pos.getPosition(mCenter, r, d, b);
  auto centerVector = Vector(mCenter);
  mScaledEigvecs = { Vector(r), Vector(d), Vector(b) };
  for(auto& iVec : mScaledEigvecs)
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
  for(int iCol = 0; iCol < mDIMENSIONS; ++iCol)
    for(int iRow = 0; iRow < mDIMENSIONS; ++iRow)
      scaleTrans[iCol][iRow] = mEigvecs[iCol][iRow];

  addFaceCylinder(idGenerator, scaleTrans, widgetNum);
}

void BoundingBoxWidget::initWidgetCreation(const GeometryIDGenerator& idGenerator, int widgetNum)
{
  mSmallestEigval = std::min(mEigvals[0], std::min(mEigvals[1], mEigvals[2]));
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
  mBbox = BBox(mCorners);
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
  for(auto &iWidget : mTranslateWidgets)
    {
      iWidget->addMovementMap(WidgetMovement::TRANSLATE,
                              std::make_pair(WidgetMovement::TRANSLATE, mAllIds));
      iWidget->addMovementMap(WidgetMovement::TRANSLATE_AXIS,
                              std::make_pair(WidgetMovement::TRANSLATE_AXIS, mAllIds));
    }
}

void BoundingBoxWidget::assignRotateMaps()
{
  for(auto& iWidget : mRotateWidgets)
    iWidget->addMovementMap(WidgetMovement::ROTATE, std::make_pair(WidgetMovement::ROTATE, mAllIds));
}

void BoundingBoxWidget::assignScaleMaps()
{
  for(auto& iWidget : mScaleWidgets)
    {
      iWidget->addMovementMap(WidgetMovement::SCALE, std::make_pair(WidgetMovement::SCALE, mAllIds));
      iWidget->addMovementMap(WidgetMovement::SCALE_UNIDIRECTIONAL,
                              std::make_pair(WidgetMovement::SCALE_UNIDIRECTIONAL, mAllIds));
    }
}

void BoundingBoxWidget::assignScaleAxisMaps()
{
  std::vector<std::vector<std::vector<std::pair<WidgetMovement, std::vector<std::string>>>>>
    maps(mDIMENSIONS, std::vector<std::vector<std::pair<WidgetMovement, std::vector<std::string>>>>());
  for(auto &iMapVec : maps)
    iMapVec.resize(2, std::vector<std::pair<WidgetMovement, std::vector<std::string>>>());
  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      for(int iNestedDim = 0; iNestedDim < mDIMENSIONS; ++iNestedDim)
      {
        // Scales the cylinders parallel to the eigenvector.
        maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::SCALE_AXIS,
                                                            mTranslateIdsBySide[iDim]));
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
                                                       mTranslateIdsByFace[iNestedDim][iNestedSign]));
            maps[iDim][iSign].push_back(std::make_pair(plusAndMinusMove[iNestedSign],
                                                       mScaleAxisIds[iNestedDim][iNestedSign]));
            maps[iDim][iSign].push_back(std::make_pair(plusAndMinusMove[iNestedSign],
                                                       mRotateIdsByFace[iNestedDim][iNestedSign]));
            maps[iDim][iSign].push_back(std::make_pair(plusAndMinusMove[iNestedSign],
                                                       mScaleIdsByFace[iNestedDim][iNestedSign]));
          }
        }
      }
  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      for(auto &iWidget : mScaleAxisWidgets[iDim][iSign])
        for (auto &iMap : maps[iDim][iSign])
          iWidget->addMovementMap(WidgetMovement::SCALE_AXIS, iMap);
}

void BoundingBoxWidget::assignScaleAxisUnidirectionalMaps()
{
  std::vector<std::vector<std::vector<std::pair<WidgetMovement, std::vector<std::string>>>>>
    maps(mDIMENSIONS, std::vector<std::vector<std::pair<WidgetMovement, std::vector<std::string>>>>());
  for(auto &iMapVec : maps)
    iMapVec.resize(2, std::vector<std::pair<WidgetMovement, std::vector<std::string>>>());
  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
    {
      // Scales the cylinders parallel to the eigenvector in only one direction.
      maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::SCALE_AXIS_UNIDIRECTIONAL,
                                                 mTranslateIdsBySide[iDim]));

      // Only need to translate widgets in positive direction along eigenvector.
      // Ignores the stationary widgets
      maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS,
                                                 mScaleAxisIds[iDim][iSign]));
      maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS,
                                                 mTranslateIdsByFace[iDim][iSign]));
      maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS,
                                                 mRotateIdsByFace[iDim][iSign]));
      maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS,
                                                 mScaleIdsByFace[iDim][iSign]));

      // Unlike the regular scale axis, the widgets in the center of the box need to move.
      // Here we need to translate them at half distance to stay in the center of the box.
      for(int iNestedDim = 0; iNestedDim < mDIMENSIONS; ++iNestedDim)
        if(iDim != iNestedDim)
          for(int iNestedSign = 0; iNestedSign < 2; ++iNestedSign)
          {
            maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS_HALF,
                                                       mScaleAxisIds[iNestedDim][iNestedSign]));
            maps[iDim][iSign].push_back(std::make_pair(WidgetMovement::TRANSLATE_AXIS_HALF,
                                                       mRotateIdsByFace[iNestedDim][iNestedSign]));
          }
    }
  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      for(auto &iWidget : mScaleAxisWidgets[iDim][iSign])
        for (auto &iMap : maps[iDim][iSign])
          iWidget->addMovementMap(WidgetMovement::SCALE_AXIS_UNIDIRECTIONAL, iMap);
}

void BoundingBoxWidget::getEigenValuesAndEigenVectors()
{
  mEigvals.reserve(mDIMENSIONS);
  mEigvecs.reserve(mDIMENSIONS);
  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
  {
    mEigvals.emplace_back(mScaledEigvecs[iDim].length());
    mEigvecs.emplace_back(mScaledEigvecs[iDim] / mEigvals[iDim]);
  }
}

void BoundingBoxWidget::getCorners()
{
  // NOTE: If you change the order which corners are generated, update the diagram at the top of the file
  mCorners.reserve(mCORNERS);
  double sign0, sign1, sign2;
  sign0 = sign1 = sign2 = 1.0;
  for(int iEig0Sign = 0; iEig0Sign < 2; ++iEig0Sign)
  {
    for(int iEig1Sign = 0; iEig1Sign < 2; ++iEig1Sign)
    {
      for(int iEig2Sign = 0; iEig2Sign < 2; ++iEig2Sign)
      {
        mCorners.emplace_back(mCenter + sign0 * mScaledEigvecs[0]
                                      + sign1 * mScaledEigvecs[1]
                                      + sign2 * mScaledEigvecs[2]);
        sign2 = -sign2;
      }
      sign1 = -sign1;
    }
    sign0 = -sign0;
  }
}

void BoundingBoxWidget::getFacesStart()
{
  mFacesStart.reserve(mFACES);
  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
  {
    mFacesStart.emplace_back(mCenter + mScaledEigvecs[iDim]);
    mFacesStart.emplace_back(mCenter - mScaledEigvecs[iDim]);
  }
}

void BoundingBoxWidget::getFacesEnd()
{
  mFacesEnd.reserve(mFACES);
  for(int iFace = 0; iFace < mFACES; iFace+=2)
  {
    auto dist = mEigvecs[iFace/2] * mSmallestEigval * mDISK_WIDTH * mScale;
    mFacesEnd.emplace_back(mFacesStart[iFace]   + dist);
    mFacesEnd.emplace_back(mFacesStart[iFace+1] - dist);
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
    std::string cylinderName = widgetName(BoundingBoxWidgetSection::BOX, widgetNum, ++mWidgetsIndex);
    mWidgets.push_back(WidgetFactory::createCylinder(idGenerator, cylinderName,
        mSmallestEigval * mScale * mBOX_SCALE, ColorRGB().toString(),
        mCorners[pointIndices[iPoint]], mCorners[pointIndices[iPoint + 1]], mCenter,
        mBbox, mRESOLUTION));
    mWidgets[mWidgetsIndex]->setToTranslate(MouseButton::LEFT);
    mWidgets[mWidgetsIndex]->setToTranslationAxis(MouseButton::RIGHT,
                               (mCorners[pointIndices[iPoint]] - mCorners[pointIndices[iPoint + 1]]).normal());
    mTranslateWidgets.push_back(mWidgets[mWidgetsIndex]);
  }
  mTranslateIdsByFace.resize(mDIMENSIONS, std::vector<std::vector<std::string>>());
  mTranslateIdsBySide.resize(mDIMENSIONS, std::vector<std::string>());
  for(auto& iIdVec : mTranslateIdsByFace)
    iIdVec.resize(2, std::vector<std::string>());

  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
  {
    for (int iSign = 0; iSign < 2; ++iSign)
      for(auto iFace : faceIndices[iDim][iSign])
        mTranslateIdsByFace[iDim][iSign].push_back(mTranslateWidgets[iFace]->uniqueID());
    for (auto iSide : sideIndices[iDim])
      mTranslateIdsBySide[iDim].push_back(mTranslateWidgets[iSide]->uniqueID());
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

  for(int iCorner = 0; iCorner < mCORNERS; ++iCorner)
  {
    std::string name = widgetName(BoundingBoxWidgetSection::CORNER_SCALE, widgetNum, ++mWidgetsIndex);
    mWidgets.push_back(WidgetFactory::createSphere(idGenerator, name,
                                                   mSmallestEigval * mScale * mRESIZE_SPHERE_SCALE,
                                                   mRESIZE_COL, mCorners[iCorner], mCenter, mBbox, mRESOLUTION));
    Vector centerToCornerAxis = (mCorners[iCorner] - mCenter).normal();
    mWidgets[mWidgetsIndex]->setToScale(MouseButton::LEFT);
    mWidgets[mWidgetsIndex]->setToScaleUnidirectional(MouseButton::RIGHT, centerToCornerAxis);
    mScaleWidgets.push_back(mWidgets[mWidgetsIndex]);
  }
  mScaleIdsByFace.resize(mDIMENSIONS, std::vector<std::vector<std::string>>());
  for(auto& iIdVec : mScaleIdsByFace)
    iIdVec.resize(2, std::vector<std::string>());

  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      for(auto iCornerAxis : cornerIndicesByAxis[iDim][iSign])
        mScaleIdsByFace[iDim][iSign].push_back(mScaleWidgets[iCornerAxis]->uniqueID());
}
void BoundingBoxWidget::addFaceSphere(const Core::GeometryIDGenerator& idGenerator,
                                      int widgetNum)
{
  for (int iFace = 0; iFace < mFACES; ++iFace)
  {
    std::string name = widgetName(BoundingBoxWidgetSection::FACE_ROTATE, widgetNum, ++mWidgetsIndex);
    mWidgets.push_back(WidgetFactory::createSphere(idGenerator, name,
                                                   mSmallestEigval * mScale * mROT_SPHERE_SCALE,
                                                   mDEFL_COL, mFacesStart[iFace], mCenter, mBbox, mRESOLUTION));
    mWidgets[mWidgetsIndex]->setToRotate(MouseButton::LEFT);
    mRotateWidgets.push_back(mWidgets[mWidgetsIndex]);
  }
  mRotateIdsByFace.resize(mDIMENSIONS, std::vector<std::vector<std::string>>());
  for(auto& iIdVec : mRotateIdsByFace)
    iIdVec.resize(2, std::vector<std::string>());

  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      mRotateIdsByFace[iDim][iSign].push_back(mRotateWidgets[2*iDim + iSign]->uniqueID());
}

void BoundingBoxWidget::addFaceCylinder(const Core::GeometryIDGenerator& idGenerator,
                                        glm::mat4& scaleTrans, int widgetNum)
{
  mScaleAxisWidgets.resize(mDIMENSIONS, std::vector<std::vector<WidgetHandle>>());
  for(auto &iWidgetVec : mScaleAxisWidgets)
    iWidgetVec.resize(2, std::vector<WidgetHandle>());

  for (int iDim = 0; iDim < mDIMENSIONS; ++iDim)
    for (int iSign = 0; iSign < 2; ++iSign)
    {
      float signMultiplier = (iSign == 0) ? 1.0 : -1.0;
      std::string name = widgetName(BoundingBoxWidgetSection::FACE_SCALE, widgetNum, ++mWidgetsIndex);
      mWidgets.push_back(WidgetFactory::createDisk(idGenerator, name,
                                                   mSmallestEigval * mScale * mDISK_RADIUS, mRESIZE_COL,
                                                   mFacesStart[2*iDim + iSign], mFacesEnd[2*iDim + iSign],
                                                   mCenter, mBbox, mRESOLUTION));
      mWidgets[mWidgetsIndex]->setToScaleAxis(MouseButton::LEFT, signMultiplier * mScaledEigvecs[iDim],
                                                scaleTrans, iDim);
      mWidgets[mWidgetsIndex]->setToScaleAxisUnidirectional(MouseButton::RIGHT,
                                                            signMultiplier * mScaledEigvecs[iDim],
                                                            scaleTrans, iDim);
      mScaleAxisWidgets[iDim][iSign].push_back(mWidgets[mWidgetsIndex]);
    }
}

void BoundingBoxWidget::getTranslateIds()
{
  mTranslateIds.clear();
  for(auto& iWidget : mTranslateWidgets)
  {
    mTranslateIds.push_back(iWidget->uniqueID());
    mAllIds.push_back(iWidget->uniqueID());
  }
}

void BoundingBoxWidget::getRotateIds()
{
  mRotateIds.clear();
  for(auto& iWidget : mRotateWidgets)
  {
    mRotateIds.push_back(iWidget->uniqueID());
    mAllIds.push_back(iWidget->uniqueID());
  }
}

void BoundingBoxWidget::getScaleIds()
{
  mScaleIds.clear();
  for(auto& iWidget : mScaleWidgets)
  {
    mScaleIds.push_back(iWidget->uniqueID());
    mAllIds.push_back(iWidget->uniqueID());
  }
}

void BoundingBoxWidget::getScaleAxisIds()
{
  mScaleAxisIds.clear();
  mScaleAxisIds.resize(mDIMENSIONS, std::vector<std::vector<std::string>>());
  for(auto &iIdVec : mScaleAxisIds)
    iIdVec.resize(2, std::vector<std::string>());
  for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
    for(int iSign = 0; iSign < 2; ++iSign)
      for(auto& iWidget : mScaleAxisWidgets[iDim][iSign])
      {
        mScaleAxisIds[iDim][iSign].push_back(iWidget->uniqueID());
        mAllIds.push_back(iWidget->uniqueID());
      }
}

std::string BoundingBoxWidget::widgetName(size_t i, size_t id, size_t iter)
{
  return "BoundingBoxWidget(" + std::to_string(i) + ")" + "(" + std::to_string(id) + ")" +
         "(" + std::to_string(iter) + ")";
}
