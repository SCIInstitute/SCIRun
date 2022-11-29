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
