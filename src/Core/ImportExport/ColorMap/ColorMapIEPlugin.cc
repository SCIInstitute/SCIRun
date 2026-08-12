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

#include <Core/ImportExport/ColorMap/ColorMapIEPlugin.h>
#include <Core/Datatypes/ColorMap.h>
#include <pugixml/pugixml.hpp>

using namespace SCIRun::ColorXml;
using namespace SCIRun::Core::Datatypes;

ColorMaps ColorMapXmlIO::readColorMapXml(const std::string& filename)
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
    for (const auto& p : cm.children("Point"))
    {
      colorMap.points.push_back({
        p.attribute("x").as_double(),
        p.attribute("o").as_double(),
        p.attribute("r").as_double(),
        p.attribute("g").as_double(),
        p.attribute("b").as_double()}
      );
    }
    colorMaps.maps.push_back(colorMap);
  }
  return colorMaps;
}

ColorMapHandle ColorMapXmlIO::createColorMapFromXmlData(const ColorMap& cmXml)
{
  std::vector<ColorRGB> convertedColors;
  std::transform(cmXml.points.begin(), cmXml.points.end(), std::back_inserter(convertedColors),
    [](const auto& p) { return ColorRGB(p.r, p.g, p.b, p.o); });
  return StandardColorMapFactory::create(convertedColors, cmXml.name);
}

// Note: within namespace ColorXml the unqualified name ColorMap resolves to
// ColorXml::ColorMap, so the SCIRun datatype must be spelled out explicitly.
SCIRun::ColorXml::ColorMap ColorMapXmlIO::createXmlDataFromColorMap(const Core::Datatypes::ColorMap& cm)
{
  ColorMap cmXml;
  cmXml.name = cm.getColorMapName();
  cmXml.space = "RGB";

  const auto colors = cm.getColorData();
  const auto n = colors.size();
  for (size_t i = 0; i < n; ++i)
  {
    const auto& c = colors[i];
    // Position is not retained on the SCIRun ColorMap, so distribute the stops
    // evenly across [0, 1] to match the strict schema readColorMapXml expects.
    const double x = (n > 1) ? static_cast<double>(i) / (n - 1) : 0.0;
    cmXml.points.push_back({x, c.a(), c.r(), c.g(), c.b()});
  }
  return cmXml;
}

bool ColorMapXmlIO::writeColorMapXml(const std::string& filename, const ColorMaps& colorMaps)
{
  pugi::xml_document doc;
  auto root = doc.append_child("ColorMaps");
  for (const auto& cm : colorMaps.maps)
  {
    auto node = root.append_child("ColorMap");
    node.append_attribute("name") = cm.name.c_str();
    node.append_attribute("space") = cm.space.c_str();
    for (const auto& p : cm.points)
    {
      auto point = node.append_child("Point");
      point.append_attribute("x") = p.x;
      point.append_attribute("o") = p.o;
      point.append_attribute("r") = p.r;
      point.append_attribute("g") = p.g;
      point.append_attribute("b") = p.b;
    }
  }
  return doc.save_file(filename.c_str());
}
