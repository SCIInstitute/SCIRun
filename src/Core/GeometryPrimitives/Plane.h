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

/// @todo Documentation Core/GeometryPrimitives/Plane.h

#ifndef CORE_GEOMETRY_PLANE_H
#define CORE_GEOMETRY_PLANE_H 1

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>

#include <Core/GeometryPrimitives/share.h>

namespace SCIRun {
  namespace Core {
    namespace Geometry {

class Point;
class SCISHARE Plane {
  private:
    Vector n;
    double d;
    
  public:
    Plane(const Plane &copy);
    Plane(const Point &p1, const Point &p2, const Point &p3);
    Plane(const Point &p, const Vector &n);
    Plane(const Vector& normal, double distance);
    Plane();
    Plane(double a, double b, double c, double d);
    ~Plane();
    
    double eval_point(const Point &p) const;
    void flip();
    Point project(const Point& p) const;
    Vector project(const Vector& v) const;
    Vector normal() const;
    double distance() const;
    void get(double (&abcd)[4]) const;

    // Not a great ==, doesnt take into account for floating point error.
    bool operator==(const Plane &rhs) const; 
    // changes the plane ( n and d )
     
    void ChangePlane( const Point &p1, const Point &p2, const Point &p3 );
    void ChangePlane( const Point &p1, const Vector &v); 

    // returns true if the line  v*t+s  for -inf < t < inf intersects
    // the plane.  if so, hit contains the point of intersection.

    int Intersect( const Point& s, const Vector& v, Point& hit ) const;
    int Intersect( const Point& s, const Vector& v, double &t ) const;
};

}}}


#endif
