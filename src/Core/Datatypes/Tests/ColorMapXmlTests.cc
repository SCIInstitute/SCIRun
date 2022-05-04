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
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Point.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

namespace ColorXml
{
  struct SCISHARE Point
  {
    double x, o, r, g, b;
  };
  struct SCISHARE ColorMap
  {
    std::string name, space;
    std::vector<Point> points;
  };
  struct SCISHARE ColorMaps
  {
    std::vector<ColorMap> maps;
  };
}

TEST(ColorMapXmlTests, CanReadColorMapXmlFromParaview)
{
  FAIL() << "TODO";
  // ColorRGB c(1.0,2.0,3.0);
  // const std::string expected = "Color(1,2,3)";
  // EXPECT_EQ(expected, c.toString());
  // ColorRGB c2(expected);
  // EXPECT_EQ(c, c2);
}
