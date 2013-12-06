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

#include "../namespaces.h"

#include "SciBall.h"

#include "spire/src/GLMathUtil.h"

namespace SCIRun {
namespace Gui {

//------------------------------------------------------------------------------
SciBall::SciBall(const spire::V3& center, float radius, const spire::M44& screenToTCS) :
    mCenter(center),
    mRadius(radius),
    mScreenToTCS(screenToTCS)
{
  // glm uses the following format for quaternions: w,x,y,z.
  //        w,    x,    y,    z
  spire::Quat qOne(1.0f, 0.0f, 0.0f, 0.0f);
  spire::V3   vZero(0.0f, 0.0f, 0.0f);

  mVDown    = vZero;
  mVNow     = vZero;
  mQDown    = qOne;
  mQNow     = qOne;
}

//------------------------------------------------------------------------------
SciBall::~SciBall()
{
}

//------------------------------------------------------------------------------
spire::V3 SciBall::mouseOnSphere(const spire::V3& tscMouse)
{
  spire::V3 ballMouse;

  // (m - C) / R
  ballMouse.x = (tscMouse.x - mCenter.x) / mRadius;
  ballMouse.y = (tscMouse.y - mCenter.y) / mRadius;

  float mag = glm::dot(ballMouse, ballMouse);
  if (mag > 1.0f)
  {
    // Since we are outside of the sphere, map to the visible boundary of
    // the sphere.
    ballMouse *= 1.0f / sqrtf(mag);
    ballMouse.z = 0.0f;
  }
  else
  {
    // We are not at the edge of the sphere, we are inside of it.
    // Essentially, we are normalizing the vector by adding the missing z
    // component.
    ballMouse.z = sqrtf(1.0f - mag);
  }

  return ballMouse;
}

//------------------------------------------------------------------------------
void SciBall::beginDrag(const spire::V2& msc)
{
  // The next two lines are usually a part of end drag. But end drag introduces
  // too much statefullness, so we are shortcircuiting it.
  mQDown      = mQNow;

  // Normal 'begin' code.
  mVDown      = (mScreenToTCS * spire::V4(msc.x, msc.y, 0.0f, 1.0f)).xyz();
}

//------------------------------------------------------------------------------
void SciBall::drag(const spire::V2& msc)
{
  // Regular drag code to follow...
  mVNow       = (mScreenToTCS * spire::V4(msc.x, msc.y, 0.0f, 1.0f)).xyz();
  mVSphereFrom= mouseOnSphere(mVDown);
  mVSphereTo  = mouseOnSphere(mVNow);

  /// \todo Perform constraints here.

  // Construct a quaternion from two points on the unit sphere.
  mQDrag = quatFromUnitSphere(mVSphereFrom, mVSphereTo); 
  mQNow = mQDrag * mQDown;

  // Perform complex conjugate
  spire::Quat q = mQNow;
  q.x = -q.x;
  q.y = -q.y;
  q.z = -q.z;
  q.w =  q.w;
  mMatNow = glm::mat4_cast(q);
}

//------------------------------------------------------------------------------
spire::Quat SciBall::quatFromUnitSphere(const spire::V3& from, const spire::V3& to)
{
  spire::Quat q;
  q.x = from.y*to.z - from.z*to.y;
  q.y = from.z*to.x - from.x*to.z;
  q.z = from.x*to.y - from.y*to.x;
  q.w = from.x*to.x + from.y*to.y + from.z*to.z;
  return q;
}

//------------------------------------------------------------------------------
spire::M44 SciBall::getTransformation() const
{
  return mMatNow;
}


} // namespace Gui
} // namespace SCIRun 

