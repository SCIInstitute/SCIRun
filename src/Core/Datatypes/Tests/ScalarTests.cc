/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;
using namespace SCIRun::Core::Datatypes;

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
