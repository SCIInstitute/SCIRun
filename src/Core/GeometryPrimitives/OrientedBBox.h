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

#ifndef CORE_GEOMETRY_ORIENTEDBBOX_H
#define CORE_GEOMETRY_ORIENTEDBBOX_H

#include <Core/Utils/Legacy/Assert.h>
#include <Core/GeometryPrimitives/BBoxBase.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/share.h>

namespace SCIRun {
namespace Core {
namespace Geometry {
class OrientedBBox : public Core::Geometry::BBoxBase
{
public:
  OrientedBBox(const Core::Geometry::Vector &e1, const Core::Geometry::Vector &e2, const Core::Geometry::Vector &e3);
  /// Expand the bounding box to include point p
  inline void extend(const Point &p)
  {
    auto originToPos = Vector(p);
    if (is_valid_)
      for (int iDim = 0; iDim < 3; ++iDim)
      {
        auto projectedPos = Dot(originToPos, eigvecs_[iDim]);
        cmin_[iDim] = std::min(projectedPos, cmin_[iDim]);
        cmax_[iDim] = std::max(projectedPos, cmax_[iDim]);
      }
    else
    {
      is_valid_ = true;
      for (int iDim = 0; iDim < 3; ++iDim)
      {
        auto projectedPos = Dot(originToPos, eigvecs_[iDim]);
        cmin_[iDim] = projectedPos;
        cmax_[iDim] = projectedPos;
      }
    }
  }

  /// Extend the bounding box on all sides by a margin
  /// For example to expand it by a certain epsilon to make
  /// sure that a lookup will be inside the bounding box
  inline void extend(double val)
  {
    if (is_valid_)
    {
      cmin_.x(cmin_.x()-val);
      cmin_.y(cmin_.y()-val);
      cmin_.z(cmin_.z()-val);
      cmax_.x(cmax_.x()+val);
      cmax_.y(cmax_.y()+val);
      cmax_.z(cmax_.z()+val);
    }
  }

  inline bool valid() const
  { return is_valid_; }

  inline Vector diagonal() const
  {
    //TODO: needs invariant check, or refactoring.
    ASSERT(is_valid_);
    return cmax_-cmin_;
  }

  inline Point get_min() const
  { return cmin_; }

  inline Point get_max() const
  { return cmax_; }

  inline Point center() const
  {
    /// @todo: C assert: assert(is_valid_);
    Vector d = diagonal();
    return cmin_ + (d * 0.5);
  }

private:
  std::vector<Vector> eigvecs_;
};

SCISHARE std::ostream &operator<<(std::ostream &out, const OrientedBBox &b);
SCISHARE void Pio(Piostream &, OrientedBBox &);

}}}

#endif
