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
#include <pugixml/pugixml.hpp>

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

  ColorMaps readColorMapXml(const std::string& filename)
  {
    pugi::xml_document doc;
    auto result = doc.load_file(filename.c_str());
    if (!result)
      return {};

    ColorMaps colorMaps;
    for (const auto& cm : doc.child("ColorMaps"))
    {
      ColorMap colorMap;
      colorMap.name = cm.attribute("name").as_string();
      colorMap.space = cm.attribute("space").as_string();
      colorMaps.maps.push_back(colorMap);
    }
    return colorMaps;
  }
}


TEST(ColorMapXmlTests, CanReadColorMapXmlFromParaview)
{
  pugi::xml_document doc;
  auto result = doc.load_file("/Users/dan/Downloads/colormaps/All_idl_cmaps.xml");
  if (!result)
    FAIL() << "Couldn't read xml file";

  std::vector<std::string> colorMapNames;
  for (const auto& cm : doc.child("ColorMaps"))
  {
    auto name = cm.attribute("name").as_string();
    //std::cout << "\"" << name << "\",\n";
    colorMapNames.push_back(name);
  }
  std::vector<std::string> expectedColorMapNames =
  {
    "B-W_LINEAR",
    "BLUE-WHITE",
    "GRN-RED-BLU-WHT",
    "RED_TEMPERATURE",
    "BLUE-GREEN-RED-YELLOW",
    "STD_GAMMA-II",
    "PRISM",
    "RED-PURPLE",
    "GREEN-WHITE_LINEAR",
    "GRN-WHT_EXPONENTIAL",
    "GREEN-PINK",
    "BLUE-RED",
    "16_LEVEL",
    "RAINBOW",
    "STEPS",
    "STERN_SPECIAL",
    "Haze",
    "Blue_-_Pastel_-_Red",
    "Pastels",
    "Hue_Sat_Lightness_1",
    "Hue_Sat_Lightness_2",
    "Hue_Sat_Value_1",
    "Hue_Sat_Value_2",
    "Purple-Red_Stripes",
    "Beach",
    "Mac_Style",
    "Eos_A",
    "Eos_B",
    "Hardcandy",
    "Nature",
    "Ocean",
    "Peppermint",
    "Plasma",
    "Blue-Red",
    "Rainbow",
    "Blue_Waves",
    "Volcano",
    "Waves",
    "Rainbow18",
    "Rainbow_white",
    "Rainbow_black"
  };
  EXPECT_EQ(colorMapNames, expectedColorMapNames);
}

TEST(ColorMapXmlTests, CanCreateColorMapXmlData)
{
  auto colorMaps = ColorXml::readColorMapXml("/Users/dan/Downloads/colormaps/All_idl_cmaps.xml");
  EXPECT_EQ(colorMaps.maps.size(), 41);
}
