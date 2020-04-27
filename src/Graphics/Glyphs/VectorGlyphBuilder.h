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


#ifndef Graphics_Glyphs_VECTOR_GLYPH_BUILDER_H
#define Graphics_Glyphs_VECTOR_GLYPH_BUILDER_H

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Color.h>
#include <Core/Math/TrigTable.h>
#include <Graphics/Glyphs/GlyphConstructor.h>
#include <Graphics/Glyphs/share.h>

namespace SCIRun {
namespace Graphics {
class SCISHARE VectorGlyphBuilder
{
public:
  VectorGlyphBuilder(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2);
  void setResolution(double resolution);
  void setColor1(const Core::Datatypes::ColorRGB& color);
  void setColor2(const Core::Datatypes::ColorRGB& color);
  void setP1(const Core::Geometry::Point& p);
  void setP2(const Core::Geometry::Point& p);
  void generateCylinder(GlyphConstructor& constructor, double radius1, double radius2, bool renderBase1, bool renderBase2);
  void generateComet(GlyphConstructor& constructor, double radius, double sphere_extrusion);
  void generateCone(GlyphConstructor& constructor, double radius, bool renderBase);
  void generateTorus(GlyphConstructor& constructor, double major_radius, double minor_radius);
  void generateArrow(GlyphConstructor& constructor, double radius, double ratio, bool render_cylinder_base, bool render_cone_base);

private:
  Core::Geometry::Point p1_ = {0, 0, 0};
  Core::Geometry::Point p2_ = {0, 0, 0};
  int resolution_ = 3;
  Core::Datatypes::ColorRGB color1_ = {1.0, 1.0, 1.0};
  Core::Datatypes::ColorRGB color2_ = {1.0, 1.0, 1.0};
};
}}

#endif
