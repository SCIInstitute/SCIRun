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

/// \author James Hughes
/// \date   March 2013

#include <gl-platform/GLPlatform.hpp>
#include <Interface/Modules/Render/namespaces.h>

#include <Interface/Modules/Render/ES/SRCamera.h>

namespace SCIRun {
namespace Render {

//------------------------------------------------------------------------------
SRCamera::SRCamera(SRInterface& iface) :
    mTrafoSeq(0),
    mInvertVal(-1),
    mPerspective(true),
    mFOV(getDefaultFOVY()),
    mZNear(getDefaultZNear()),
    mZFar(getDefaultZFar()),
    mInterface(iface),
    mArcLookAt(new CPM_LOOK_AT_NS::ArcLookAt())
{
  setAsPerspective();

  glm::mat4 cam;
  cam[3] = (glm::vec4(0.0f, 0.0f, 7.0f, 1.0f));
}

//------------------------------------------------------------------------------
SRCamera::~SRCamera()
{
}

//------------------------------------------------------------------------------
void SRCamera::setAsPerspective()
{
  mPerspective = true;

  float aspect = static_cast<float>(mInterface.getScreenWidthPixels()) / 
                 static_cast<float>(mInterface.getScreenHeightPixels());
  mP = glm::perspective(mFOV, aspect, mZNear, mZFar);
}

//------------------------------------------------------------------------------
void SRCamera::setAsOrthographic(float halfWidth, float halfHeight)
{
  mPerspective = false;

	mP = glm::ortho(-halfWidth, halfWidth, 
                  -halfHeight, halfHeight, 
                  mZNear, mZFar);
}

//------------------------------------------------------------------------------
void SRCamera::buildTransform()
{
  ++mTrafoSeq;

  mV    = mArcLookAt->getWorldViewTransform();
  mIV   = glm::affineInverse(mV);
  mPIV  = mP * mIV;
}

//------------------------------------------------------------------------------
void SRCamera::applyTransform()
{
  buildTransform();
}

//------------------------------------------------------------------------------
void SRCamera::mouseDownEvent(const glm::ivec2& pos, SRInterface::MouseButton btn)
{
  glm::vec2 screenSpace = calculateScreenSpaceCoords(pos);
  mArcLookAt->doReferenceDown(screenSpace);
}

//------------------------------------------------------------------------------
void SRCamera::mouseMoveEvent(const glm::ivec2& pos, SRInterface::MouseButton btn)
{
  glm::vec2 screenSpace = calculateScreenSpaceCoords(pos);
  switch (mInterface.getMouseMode())
  {
    case SRInterface::MOUSE_OLDSCIRUN:
      if (btn == SRInterface::MOUSE_LEFT)    mArcLookAt->doPan(screenSpace);
      if (btn == SRInterface::MOUSE_RIGHT)   mArcLookAt->doZoom(screenSpace);
      if (btn == SRInterface::MOUSE_MIDDLE)  mArcLookAt->doRotation(screenSpace);
      break;

    case SRInterface::MOUSE_NEWSCIRUN:
      if (btn == SRInterface::MOUSE_LEFT)    mArcLookAt->doRotation(screenSpace);
      if (btn == SRInterface::MOUSE_RIGHT)   mArcLookAt->doPan(screenSpace);
      break;
  }
}

//------------------------------------------------------------------------------
void SRCamera::mouseWheelEvent(int32_t delta, int zoomSpeed)
{
  if (mInterface.getMouseMode() != SRInterface::MOUSE_OLDSCIRUN)
  {
    //mArcLookAt->doZoom(-static_cast<float>(delta) / 100.0f, zoomSpeed);
    mArcLookAt->doZoom(mInvertVal*static_cast<float>(delta) / 100.0f, zoomSpeed);
  }
}

//------------------------------------------------------------------------------
void SRCamera::doAutoView(const Core::Geometry::BBox& bbox)
{
  // Convert core geom bbox to AABB.
  Core::Geometry::Point bboxMin = bbox.get_min();
  Core::Geometry::Point bboxMax = bbox.get_max();
  glm::vec3 min(bboxMin.x(), bboxMin.y(), bboxMin.z());
  glm::vec3 max(bboxMax.x(), bboxMax.y(), bboxMax.z());

  CPM_GLM_AABB_NS::AABB aabb(min, max);

  /// \todo Use real FOV-Y when we allow the user to change the FOV.
  mArcLookAt->autoview(aabb, getDefaultFOVY());
}

//------------------------------------------------------------------------------
void SRCamera::setView(const glm::vec3& view, const glm::vec3& up)
{
    mArcLookAt->setView(view, up);
}

//------------------------------------------------------------------------------
void SRCamera::setZoomInverted(bool value)
{
  if (value)
    mInvertVal = 1;
  else
    mInvertVal = -1;
}

//------------------------------------------------------------------------------
glm::vec2 SRCamera::calculateScreenSpaceCoords(const glm::ivec2& mousePos)
{
  float windowOriginX = 0.0f;
  float windowOriginY = 0.0f;

  // Transform incoming mouse coordinates into screen space.
  glm::vec2 mouseScreenSpace;
  mouseScreenSpace.x = 2.0f * (static_cast<float>(mousePos.x) - windowOriginX) 
      / static_cast<float>(mInterface.getScreenWidthPixels()) - 1.0f;
  mouseScreenSpace.y = 2.0f * (static_cast<float>(mousePos.y) - windowOriginY)
      / static_cast<float>(mInterface.getScreenHeightPixels()) - 1.0f;

  // Rotation with flipped axes feels much more natural. It places it inside
  // the correct OpenGL coordinate system (with origin in the center of the
  // screen).
  mouseScreenSpace.y = -mouseScreenSpace.y;

  return mouseScreenSpace;
}

} // namespace Render
} // namespace SCIRun 

