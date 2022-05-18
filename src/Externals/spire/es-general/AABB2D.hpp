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


#ifndef SPIRE_ES_GENERAL_COMP_AABB2D_HPP
#define SPIRE_ES_GENERAL_COMP_AABB2D_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <spire/scishare.h>

namespace gen {

/// An axis aligned bounding box.
struct AABB2D
{
  /// Verify that the bounds are sorted.
  bool isValid() const
  {
    glm::vec2 d = upperBound - lowerBound;
    bool valid = d.x >= 0.0f && d.y >= 0.0f;
    //valid = valid && lowerBound.IsValid() && upperBound.IsValid();
    return valid;
  }

  /// Get the center of the AABB.
  glm::vec2 getCenter() const
  {
    return 0.5f * (lowerBound + upperBound);
  }

  /// Get the extents of the AABB (half-widths).
  glm::vec2 getExtents() const
  {
    return 0.5f * (upperBound - lowerBound);
  }

  void setCenter(const glm::vec2& center, float width, float height)
  {
    glm::vec2 halfDims(width / 2.0f, height / 2.0f);
    lowerBound = center - halfDims;
    upperBound = center + halfDims;
  }

  /// Combine two AABBs into this one.
  // void Combine(const AABB2D& aabb1, const AABB2D& aabb2)
  // {
  //   lowerBound = ionMin(aabb1.lowerBound, aabb2.lowerBound);
  //   upperBound = ionMax(aabb1.upperBound, aabb2.upperBound);
  // }

  /// Does this aabb contain the provided AABB.
  bool contains(const AABB2D& aabb) const
  {
    bool result = true;
    result = result && lowerBound.x <= aabb.lowerBound.x;
    result = result && lowerBound.y <= aabb.lowerBound.y;
    result = result && aabb.upperBound.x <= upperBound.x;
    result = result && aabb.upperBound.y <= upperBound.y;
    return result;
  }

  bool contains(const glm::vec2& pos) const
  {
    return (    pos.x >= lowerBound.x
            &&  pos.y >= lowerBound.y
            &&  pos.x <= upperBound.x
            &&  pos.y <= upperBound.y);
  }

  //bool RayCast(b2RayCastOutput* output, const b2RayCastInput& input) const;

  glm::vec2 lowerBound;  ///< the lower vertex
  glm::vec2 upperBound;  ///< the upper vertex
};

inline bool aabb2DTestOverlap(const AABB2D& a, const AABB2D& b)
{
  glm::vec2 d1, d2;
  d1 = b.lowerBound - a.upperBound;
  d2 = a.lowerBound - b.upperBound;

  if (d1.x > 0.0f || d1.y > 0.0f)
    return false;

  if (d2.x > 0.0f || d2.y > 0.0f)
    return false;

  return true;
}

} // namespace gen

#endif
