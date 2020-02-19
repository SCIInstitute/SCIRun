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


#include "AABB.hpp"
#include <glm/gtx/component_wise.hpp>

namespace spire {

AABB::AABB()
{
  setNull();
}

AABB::AABB(const glm::vec3& center, glm::float_t radius)
{
  setNull();
  extend(center, radius);
}

AABB::AABB(const glm::vec3& p1, const glm::vec3& p2)
{
  setNull();
  extend(p1);
  extend(p2);
}

AABB::AABB(const AABB& aabb)
{
  setNull();
  extend(aabb);
}

AABB::~AABB()
{
}

void AABB::extend(glm::float_t val)
{
  if (!isNull())
  {
    mMin -= glm::vec3(val);
    mMax += glm::vec3(val);
  }
}

void AABB::extend(const glm::vec3& p)
{
  if (!isNull())
  {
    mMin = glm::min(p, mMin);
    mMax = glm::max(p, mMax);
  }
  else
  {
    mMin = p;
    mMax = p;
  }
}

void AABB::extend(const glm::vec3& p, glm::float_t radius)
{
  glm::vec3 r(radius);
  if (!isNull())
  {
    mMin = glm::min(p - r, mMin);
    mMax = glm::max(p + r, mMax);
  }
  else
  {
    mMin = p - r;
    mMax = p + r;
  }
}

void AABB::extend(const AABB& aabb)
{
  if (!aabb.isNull())
  {
    extend(aabb.mMin);
    extend(aabb.mMax);
  }
}

void AABB::extendDisk(const glm::vec3& c, const glm::vec3& n, glm::float_t r)
{
  if (glm::length(n) < 1.e-12) { extend(c); return; }
  glm::vec3 norm = glm::normalize(n);
  glm::float_t x = sqrt(1 - norm.x) * r;
  glm::float_t y = sqrt(1 - norm.y) * r;
  glm::float_t z = sqrt(1 - norm.z) * r;
  extend(c + glm::vec3(x,y,z));
  extend(c - glm::vec3(x,y,z));
}

glm::vec3 AABB::getDiagonal() const
{
  if (!isNull())
    return mMax - mMin;
  else
    return glm::vec3(0);
}

glm::float_t AABB::getLongestEdge() const
{
  return glm::compMax(getDiagonal());
}

glm::float_t AABB::getShortestEdge() const
{
  return glm::compMin(getDiagonal());
}

glm::vec3 AABB::getCenter() const
{
  if (!isNull())
  {
    glm::vec3 d = getDiagonal();
    return mMin + (d * glm::float_t(0.5));
  }
  else
  {
    return glm::vec3(0.0);
  }
}

void AABB::translate(const glm::vec3& v)
{
  if (!isNull())
  {
    mMin += v;
    mMax += v;
  }
}

void AABB::scale(const glm::vec3& s, const glm::vec3& o)
{
  if (!isNull())
  {
    mMin -= o;
    mMax -= o;

    mMin *= s;
    mMax *= s;

    mMin += o;
    mMax += o;
  }
}

bool AABB::overlaps(const AABB& bb) const
{
  if (isNull() || bb.isNull())
    return false;

  if( bb.mMin.x > mMax.x || bb.mMax.x < mMin.x)
    return false;
  else if( bb.mMin.y > mMax.y || bb.mMax.y < mMin.y)
    return false;
  else if( bb.mMin.z > mMax.z || bb.mMax.z < mMin.z)
    return false;

  return true;
}

AABB::INTERSECTION_TYPE AABB::intersect(const AABB& b) const
{
  if (isNull() || b.isNull())
    return OUTSIDE;

  if ((mMax.x < b.mMin.x) || (mMin.x > b.mMax.x) ||
      (mMax.y < b.mMin.y) || (mMin.y > b.mMax.y) ||
      (mMax.z < b.mMin.z) || (mMin.z > b.mMax.z))
  {
    return OUTSIDE;
  }

  if ((mMin.x <= b.mMin.x) && (mMax.x >= b.mMax.x) &&
      (mMin.y <= b.mMin.y) && (mMax.y >= b.mMax.y) &&
      (mMin.z <= b.mMin.z) && (mMax.z >= b.mMax.z))
  {
    return INSIDE;
  }

  return INTERSECT;
}


bool AABB::isSimilarTo(const AABB& b, glm::float_t diff) const
{
  if (isNull() || b.isNull()) return false;

  glm::vec3 acceptable_diff=( (getDiagonal()+b.getDiagonal()) / glm::float_t(2.0))*diff;
  glm::vec3 min_diff(mMin-b.mMin);
  min_diff = glm::vec3(fabs(min_diff.x),fabs(min_diff.y),fabs(min_diff.z));
  if (min_diff.x > acceptable_diff.x) return false;
  if (min_diff.y > acceptable_diff.y) return false;
  if (min_diff.z > acceptable_diff.z) return false;
  glm::vec3 max_diff(mMax-b.mMax);
  max_diff = glm::vec3(fabs(max_diff.x),fabs(max_diff.y),fabs(max_diff.z));
  if (max_diff.x > acceptable_diff.x) return false;
  if (max_diff.y > acceptable_diff.y) return false;
  if (max_diff.z > acceptable_diff.z) return false;
  return true;
}

} // namespace spire
