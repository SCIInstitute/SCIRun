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


#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Glyphs/VectorGlyphBuilder.h>
#include <Graphics/Glyphs/TensorGlyphBuilder.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace Graphics;
using namespace Datatypes;
using namespace Core::Geometry;
using namespace Core::Datatypes;


GlyphGeom::GlyphGeom()
{
  constructor_ = GlyphConstructor();
}
void GlyphGeom::buildObject(GeometryObjectSpire& geom, const std::string& uniqueNodeID,
                                   const bool isTransparent, const double transparencyValue,
                            const ColorScheme& colorScheme, RenderState state,
                            const SpireIBO::PRIMITIVE& primIn, const BBox& bbox,
                            const bool isClippable, const Core::Datatypes::ColorMapHandle colorMap)
{
  constructor_.buildObject(geom, uniqueNodeID, isTransparent, transparencyValue, colorScheme, state,
                           primIn, bbox, isClippable, colorMap);
}

void GlyphGeom::addArrow(const Point& p1, const Point& p2, double radius, double ratio, int resolution,
                         const ColorRGB& color1, const ColorRGB& color2, bool render_cylinder_base, bool render_cone_base)
{

  VectorGlyphBuilder builder(p1, p2);
  builder.setResolution(resolution);
  builder.setColor1(color1);
  builder.setColor2(color2);
  builder.generateArrow(constructor_, radius, ratio, render_cylinder_base, render_cone_base);
}

void GlyphGeom::addSphere(const Point& p, double radius, int resolution, const ColorRGB& color)
{
  generateSphere(p, radius, resolution, color);
}

void GlyphGeom::addComet(const Point& p1, const Point& p2, double radius, int resolution,
                         const ColorRGB& color1, const ColorRGB& color2, double sphere_extrusion)
{
  VectorGlyphBuilder builder(p1, p2);
  builder.setResolution(resolution);
  builder.setColor1(color1);
  builder.setColor2(color2);
  builder.generateComet(constructor_, radius, sphere_extrusion);
}

void GlyphGeom::addBox(const Point& center, Tensor& t, double scale, ColorRGB& color, bool normalize)
{
  TensorGlyphBuilder builder(t, center);
  builder.setColor(color);
  if (normalize)
    builder.normalizeTensor();
  builder.scaleTensor(scale);
  builder.makeTensorPositive();
  builder.generateBox(constructor_);
}

void GlyphGeom::addEllipsoid(const Point& center, Tensor& t, double scale, int resolution, const ColorRGB& color, bool normalize)
{
  TensorGlyphBuilder builder(t, center);
  builder.setResolution(resolution);
  builder.setColor(color);
  if (normalize)
    builder.normalizeTensor();
  builder.scaleTensor(scale);
  builder.makeTensorPositive();
  builder.generateEllipsoid(constructor_, false);
}

void GlyphGeom::addSuperquadricTensor(const Point& center, Tensor& t, double scale, int resolution,
                                      const ColorRGB& color, bool normalize, double emphasis)
{
  TensorGlyphBuilder builder(t, center);
  builder.setResolution(resolution);
  builder.setColor(color);
  if (normalize)
    builder.normalizeTensor();
  builder.scaleTensor(scale);
  builder.makeTensorPositive();
  builder.generateSuperquadricTensor(constructor_, emphasis);
}

void GlyphGeom::addCylinder(const Point& p1, const Point& p2, double radius, int resolution,
                            const ColorRGB& color1, const ColorRGB& color2,
                            bool renderBase1, bool renderBase2)
{
  VectorGlyphBuilder builder(p1, p2);
  builder.setResolution(resolution);
  builder.setColor1(color1);
  builder.setColor2(color2);
  builder.generateCylinder(constructor_, radius, radius, renderBase1, renderBase2);
}

void GlyphGeom::addCylinder(const Point& p1, const Point& p2, double radius1, double radius2,
                            int resolution, const ColorRGB& color1, const ColorRGB& color2,
                            bool renderBase1, bool renderBase2)
{
  VectorGlyphBuilder builder(p1, p2);
  builder.setResolution(resolution);
  builder.setColor1(color1);
  builder.setColor2(color2);
  builder.generateCylinder(constructor_, radius1, radius2, renderBase1, renderBase2);
}

void GlyphGeom::addDisk(const Point& p1, const Point& p2, double radius, int resolution,
                            const ColorRGB& color1, const ColorRGB& color2)
{
  VectorGlyphBuilder builder(p1, p2);
  builder.setResolution(resolution);
  builder.setColor1(color1);
  builder.setColor2(color2);
  builder.generateCylinder(constructor_, radius, radius, true, true);
}

void GlyphGeom::addTorus(const Point& p1, const Point& p2, double major_radius, double minor_radius, int resolution,
                        const ColorRGB& color1, const ColorRGB& color2)
{
  VectorGlyphBuilder builder(p1, p2);
  builder.setResolution(resolution);
  builder.setColor1(color1);
  builder.generateTorus(constructor_, major_radius, minor_radius);
}

void GlyphGeom::addCone(const Point& p1, const Point& p2, double radius, int resolution,
                        bool render_base, const ColorRGB& color1, const ColorRGB& color2)
{
  VectorGlyphBuilder builder(p1, p2);
  builder.setResolution(resolution);
  builder.setColor1(color1);
  builder.setColor2(color2);
  builder.generateCone(constructor_, radius, render_base);
}

void GlyphGeom::addClippingPlane(const Point& p1, const Point& p2,
  const Point& p3, const Point& p4, double radius, int resolution,
  const ColorRGB& color1, const ColorRGB& color2)
{
  addSphere(p1, radius, resolution, color1);
  addSphere(p2, radius, resolution, color1);
  addSphere(p3, radius, resolution, color1);
  addSphere(p4, radius, resolution, color1);
  addCylinder(p1, p2, radius, resolution, color1, color2);
  addCylinder(p2, p3, radius, resolution, color1, color2);
  addCylinder(p3, p4, radius, resolution, color1, color2);
  addCylinder(p4, p1, radius, resolution, color1, color2);
}

void GlyphGeom::addPlane(const Point& p1, const Point& p2,
  const Point& p3, const Point& p4,
  const ColorRGB& color1)
{
  generatePlane(p1, p2, p3, p4, color1);
}

void GlyphGeom::addLine(const Point& p1, const Point& p2, const ColorRGB& color1, const ColorRGB& color2)
{
  constructor_.addLine(Vector(p1), Vector(p2), color1, color2);
}

void GlyphGeom::addNeedle(const Point& p1, const Point& p2, const ColorRGB& color1, const ColorRGB& color2)
{
  Point mid(0.5 * (p1.x() + p2.x()), 0.5 * (p1.y() + p2.y()), 0.5 * (p1.z() + p2.z()));
  ColorRGB endColor(color2.r(), color2.g(), color2.b(), 0.5);
  constructor_.addLine(Vector(p1), Vector(mid), color1, endColor);
  constructor_.addLine(Vector(mid), Vector(p2), color1, endColor);
}

void GlyphGeom::addPoint(const Point& p, const ColorRGB& color)
{
  constructor_.addPoint(Vector(p), color);
}

void GlyphGeom::generateSphere(const Point& center, double radius, int resolution, const ColorRGB& color)
{
  if (resolution < 3) resolution = 3;
  if (radius < 0) radius = 1.0;
  double theta_inc = M_PI / resolution;
  double phi_inc = 0.5 * M_PI / resolution;

  //generate triangles for the spheres
  for(int v = 1; v <= 2*resolution; ++v)
  {
    double phi = v * phi_inc;
    for(int u = 0; u <= 2*resolution; ++u)
    {
      double theta = u * theta_inc;
      Vector p1 = Vector(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
      Vector p2 = Vector(sin(theta) * cos(phi + phi_inc), sin(theta) * sin(phi + phi_inc), cos(theta));

      constructor_.setOffset();
      constructor_.addVertex(radius * p1 + Vector(center), p1, color);
      constructor_.addVertex(radius * p2 + Vector(center), p2, color);

      int v1 = 1, v2 = 2;
      if(u < resolution)
        std::swap(v1, v2);

      constructor_.addIndicesToOffset(0, v1, v2);
      constructor_.addIndicesToOffset(v2, v1, 3);
    }
    constructor_.popIndicesNTimes(6);
  }
}

void GlyphGeom::generatePlane(const Point& p1, const Point& p2,
  const Point& p3, const Point& p4, const ColorRGB& color)
{
  Vector n1 = Cross(p2 - p1, p4 - p1).normal();
  Vector n2 = Cross(p3 - p2, p1 - p2).normal();
  Vector n3 = Cross(p4 - p3, p2 - p3).normal();
  Vector n4 = Cross(p1 - p4, p3 - p4).normal();

  constructor_.setOffset();

  constructor_.addVertex(Vector(p1), n1, color);
  constructor_.addVertex(Vector(p2), n2, color);
  constructor_.addVertex(Vector(p3), n3, color);
  constructor_.addVertex(Vector(p4), n4, color);

  constructor_.addIndicesToOffset(0, 1, 2);
  constructor_.addIndicesToOffset(2, 3, 0);
}
