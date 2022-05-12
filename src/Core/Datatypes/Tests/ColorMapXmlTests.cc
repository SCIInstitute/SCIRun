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
#include <Core/Datatypes/ColorMap.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/ImportExport/ColorMap/ColorMapIEPlugin.h>
#include <pugixml/pugixml.hpp>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;
using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::TestUtils;

std::string example1File()
{
  return (TestResources::rootDir() / "Other/colormaps/All_idl_cmaps.xml").string();
}

ColorXml::ColorMaps example1()
{
  return ColorXml::ColorMapXmlIO::readColorMapXml(example1File());
}

ColorXml::ColorMaps example2()
{
  return ColorXml::ColorMapXmlIO::readColorMapXml((TestResources::rootDir() / "Other/colormaps/All_mpl_cmaps.xml").string());
}


TEST(ColorMapXmlTests, CanReadColorMapXmlFromParaview)
{
  pugi::xml_document doc;
  auto result = doc.load_file(example1File().c_str());
  if (!result)
    FAIL() << "Couldn't read xml file";

  std::vector<std::string> colorMapNames;
  for (const auto& cm : doc.child("ColorMaps"))
  {
    auto name = cm.attribute("name").as_string();
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
  const auto colorMaps = example1();
  EXPECT_EQ(colorMaps.maps.size(), 41);

  const auto cm0 = colorMaps.maps[0];
  EXPECT_EQ(cm0.name, "B-W_LINEAR");
  EXPECT_EQ(cm0.space, "RGB");
  EXPECT_EQ(cm0.points.size(), 256);
  EXPECT_EQ(cm0.points[100].x, -0.215686);
  EXPECT_EQ(cm0.points[100].o, 0.392157);
  EXPECT_EQ(cm0.points[100].r, 0.392157);
  EXPECT_EQ(cm0.points[100].g, 0.392157);
  EXPECT_EQ(cm0.points[100].b, 0.392157);

  const auto cm7 = colorMaps.maps[7];
  EXPECT_EQ(cm7.name, "RED-PURPLE");
  EXPECT_EQ(cm7.space, "RGB");
  EXPECT_EQ(cm7.points.size(), 256);
  EXPECT_EQ(cm7.points[100].x, -0.215686);
  EXPECT_EQ(cm7.points[100].o, 0.392157);
  EXPECT_EQ(cm7.points[100].r, 0.737255);
  EXPECT_EQ(cm7.points[100].g, 0.031373);
  EXPECT_EQ(cm7.points[100].b, 0.286275);

  const auto cm35 = colorMaps.maps[35];
  EXPECT_EQ(cm35.name, "Blue_Waves");
  EXPECT_EQ(cm35.space, "RGB");
  EXPECT_EQ(cm35.points.size(), 256);
  EXPECT_EQ(cm35.points[100].x, -0.215686);
  EXPECT_EQ(cm35.points[100].o, 0.392157);
  EXPECT_EQ(cm35.points[100].r, 0.705882);
  EXPECT_EQ(cm35.points[100].g, 0.2);
  EXPECT_EQ(cm35.points[100].b, 0.133333);

  EXPECT_EQ(example2().maps.size(), 50);
}

TEST(ColorMapXmlTests, CheckXRangeAndSizeOfExamples)
{
  for (const auto& colorMaps : {example1(), example2()})
  {
    for (const auto& colorMap : colorMaps.maps)
    {
      for (const auto& [x,o,r,g,b] : colorMap.points)
      {
        EXPECT_LE(x, 1.0);
        EXPECT_GE(x, -1.0);
        EXPECT_LE(o, 1.0);
        EXPECT_GE(o, 0.0);
        EXPECT_LE(r, 1.0);
        EXPECT_GE(r, 0.0);
        EXPECT_LE(g, 1.0);
        EXPECT_GE(g, 0.0);
        EXPECT_LE(b, 1.0);
        EXPECT_GE(b, 0.0);
      }
      EXPECT_EQ(colorMap.points.size(), 256);
    }
  }
}

TEST(ColorMapXmlTests, CanConvertXmlColorMapToSCIRunColorMap)
{
  const auto cmXmls = example1();
  const auto cmXml = cmXmls.maps[7];

  auto cm = ColorXml::ColorMapXmlIO::createColorMapFromXmlData(cmXml);

  ASSERT_TRUE(cm != nullptr);
  EXPECT_EQ(cm->getColorMapName(), cmXml.name);
  EXPECT_EQ(cm->getColorData()[42].r(), cmXml.points[42].r);
  EXPECT_EQ(cm->getColorData()[42].g(), cmXml.points[42].g);
  EXPECT_EQ(cm->getColorData()[42].b(), cmXml.points[42].b);
  EXPECT_EQ(cm->getColorData()[42].a(), cmXml.points[42].o);
}

TEST(ColorMapXmlTests, CanGenerateQtStyleSheet)
{
  auto cm = StandardColorMapFactory::create("Rainbow", 10);

  ASSERT_TRUE(cm != nullptr);
  EXPECT_EQ(cm->styleSheet().substr(0, 135),
    "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0.001 rgba(0, 0, 255, 255), stop:0.002 rgba(0, 0, 255, 255),"
  );
  EXPECT_EQ(cm->describe(), "Name: Rainbow\nResolution: 10\nInvert: false\nShift: 0\nScale: 0.5\nRescale Shift: 1 & " + cm->styleSheet());
}
