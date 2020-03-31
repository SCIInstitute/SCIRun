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


#ifndef Graphics_Glyphs_GLYPH_GEOM_UTILITY_H
#define Graphics_Glyphs_GLYPH_GEOM_UTILITY_H

#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Graphics/Glyphs/share.h>

namespace SCIRun {
namespace Graphics {
class SCISHARE GlyphGeomUtility
{
public:
  static void generateTransforms(const Core::Geometry::Point& center,
                                 const Core::Geometry::Vector& normal,
                                 Core::Geometry::Transform& trans,
                                 Core::Geometry::Transform& rotate);
  static void generateTransforms(const Core::Geometry::Point& center,
                                 const Core::Geometry::Vector& eigvec1,
                                 const Core::Geometry::Vector& eigvec2,
                                 const Core::Geometry::Vector& eigvec3,
                                 Core::Geometry::Transform& translate,
                                 Core::Geometry::Transform& rotate);
  static inline double spow(double e, double x);
};

inline double GlyphGeomUtility::spow(double e, double x)
{
  // This for round off of very small numbers.
  if (std::abs(e) < 1.0e-6)
    e = 0.0;

  if (e < 0.0)
    return std::pow(std::abs(e), x) * -1.0;
  else
    return std::pow(e, x);
}
}}

#endif
