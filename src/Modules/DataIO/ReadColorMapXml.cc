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

#include <Modules/DataIO/ReadColorMapXml.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/ImportExport/ColorMap/ColorMapIEPlugin.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Logging/Log.h>
#include <boost/filesystem.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::DataIO;

MODULE_INFO_DEF(ReadColorMapXml, DataIO, SCIRun)

ReadColorMapXml::ReadColorMapXml() : Module(staticInfo_)
{
  INITIALIZE_PORT(Filename);
  INITIALIZE_PORT(FirstColorMap);
  INITIALIZE_PORT(ColorMaps);
}

void ReadColorMapXml::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Variables::Filename, std::string("<load any file>"));
}

void ReadColorMapXml::execute()
{
  auto fileOption = getOptionalInput(Filename);
  if (fileOption && *fileOption)
  {
    get_state()->setValue(Variables::Filename, (*fileOption)->value());
  }

  if (needToExecute())
  {
    const auto filename = get_state()->getValue(Variables::Filename).toFilename();

    if (filename.empty())
    {
      THROW_ALGORITHM_INPUT_ERROR("Empty filename, try again.");
    }

    if (!exists(filename))
    {
      THROW_ALGORITHM_INPUT_ERROR("File does not exist.");
    }

    const auto filenameStr = filename.string();

    remark("Loaded file " + filenameStr);

    auto cmXmls = ColorXml::ColorMapXmlIO::readColorMapXml(filenameStr);
    if (cmXmls.maps.empty())
    {
      THROW_ALGORITHM_INPUT_ERROR("No colormaps found in xml file: " + filenameStr);
    }

    const auto firstColorMap = ColorXml::ColorMapXmlIO::createColorMapFromXmlData(cmXmls.maps[0]);
    sendOutput(FirstColorMap, firstColorMap);

    BundleHandle bundle(new Bundle);
    int i = 0;
    for (const auto& cmXml : cmXmls.maps)
    {
      auto cm = ColorXml::ColorMapXmlIO::createColorMapFromXmlData(cmXml);
      bundle->set("ColorMap #" + std::to_string(++i) + " (" + cm->getColorMapName() + ")", cm);
    }
    sendOutput(ColorMaps, bundle);
  }
}
