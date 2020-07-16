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


/// @todo Documentation Core/GeometryPrimitives/BBox.h

#ifndef CORE_GEOMETRY_BBOX_H
#define CORE_GEOMETRY_BBOX_H 1

#define NOMINMAX
#include <Core/Utils/Exception.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/BBoxBase.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/share.h>
#define NOMINMAX

/// @todo replace asserts in this code with warnings or other reporting
/// mechanism that doesn't abort the program

namespace SCIRun {
namespace Core {
namespace Geometry {
  class SCISHARE BBox : public BBoxBase
  {
  public:
    enum { INSIDE, INTERSECT, OUTSIDE };

    BBox() : BBoxBase(false)
    { }

    BBox(const BBox& copy) : BBoxBase(copy.is_valid_, copy.cmin_, copy.cmax_)
    { }

    BBox& operator=(const BBox& copy)
    {
      is_valid_ = copy.is_valid_;
      cmin_ = copy.cmin_;
      cmax_ = copy.cmax_;
      return *this;
    }

    BBox(const BBox& b1, const BBox& b2) : BBoxBase(true, b1.cmin_, b1.cmax_)
    {
      extend(b2.cmin_);
      extend(b2.cmax_);
    }


    BBox(const Point& p1, const Point& p2) : BBoxBase(true, p1, p2)
    {
      extend(p2);
    }

    BBox(const Point& p1, const Point& p2, const Point& p3) : BBoxBase(true, p1, p2)
    {
      extend(p2);
      extend(p3);
    }

    explicit BBox(const std::vector<Point>& points) : BBoxBase(false)
    {
      is_valid_ = false;
      for (auto& p : points)
        extend(p);
    }

    /// Expand the bounding box to include point p
    inline void extend(const Point& p) override
    {
      if(is_valid_)
      {
        cmin_=Min(p, cmin_);
        cmax_=Max(p, cmax_);
      }
      else
      {
        cmin_=p;
        cmax_=p;
        is_valid_ = true;
      }
    }

    /// Extend the bounding box on all sides by a margin
    /// For example to expand it by a certain epsilon to make
    /// sure that a lookup will be inside the bounding box
    inline void extend(double val) override
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

    /// Expand the bounding box to include a sphere of radius radius
    /// and centered at point p
    inline void extend(const Point& p, double radius)
    {
      Vector r(radius,radius,radius);
      if(is_valid_)
      {
        cmin_=Min(p-r, cmin_);
        cmax_=Max(p+r, cmax_);
      }
      else
      {
        cmin_=p-r;
        cmax_=p+r;
        is_valid_ = true;
      }
    }

    /// Expand the bounding box to include bounding box b
    inline void extend(const BBox& b)
    {
      if(b.valid())
      {
        extend(b.get_min());
        extend(b.get_max());
      }
    }

    /// Expand the bounding box to include a disk centered at cen,
    /// with normal normal, and radius r.
    void extend_disk(const Point& cen, const Vector& normal, double r);

    inline Point center() const override
    {
      /// @todo: C assert: assert(is_valid_);
      Vector d = diagonal();
      return cmin_ + (d * 0.5);
    }

    inline double longest_edge() const
    {
      /// @todo: C assert: assert(is_valid_);
      Vector diagonal(cmax_-cmin_);
      return Max(diagonal.x(), diagonal.y(), diagonal.z());
    }

    inline double shortest_edge() const
    {
      /// @todo: C assert: assert(is_valid_);
      Vector diagonal(cmax_-cmin_);
      return Min(diagonal.x(), diagonal.y(), diagonal.z());
    }

    /// Check whether two BBoxes are similar
    bool is_similar_to(const BBox &b, double diff=0.5) const;

    /// Move the bounding box
    void translate(const Vector &v);

    /// Scale the bounding box by s, centered around o
    void scale(double s, const Vector &o);

    inline Point get_min() const override
      { return cmin_; }

    inline Point get_max() const override
      { return cmax_; }

    inline Vector diagonal() const override
    {
      //TODO: needs invariant check, or refactoring.
      if (!is_valid_)
        THROW_INVALID_STATE("BBox is not valid.");
      return cmax_-cmin_;
    }

    inline bool inside(const Point &p) const
    {
      return (is_valid_ && p.x() >= cmin_.x() &&
        p.y() >= cmin_.y() && p.z() >= cmin_.z() &&
        p.x() <= cmax_.x() && p.y() <= cmax_.y() &&
        p.z() <= cmax_.z());
    }

    inline int intersect(const BBox& b) const
    {
      if ((cmax_.x() < b.cmin_.x()) || (cmin_.x() > b.cmax_.x()) ||
          (cmax_.y() < b.cmin_.y()) || (cmin_.y() > b.cmax_.y()) ||
          (cmax_.z() < b.cmin_.z()) || (cmin_.z() > b.cmax_.z()))
      {
        return OUTSIDE;
      }

      if ((cmin_.x() <= b.cmin_.x()) && (cmax_.x() >= b.cmax_.x()) &&
          (cmin_.y() <= b.cmin_.y()) && (cmax_.y() >= b.cmax_.y()) &&
          (cmin_.z() <= b.cmin_.z()) && (cmax_.z() >= b.cmax_.z()))
      {
        return INSIDE;
      }

      return INTERSECT;
    }

    inline double x_length() { return (cmax_.x() - cmin_.x()); }
    inline double y_length() { return (cmax_.y() - cmin_.y()); }
    inline double z_length() { return (cmax_.z() - cmin_.z()); }

    /// bbox's that share a face overlap
    bool overlaps(const BBox& bb) const;
    /// bbox's that share a face do not overlap_inside
    bool overlaps_inside(const BBox& bb) const;

    /// returns true if the ray hit the bbox and returns the hit point
    /// in hitNear
    bool intersect(const Point& e, const Vector& v, Point& hitNear) const;
};

SCISHARE std::ostream& operator<<(std::ostream& out, const BBox& b);
SCISHARE void Pio( Piostream &, BBox& );

}}}

#endif
