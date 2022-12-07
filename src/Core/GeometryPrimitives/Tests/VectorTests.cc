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

#include <Core/GeometryPrimitives/Vector.h>

using namespace SCIRun::Core::Geometry;

TEST(VectorTests, CanDefaultConstruct)
{
  Vector p;
  EXPECT_EQ(0, p.x());
  EXPECT_EQ(0, p.y());
  EXPECT_EQ(0, p.z());
}

TEST(VectorTests, CanScale)
{
  Vector p(1,1,1);
  Vector p2 = p * 2;
  EXPECT_EQ(Vector(2,2,2), p2);
  EXPECT_EQ(Vector(2,2,2), 2 * p);
}

TEST(VectorTests, Norm)
{
  const static auto epsilon = 2e-12;
  Vector p(1,1,1);
  EXPECT_NEAR(std::sqrt(3), p.norm(), epsilon);
}
