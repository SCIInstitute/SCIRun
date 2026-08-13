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

#include <Modules/DataIO/WriteColorMapXml.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/ImportExport/ColorMap/ColorMapIEPlugin.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::DataIO;

MODULE_INFO_DEF(WriteColorMapXml, DataIO, SCIRun)

WriteColorMapXml::WriteColorMapXml() : Module(staticInfo_)
{
  INITIALIZE_PORT(ColorMapToWrite);
  INITIALIZE_PORT(ColorMaps);
  INITIALIZE_PORT(Filename);
}

void WriteColorMapXml::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Variables::Filename, std::string("<save file>"));
}

void WriteColorMapXml::execute()
{
  auto fileOption = getOptionalInput(Filename);
  if (fileOption && *fileOption)
  {
    get_state()->setValue(Variables::Filename, (*fileOption)->value());
  }

  auto singleColorMap = getOptionalInput(ColorMapToWrite);
  auto bundleOption = getOptionalInput(ColorMaps);

  if (needToExecute())
  {
    const auto filename = get_state()->getValue(Variables::Filename).toFilename();

    if (filename.empty())
    {
      THROW_ALGORITHM_INPUT_ERROR("Empty filename, try again.");
    }

    // Collect the colormaps to serialize: everything in the bundle (if any) plus
    // the single colormap input (if connected).
    ColorXml::ColorMaps colorMaps;
    if (bundleOption && *bundleOption)
    {
      for (const auto& cm : (*bundleOption)->getColorMaps())
      {
        if (cm)
          colorMaps.maps.push_back(ColorXml::ColorMapXmlIO::createXmlDataFromColorMap(*cm));
      }
    }
    if (singleColorMap && *singleColorMap)
    {
      colorMaps.maps.push_back(ColorXml::ColorMapXmlIO::createXmlDataFromColorMap(**singleColorMap));
    }

    if (colorMaps.maps.empty())
    {
      THROW_ALGORITHM_INPUT_ERROR("No colormap connected to write.");
    }

    const auto filenameStr = filename.string();
    if (!ColorXml::ColorMapXmlIO::writeColorMapXml(filenameStr, colorMaps))
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Failed to write colormap xml file: " + filenameStr);
    }

    remark("Wrote file " + filenameStr);
  }
}
