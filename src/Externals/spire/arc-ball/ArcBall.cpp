/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


#include "ArcBall.hpp"
#include <iostream>

namespace spire {

//------------------------------------------------------------------------------
  ArcBall::ArcBall(const glm::vec3& center, glm::float_t radius, bool inverted, const glm::mat4& screenToTCS) :
    mScreenToTCS(screenToTCS),
    mCenter(center),
    mRadius(radius),
    invertHemisphere(inverted)
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

  glm::float_t mag_sq = glm::dot(ballMouse, ballMouse);
  if (mag_sq > 1.0)
  {
    // Since we are outside of the sphere, map to the visible boundary of the sphere.
    ballMouse *= 1.0 / sqrtf(mag_sq);
  }
  else
  {
    // Essentially, we are normalizing the vector by adding the missing z component.
    ballMouse.z = sqrtf(1.0 - mag_sq);
  }

  if(invertHemisphere)
    ballMouse.z = -ballMouse.z;

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
  glm::quat mQDrag = quatFromUnitSphere(mVSphereDown, mVSphereNow);
  mQNow = mQDrag * mQDown;
  if(glm::dot(mVSphereDown, mVSphereNow) < 0.0)
    beginDrag(msc);
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
  //TODO: check if cross is 0 before normalize. Crashes on Windows
  auto c = glm::cross(from, to);
  if (c == glm::vec3(0, 0, 0))
    return {};

  glm::vec3 axis = glm::normalize(c);

  // Give arbitrary non-zero vector because no rotation
  if (std::isnan(axis[0]))
    axis = from;

  float angle = std::acos(glm::dot(from, to));

  if(angle <= 0.00001 || std::isnan(angle))
    return glm::quat(1.0, 0.0, 0.0, 0.0);

  return glm::angleAxis(angle, axis);
}

//------------------------------------------------------------------------------
glm::mat4 ArcBall::getTransformation() const
{
  return glm::mat4_cast(mQNow);
}

} // namespace spire
