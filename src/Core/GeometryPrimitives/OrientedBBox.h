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

#ifndef CORE_GEOMETRY_ORIENTEDBBOX_H
#define CORE_GEOMETRY_ORIENTEDBBOX_H

#include <Core/GeometryPrimitives/BBoxBase.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/share.h>

namespace SCIRun {
namespace Core {
namespace Geometry {
class SCISHARE OrientedBBox : public Core::Geometry::BBoxBase
{
public:
  OrientedBBox(const Core::Geometry::Vector &e1, const Core::Geometry::Vector &e2, const Core::Geometry::Vector &e3);
  /// Expand the bounding box to include point p

  Core::Geometry::Point center() const override;
  Core::Geometry::Point get_max() const override;
  Core::Geometry::Point get_min() const override;
  Core::Geometry::Vector diagonal() const override;
  void extend(double val) override;
  void extend(const Core::Geometry::Point &p) override;

private:
  std::vector<Vector> eigvecs_;
};

SCISHARE std::ostream &operator<<(std::ostream &out, const OrientedBBox &b);
SCISHARE void Pio(Piostream &, OrientedBBox &);

}}}

#endif
