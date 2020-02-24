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


/// @todo Documentation Core/GeometryPrimitives/SearchGridT.h

#ifndef CORE_DATATYPES_SEARCHGRIDT_H
#define CORE_DATATYPES_SEARCHGRIDT_H 1

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/Datatypes/Legacy/Base/Types.h>

#include <algorithm>
#include <vector>

#include <Core/GeometryPrimitives/share.h>

namespace SCIRun {

template<class INDEX>
class SearchGridT
{
  public:
    /// Include the types defined in Types into this class
    typedef SCIRun::index_type                    index_type;
    typedef SCIRun::size_type                     size_type;
    typedef typename std::vector<INDEX>::iterator iterator;

    SearchGridT(size_type x, size_type y, size_type z,
               const Core::Geometry::Point &min, const Core::Geometry::Point &max) :
        ni_(x), nj_(y), nk_(z)
      {
        transform_.pre_scale(Core::Geometry::Vector(1.0 / x, 1.0 / y, 1.0 / z));
        transform_.pre_scale(max - min);

        transform_.pre_translate(Core::Geometry::Vector(min));
        transform_.compute_imat();
        bin_.resize(x*y*z);
      }

    inline void transform(const Core::Geometry::Transform &t)
      { transform_.pre_trans(t);}

    inline const Core::Geometry::Transform &get_transform() const
      { return transform_; }

    inline Core::Geometry::Transform &set_transform(const Core::Geometry::Transform &trans)
      { transform_ = trans; return transform_; }


    /// Get the size of the search grid
    inline size_type get_ni() const { return ni_; }
    inline size_type get_nj() const { return nj_; }
    inline size_type get_nk() const { return nk_; }

    inline bool locate(index_type &i, index_type &j,
                       index_type &k, const Core::Geometry::Point &p) const
    {
      const Core::Geometry::Point r = transform_.unproject(p);

      const double rx = floor(r.x());
      const double ry = floor(r.y());
      const double rz = floor(r.z());

      // Clamp in double space to avoid overflow errors.
      if (rx < 0.0      || ry < 0.0      || rz < 0.0    ||
          rx >= ni_     || ry >= nj_     || rz >= nk_   )
      {
        return (false);
      }

      i = static_cast<index_type>(rx);
      j = static_cast<index_type>(ry);
      k = static_cast<index_type>(rz);
      return (true);
    }


    inline bool locate_clamp(index_type &i, index_type &j,
                       index_type &k, const Core::Geometry::Point &p) const
    {
      const Core::Geometry::Point r = transform_.unproject(p);

      double rx = floor(r.x());
      double ry = floor(r.y());
      double rz = floor(r.z());

      // Clamp in double space to avoid overflow errors.
      if (rx < 0.0) rx = 0.0;
      if (ry < 0.0) ry = 0.0;
      if (rz < 0.0) rz = 0.0;

      if (rx >= ni_) rx = ni_;
      if (ry >= nj_) ry = nj_;
      if (rz >= nk_) rz = nk_;

      i = static_cast<index_type>(rx);
      j = static_cast<index_type>(ry);
      k = static_cast<index_type>(rz);
      return (true);
    }

    inline void unsafe_locate(index_type &i, index_type &j,
                              index_type &k, const Core::Geometry::Point &p) const
    {
      Core::Geometry::Point r;
      transform_.unproject(p, r);

      i = static_cast<index_type>(r.x());
      j = static_cast<index_type>(r.y());
      k = static_cast<index_type>(r.z());
    }

    void insert(INDEX val, const Core::Geometry::BBox &bbox)
    {
      index_type mini=0, minj=0, mink=0, maxi=0, maxj=0, maxk=0;

      locate(mini, minj, mink, bbox.get_min());
      locate(maxi, maxj, maxk, bbox.get_max());

      for (index_type i = mini; i <= maxi; i++)
      {
        for (index_type j = minj; j <= maxj; j++)
        {
          for (index_type k = mink; k <= maxk; k++)
          {
            bin_[linearize(i, j, k)].push_back(val);
          }
        }
      }
    }

    void remove(INDEX val, const Core::Geometry::BBox &bbox)
    {
      index_type mini, minj, mink, maxi, maxj, maxk;

      unsafe_locate(mini, minj, mink, bbox.get_min());
      unsafe_locate(maxi, maxj, maxk, bbox.get_max());

      for (index_type i = mini; i <= maxi; i++)
      {
        for (index_type j = minj; j <= maxj; j++)
        {
          for (index_type k = mink; k <= maxk; k++)
          {
            index_type q = linearize(i, j, k);
            std::remove(bin_[q].begin(),bin_[q].end(),val);
          }
        }
      }
    }

    void insert(INDEX val, const Core::Geometry::Point &point)
    {
      index_type i, j, k;
      unsafe_locate(i, j, k, point);
      bin_[linearize(i, j, k)].push_back(val);
    }

    void remove(INDEX val, const Core::Geometry::Point &point)
    {
      index_type i, j, k;
      unsafe_locate(i, j, k, point);
      index_type q = linearize(i, j, k);
      std::remove(bin_[q].begin(),bin_[q].end(),val);
    }

    inline bool lookup(iterator &begin, iterator &end, const Core::Geometry::Point &p)
    {
      index_type i, j, k;
      if (locate(i, j, k, p))
      {
        index_type q = linearize(i, j, k);
        begin = bin_[q].begin();
        end   = bin_[q].end();
        return (true);
      }
      return (false);
    }

    inline void lookup_ijk(iterator &begin, iterator &end, size_type i, size_type j,
                    size_type k)
    {
      index_type q = linearize(i, j, k);
      begin = bin_[q].begin();
      end   = bin_[q].end();
    }


    double min_distance_squared(const Core::Geometry::Point &p, size_type i,
                              size_type j, size_type k) const
    {
      Core::Geometry::Point r;
      transform_.unproject(p, r);

      // Splat the point onto the cell.
      if (r.x() < i) { r.x(i); }
      else if (r.x() > i+1) { r.x(i+1); }

      if (r.y() < j) { r.y(j); }
      else if (r.y() > j+1) { r.y(j+1); }

      if (r.z() < k) { r.z(k); }
      else if (r.z() > k+1) { r.z(k+1); }

      // Project the cell intersection back to world space.
      Core::Geometry::Point q;
      transform_.project(r, q);

      // Return distance from point to projected cell point.
      return (p - q).length2();
    }

  private:
    index_type linearize(index_type i, index_type j, index_type k) const
      { return (((i * nj_) + j) * nk_ + k); }


  private:
    /// Size of the search grid
    index_type ni_, nj_, nk_;
    /// Transformation to unitary coordinate system
    Core::Geometry::Transform transform_;

    /// Where to store the lookup table
    std::vector<std::vector<INDEX> > bin_;
};


} // namespace SCIRun

#endif
