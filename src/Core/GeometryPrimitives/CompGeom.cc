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


///
///@file CompGeom.cc
///
///@author
///      Allen Sanderson
///      SCI Institute
///      University of Utah
///@date August 2005
///

#include <Core/GeometryPrimitives/CompGeom.h>
#include <iostream>

namespace SCIRun {
namespace Core {
namespace Geometry {

double
distance_to_line2(const Point &p, const Point &a, const Point &b,
                  const double epsilon)
{
  Vector m = b - a;
  Vector n = p - a;
  if (m.length2() < epsilon) {
    return n.length2();
  }
  else {
    const double t0 = Dot(m, n) / Dot(m, m);
    if (t0 <= 0) return (n).length2();
    else if (t0 >= 1.0) return (p - b).length2();
    else return (n - m * t0).length2();
  }
}


void
distance_to_line2_aux(Point &result,
                      const Point &p, const Point &a, const Point &b,
                      const double epsilon)
{
  Vector m = b - a;
  Vector n = p - a;
  if (m.length2() < epsilon) {
    result = a;
  }
  else
  {
    const double t0 = Dot(m, n) / Dot(m, m);
    if (t0 <= 0)
    {
      result = a;
    }
    else if (t0 >= 1.0)
    {
      result = b;
    }
    else
    {
      Vector offset = m * t0;
      result = a + offset;
    }
  }
}


void
distance_to_line2_aux(Point &result, int& node,
                      const Point &p, const Point &a, const Point &b,
                      const double epsilon)
{
  node = -1;
  Vector m = b - a;
  Vector n = p - a;
  if (m.length2() < epsilon)
  {
    node = 0;
    result = a;
  }
  else
  {
    const double t0 = Dot(m, n) / Dot(m, m);
    if (t0 <= 0)
    {
      node = 0;
      result = a;
    }
    else if (t0 >= 1.0)
    {
      node = 1;
      result = b;
    }
    else
    {
      Vector offset = m * t0;
      result = a + offset;
    }
  }
}


void
closest_point_on_tri(Point &result, const Point &orig,
                     const Point &p0, const Point &p1, const Point &p2,
                     const double epsilon)
{
  const Vector edge1 = p1 - p0;
  const Vector edge2 = p2 - p0;



  const Vector dir = Cross(edge1, edge2);

  const Vector pvec = Cross(dir, edge2);

  const double inv_det = 1.0 / Dot(edge1, pvec);

  const Vector tvec = orig - p0;
  double u = Dot(tvec, pvec) * inv_det;

  const Vector qvec = Cross(tvec, edge1);
  double v = Dot(dir, qvec) * inv_det;

  int bound=0;
  Point tmp_r;

  if (u < 0.0)
  {
    distance_to_line2_aux(result, orig, p0, p2,epsilon);
    bound++;
  }
  if (v < 0.0)
  {
    distance_to_line2_aux(tmp_r, orig, p0, p1,epsilon);

    if (bound>0)
    {
      double tmp_dist1 = Vector(orig-result).length2();
      double tmp_dist2 = Vector(orig-tmp_r).length2();
      if (tmp_dist2<tmp_dist1) result = tmp_r;
    }
    else
    {
      result = tmp_r;
    }
    bound++;
  }
  if (u + v > 1.0)
  {
    distance_to_line2_aux(tmp_r, orig, p1, p2,epsilon);

    if (bound>0)
    {
      double tmp_dist1 = Vector(orig-result).length2();
      double tmp_dist2 = Vector(orig-tmp_r).length2();
      if (tmp_dist2<tmp_dist1) result = tmp_r;
    }
    else
    {
      result = tmp_r;
    }
    bound++;
  }

  if (bound==0)
  {
    result = p0 + u * edge1 + v * edge2;
  }
}


void
closest_point_on_tri(Point &result,
                     int& edge,
                     int& node,
                     const Point &orig,
                     const Point &p0,
                     const Point &p1,
                     const Point &p2,
                     const double epsilon)
{
  const Vector edge1 = p1 - p0;
  const Vector edge2 = p2 - p0;

  const Vector dir = Cross(edge1, edge2);

  const Vector pvec = Cross(dir, edge2);

  const double inv_det = 1.0 / Dot(edge1, pvec);

  const Vector tvec = orig - p0;
  double u = Dot(tvec, pvec) * inv_det;

  const Vector qvec = Cross(tvec, edge1);
  double v = Dot(dir, qvec) * inv_det;

  int bound=0;
  int tmp_node;
  Point tmp_r;

  if (u < 0.0)
  {
    distance_to_line2_aux(result, node, orig, p0, p2,epsilon);
    if (node == 1) node = 2;
    edge = 2;
    bound++;
  }
  if (v < 0.0)
  {
    distance_to_line2_aux(tmp_r, tmp_node, orig, p0, p1,epsilon);
    if (bound>0)
    {
      double tmp_dist1 = Vector(orig-result).length2();
      double tmp_dist2 = Vector(orig-tmp_r).length2();
      if (tmp_dist2<tmp_dist1 || (v<u && tmp_dist2==tmp_dist1))
      {
        result = tmp_r;
        node=tmp_node;
        edge = 0;
      }
    }
    else
    {
      result = tmp_r;
      node=tmp_node;
      edge = 0;
    }
    bound++;
  }
  if (u + v > 1.0)
  {
    distance_to_line2_aux(tmp_r, tmp_node, orig, p1, p2,epsilon);

    if (bound>0)
    {
      double tmp_dist1 = Vector(orig-result).length2();
      double tmp_dist2 = Vector(orig-tmp_r).length2();
      if (tmp_dist2<tmp_dist1 || (((u+v-1)>fabs(v) || (u+v-1)>fabs(u)) && tmp_dist2==tmp_dist1) )
      {
        result = tmp_r;
        if (tmp_node>=0) tmp_node = tmp_node + 1;
        node=tmp_node;
        edge = 1;
      }
    }
    else
    {
      result = tmp_r;
      if (tmp_node>=0) tmp_node = tmp_node + 1;
      node = tmp_node;
      edge = 1;
    }
    bound++;
  }
  if (bound==0)
  {
    result = p0 + u * edge1 + v * edge2;
    edge = -1;
    node = -1;
  }
}


void
est_closest_point_on_quad(Point &result, const Point &orig,
                      const Point &p0, const Point &p1,
                      const Point &p2,const Point &p3,
                      const double epsilon)
{
  // This implementation is only an approximation
  Point r1,r2;
  closest_point_on_tri(r1,orig,p0,p1,p3,epsilon);
  closest_point_on_tri(r2,orig,p3,p1,p2,epsilon);

  Point r3,r4;
  closest_point_on_tri(r3,orig,p0,p2,p3,epsilon);
  closest_point_on_tri(r4,orig,p0,p1,p2,epsilon);

  if ((r2-orig).length2() < (r1-orig).length2()) r1 = r2;
  if ((r4-orig).length2() < (r3-orig).length2()) r3 = r4;

  r1 += r3;
  result = 0.5*r1;
}


double
RayPlaneIntersection(const Point &p,  const Vector &dir,
		     const Point &p0, const Vector &pn, const double epsilon)
{
  // Compute divisor.
  const double Vd = Dot(dir, pn);

  // Return no intersection if parallel to plane or no cross product.
  if (Vd < epsilon)
    return 1.0e30*epsilon;

  const double D = - Dot(pn, p0);

  const double V0 = - (Dot(pn, p) + D);

  return V0 / Vd;
}



bool
RayTriangleIntersection(double &t, double &u, double &v, bool backface_cull,
                        const Point &orig,  const Vector &dir,
                        const Point &p0, const Point &p1, const Point &p2,
                        const double epsilon)
{
  const Vector edge1 = p1 - p0;
  const Vector edge2 = p2 - p0;
  const double vepsilon = epsilon*epsilon*epsilon;

  const Vector pvec = Cross(dir, edge2);

  const double det = Dot(edge1, pvec);

  if (det < vepsilon && (backface_cull || det > -vepsilon))
    return false;

  const double inv_det = 1.0 / det;

  const Vector tvec = orig - p0;

  u = Dot(tvec, pvec) * inv_det;
  if (u < 0.0 || u > 1.0)
    return false;

  const Vector qvec = Cross(tvec, edge1);

  v = Dot(dir, qvec) * inv_det;
  if (v < 0.0 || u+v > 1.0)
    return false;

  t = Dot(edge2, qvec) * inv_det;

  return true;
}


bool
closest_line_to_line(double &s, double &t,
                     const Point &a0, const Point &a1,
                     const Point &b0, const Point &b1,
                     const double epsilon)
{
  const Vector u = a1 - a0;
  const Vector v = b1 - b0;
  const Vector w = a0 - b0;

  const double a = Dot(u, u);
  const double b = Dot(u, v);
  const double c = Dot(v, v);
  const double d = Dot(u, w);
  const double e = Dot(v, w);
  const double D = a*c - b*b;

  if (D < epsilon*epsilon)
  {
    s = 0.0;
    t = (b>c?d/b:e/c);
    return false;
  }
  else
  {
    s = (b*e - c*d) / D;
    t = (a*e - b*d) / D;
    return true;
  }
}


void
uniform_sample_triangle(Point &p, const Point &p0,
                        const Point &p1, const Point &p2,
                        MusilRNG &rng)
{
  // Fold the quad sample into a triangle.
  double u = rng();
  double v = rng();
  if (u + v > 1.0) { u = 1.0 - u; v = 1.0 - v; }

  // Compute the position of the random point.
  p = p0+((p1-p0)*u)+((p2-p0)*v);
}


void
uniform_sample_tetrahedra(Point &p, const Point &p0, const Point &p1,
                          const Point &p2, const Point &p3,
                          MusilRNG &rng)
{
  double t = rng();
  double u = rng();
  double v = rng();

  // Fold cube into prism.
  if (t + u > 1.0)
  {
    t = 1.0 - t;
    u = 1.0 - u;
  }

  // Fold prism into tet.
  if (u + v > 1.0)
  {
    const double tmp = v;
    v = 1.0 - t - u;
    u = 1.0 - tmp;
  }
  else if (t + u + v > 1.0)
  {
    const double tmp = v;
    v = t + u + v - 1.0;
    t = 1.0 - u - tmp;
  }

  // Convert to Barycentric and compute new point.
  const double a = 1.0 - t - u - v;
  p = Point(p0*a + p1*t + p2*u + p3*v);
}


double
tetrahedra_volume(const Point &p0, const Point &p1,
                  const Point &p2, const Point &p3)
{
  return fabs(Dot(Cross(p1-p0,p2-p0),p3-p0)) / 6.0;
}


void
TriTriIntersection(const Point &A0, const Point &A1, const Point &A2,
                   const Point &B0, const Point &B1, const Point &B2,
                   std::vector<Point> &results)
{
  double t, u, v;
  if (RayTriangleIntersection(t, u, v, false, A0, A1-A0, B0, B1, B2) &&
      t >= 0.0 && t <= 1.0)
  {
    results.push_back(A0 + (A1-A0) * t);
  }
  if (RayTriangleIntersection(t, u, v, false, A1, A2-A1, B0, B1, B2) &&
      t >= 0.0 && t <= 1.0)
  {
    results.push_back(A1 + (A2-A1) * t);
  }
  if (RayTriangleIntersection(t, u, v, false, A2, A0-A2, B0, B1, B2) &&
      t >= 0.0 && t <= 1.0)
  {
    results.push_back(A2 + (A0-A2) * t);
  }
  if (RayTriangleIntersection(t, u, v, false, B0, B1-B0, A0, A1, A2) &&
      t >= 0.0 && t <= 1.0)
  {
    results.push_back(B0 + (B1-B0) * t);
  }
  if (RayTriangleIntersection(t, u, v, false, B1, B2-B1, A0, A1, A2) &&
      t >= 0.0 && t <= 1.0)
  {
    results.push_back(B1 + (B2-B1) * t);
  }
  if (RayTriangleIntersection(t, u, v, false, B2, B0-B2, A0, A1, A2) &&
      t >= 0.0 && t <= 1.0)
  {
    results.push_back(B2 + (B0-B2) * t);
  }
}

}}}
