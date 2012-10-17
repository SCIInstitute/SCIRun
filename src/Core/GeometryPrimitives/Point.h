/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
#include <Core/GeometryPrimitives/Share.h>

namespace SCIRun {
namespace Core {
namespace Geometry {

  class Vector;

class Point 
{
private:
  double d_[3];
public:
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  inline explicit Point(const Vector& v);
#endif
  inline Point(double x, double y, double z)
  { d_[0] = x; d_[1] = y; d_[2] = z; }
    Point(double, double, double, double);
  inline Point(const Point&);
  inline Point();
  inline Point& operator=(const Point&);
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  inline Vector operator+(const Point&) const;
  inline Vector operator-(const Point&) const;
  inline Point operator+(const Vector&) const;
  inline Point operator-(const Vector&) const;
#endif
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
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  inline const Vector &vector() const;
  inline Vector &asVector() const;
#endif
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

// Actual declarations of these functions (as 'friend' above doesn't
// (depending on the compiler) actually declare them.
SCISHARE Point AffineCombination(const Point&, double, const Point&, double,
				  const Point&, double, const Point&, double);
SCISHARE Point AffineCombination(const Point&, double, const Point&, double, 
				  const Point&, double);
SCISHARE Point AffineCombination(const Point&, double, const Point&, double);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
SCISHARE void Pio( Piostream&, Point& );

// TODO: This one is obsolete when last part dynamic compilation is gone
SCISHARE const std::string& Point_get_h_file_path();
SCISHARE const TypeDescription* get_type_description(Point*);
#endif

inline 
Point operator*(double d, const Point &p) {
  return p*d;
}
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
inline 
Point operator+(const Vector &v, const Point &p) {
  return p+v;
}
#endif

SCISHARE std::ostream& operator<<(std::ostream& os, const Point& p);
SCISHARE std::istream& operator>>(std::istream& os, Point& p);
SCISHARE Point centroid(const std::vector<Point>& points);


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

// This cannot be above due to circular dependencies
#include <Core/Geometry/Vector.h>

namespace SCIRun {

inline Point::Point(const Vector& v)
{
  d_[0] = v.d_[0];
  d_[1] = v.d_[1]; 
  d_[2] = v.d_[2];
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



inline Point& Point::operator+=(const Point& v)
{
  d_[0]+=v.d_[0];
  d_[1]+=v.d_[1];
  d_[2]+=v.d_[2];
  return *this;
}

inline Point& Point::operator-=(const Point& v)
{
  d_[0]-=v.d_[0];
  d_[1]-=v.d_[1];
  d_[2]-=v.d_[2];
  return *this;
}


inline Point& Point::operator*=(const double d)
{
  d_[0]*=d;
  d_[1]*=d;
  d_[2]*=d;
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
  return Point(v2.d_[0]*weight+v1.d_[0]*weight1,
               v2.d_[1]*weight+v1.d_[1]*weight1,
               v2.d_[2]*weight+v1.d_[2]*weight1);
}

inline void Point::addscaled(const Point& p, const double scale) 
{
  // this += p * w;
  d_[0] += p.d_[0] * scale;
  d_[1] += p.d_[1] * scale;
  d_[2] += p.d_[2] * scale;
}

#endif
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
inline const Vector &Point::vector() const
{
  return reinterpret_cast<const Vector &>(*this);
}

inline Vector &Point::asVector() const
{
  return reinterpret_cast<Vector &>(const_cast<Point &>(*this));
}
#endif
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
inline Point Min(const Point& p1, const Point& p2)
{
  double x=std::min(p1.d_[0], p2.d_[0]);
  double y=std::min(p1.d_[1], p2.d_[1]);
  double z=std::min(p1.d_[2], p2.d_[2]);
  return Point(x,y,z);
}

inline Point Max(const Point& p1, const Point& p2)
{
  double x=std::max(p1.d_[0], p2.d_[0]);
  double y=std::max(p1.d_[1], p2.d_[1]);
  double z=std::max(p1.d_[2], p2.d_[2]);
  return Point(x,y,z);
}

inline double Dot(const Point& p, const Vector& v)
{
  return p.d_[0]*v.d_[0]+p.d_[1]*v.d_[1]+p.d_[2]*v.d_[2];
}

inline double Dot(const Point& p1, const Point& p2)
{
  return p1.d_[0]*p2.d_[0] + p1.d_[1]*p2.d_[1] + p1.d_[2]*p2.d_[2];
}
#endif

}}}

#include <Core/GeometryPrimitives/PointVectorOperators.h>

#endif
