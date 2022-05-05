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
#include <Core/ImportExport/Nrrd/NrrdIEPlugin.h>
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

    // const bool asMatrix = oport_connected(Matrix);
    // const bool asField = oport_connected(Field);
    // if (asMatrix && asField)
    // {
    //   THROW_ALGORITHM_INPUT_ERROR("Please specify which type of file this is by connecting to only one output port.");
    // }
    //
    // using namespace detail;
    // if (asMatrix)
    // {
    //   const QuickReader<MatrixHandle> matrixReader;
    //   const auto m = matrixReader.read(filename);
    //   sendOutput(Matrix, m);
    // }
    // else if (asField)
    // {
    //   const QuickReader<FieldHandle> fieldReader;
    //   const auto f = fieldReader.read(filename);
    //   sendOutput(Field, f);
    // }
    // else
    // {
    //   remark("No file loaded as no output port was connected.");
    //   return;
    // }

    remark("Loaded file " + filename.string());
  }
}
