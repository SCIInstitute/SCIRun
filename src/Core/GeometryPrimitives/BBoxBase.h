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

#ifndef CORE_GEOMETRY_BBOXBASE_H
#define CORE_GEOMETRY_BBOXBASE_H 1

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/share.h>

namespace SCIRun {
namespace Core {
namespace Geometry {
class SCISHARE BBoxBase
{
protected:
 Point cmin_;
 Point cmax_;
 bool is_valid_;
public:
  explicit BBoxBase(bool valid);
  BBoxBase(bool valid, const Point& cmin, const Point& cmax);
  virtual ~BBoxBase() {}
  inline bool valid() const { return is_valid_; }
  inline void set_valid(bool v) { is_valid_ = v; }
  inline void reset() { is_valid_ = false; }
  virtual void extend(const Point& p) = 0;
  virtual void extend(double val) = 0;
  virtual Vector diagonal() const = 0;
  virtual Point get_min() const = 0;
  virtual Point get_max() const = 0;
  virtual Point center() const = 0;
};
}}}

#endif
