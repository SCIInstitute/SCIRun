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


///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

/// @todo Documentation Core/GeometryPrimitives/PointVectorOperators.h

#ifndef CORE_GEOMETRY_POINTVECTOROPERATORS_H
#define CORE_GEOMETRY_POINTVECTOROPERATORS_H

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/share.h>

namespace SCIRun {
namespace Core {
namespace Geometry {

  inline Point& Point::operator+=(const Vector& v)
  {
    d_[0]+=v.d_[0];
    d_[1]+=v.d_[1];
    d_[2]+=v.d_[2];
    return *this;
  }

  inline Point& Point::operator-=(const Vector& v)
  {
    d_[0]-=v.d_[0];
    d_[1]-=v.d_[1];
    d_[2]-=v.d_[2];
    return *this;
  }

  inline Vector::Vector(const Point& p)
  {
    d_[0] = p[0];
    d_[1] = p[1];
    d_[2] = p[2];
  }

  inline Vector Point::operator+(const Point& p) const
  {
    return Vector(d_[0]+p.d_[0], d_[1]+p.d_[1], d_[2]+p.d_[2]);
  }

  inline Vector Point::operator-(const Point& p) const
  {
    return Vector(d_[0]-p.d_[0], d_[1]-p.d_[1], d_[2]-p.d_[2]);
  }

  inline Point Point::operator+(const Vector& v) const
  {
    return Point(d_[0]+v.d_[0], d_[1]+v.d_[1], d_[2]+v.d_[2]);
  }

  inline Point Point::operator-(const Vector& v) const
  {
    return Point(d_[0]-v.d_[0], d_[1]-v.d_[1], d_[2]-v.d_[2]);
  }

  inline double Dot(const Vector& v1, const Vector& v2)
  {
    return v1.d_[0]*v2.d_[0]+v1.d_[1]*v2.d_[1]+v1.d_[2]*v2.d_[2];
  }

  inline double Dot(const Vector& v, const Point& p)
  {
    return v[0]*p[0]+v[1]*p[1]+v[2]*p[2];
  }
  inline Vector Vector::operator-(const Point& v2) const
  {
    return Vector(d_[0]-v2[0], d_[1]-v2[1], d_[2]-v2[2]);
  }


  inline Point::Point(const Vector& v)
  {
    d_[0] = v.d_[0];
    d_[1] = v.d_[1];
    d_[2] = v.d_[2];
  }


  inline Point& Point::operator-=(const Point& v)
  {
    d_[0]-=v.d_[0];
    d_[1]-=v.d_[1];
    d_[2]-=v.d_[2];
    return *this;
  }




  inline Point& Point::operator/=(const double d)
  {
    d_[0]/=d;
    d_[1]/=d;
    d_[2]/=d;
    return *this;
  }

  inline Point Point::operator-() const
  {
    return Point(-d_[0], -d_[1], -d_[2]);
  }

  inline Point Point::operator*(double d) const
  {
    return Point(d_[0]*d, d_[1]*d, d_[2]*d);
  }

  inline Point Point::operator/(const double d) const
  {
    return Point(d_[0]/d,d_[1]/d,d_[2]/d);
  }

  inline double& Point::operator()(int idx)
  {
    return d_[idx];
  }

  inline double Point::operator()(int idx) const
  {
    return d_[idx];
  }

  inline Point Interpolate(const Point& v1, const Point& v2,
    double weight)
  {
    double weight1 = 1.0 - weight;
    return Point(v2[0]*weight+v1[0]*weight1,
      v2[1]*weight+v1[1]*weight1,
      v2[2]*weight+v1[2]*weight1);
  }

  inline void Point::addscaled(const Point& p, const double scale)
  {
    // this += p * w;
    d_[0] += p.d_[0] * scale;
    d_[1] += p.d_[1] * scale;
    d_[2] += p.d_[2] * scale;
  }

  inline double Dot(const Point& p, const Vector& v)
  {
    return p[0]*v[0]+p[1]*v[1]+p[2]*v[2];
  }

  inline double Dot(const Point& p1, const Point& p2)
  {
    return p1[0]*p2[0] + p1[1]*p2[1] + p1[2]*p2[2];
  }
}}}


#endif
