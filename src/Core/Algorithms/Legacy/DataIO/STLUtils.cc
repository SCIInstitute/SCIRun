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


#include <Core/Algorithms/Legacy/DataIO/STLUtils.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <boost/unordered_map.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;

std::vector<float> SCIRun::Core::Algorithms::computeFaceNormal(const Point& p1, const Point& p2, const Point& p3)
{
  Vector U = p2 - p1;
  Vector V = p3 - p1;

  std::vector<float> normal(3);
  normal[0] = U.y() * V.z() - U.z() * V.y();
  normal[1] = U.z() * V.x() - U.x() * V.z();
  normal[2] = U.x() * V.y() - U.y() * V.x();
  return normal;
}


std::size_t PointHash::operator()(Geometry::Point const& point) const
{
  std::size_t seed = 0;
  boost::hash_combine( seed, point.x() );
  boost::hash_combine( seed, point.y() );
  boost::hash_combine( seed, point.z() );
  return seed;
}
