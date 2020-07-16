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

#ifndef CORE_GEOMETRY_POINT_H
#define CORE_GEOMETRY_POINT_H

#include <string>
#include <vector>
#include <algorithm>
#include <boost/static_assert.hpp>
#include <Core/Utils/Legacy/TypeDescription.h>
#include <Core/Persistent/PersistentFwd.h>
#include <Core/GeometryPrimitives/share.h>

namespace SCIRun {
namespace Core {
namespace Geometry {

  class Vector;

class Point
{
private:
  double d_[3];
public:
  inline explicit Point(const Vector& v);
  inline Point(double x, double y, double z)
  { d_[0] = x; d_[1] = y; d_[2] = z; }

  template <typename T>
  inline Point(T x, T y, T z)
  {
    BOOST_STATIC_ASSERT(std::is_arithmetic<T>::value);
    d_[0] = static_cast<double>(x);
    d_[1] = static_cast<double>(y);
    d_[2] = static_cast<double>(z);
  }

  SCISHARE Point(double, double, double, double);
  inline Point(const Point&);
  inline Point();
  inline Point& operator=(const Point&);
  inline Vector operator+(const Point&) const;
  inline Vector operator-(const Point&) const;
  inline Point operator+(const Vector&) const;
  inline Point operator-(const Vector&) const;
  inline Point operator*(double) const;
  inline Point& operator*=(const double);
  Point& operator+=(const Vector&);
  Point& operator-=(const Vector&);
  inline Point& operator+=(const Point&);
  inline Point& operator-=(const Point&);
  inline Point& operator/=(const double);
  inline Point operator/(const double) const;
  inline Point operator-() const;
  inline double& operator()(int idx);
  inline double operator()(int idx) const;
  inline void addscaled(const Point& p, const double scale);  // this += p * w;
  inline void x(const double);
  inline double x() const;
  inline void y(const double);
  inline double y() const;
  inline void z(const double);
  inline double z() const;

  inline double& operator[](int idx)
  {
    return d_[idx];
  }

  inline double operator[](int idx) const
  {
    return d_[idx];
  }

  SCISHARE std::string get_string() const;

  SCISHARE friend std::ostream& operator<<(std::ostream& os, const Point& p);
  SCISHARE friend std::istream& operator>>(std::istream& os, Point& p);
};

SCISHARE bool operator==(const Point& p1, const Point& p2);
SCISHARE bool operator!=(const Point& p1, const Point& p2);
SCISHARE Point pointFromString(const std::string& str);

inline Point::Point(const Point& p)
{
  d_[0] = p.d_[0];
  d_[1] = p.d_[1];
  d_[2] = p.d_[2];
}

inline Point::Point()
{
  d_[0] = 0.0;
  d_[1] = 0.0;
  d_[2] = 0.0;
}

inline Point& Point::operator=(const Point& p)
{
  d_[0] = p.d_[0];
  d_[1] = p.d_[1];
  d_[2] = p.d_[2];
  return *this;
}

inline void Point::x(const double d)
{
  d_[0]=d;
}

inline double Point::x() const
{
  return d_[0];
}

inline void Point::y(const double d)
{
  d_[1]=d;
}

inline double Point::y() const
{
  return d_[1];
}

inline void Point::z(const double d)
{
  d_[2]=d;
}

inline double Point::z() const
{
  return d_[2];
}

inline Point& Point::operator*=(const double d)
{
  d_[0]*=d;
  d_[1]*=d;
  d_[2]*=d;
  return *this;
}

inline Point& Point::operator+=(const Point& v)
{
  d_[0]+=v.d_[0];
  d_[1]+=v.d_[1];
  d_[2]+=v.d_[2];
  return *this;
}

// Actual declarations of these functions (as 'friend' above doesn't
// (depending on the compiler) actually declare them.
SCISHARE Point AffineCombination(const Point&, double, const Point&, double,
				  const Point&, double, const Point&, double);
SCISHARE Point AffineCombination(const Point&, double, const Point&, double,
				  const Point&, double);
SCISHARE Point AffineCombination(const Point&, double, const Point&, double);

SCISHARE void Pio( Piostream&, Point& );

inline
Point operator*(double d, const Point &p) {
  return p*d;
}
inline
Point operator+(const Vector &v, const Point &p) {
  return p+v;
}

SCISHARE std::ostream& operator<<(std::ostream& os, const Point& p);
SCISHARE std::istream& operator>>(std::istream& os, Point& p);
SCISHARE Point centroid(const std::vector<Point>& points);

inline Point Min(const Point& p1, const Point& p2)
{
  double x=(std::min)(p1[0], p2[0]);
  double y=(std::min)(p1[1], p2[1]);
  double z=(std::min)(p1[2], p2[2]);
  return Point(x,y,z);
}

inline Point Max(const Point& p1, const Point& p2)
{
  double x=(std::max)(p1[0], p2[0]);
  double y=(std::max)(p1[1], p2[1]);
  double z=(std::max)(p1[2], p2[2]);
  return Point(x,y,z);
}


}}

/// @todo: This one is obsolete when last part dynamic compilation is gone
SCISHARE const std::string& Point_get_h_file_path();
SCISHARE const SCIRun::TypeDescription* get_type_description(Core::Geometry::Point*);
}

#include <Core/GeometryPrimitives/PointVectorOperators.h>

#endif
