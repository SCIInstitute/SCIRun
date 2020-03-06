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


#include <Graphics/Glyphs/GlyphGeomUtility.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace Graphics;
using namespace Core::Geometry;

void GlyphGeomUtility::generateTransforms(const Point& center, const Vector& normal,
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

  trans.pre_translate((Vector) center);
  trans.post_rotate(zrotangle, zrotaxis);

  rotate.post_rotate(zrotangle, zrotaxis);
}

void GlyphGeomUtility::generateTransforms(const Point& center, const Vector& eigvec1,
                                          const Vector& eigvec2, const Vector& eigvec3,
                                          Transform& translate, Transform& rotate)
{
  static const Point origin(0.0, 0.0, 0.0);
  rotate = Transform(origin, eigvec1, eigvec2, eigvec3);
  translate = rotate;
  translate.pre_translate((Vector) center);
}

