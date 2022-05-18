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

#include <Core/Utils/Exception.h>
#include <Core/GeometryPrimitives/OrientedBBox.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;

OrientedBBox::OrientedBBox(const Vector &e1, const Vector &e2, const Vector &e3)
  : BBoxBase(false)
{
  eigvecs_ = {e1, e2, e3};
}

void OrientedBBox::extend(const Point &p)
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
void OrientedBBox::extend(double val)
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

Vector OrientedBBox::diagonal() const
{
  //TODO: needs invariant check, or refactoring.
  if (!is_valid_)
    THROW_INVALID_STATE("OrientedBBox is not valid.");
  return cmax_-cmin_;
}

Point OrientedBBox::get_min() const
{ return cmin_; }

Point OrientedBBox::get_max() const
{ return cmax_; }

Point OrientedBBox::center() const
{
  Vector d = diagonal();
  return cmin_ + (d * 0.5);
}

std::ostream& SCIRun::Core::Geometry::operator<<(std::ostream& out, const OrientedBBox& b)
{
  return out << b.get_min() << " : " << b.get_max();
}
