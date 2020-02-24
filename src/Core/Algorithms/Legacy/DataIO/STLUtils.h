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


#ifndef CORE_ALGORITHMS_DATAIO_STLUTILS_H
#define CORE_ALGORITHMS_DATAIO_STLUTILS_H 1

/// TODO: use std::unordered_map when porting to SCIRun 5
#include <boost/unordered_map.hpp>
#include <boost/shared_array.hpp>

#include <functional>
#include <list>

#include <Core/GeometryPrimitives/Point.h>

#include <Core/Algorithms/Legacy/DataIO/share.h>

namespace SCIRun {
namespace Core {
  namespace Algorithms {

struct SCISHARE PointHash : std::unary_function<Geometry::Point, std::size_t>
{
  std::size_t operator()(Geometry::Point const& point) const
  {
    std::size_t seed = 0;
    boost::hash_combine( seed, point.x() );
    boost::hash_combine( seed, point.y() );
    boost::hash_combine( seed, point.z() );
    return seed;
  }
};

struct SCISHARE Facet
{
  Facet(const Geometry::Point& point1,
        const Geometry::Point& point2,
        const Geometry::Point& point3) :
  point1_( point1 ),
  point2_( point2 ),
  point3_( point3 )
  {}

  const Geometry::Point point1_;
  const Geometry::Point point2_;
  const Geometry::Point point3_;
};

// point(vertex) lookup table
typedef boost::unordered_map< Geometry::Point, unsigned int, PointHash > PointTable;
typedef std::list<Facet> FacetList;

/// compute face normal:
///   U = p2 - p1
///   V = p3 - p1
///   Ni = UyVz - UzVy
///   Nj = UzVx - UxVz
///   Nk = UxVy - UyVx
SCISHARE boost::shared_array<float> computeFaceNormal(const Geometry::Point& p1, const Geometry::Point& p2, const Geometry::Point& p3);

}}}

#endif
