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

#ifndef CORE_GEOMETRY_POINTVECTOROPERATORS_H
#define CORE_GEOMETRY_POINTVECTOROPERATORS_H

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Share.h>

namespace SCIRun {
namespace Core {
namespace Geometry {

  inline Vector operator-(const Point& p1, const Point& p2)
  {
    return Vector(p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2]);
  }

  inline Point& Point::operator+=(const Vector& v)
  {
    d_[0]+=v.d_[0];
    d_[1]+=v.d_[1];
    d_[2]+=v.d_[2];
    return *this;
  }

  inline Point& Point::operator-=(const Vector& v)
  {
    d_[0]-=v.d_[0];
    d_[1]-=v.d_[1];
    d_[2]-=v.d_[2];
    return *this;
  }

}}}


#endif
