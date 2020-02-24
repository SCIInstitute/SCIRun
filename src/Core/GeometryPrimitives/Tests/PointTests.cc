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


#include <gtest/gtest.h>

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/PointVectorOperators.h>

using namespace SCIRun::Core::Geometry;

TEST(PointTests, CanDefaultConstruct)
{
  Point p;
  EXPECT_EQ(0, p.x());
  EXPECT_EQ(0, p.y());
  EXPECT_EQ(0, p.z());
}

TEST(PointTests, CanSubtractAsVectorDifference)
{
  Point p1(1,2,3);
  Point p2(0,-1,0);

  auto diff = p1 - p2;
  Vector expected(1,3,3);
  EXPECT_EQ(expected, diff);
  std::cout << diff << std::endl;
}

TEST(PointTests, CanAddVector)
{
  Point p1(1,2,3);
  Vector v(3,-1,1);

  p1 += v;

  EXPECT_EQ(Point(4,1,4), p1);
}

TEST(PointTests, CanSubtractVector)
{
  Point p1(1,2,3);
  Vector v(2,-1,1);

  p1 -= v;

  EXPECT_EQ(Point(-1,3,2), p1);
}
