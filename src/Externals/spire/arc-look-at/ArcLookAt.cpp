/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
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
/// \date   April 2013

#include "ArcLookAt.hpp"
#include <arc-ball/ArcBall.hpp>

namespace spire {

//------------------------------------------------------------------------------
ArcLookAt::ArcLookAt() :
    mArcBall(new ArcBall(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f)),
    mCamLookAt(0.0f),
    mCamDistance(3.0f),
    mReferenceCamDistance(0.0f)
{
}

//------------------------------------------------------------------------------
ArcLookAt::~ArcLookAt()
{
}

//------------------------------------------------------------------------------
void ArcLookAt::doReferenceDown(const glm::vec2& ssPos)
{
  mReferenceLookAt      = mCamLookAt;
  mReferenceScreenPos   = ssPos;
  mReferenceTransform   = getWorldViewTransform();
  mReferenceCamDistance = mCamDistance;

  mArcBall->beginDrag(ssPos);
}

//------------------------------------------------------------------------------
void ArcLookAt::doPan(const glm::vec2& ssPos)
{
  glm::vec2 delta = mReferenceScreenPos - ssPos;
  glm::vec2 trans = delta * mCamDistance / 2.0f;

  glm::mat4 camRot = mArcBall->getTransformation();
  glm::vec3 translation =
        static_cast<glm::vec3>(camRot[0]) * trans.x
      + static_cast<glm::vec3>(camRot[1]) * trans.y;
  mCamLookAt = mReferenceLookAt + translation;
}

//------------------------------------------------------------------------------
void ArcLookAt::doRotation(const glm::vec2& ssPos)
{
  mArcBall->drag(ssPos);
}

//------------------------------------------------------------------------------
void ArcLookAt::doZoom(glm::float_t camZoom)
{
  //mCamDistance += camZoom;

  glm::float_t prevDistance = mCamDistance;
  camZoom /= 65;
  camZoom *= prevDistance;
  mCamDistance += camZoom;
  if (mCamDistance <= 0)
  {
    mCamDistance = prevDistance;
  }
}

//------------------------------------------------------------------------------
void ArcLookAt::doZoom(glm::float_t camZoom, int zoomSpeed)
{
  glm::float_t prevDistance = mCamDistance;
  zoomSpeed = zoomSpeed > 1 ? zoomSpeed : 2;
  camZoom /= zoomSpeed;
  camZoom *= prevDistance;
  mCamDistance += camZoom;
  if (mCamDistance <= 0)
  {
    mCamDistance = prevDistance;
  }
}

//------------------------------------------------------------------------------
void ArcLookAt::doZoom(const glm::vec2& ssPos)
{
  // Use distance delta from center of screen to control zoom.
  // Will need a new variable to control this.
  glm::vec2 delta = ssPos - mReferenceScreenPos;
  glm::float_t xScale = 4.0f;
  glm::float_t yScale = 4.0f;
  //mCamDistance = mReferenceCamDistance + (delta.x) * xScale + (-delta.y) * yScale;

	glm::float_t prevDistance = mCamDistance;
	glm::float_t camZoom = mCamDistance + (delta.x) * xScale + (-delta.y) * yScale;
	mCamDistance = camZoom;
	if (mCamDistance <= 0)
	{
		mCamDistance = prevDistance;
	}
}

//------------------------------------------------------------------------------
glm::mat4 ArcLookAt::getWorldViewTransform() const
{
  glm::mat4 camRot      = mArcBall->getTransformation();
  glm::mat4 finalTrafo  = camRot;

  // Translation is a post rotation operation where as zoom is a pre transform
  // operation. We should probably ensure the user doesn't scroll passed zero.
  // Remember, we are looking down NEGATIVE z.
  // NOTE: We are translating both the lookat and the eye point.
  // Eyepoint is a function of the lookat, the camera transform, and the
  // camera distance.
  finalTrafo[3].xyz() = mCamLookAt + static_cast<glm::vec3>(camRot[2]) * mCamDistance;
  return finalTrafo;
}

//------------------------------------------------------------------------------
void ArcLookAt::autoview(const spire::AABB& bbox, float fov)
{
  if (bbox.isNull()) return;

  glm::vec3 diag(bbox.getDiagonal());
  double w = glm::length(diag);

  if (w < 0.000001)
  {
    spire::AABB bb;
    bb.setNull();
    glm::vec3 epsilon(0.001, 0.001, 0.001);
    bb.extend( bbox.getMin() - epsilon );
    bb.extend( bbox.getMax() + epsilon );
    w = glm::length(bb.getDiagonal());
  }

  mCamLookAt = bbox.getCenter();

  // We are calculating the distance the camera would need to be away from
  // the length of the diagonal of the bbox. See Van Dam, Foley, third edition
  // page 304:

  //    AC = f*tan(O(v)/2)
  // => f = AC / tan(O(v)/2)
  // Where AC is half the size of the diagonal.
  // So, takning into account half the size:
  // => f = AC / (2 * tan(O(v) / 2)

  mCamDistance = w / (2 * tan(fov / 2.0));
}

//------------------------------------------------------------------------------
void ArcLookAt::setView(const glm::vec3& view, const glm::vec3& up)
{
  mReferenceLookAt = mCamLookAt;
  glm::vec3 location = mCamDistance * view;
  mArcBall->setLocationOnSphere(location, up);
  mCamLookAt = mReferenceLookAt;
}

} // namespace spire
