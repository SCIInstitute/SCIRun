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


/// @todo Documentation Core/GeometryPrimitives/Vector.h

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#ifndef CORE_GEOMETRY_VECTOR_H
#define CORE_GEOMETRY_VECTOR_H

#include <cmath>
#include <algorithm>
#include <Core/Persistent/PersistentFwd.h>
#include <Core/Utils/Legacy/TypeDescription.h>
#include <Core/GeometryPrimitives/share.h>

#define NOMINMAX

namespace SCIRun {
namespace Core {
namespace Geometry {

  /// @todo move to math header
  template <typename T>
  inline const T& Min(const T& t1, const T& t2, const T& t3)
  {
    return std::min(std::min(t1,t2), t3);
  }

  template <typename T>
  inline const T& Max(const T& t1, const T& t2, const T& t3)
  {
    return std::max(std::max(t1,t2), t3);
  }

class Point;

class Vector
{
  private:
    double d_[3];
  public:
    inline explicit Vector(const Point&);
    inline Vector(double x, double y, double z)
    { d_[0] = x; d_[1] = y; d_[2] = z; }
    inline Vector(const Vector&);
    inline Vector();
    inline explicit Vector(double init)
    { d_[0] = init; d_[1] = init; d_[2] = init; }
    inline double length() const;
    inline double length2() const;
    friend inline double Dot(const Vector&, const Vector&);
    friend inline double Dot(const Point&, const Vector&);
    friend inline double Dot(const Vector&, const Point&);
    inline Vector& operator=(const Vector&);
    inline Vector& operator=(const double&);
    inline Vector& operator=(const int&);

    //Note vector[0]=vector.x();vector[1]=vector.y();vector[2]=vector.z()
    inline double& operator[](int idx)
    {
      return d_[idx];
    }

    //Note vector[0]=vector.x();vector[1]=vector.y();vector[2]=vector.z()
    inline double operator[](int idx) const
    {
      return d_[idx];
    }

    inline Vector operator*(const double) const;
    inline Vector operator*(const Vector&) const;
    inline Vector& operator*=(const double);
    inline Vector& operator*=(const Vector&);
    inline Vector operator/(const double) const;
    inline Vector operator/(const Vector&) const;
    inline Vector& operator/=(const double);
    inline Vector operator+(const Vector&) const;
    inline Vector& operator+=(const Vector&);
    inline Vector operator-() const;
    inline Vector operator-(const Vector&) const;
    inline Vector operator-(const Point&) const;
    inline Vector& operator-=(const Vector&);
    inline double normalize();
    inline double safe_normalize();
    inline Vector getArbitraryTangent() const;
    SCISHARE Vector normal() const;
    SCISHARE Vector safe_normal() const;
    friend inline Vector Cross(const Vector&, const Vector&);
    friend inline Vector Abs(const Vector&);
    inline void x(double);
    inline double x() const;
    inline void y(double);
    inline double y() const;
    inline void z(double);
    inline double z() const;

    inline void u(double);
    inline double u() const;
    inline void v(double);
    inline double v() const;
    inline void w(double);
    inline double w() const;

    SCISHARE void rotz90(const int);
    SCISHARE std::string get_string() const;

    friend class Point;

    friend inline Vector Interpolate(const Vector&, const Vector&, double);

    SCISHARE void find_orthogonal(Vector&, Vector&) const;
    SCISHARE bool check_find_orthogonal(Vector&, Vector&) const;

    inline const Point &point() const;
    inline Point &asPoint() const;

    inline double minComponent() const;
    inline double maxComponent() const;

    inline void Set(double x, double y, double z);
};

SCISHARE bool operator==(const Vector& v1, const Vector& v2);
SCISHARE bool operator!=(const Vector& v1, const Vector& v2);
SCISHARE std::ostream& operator<<(std::ostream& os, const Vector& p);
SCISHARE std::istream& operator>>(std::istream& os, Vector& p);
SCISHARE Vector vectorFromString(const std::string& str);

inline Vector::Vector()
{
  d_[0] = 0.0;
  d_[1] = 0.0;
  d_[2] = 0.0;
}

inline Vector::Vector(const Vector& p)
{
  d_[0] = p.d_[0];
  d_[1] = p.d_[1];
  d_[2] = p.d_[2];
}

inline Vector& Vector::operator=(const Vector& v)
{
  d_[0]=v.d_[0];
  d_[1]=v.d_[1];
  d_[2]=v.d_[2];
  return *this;
}

inline void Vector::x(double d)
{
  d_[0]=d;
}

inline double Vector::x() const
{
  return d_[0];
}

inline void Vector::y(double d)
{
  d_[1]=d;
}

inline double Vector::y() const
{
  return d_[1];
}

inline void Vector::z(double d)
{
  d_[2]=d;
}

inline double Vector::z() const
{
  return d_[2];
}

inline Vector& Vector::operator*=(const double d)
{
  d_[0]*=d;
  d_[1]*=d;
  d_[2]*=d;
  return *this;
}

// Allows for double * Vector so that everything doesn't have to be
// Vector * double
inline Vector operator*(const double s, const Vector& v) {
  return v*s;
}

inline Vector Min(const Vector &v1, const Vector &v2)
{
  return Vector((std::min)(v1.x(), v2.x()),
    (std::min)(v1.y(), v2.y()),
    (std::min)(v1.z(), v2.z()));
}

inline Vector Max(const Vector &v1, const Vector &v2)
{
  return Vector((std::max)(v1.x(), v2.x()),
    (std::max)(v1.y(), v2.y()),
    (std::max)(v1.z(), v2.z()));
}

SCISHARE void Pio( Piostream&, Vector& );

inline
  double Vector::safe_normalize()
{
  double l = std::sqrt(d_[0]*d_[0] + d_[1]*d_[1] + d_[2]*d_[2]);
  if (l > 0.0)
  {
    d_[0]/=l;
    d_[1]/=l;
    d_[2]/=l;
  }
  return l;
}

inline double Vector::length2() const
{
  return d_[0]*d_[0]+d_[1]*d_[1]+d_[2]*d_[2];
}


inline double Vector::minComponent() const
{
  return Min(d_[0], d_[1], d_[2]);
}

inline double Vector::maxComponent() const
{
  return Max(d_[0], d_[1], d_[2]);
}

inline Vector Vector::operator/(const double d) const
{
  return Vector(d_[0]/d, d_[1]/d, d_[2]/d);
}

inline Vector Vector::operator/(const Vector& v2) const
{
  return Vector(d_[0]/v2.d_[0], d_[1]/v2.d_[1], d_[2]/v2.d_[2]);
}

inline Vector Vector::operator+(const Vector& v2) const
{
  return Vector(d_[0]+v2.d_[0], d_[1]+v2.d_[1], d_[2]+v2.d_[2]);
}

inline Vector Vector::operator*(const Vector& v2) const
{
  return Vector(d_[0]*v2.d_[0], d_[1]*v2.d_[1], d_[2]*v2.d_[2]);
}

inline Vector Vector::operator-(const Vector& v2) const
{
  return Vector(d_[0]-v2.d_[0], d_[1]-v2.d_[1], d_[2]-v2.d_[2]);
}


inline Vector& Vector::operator=(const double& d)
{
  d_[0] = d;
  d_[1] = d;
  d_[2] = d;
  return *this;
}

inline Vector& Vector::operator=(const int& d)
{
  d_[0] = static_cast<double>(d);
  d_[1] = static_cast<double>(d);
  d_[2] = static_cast<double>(d);
  return *this;
}

inline bool operator<(Vector v1, Vector v2)
{
  return(v1.length()<v2.length());
}

inline bool operator<=(Vector v1, Vector v2)
{
  return(v1.length()<=v2.length());
}

inline bool operator>(Vector v1, Vector v2)
{
  return(v1.length()>v2.length());
}

inline bool operator>=(Vector v1, Vector v2)
{
  return(v1.length()>=v2.length());
}



inline Vector Vector::operator*(const double s) const
{
  return Vector(d_[0]*s, d_[1]*s, d_[2]*s);
}

inline Vector& Vector::operator*=(const Vector& v)
{
  d_[0] *= v.d_[0];
  d_[1] *= v.d_[1];
  d_[2] *= v.d_[2];
  return *this;
}




inline Vector& Vector::operator+=(const Vector& v2)
{
  d_[0]+=v2.d_[0];
  d_[1]+=v2.d_[1];
  d_[2]+=v2.d_[2];
  return *this;
}

inline Vector& Vector::operator-=(const Vector& v2)
{
  d_[0]-=v2.d_[0];
  d_[1]-=v2.d_[1];
  d_[2]-=v2.d_[2];
  return *this;
}

inline Vector Vector::operator-() const
{
  return Vector(-d_[0],-d_[1],-d_[2]);
}

inline double Vector::length() const
{
  return sqrt(d_[0]*d_[0]+d_[1]*d_[1]+d_[2]*d_[2]);
}

inline Vector Abs(const Vector& v)
{
  double x=v.d_[0]<0?-v.d_[0]:v.d_[0];
  double y=v.d_[1]<0?-v.d_[1]:v.d_[1];
  double z=v.d_[2]<0?-v.d_[2]:v.d_[2];
  return Vector(x,y,z);
}

inline Vector Cross(const Vector& v1, const Vector& v2)
{
  return Vector(v1.d_[1]*v2.d_[2]-v1.d_[2]*v2.d_[1],
		v1.d_[2]*v2.d_[0]-v1.d_[0]*v2.d_[2],
		v1.d_[0]*v2.d_[1]-v1.d_[1]*v2.d_[0]);
}

//returns a unit vector
static const double THRESHOLD = std::sqrt(2.0)/2;
static const Vector i(1,0,0);
static const Vector j(0,1,0);
inline Vector Vector::getArbitraryTangent() const
{
  Vector normalized = this->normal();

  if (fabs(normalized.x()) < THRESHOLD)
     return (i - normalized * normalized.x()).normal();

  return (j - normalized * normalized.y()).normal();
}

inline Vector Interpolate(const Vector& v1, const Vector& v2,
			  double weight)
{
  double weight1=1.0-weight;
  return Vector(v2.d_[0]*weight+v1.d_[0]*weight1,
		v2.d_[1]*weight+v1.d_[1]*weight1,
		v2.d_[2]*weight+v1.d_[2]*weight1);
}



inline Vector& Vector::operator/=(const double d)
{
  d_[0]/=d;
  d_[1]/=d;
  d_[2]/=d;
  return *this;
}

inline void Vector::u(double d)
{
  d_[0]=d;
}

inline double Vector::u() const
{
  return d_[0];
}

inline void Vector::v(double d)
{
  d_[1]=d;
}

inline double Vector::v() const
{
  return d_[1];
}

inline void Vector::w(double d)
{
  d_[2]=d;
}

inline double Vector::w() const
{
  return d_[2];
}



inline
double Vector::normalize()
{
  double l=sqrt(d_[0]*d_[0] + d_[1]*d_[1] + d_[2]*d_[2]);
  if (l > 0.0)
  {
    d_[0]/=l;
    d_[1]/=l;
    d_[2]/=l;
  }
  return l;
}




inline const Point &Vector::point() const
{
  return reinterpret_cast<const Point &>(*this);
}

inline Point &Vector::asPoint() const
{
  return reinterpret_cast<Point &>(const_cast<Vector &>(*this));
}



inline void Vector::Set(double x, double y, double z)
{
  d_[0] = x;
  d_[1] = y;
  d_[2] = z;
}

SCISHARE const TypeDescription* get_type_description(Vector*);

}}
/// @todo: This one is obsolete when dynamic compilation will be abandoned
const std::string& Vector_get_h_file_path();
}

#endif
