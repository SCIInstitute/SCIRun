#include "ArcBall.hpp"

namespace CPM_ARC_BALL_NS {

//------------------------------------------------------------------------------
ArcBall::ArcBall(const glm::vec3& center, glm::float_t radius, const glm::mat4& screenToTCS) :
    mCenter(center),
    mRadius(radius),
    mScreenToTCS(screenToTCS)
{
  // glm uses the following format for quaternions: w,x,y,z.
  //        w,    x,    y,    z
  glm::quat qOne(1.0, 0.0, 0.0, 0.0);
  glm::vec3 vZero(0.0, 0.0, 0.0);

  mVDown    = vZero;
  mVNow     = vZero;
  mQDown    = qOne;
  mQNow     = qOne;
}

//------------------------------------------------------------------------------
ArcBall::~ArcBall()
{
}

//------------------------------------------------------------------------------
glm::vec3 ArcBall::mouseOnSphere(const glm::vec3& tscMouse)
{
  glm::vec3 ballMouse;

  // (m - C) / R
  ballMouse.x = (tscMouse.x - mCenter.x) / mRadius;
  ballMouse.y = (tscMouse.y - mCenter.y) / mRadius;

  glm::float_t mag = glm::dot(ballMouse, ballMouse);
  if (mag > 1.0)
  {
    // Since we are outside of the sphere, map to the visible boundary of
    // the sphere.
    ballMouse *= 1.0 / sqrtf(mag);
    ballMouse.z = 0.0;
  }
  else
  {
    // We are not at the edge of the sphere, we are inside of it.
    // Essentially, we are normalizing the vector by adding the missing z
    // component.
    ballMouse.z = sqrtf(1.0 - mag);
  }

  return ballMouse;
}

//------------------------------------------------------------------------------
void ArcBall::beginDrag(const glm::vec2& msc)
{
  // The next two lines are usually a part of end drag. But end drag introduces
  // too much statefullness, so we are shortcircuiting it.
  mQDown      = mQNow;

  // Normal 'begin' code.
  mVDown      = (mScreenToTCS * glm::vec4(msc.x, msc.y, 0.0f, 1.0)).xyz();
}

//------------------------------------------------------------------------------
void ArcBall::drag(const glm::vec2& msc)
{
  // Regular drag code to follow...
  mVNow       = (mScreenToTCS * glm::vec4(msc.x, msc.y, 0.0, 1.0)).xyz();
  mVSphereFrom= mouseOnSphere(mVDown);
  mVSphereTo  = mouseOnSphere(mVNow);

  // Construct a quaternion from two points on the unit sphere.
  mQDrag = quatFromUnitSphere(mVSphereFrom, mVSphereTo);
  mQNow = mQDrag * mQDown;

  // Perform complex conjugate
  glm::quat q = mQNow;
  q.x = -q.x;
  q.y = -q.y;
  q.z = -q.z;
  q.w =  q.w;
  mMatNow = glm::mat4_cast(q);
}

//------------------------------------------------------------------------------
void ArcBall::setLocationOnSphere(glm::vec3 location, glm::vec3 up)
{
  mMatNow = glm::lookAt(location, glm::vec3(0.0f), up);
  glm::quat q = glm::quat_cast(mMatNow);
  q.x = -q.x;
  q.y = -q.y;
  q.z = -q.z;
  q.w = q.w;
  mQNow = q;

}

//------------------------------------------------------------------------------
glm::quat ArcBall::quatFromUnitSphere(const glm::vec3& from, const glm::vec3& to)
{
  glm::quat q;
  q.x = from.y*to.z - from.z*to.y;
  q.y = from.z*to.x - from.x*to.z;
  q.z = from.x*to.y - from.y*to.x;
  q.w = from.x*to.x + from.y*to.y + from.z*to.z;
  return q;
}

//------------------------------------------------------------------------------
glm::mat4 ArcBall::getTransformation() const
{
  return mMatNow;
}


} // namespace CPM_ARC_BALL_NS
