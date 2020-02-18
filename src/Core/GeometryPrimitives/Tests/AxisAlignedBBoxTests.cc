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

#include <Core/GeometryPrimitives/AxisAlignedBBox.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <chrono>
#include <stdlib.h>

using namespace SCIRun::Core::Geometry;

TEST(AxisAlignedBBoxTests, ExtendPerformance)
{
  AxisAlignedBBox aabbox;
  auto start = std::chrono::steady_clock::now();

  for(int i = 0; i < 1000000; ++i)
    aabbox.extend(Point(rand(), rand(), rand()));

  auto end = std::chrono::steady_clock::now();
  auto diff = end - start;
  std::cout << "1,000,000 Extend Calls" << " : " << std::chrono::duration<double, std::milli>(diff).count() << " ms\n";
}

TEST(AxisAlignedBBoxTests, Constructor)
{
  AxisAlignedBBox aabbox;
}

TEST(AxisAlignedBBoxTests, Extend)
{
  AxisAlignedBBox aabbox;

  aabbox.extend(Point(1, 2, 3));
  aabbox.extend(Point(4, 5, 6));

  EXPECT_EQ(Point(1, 2, 3), aabbox.get_min());
  EXPECT_EQ(Point(4, 5, 6), aabbox.get_max());
}

TEST(AxisAlignedBBoxTests, ExtendScalar)
{
  AxisAlignedBBox aabbox;
  aabbox.extend(Point(0, 0, 0));
  aabbox.extend(2);

  EXPECT_EQ(Point(-2, -2, -2), aabbox.get_min());
  EXPECT_EQ(Point(2, 2, 2), aabbox.get_max());
  EXPECT_EQ(Point(0, 0, 0), aabbox.center());
}
