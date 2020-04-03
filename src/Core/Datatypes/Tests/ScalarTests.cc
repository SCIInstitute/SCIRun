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
#include <gmock/gmock.h>
#include <Core/Datatypes/Color.h>
#include <complex>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Point.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

TEST(ColorTests, CanParseString)
{
  ColorRGB c(1.0,2.0,3.0);
  const std::string expected = "Color(1,2,3)";
  EXPECT_EQ(expected, c.toString());
  ColorRGB c2(expected);
  EXPECT_EQ(c, c2);
}

TEST(ColorTests, CanParseStringFloating)
{
  ColorRGB c(1.3,2.9,3.00014);
  const std::string expected = "Color(1.3,2.9,3.00014)";
  EXPECT_EQ(expected, c.toString());
  ColorRGB c2(expected);
  EXPECT_EQ(c, c2);
}

TEST(ColorTests, EmptyStringYieldsWhite)
{
  ColorRGB c(1.0,1.0,1.0);
  ColorRGB c2("");
  EXPECT_EQ(c, c2);
}

TEST(ColorTests, CanParseHexValue)
{
  ColorRGB c(0x27213cu);
  ColorRGB c2(39/255.0, 33/255.0, 60/255.0);
  EXPECT_EQ(c, c2);
}

TEST(BasicDatatypeIOTests, AssertSizesOfFieldDatatypesAreConsistent)
{
  ASSERT_EQ(1, sizeof(char));
  ASSERT_EQ(1, sizeof(unsigned char));
  ASSERT_EQ(2, sizeof(short));
  ASSERT_EQ(2, sizeof(unsigned short));
  ASSERT_EQ(4, sizeof(int));
  ASSERT_EQ(4, sizeof(unsigned int));
  ASSERT_EQ(8, sizeof(long long));
  ASSERT_EQ(8, sizeof(unsigned long long));
  ASSERT_EQ(4, sizeof(float));
  ASSERT_EQ(8, sizeof(double));
  ASSERT_EQ(2 * sizeof(double), sizeof(std::complex<double>));
  ASSERT_EQ(3 * sizeof(double), sizeof(Vector));
  ASSERT_EQ(3 * sizeof(double), sizeof(Point));
  ASSERT_EQ(
    3 * 3 * sizeof(double) +  // matrix data
    3 * sizeof(Vector) +      // eigenvectors
    3 * sizeof(double) +      // eigenvalues
    sizeof(int) +             // eigens computed flag (must stay int, not bool)
    4,                        // to get to 8-byte packing
    sizeof(Tensor));
}

TEST(BasicDatatypeIOTests, AssertSizesOfSizeTypesAreConsistent)
{
  ASSERT_EQ(8, sizeof(SCIRun::size_type));
  ASSERT_EQ(8, sizeof(SCIRun::index_type));
}
