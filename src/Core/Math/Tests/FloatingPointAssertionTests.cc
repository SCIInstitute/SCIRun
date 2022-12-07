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
#include <boost/test/floating_point_comparison.hpp>
#include <Core/Math/MiscMath.h>

using namespace SCIRun;

TEST(FloatComparisonTest, BoostCheck)
{
#if BOOST_VERSION >= 106700
	namespace btt = boost::math::fpc;
#else
	namespace btt = boost::test_tools;
#endif

  btt::close_at_tolerance<double> comp(btt::percent_tolerance(1e-5));
  EXPECT_TRUE(comp(1, 1));
  EXPECT_FALSE(comp(1.0/3, 0.33333));
  EXPECT_TRUE(comp(1.0/3, 0.333333333));
}

TEST(InfinityTests, IsFiniteTest)
{
  EXPECT_FALSE(IsFinite(std::numeric_limits<double>::infinity()));
  EXPECT_FALSE(IsFinite(-std::numeric_limits<double>::infinity()));
  EXPECT_FALSE(IsFinite(std::numeric_limits<double>::quiet_NaN()));
  EXPECT_FALSE(IsFinite(std::numeric_limits<double>::signaling_NaN()));
  EXPECT_TRUE(IsFinite(0.3));
}

TEST(InfinityTests, IsInfiniteTest)
{
  EXPECT_TRUE(IsInfinite(std::numeric_limits<double>::infinity()));
  EXPECT_TRUE(IsInfinite(-std::numeric_limits<double>::infinity()));
  EXPECT_FALSE(IsInfinite(std::numeric_limits<double>::quiet_NaN()));
  EXPECT_FALSE(IsInfinite(std::numeric_limits<double>::signaling_NaN()));
  EXPECT_FALSE(IsInfinite(0.3));
}

TEST(NanTests, IsNanTest)
{
  EXPECT_FALSE(IsNan(std::numeric_limits<double>::infinity()));
  EXPECT_FALSE(IsNan(-std::numeric_limits<double>::infinity()));
  EXPECT_TRUE(IsNan(std::numeric_limits<double>::quiet_NaN()));
  EXPECT_TRUE(IsNan(std::numeric_limits<double>::signaling_NaN()));
  EXPECT_FALSE(IsNan(0.3));
}
