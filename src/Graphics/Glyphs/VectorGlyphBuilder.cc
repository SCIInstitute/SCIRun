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


#include <Graphics/Glyphs/VectorGlyphBuilder.h>
#include <Graphics/Glyphs/GlyphGeomUtility.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace Graphics;
using namespace Core::Geometry;
using namespace Core::Datatypes;

VectorGlyphBuilder::VectorGlyphBuilder(const Point& p1, const Point& p2)
{
  p1_ = p1;
  p2_ = p2;
}

void VectorGlyphBuilder::setResolution(double resolution)
{
  resolution_ = (resolution < 3) ? 20 : resolution;
}

void VectorGlyphBuilder::setColor1(const ColorRGB& color)
{
  color1_ = color;
}

void VectorGlyphBuilder::setColor2(const ColorRGB& color)
{
  color2_ = color;
}

void VectorGlyphBuilder::setP1(const Point& p)
{
  p1_ = p;
}

void VectorGlyphBuilder::setP2(const Point& p)
{
  p2_ = p;
}

void VectorGlyphBuilder::setShowNormals(bool showNormals)
{
  showNormals_ = showNormals;
}

void VectorGlyphBuilder::setShowNormalsScale(double showNormalsScale)
{
  showNormalsScale_ = showNormalsScale * (p2_ - p1_).length();
}

void VectorGlyphBuilder::generateCylinder(GlyphConstructor& constructor, double radius1, double radius2, bool renderBase1, bool renderBase2)
{
  if (radius1 < 0) radius1 = 1.0;
  if (radius2 < 0) radius2 = 1.0;

  auto prim = Datatypes::SpireIBO::PRIMITIVE::TRIANGLES;
  //generate triangles for the cylinders.
  Vector n((p1_ - p2_).normal());
  Vector crx = n.getArbitraryTangent();
  Vector u = Cross(crx, n).normal();

  int points_per_loop = 2 + renderBase1 + renderBase2;

  // Add center points so flat sides can be drawn
  int p1_index, p2_index;
  if(renderBase1)
  {
    p1_index = constructor.getCurrentIndex(prim);
    constructor.addVertex(prim, Vector(p1_), n, color1_);
  }
  if(renderBase2)
  {
    p2_index = constructor.getCurrentIndex(prim);
    constructor.addVertex(prim, Vector(p2_), -n, color2_);
  }

  // Precalculate
  double length = (p2_-p1_).length();
  double strip_angle = 2. * M_PI / resolution_;

  Vector p;
  auto startOffset = constructor.getCurrentIndex(prim);
  for (int strip = 0; strip <= resolution_; ++strip)
  {
    p = std::cos(strip_angle * strip) * u +
        std::sin(strip_angle * strip) * crx;
    p.normalize();
    Vector normals((length * p + (radius2-radius1)*n).normal());
    auto vert1 = radius1 * p + Vector(p1_);
    auto vert2 = radius2 * p + Vector(p2_);

    constructor.setOffset(prim);
    constructor.addVertex(prim, vert1, normals, color1_);
    constructor.addVertex(prim, vert2, normals, color2_);
    constructor.addIndicesToOffset(prim, 0, 1, points_per_loop);
    constructor.addIndicesToOffset(prim, points_per_loop, 1, points_per_loop + 1);

    if(renderBase1)
    {
      constructor.addVertex(prim, vert1, n, color1_);
      if (showNormals_)
        constructor.addLine(vert1, vert1 + showNormalsScale_ * n, color1_, color1_);
    }
    if(renderBase2)
    {
      constructor.addVertex(prim, vert2, -n, color2_);
      if (showNormals_)
        constructor.addLine(vert2, vert2 - showNormalsScale_ * n, color2_, color2_);
    }
    if (showNormals_)
    {
      auto normScaled = showNormalsScale_ * normals;
      constructor.addLine(vert1, vert1 + normScaled, color1_, color1_);
      constructor.addLine(vert2, vert2 + normScaled, color2_, color2_);
    }
  }
  constructor.popIndicesNTimes(prim, 6);

  for (int strip = 0; strip < resolution_; ++strip)
  {
    int offset = strip * points_per_loop + startOffset;
    if(renderBase1)
      constructor.addIndices(prim, p1_index, offset + 2, offset + points_per_loop + 2);
    if(renderBase2)
      constructor.addIndices(prim, offset + renderBase1 + 2, p2_index,
                              offset + points_per_loop + renderBase1 + 2);
  }
}

void VectorGlyphBuilder::generateComet(GlyphConstructor& constructor, double radius, double sphere_extrusion)
{
  if (radius < 0) radius = 1;
  Vector dir = (p2_-p1_).normal();

  auto prim = Datatypes::SpireIBO::PRIMITIVE::TRIANGLES;
  //Generate triangles for the cone.
  Vector n((p1_ - p2_).normal());
  Vector crx = n.getArbitraryTangent();
  Vector u = Cross(crx, n).normal();
  Point cone_p2 = p2_ - dir * radius * sphere_extrusion * M_PI;
  double cone_radius = radius * cos(sphere_extrusion * M_PI);

  // Center of base
  int points_per_loop = 2;

  // Precalculate
  double length = (p2_-p1_).length();
  double strip_angle = 2. * M_PI / resolution_;

  std::vector<Vector> cone_rim_points;

  Vector p;
  for (int strip = 0; strip <= resolution_; ++strip)
  {
    p = std::cos(strip_angle * strip) * u +
        std::sin(strip_angle * strip) * crx;
    p.normalize();
    Vector normals((length * p + radius * n).normal());

    Vector new_point = cone_radius * p + Vector(cone_p2);
    cone_rim_points.push_back(new_point);

    constructor.setOffset(prim);
    constructor.addVertex(prim, new_point, normals, color1_);
    constructor.addVertex(prim, Vector(p1_), normals, color2_);
    constructor.addIndicesToOffset(prim, 0, points_per_loop, 1);

    if (showNormals_)
    {
      auto normScaled = showNormalsScale_ * normals;
      constructor.addLine(new_point, normScaled + new_point, color1_, color1_);
      constructor.addLine(Vector(p1_), normScaled + Vector(p1_), color2_, color2_);
    }
  }

  // Generate ellipsoid
  Vector tangent = dir.getArbitraryTangent();
  Vector bitangent = Cross(dir, tangent);

  Transform trans, rotate;
  GlyphGeomUtility::generateTransforms(p2_, tangent, bitangent, dir, trans, rotate);

  trans.post_scale ( Vector(1.0,1.0,1.0) * radius );
  rotate.post_scale( Vector(1.0,1.0,1.0) / radius );

  int nu = resolution_ + 1;

  // Half ellipsoid criteria.
  int nv = resolution_ * (0.5 + sphere_extrusion);

  // Should only happen when doing half ellipsoids.
  if (nv < 2) nv = 2;

  double end = M_PI * (0.5 + sphere_extrusion);

  SinCosTable tab1(nu, 0, 2 * M_PI);
  SinCosTable tab2(nv, 0, end);

  int cone_rim_index = 0;

  // Draw the ellipsoid
  for (int v = 0; v<nv - 1; v++)
  {
    double nr1 = tab2.sin(v + 1);
    double nr2 = tab2.sin(v);

    double nz1 = tab2.cos(v + 1);
    double nz2 = tab2.cos(v);

    for (int u = 0; u<nu; u++)
    {
      double nx = tab1.sin(u);
      double ny = tab1.cos(u);

      Point modelVert1 = Point(nr1 * nx, nr1 * ny, nz1);
      Point modelVert2 = Point(nr2 * nx, nr2 * ny, nz2);

      // Rotate and translate points
      Vector vert1 = Vector(trans * modelVert1);
      Vector vert2 = Vector(trans * modelVert2);

      // Rotate norms
      Vector norm1 = (rotate * Vector(modelVert1)).normal();
      Vector norm2 = (rotate * Vector(modelVert2)).normal();

      constructor.setOffset(prim);

      // Use cone points around rim of ellipsoid
      if(v == nv - 2)
        vert1 = cone_rim_points[cone_rim_index++];

      constructor.addVertex(prim, vert1, norm1, color1_);
      constructor.addVertex(prim, vert2, norm2, color1_);
      constructor.addIndicesToOffset(prim, 0, 1, 2);
      constructor.addIndicesToOffset(prim, 2, 1, 3);
      if (showNormals_)
      {
        constructor.addLine(vert1, vert1 + showNormalsScale_ * norm1, color1_, color1_);
        constructor.addLine(vert2, vert2 + showNormalsScale_ * norm2, color2_, color2_);
      }
    }
    constructor.popIndicesNTimes(prim, 6);
  }
}

void VectorGlyphBuilder::generateCone(GlyphConstructor& constructor, double radius, bool renderBase)
{
  const auto prim = Datatypes::SpireIBO::PRIMITIVE::TRIANGLES;
  if (radius < 0) radius = 1;

  //generate triangles for the cylinders.
  Vector n((p1_ - p2_).normal());
  Vector crx = n.getArbitraryTangent();
  Vector u = Cross(crx, n).normal();

  // Center of base
  int points_per_loop = 2;
  size_t base_index = constructor.getCurrentIndex(prim);
  if(renderBase)
  {
    constructor.addVertex(prim, Vector(p1_), n, color1_);
    points_per_loop = 3;
  }

  Vector p;
  double length = (p2_-p1_).length();
  double strip_angle = 2. * M_PI / resolution_;

  for (int strip = 0; strip <= resolution_; ++strip)
  {
    p = std::cos(strip_angle * strip) * u +
        std::sin(strip_angle * strip) * crx;
    p.normalize();
    Vector normals((length * p - radius * n).normal());

    auto offset = constructor.setOffset(prim);
    auto vert1 = radius * p + Vector(p1_);
    constructor.addVertex(prim, vert1, normals, color1_);
    constructor.addVertex(prim, Vector(p2_), normals, color2_);
    constructor.addIndicesToOffset(prim, 0, 1, points_per_loop);

    if(renderBase)
    {
      auto point = radius * p + Vector(p1_);
      constructor.addVertex(prim, point, n, color1_);
      constructor.addIndices(prim, base_index, offset + 2, offset + points_per_loop + 2);
      if (showNormals_)
        constructor.addLine(Vector(point), Vector(point) + showNormalsScale_ * n, color2_, color2_);
    }
    if (showNormals_)
    {
      auto normScaled = showNormalsScale_ * normals;
      constructor.addLine(vert1, vert1 + normScaled, color1_, color1_);
      constructor.addLine(Vector(p2_), Vector(p2_) + normScaled, color2_, color2_);
    }
  }
  constructor.popIndicesNTimes(prim, 3 * (1 + renderBase));
}

void VectorGlyphBuilder::generateTorus(GlyphConstructor& constructor, double major_radius, double minor_radius)
{
  const auto prim = Datatypes::SpireIBO::PRIMITIVE::TRIANGLES;
  int nv = resolution_;
  int nu = nv + 1;

  SinCosTable tab1(nu, 0, 2*M_PI);
  SinCosTable tab2(nv, 0, 2*M_PI, minor_radius);

  Transform trans;
  Transform rotate;
  GlyphGeomUtility::generateTransforms( p1_, (p2_-p1_), trans, rotate );

  // Draw the torus
  for (int v=0; v<nv-1; v++)
  {
    double z1 = tab2.cos(v+1);
    double z2 = tab2.cos(v);

    double nr1 = tab2.sin(v+1);
    double nr2 = tab2.sin(v);

    double r1 = major_radius + nr1;
    double r2 = major_radius + nr2;

    for (int u=0; u<nu; u++)
    {
      double nx = tab1.sin(u);
      double ny = tab1.cos(u);

      double x1 = r1 * nx;
      double y1 = r1 * ny;
      double x2 = r2 * nx;
      double y2 = r2 * ny;

      Vector vert1 = Vector(trans * Point(x1, y1, z1));
      Vector vert2 = Vector(trans * Point(x2, y2, z2));

      Vector norm1 = (rotate * Vector(nr1*nx, nr1*ny, z1)).normal();
      Vector norm2 = (rotate * Vector(nr2*nx, nr2*ny, z2)).normal();

      constructor.setOffset(prim);
      constructor.addVertex(prim, vert1, norm1, color1_);
      constructor.addVertex(prim, vert2, norm2, color1_);
      constructor.addIndicesToOffset(prim, 0, 1, 2);
      constructor.addIndicesToOffset(prim, 2, 1, 3);
      if (showNormals_)
      {
        constructor.addLine(Vector(vert1), Vector(vert1) + showNormalsScale_ * norm1, color1_, color1_);
        constructor.addLine(Vector(vert2), Vector(vert2) + showNormalsScale_ * norm2, color2_, color2_);
      }
    }
  }
  constructor.popIndicesNTimes(prim, 6);
}

void VectorGlyphBuilder::generateArrow(GlyphConstructor& constructor, double radius, double ratio, bool render_cylinder_base, bool render_cone_base)
{
  Point mid((p1_.x() * ratio + p2_.x() * (1 - ratio)),
            (p1_.y() * ratio + p2_.y() * (1 - ratio)),
            (p1_.z() * ratio + p2_.z() * (1 - ratio)));

  auto p2 = p2_;
  setP2(mid);
  generateCylinder(constructor, radius / 6.0, radius / 6.0, render_cylinder_base, false);

  setP1(mid);
  setP2(p2);
  generateCone(constructor, radius, render_cone_base);
}
