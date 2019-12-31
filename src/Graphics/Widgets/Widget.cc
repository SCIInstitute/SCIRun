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

#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/share.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;

WidgetBase::WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable)
  : GeometryObjectSpire(idGenerator, tag, isClippable),
    mMovementInfo(std::vector<WidgetInfo>(MouseButton::STATE_COUNT, WidgetInfo(WidgetMovement::NONE)))
{}

WidgetBase::WidgetBase(const Core::GeometryIDGenerator &idGenerator, const std::string &tag, bool isClippable, const Point &origin)
    : GeometryObjectSpire(idGenerator, tag, isClippable),
      mOrigin(glm::vec3(origin.x(), origin.y(), origin.z())),
      mMovementInfo(std::vector<WidgetInfo>(MouseButton::STATE_COUNT, WidgetInfo(WidgetMovement::NONE)))

{}

WidgetBase::WidgetBase(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable, const Point& pos, const Point& origin)
  : GeometryObjectSpire(idGenerator, tag, isClippable),
    mOrigin(glm::vec3(origin.x(), origin.y(), origin.z())), mPosition(pos),
    mMovementInfo(std::vector<WidgetInfo>(MouseButton::STATE_COUNT, WidgetInfo(WidgetMovement::NONE)))
{}

void WidgetBase::addMovementMap(WidgetMovement key, std::pair<WidgetMovement, std::vector<std::string>> moves)
{
  if(mMoveMap.count(key))
    mMoveMap[key].push_back(moves);
  else
    mMoveMap[key] = {moves};
}

Point WidgetBase::position() const
{
  return mPosition;
}

void WidgetBase::setPosition(const Point& p)
{
  mPosition = p;
}

void WidgetBase::setToTranslationAxis(MouseButton btn, const Vector& v)
{
  if(btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::TRANSLATE_AXIS);
  info.translationAxis = glm::vec3(v.x(), v.y(), v.z());
  mMovementInfo[btn] = info;
}

void WidgetBase::setToScale(MouseButton btn, const Vector &flipAxis, bool negate)
{
  if (btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::SCALE);
  info.flipAxis = glm::vec3(flipAxis.x(), flipAxis.y(), flipAxis.z());
  info.flipInvertedWidget = true;
  info.negate = negate;
  mMovementInfo[btn] = info;
}

void WidgetBase::setToScale(MouseButton btn, bool negate)
{
  if (btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::SCALE);
  info.flipInvertedWidget = false;
  info.negate = negate;
  mMovementInfo[btn] = info;
}

void WidgetBase::setToScaleAxis(MouseButton btn, const Vector& scaleAxis, const Vector& flipAxis,
                                glm::mat4 scaleTrans, int scaleAxisIndex, bool negate)
{
  if(btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::SCALE_AXIS);
  info.scaleAxis = glm::vec3(scaleAxis.x(), scaleAxis.y(), scaleAxis.z());
  Vector scaleAxisNormal = scaleAxis.normal();
  info.translationAxis = glm::vec3(scaleAxisNormal.x(), scaleAxisNormal.y(), scaleAxisNormal.z());
  info.flipAxis = glm::vec3(flipAxis.x(), flipAxis.y(), flipAxis.z());
  info.scaleAxisIndex = scaleAxisIndex;
  info.scaleTrans = scaleTrans;
  info.flipInvertedWidget = true;
  info.negate = negate;
  mMovementInfo[btn] = info;
}

void WidgetBase::setToScaleAxis(MouseButton btn, const Vector& scaleAxis, glm::mat4 scaleTrans,
                                int scaleAxisIndex, bool negate)
{
  if(btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::SCALE_AXIS);
  info.scaleAxis = glm::vec3(scaleAxis.x(), scaleAxis.y(), scaleAxis.z());
  Vector scaleAxisNormal = scaleAxis.normal();
  info.translationAxis = glm::vec3(scaleAxisNormal.x(), scaleAxisNormal.y(), scaleAxisNormal.z());
  info.scaleAxisIndex = scaleAxisIndex;
  info.scaleTrans = scaleTrans;
  info.flipInvertedWidget = false;
  info.negate = negate;
  mMovementInfo[btn] = info;
}

void WidgetBase::setToScaleUnidirectional(MouseButton btn, const Vector &translationAxis,
                                          const Vector &flipAxis, bool negate)
{
  if (btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::SCALE_UNIDIRECTIONAL);
  info.translationAxis = glm::vec3(translationAxis.x(), translationAxis.y(), translationAxis.z());
  info.flipAxis = glm::vec3(flipAxis.x(), flipAxis.y(), flipAxis.z());
  info.flipInvertedWidget = true;
  info.negate = negate;
  mMovementInfo[btn] = info;
}

void WidgetBase::setToScaleUnidirectional(MouseButton btn, const Vector &translationAxis, bool negate)
{
  if (btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::SCALE_UNIDIRECTIONAL);
  info.translationAxis = glm::vec3(translationAxis.x(), translationAxis.y(), translationAxis.z());
  info.flipInvertedWidget = false;
  info.negate = negate;
  mMovementInfo[btn] = info;
}

void WidgetBase::setToScaleAxisUnidirectional(MouseButton btn, const Vector& scaleAxis,
                                              const Vector& flipAxis, glm::mat4 scaleTrans,
                                              int scaleAxisIndex, bool negate)
{
  if(btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::SCALE_AXIS_UNIDIRECTIONAL);
  info.scaleAxis = glm::vec3(scaleAxis.x(), scaleAxis.y(), scaleAxis.z());
  Vector scaleAxisNormal = scaleAxis.normal();
  info.translationAxis = glm::vec3(scaleAxisNormal.x(), scaleAxisNormal.y(), scaleAxisNormal.z());
  info.flipAxis = glm::vec3(flipAxis.x(), flipAxis.y(), flipAxis.z());
  info.translationAxis = info.scaleAxis;
  info.scaleAxisIndex = scaleAxisIndex;
  info.scaleTrans = scaleTrans;
  info.flipInvertedWidget = true;
  info.negate = negate;
  mMovementInfo[btn] = info;
}

void WidgetBase::setToScaleAxisUnidirectional(MouseButton btn, const Vector& scaleAxis,
                                              glm::mat4 scaleTrans, int scaleAxisIndex, bool negate)
{
  if(btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::SCALE_AXIS_UNIDIRECTIONAL);
  info.scaleAxis = glm::vec3(scaleAxis.x(), scaleAxis.y(), scaleAxis.z());
  Vector scaleAxisNormal = scaleAxis.normal();
  info.translationAxis = glm::vec3(scaleAxisNormal.x(), scaleAxisNormal.y(), scaleAxisNormal.z());
  info.scaleAxisIndex = scaleAxisIndex;
  info.scaleTrans = scaleTrans;
  info.flipInvertedWidget = false;
  info.negate = negate;
  mMovementInfo[btn] = info;
}

void WidgetBase::setToScaleAxisHalf(MouseButton btn, const Vector &scaleAxis,
                                    const Vector& flipAxis, glm::mat4 scaleTrans,
                                    int scaleAxisIndex, bool negate)
{
  if (btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::SCALE_AXIS_HALF);
  info.scaleAxis = glm::vec3(scaleAxis.x(), scaleAxis.y(), scaleAxis.z());
  Vector scaleAxisNormal = scaleAxis.normal();
  info.translationAxis = glm::vec3(scaleAxisNormal.x(), scaleAxisNormal.y(), scaleAxisNormal.z());
  info.flipAxis = glm::vec3(flipAxis.x(), flipAxis.y(), flipAxis.z());
  info.scaleAxisIndex = scaleAxisIndex;
  info.scaleTrans = scaleTrans;
  info.flipInvertedWidget = true;
  info.negate = negate;
  mMovementInfo[btn] = info;
}

void WidgetBase::setToScaleAxisHalf(MouseButton btn, const Vector &scaleAxis,
                                    glm::mat4 scaleTrans, int scaleAxisIndex, bool negate)
{
  if (btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::SCALE_AXIS_HALF);
  info.scaleAxis = glm::vec3(scaleAxis.x(), scaleAxis.y(), scaleAxis.z());
  Vector scaleAxisNormal = scaleAxis.normal();
  info.translationAxis = glm::vec3(scaleAxisNormal.x(), scaleAxisNormal.y(), scaleAxisNormal.z());
  info.scaleAxisIndex = scaleAxisIndex;
  info.scaleTrans = scaleTrans;
  info.flipInvertedWidget = false;
  info.negate = negate;
  mMovementInfo[btn] = info;
}

void WidgetBase::setToRotate(MouseButton btn)
{
  if (btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::ROTATE);
  mMovementInfo[btn] = info;
}

void WidgetBase::setToTranslate(MouseButton btn)
{
  if (btn == MouseButton::NONE) return;
  WidgetInfo info(WidgetMovement::TRANSLATE);
  mMovementInfo[btn] = info;
}

// glm::vec3 WidgetBase::getScaleVector()
// {
  // return mScaleAxis;
// }

// glm::vec3 WidgetBase::getFlipVector()
// {
  // return mFlipAxis;
// }

// glm::vec3 WidgetBase::getTranslationVector()
// {
  // return mTranslationAxis;
// }

// int WidgetBase::getScaleAxisIndex()
// {
  // return mScaleAxisIndex;
// }

// bool WidgetBase::getFlipInvertedWidget()
// {
  // return mFlipInvertedWidget;
// }

// const glm::mat4 WidgetBase::getScaleTransform()
// {
  // return mScaleTrans;
// }

std::vector<WidgetInfo> WidgetBase::getMovementInfo()
{
  return mMovementInfo;
}

void WidgetBase::addInitialId() {
  auto ids = std::vector<std::string>(1);
  ids.push_back(uniqueID());
  // connectedIds_ = ids;
}

void CompositeWidget::addToList(GeometryBaseHandle handle, GeomList& list)
{
  if (handle.get() == this)
  {
    list.insert(mWidgets.begin(), mWidgets.end());
  }
}

void CompositeWidget::addToList(WidgetHandle widget)
{
  mWidgets.push_back(widget);
}

std::vector<std::string> CompositeWidget::getListOfConnectedIds()
{
  std::vector<std::string> ids(mWidgets.size());
  for(int i = 0; i < ids.size(); i++)
  {
    ids[i] = mWidgets[i]->uniqueID();
  }
  return ids;
}

CompositeWidget::~CompositeWidget()
{
}
