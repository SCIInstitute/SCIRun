/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
University of Utah.

License for the specific language governing rights and limitations under
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

#include <Graphics/Glyphs/GlyphGeom.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace SCIRun::Graphics;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;

GlyphGeom::GlyphGeom() : numVBOElements_(0)
{

}

void GlyphGeom::getBufferInfo(int64_t& numVBOElements, std::vector<Vector>& points, std::vector<Vector>& normals,
                              std::vector<ColorRGB>& colors, std::vector<uint32_t>& indices)
{
  numVBOElements = numVBOElements_;
  points = points_;
  normals = normals_;
  colors = colors_;
  indices = indices_;
}

void GlyphGeom::addArrow(const Point& p1, const Point& p2, double radius, double resolution,
                         const ColorRGB& color1, const ColorRGB& color2)
{
  double ratio = 0.5;

  Point mid(ratio * (p1.x() + p2.x()), ratio * (p1.y() + p2.y()), ratio * (p1.z() + p2.z()));

  generateCylinder(p1, mid, radius / 3.0, radius / 3.0, resolution, color1, color2, numVBOElements_, points_, normals_, indices_, colors_);
  generateCylinder(mid, p2, radius, 0.0, resolution, color1, color2, numVBOElements_, points_, normals_, indices_, colors_);
}

void GlyphGeom::addSphere(const Point& p, double radius, double resolution, const ColorRGB& color)
{
  generateEllipsoid(p, radius, radius, resolution, color, numVBOElements_, points_, normals_, indices_, colors_);
}

void GlyphGeom::addCylinder(const Point p1, const Point& p2, double radius, double resolution,
                            const ColorRGB& color1, const ColorRGB& color2)
{
  generateCylinder(p1, p2, radius, radius, resolution, color1, color2, numVBOElements_, points_, normals_, indices_, colors_);
}

void GlyphGeom::generateCylinder(const Point& p1, const Point& p2, double radius1,
                                 double radius2, double resolution, const ColorRGB& color1, const ColorRGB& color2,
                                 int64_t& numVBOElements, std::vector<Vector>& points, std::vector<Vector>& normals,
                                 std::vector<uint32_t>& indices, std::vector<ColorRGB>& colors)
{
  double num_strips = resolution;
  if (num_strips < 0) num_strips = 20.0;
  double r1 = radius1 < 0 ? 1.0 : radius1;
  double r2 = radius2 < 0 ? 1.0 : radius2;

  //generate triangles for the cylinders.
  Vector n((p1 - p2).normal()), u = (10 * n + Vector(10, 10, 10)).normal();
  Vector crx = Cross(u, n).normal();
  u = Cross(crx, n).normal();
  Vector p;
  for (double strips = 0.; strips <= num_strips; strips += 1.)
  {
    uint32_t offset = (uint32_t)numVBOElements;
    p = std::cos(2. * M_PI * strips / num_strips) * u +
      std::sin(2. * M_PI * strips / num_strips) * crx;
    p.normalize();
    points.push_back(r1 * p + Vector(p1));
    colors.push_back(color1);
    numVBOElements++;
    points.push_back(r2 * p + Vector(p2));
    colors.push_back(color2);
    numVBOElements++;
    normals.push_back(p);
    normals.push_back(p);
    indices.push_back(0 + offset);
    indices.push_back(1 + offset);
    indices.push_back(2 + offset);
    indices.push_back(2 + offset);
    indices.push_back(1 + offset);
    indices.push_back(3 + offset);
  }
  for (int jj = 0; jj < 6; jj++) indices.pop_back();
}

void GlyphGeom::generateEllipsoid(const Point& center, double radius1, double radius2,
                                  double resolution, const ColorRGB& color,
                                  int64_t& numVBOElements, std::vector<Vector>& points, 
                                  std::vector<Vector>& normals, std::vector<uint32_t>& indices, 
                                  std::vector<ColorRGB>& colors)
{
  double num_strips = resolution;
  if (num_strips < 0) num_strips = 20.0;
  double r1 = radius1 < 0 ? 1.0 : radius1;
  double r2 = radius2 < 0 ? 1.0 : radius2;
  Vector pp1, pp2;
  double theta_inc = 2. * M_PI / num_strips, phi_inc = M_PI / num_strips;

  //generate triangles for the spheres
  for (double phi = 0.; phi <= M_PI; phi += phi_inc)
  {
    for (double theta = 0.; theta <= 2. * M_PI; theta += theta_inc)
    {
      uint32_t offset = (uint32_t)numVBOElements;
      pp1 = Vector(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
      pp2 = Vector(sin(theta) * cos(phi + phi_inc), sin(theta) * sin(phi + phi_inc), cos(theta));
      points.push_back(r1 * pp1 + Vector(center));
      colors.push_back(color);
      numVBOElements++;
      points.push_back(r2 * pp2 + Vector(center));
      colors.push_back(color);
      numVBOElements++;
      normals.push_back(pp1);
      normals.push_back(pp2);
      indices.push_back(0 + offset);
      indices.push_back(1 + offset);
      indices.push_back(2 + offset);
      indices.push_back(2 + offset);
      indices.push_back(1 + offset);
      indices.push_back(3 + offset);
    }
    for (int jj = 0; jj < 6; jj++) indices.pop_back();
  }
}


// Addarrow from SCIRun 4
void GlyphGeom::addArrow(const Point& center, const Vector& t,
                         double radius, double length, int nu, int nv)
{
  std::vector<QuadStrip> quadstrips;
  double ratio = 2.0;
  Transform trans;
  Transform rotate;
  generateTransforms(center, t, trans, rotate);
  
  Vector offset = rotate * Vector(0,0,1);
  offset.safe_normalize();
  offset *= length * ratio;
  
  generateCylinder(center, t, radius/10.0, radius/10.0, length*ratio, nu, nv, quadstrips);
  generateCylinder(center+offset, t, radius, 0.0, length, nu, nv, quadstrips);
  
  // add strips to the object
}

// from SCIRun 4
void GlyphGeom::addBox(const Point& center, const Vector& t,
                       double x_side, double y_side, double z_side)
{
  std::vector<QuadStrip> quadstrips;
  generateBox(center, t, x_side, y_side, z_side, quadstrips);
  
  // add strips to object
}

// from SCIRun 4
void GlyphGeom::addCylinder(const Point& center, const Vector& t,
                            double radius1, double length, int nu, int nv)
{
  std::vector<QuadStrip> quadstrips;
  generateCylinder(center, t, radius1, radius1, length, nu, nv, quadstrips);
  
  // add the strips to the object
}

// from SCIRun 4
void GlyphGeom::addSphere(const Point& center, double radius,
                          int nu, int nv, int half)
{
  std::vector<QuadStrip> quadstrips;
  generateEllipsoid(center, Vector(0, 0, 1), radius, nu, nv, half, quadstrips);
  
  // add strips to the object

}

// Generate cylinder from SCIRun 4
void GlyphGeom::generateCylinder(const Point& center, const Vector& t, double radius1,
                                 double radius2, double length, int nu, int nv,
                                 std::vector<QuadStrip>& quadstrips)
{
  nu++; //Bring nu to expected value for shape.

  if (nu > 20) nu = 20;
  if (nv == 0) nv = 20;
  SinCosTable& tab1 = tables_[nu];

  Transform trans;
  Transform rotate;
  generateTransforms(center, t, trans, rotate);

  // Draw the cylinder
  double dz = length / (float)nv;
  double dr = (radius2 - radius1) / (float)nv;

  for (int v = 0; v<nv; v++)
  {
    double z1 = dz * (float)v;
    double z2 = z1 + dz;

    double r1 = radius1 + dr * (float)v;
    double r2 = r1 + dr;

    QuadStrip quadstrip;

    for (int u = 0; u<nu; u++)
    {
      double nx = tab1.sin(u);
      double ny = tab1.cos(u);

      double x1 = r1 * nx;
      double y1 = r1 * ny;

      double x2 = r2 * nx;
      double y2 = r2 * ny;

      double nx1 = length * nx;
      double ny1 = length * ny;

      Point p1 = trans * Point(x1, y1, z1);
      Point p2 = trans * Point(x2, y2, z2);

      Vector v1 = rotate * Vector(nx1, ny1, -dr);
      v1.safe_normalize();

      quadstrip.push_back(std::make_pair(p1, v1));
      quadstrip.push_back(std::make_pair(p2, v1));
    }

    quadstrips.push_back(quadstrip);
  }

}

// generate ellipsoid from SCIRun 4
void GlyphGeom::generateEllipsoid(const Point& center, const Vector& t, double scales,
                                  int nu, int nv, int half, std::vector<QuadStrip>& quadstrips)
{
  nu++; //Bring nu to expected value for shape.

  double start = 0, stop =  M_PI;

  // Half ellipsoid criteria.
  if (half == -1) start = M_PI / 2.0;
  if (half == 1) stop = M_PI / 2.0;
  if (half != 0) nv /= 2;

  // Should only happen when doing half ellipsoids.
  if (nv < 2) nv = 2;

  SinCosTable tab1(nu, 0, 2 * M_PI);
  SinCosTable tab2(nv, start, stop);

  Transform trans;
  Transform rotate;
  generateTransforms(center, t, trans, rotate);

  trans.post_scale(Vector(1.0, 1.0, 1.0) * scales);
  rotate.post_scale(Vector(1.0, 1.0, 1.0) / scales);

  // Draw the ellipsoid
  for (int v = 0; v<nv - 1; v++)
  {
    double nr1 = tab2.sin(v + 1);
    double nr2 = tab2.sin(v);

    double nz1 = tab2.cos(v + 1);
    double nz2 = tab2.cos(v);

    QuadStrip quadstrip;

    for (int u = 0; u<nu; u++)
    {
      double nx = tab1.sin(u);
      double ny = tab1.cos(u);

      double x1 = nr1 * nx;
      double y1 = nr1 * ny;
      double z1 = nz1;

      double x2 = nr2 * nx;
      double y2 = nr2 * ny;
      double z2 = nz2;

      Point p1 = trans * Point(x1, y1, z1);
      Point p2 = trans * Point(x2, y2, z2);

      Vector v1 = rotate * Vector(x1, y1, z1);
      Vector v2 = rotate * Vector(x2, y2, z2);

      v1.safe_normalize();
      v2.safe_normalize();

      quadstrip.push_back(std::make_pair(p1, v1));
      quadstrip.push_back(std::make_pair(p2, v2));
    }

    quadstrips.push_back(quadstrip);
  }
}

//generate box from SCIRun 4
void GlyphGeom::generateBox(const Point& center, const Vector& t, double x_side, double y_side,
                            double z_side, std::vector<QuadStrip>& quadstrips)
{
  double half_x_side = x_side * 0.5;
  double half_y_side = y_side * 0.5;
  double half_z_side = z_side * 0.5;

  Transform trans;
  Transform rotate;
  generateTransforms(center, t, trans, rotate);

  //Draw the Box
  Point p1 = trans * Point(-half_x_side, half_y_side, half_z_side);
  Point p2 = trans * Point(-half_x_side, half_y_side, -half_z_side);
  Point p3 = trans * Point(half_x_side, half_y_side, half_z_side);
  Point p4 = trans * Point(half_x_side, half_y_side, -half_z_side);

  Point p5 = trans * Point(-half_x_side, -half_y_side, half_z_side);
  Point p6 = trans * Point(-half_x_side, -half_y_side, -half_z_side);
  Point p7 = trans * Point(half_x_side, -half_y_side, half_z_side);
  Point p8 = trans * Point(half_x_side, -half_y_side, -half_z_side);

  Vector v1 = rotate * Vector(half_x_side, 0, 0);
  Vector v2 = rotate * Vector(0, half_y_side, 0);
  Vector v3 = rotate * Vector(0, 0, half_z_side);

  Vector v4 = rotate * Vector(-half_x_side, 0, 0);
  Vector v5 = rotate * Vector(0, -half_y_side, 0);
  Vector v6 = rotate * Vector(0, 0, -half_z_side);

  QuadStrip quadstrip1;
  QuadStrip quadstrip2;
  QuadStrip quadstrip3;
  QuadStrip quadstrip4;
  QuadStrip quadstrip5;
  QuadStrip quadstrip6;

  // +X
  quadstrip1.push_back(std::make_pair(p7, v1));
  quadstrip1.push_back(std::make_pair(p8, v1));
  quadstrip1.push_back(std::make_pair(p3, v1));
  quadstrip1.push_back(std::make_pair(p4, v1));

  // +Y
  quadstrip2.push_back(std::make_pair(p3, v2));
  quadstrip2.push_back(std::make_pair(p4, v2));
  quadstrip2.push_back(std::make_pair(p1, v2));
  quadstrip2.push_back(std::make_pair(p2, v2));

  // +Z
  quadstrip3.push_back(std::make_pair(p5, v3));
  quadstrip3.push_back(std::make_pair(p7, v3));
  quadstrip3.push_back(std::make_pair(p1, v3));
  quadstrip3.push_back(std::make_pair(p3, v3));

  // -X
  quadstrip4.push_back(std::make_pair(p1, v4));
  quadstrip4.push_back(std::make_pair(p2, v4));
  quadstrip4.push_back(std::make_pair(p5, v4));
  quadstrip4.push_back(std::make_pair(p6, v4));

  // -Y
  quadstrip5.push_back(std::make_pair(p5, v5));
  quadstrip5.push_back(std::make_pair(p6, v5));
  quadstrip5.push_back(std::make_pair(p7, v5));
  quadstrip5.push_back(std::make_pair(p8, v5));

  // -Z
  quadstrip6.push_back(std::make_pair(p2, v6));
  quadstrip6.push_back(std::make_pair(p4, v6));
  quadstrip6.push_back(std::make_pair(p6, v6));
  quadstrip6.push_back(std::make_pair(p8, v6));

  quadstrips.push_back(quadstrip1);
  quadstrips.push_back(quadstrip2);
  quadstrips.push_back(quadstrip3);
  quadstrips.push_back(quadstrip4);
  quadstrips.push_back(quadstrip5);
  quadstrips.push_back(quadstrip6);
}

// from SCIRun 4
void GlyphGeom::generateTransforms(const Point& center, const Vector& normal,
                                   Transform& trans, Transform& rotate)
{
  Vector axis = normal;

  axis.normalize();

  Vector z(0, 0, 1), zrotaxis;
  
  if((Abs(axis.x()) + Abs(axis.y())) < 1.e-5)
  {
    // Only x-z plane...
    zrotaxis = Vector(0, 1, 0);
  }
  else
  {
    zrotaxis = Cross(axis, z);
    zrotaxis.normalize();
  }
  
  double cangle = Dot(z, axis);
  double zrotangle = -acos(cangle);

  rotate.post_rotate(zrotangle, zrotaxis);
}
