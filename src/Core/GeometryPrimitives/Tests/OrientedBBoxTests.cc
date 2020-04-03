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

#include <Core/Utils/Exception.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/OrientedBBox.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Geometry;

const static double errorOfMargin = 1.0e-10;
const static Vector x_axis = Vector(1,0,0);
const static Vector y_axis = Vector(0,1,0);
const static Vector z_axis = Vector(0,0,1);

TEST(OrientedBBoxTests, Constructor)
{
  OrientedBBox obbox(x_axis, y_axis, z_axis);
  EXPECT_EQ(false, obbox.valid());
  EXPECT_THROW(obbox.diagonal(), InvalidStateException);
}

TEST(OrientedBBoxTests, Extend)
{
  OrientedBBox obbox(x_axis, y_axis, z_axis);
  obbox.extend(Point(1,2,3));
  obbox.extend(Point(4,5,6));

  EXPECT_EQ(Point(1,2,3), obbox.get_min());
  EXPECT_EQ(Point(4,5,6), obbox.get_max());
}

TEST(OrientedBBoxTests, ExtendScalar)
{
  OrientedBBox obbox(x_axis, y_axis, z_axis);
  obbox.extend(Point(0,0,0));
  obbox.extend(2);

  EXPECT_EQ(Point(-2,-2,-2), obbox.get_min());
  EXPECT_EQ(Point(2,2,2), obbox.get_max());
  EXPECT_EQ(Point(0,0,0), obbox.center());
}

TEST(OrientedBBoxTests, MatchesBBox)
{
  OrientedBBox obbox(x_axis, y_axis, z_axis);
  obbox.extend(Point(1,2,3));
  obbox.extend(Point(4,5,6));

  BBox bbox;
  bbox.extend(Point(1,2,3));
  bbox.extend(Point(4,5,6));

  EXPECT_EQ(bbox.get_min(), obbox.get_min());
  EXPECT_EQ(bbox.get_max(), obbox.get_max());
}

TEST(OrientedBBoxTests, Valid)
{
  OrientedBBox obbox(x_axis, y_axis, z_axis);
  EXPECT_EQ(false, obbox.valid());
  obbox.extend(Point(0,0,0));
  EXPECT_EQ(true, obbox.valid());
}

TEST(OrientedBBoxTests, Diagonal)
{
  OrientedBBox obbox(x_axis, y_axis, z_axis);
  obbox.extend(Point(1,0,0));
  obbox.extend(Point(2,1,2));

  EXPECT_EQ(Vector(1,1,2), obbox.diagonal());
}

TEST(OrientedBBoxTests, Center)
{
  OrientedBBox obbox(x_axis, y_axis, z_axis);
  obbox.extend(Point(1,0,0));
  obbox.extend(Point(2,1,2));

  EXPECT_EQ(Point(1.5,0.5,1), obbox.center());
}

TEST(OrientedBBoxTests, ToString)
{
  OrientedBBox obbox(x_axis, y_axis, z_axis);
  obbox.extend(Point(1,0,0));
  obbox.extend(Point(2,1,2));

  std::stringstream ss;
  ss << obbox;
  EXPECT_EQ("[1 0 0] : [2 1 2]", ss.str());
}

TEST(OrientedBBoxTests, OrientatedValues)
{
  double sqrt2 = sqrt(2);
  double sqrt2_half = sqrt2/2;
  OrientedBBox obbox(Vector(sqrt2_half, sqrt2_half, 0), Vector(-sqrt2_half, sqrt2_half, 0), z_axis);
  obbox.extend(Point(1,0,0));
  obbox.extend(Point(2,1,2));

  EXPECT_EQ(Point(sqrt2_half, -sqrt2_half, 0), obbox.get_min());
  EXPECT_EQ(Point(1.5*sqrt2, -sqrt2_half, 2), obbox.get_max());
  auto center = obbox.center();
  EXPECT_NEAR(sqrt2, center.x(), errorOfMargin);
  EXPECT_NEAR(-sqrt2_half, center.y(), errorOfMargin);
  EXPECT_NEAR(1, center.z(), errorOfMargin);
}
