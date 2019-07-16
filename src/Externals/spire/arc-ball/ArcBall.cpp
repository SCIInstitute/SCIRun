#include "ArcBall.hpp"

namespace spire {

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

  mQDown       = qOne;
  mQNow        = qOne;
  mVSphereDown = mouseOnSphere(vZero);
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
    // Since we are outside of the sphere, map to the visible boundary of the sphere.
    ballMouse *= 1.0 / sqrtf(mag);
  }
  else
  {
    // Essentially, we are normalizing the vector by adding the missing z component.
    ballMouse.z = sqrtf(1.0 - mag);
  }

  return ballMouse;
}

//------------------------------------------------------------------------------
void ArcBall::beginDrag(const glm::vec2& msc)
{
  mQDown       = mQNow;
  mVSphereDown = mouseOnSphere((mScreenToTCS * glm::vec4(msc, 0.0f, 1.0)).xyz());
}

//------------------------------------------------------------------------------
void ArcBall::drag(const glm::vec2& msc)
{
  glm::vec3 mVSphereNow = mouseOnSphere((mScreenToTCS * glm::vec4(msc, 0.0, 1.0)).xyz());

  // Construct a quaternion from two points on the unit sphere.
  glm:: quat mQDrag = quatFromUnitSphere(mVSphereDown, mVSphereNow);
  mQNow = mQDrag * mQDown;
}

//------------------------------------------------------------------------------
void ArcBall::setLocationOnSphere(glm::vec3 location, glm::vec3 up)
{
  glm::mat4 mMatNow = glm::lookAt(location, glm::vec3(0.0f), up);
  mQNow   = glm::quat_cast(mMatNow);
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
  return glm::mat4_cast(mQNow);
}

} // namespace spire
