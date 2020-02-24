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


#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromSourceToDestination.h>

#include <Modules/Legacy/Fields/MapFieldDataFromSourceToDestination.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

/// @class MapFieldDataFromSourceToDestination
/// @brief MapFieldDataFromSourceToDestination takes a field and finds data
/// values for the destination geometry and outputs the resulting Field.
///
/// @detail The MapFieldDataFromSourceToDestination module takes two Fields as
/// input, the first of which, Source, contains geometry and data values;
/// the second, Destination, contains geometry only.

MODULE_INFO_DEF(MapFieldDataFromSourceToDestination, ChangeFieldData, SCIRun)

MapFieldDataFromSourceToDestination::MapFieldDataFromSourceToDestination() :
  Module(staticInfo_)
{
  INITIALIZE_PORT(Source);
  INITIALIZE_PORT(Destination);
  INITIALIZE_PORT(Remapped_Destination);
}

void MapFieldDataFromSourceToDestination::setStateDefaults()
{
  setStateDoubleFromAlgo(Parameters::DefaultValue);
  setStateStringFromAlgoOption(Parameters::MappingMethod);
  setStateDoubleFromAlgo(Parameters::MaxDistance);
}

void MapFieldDataFromSourceToDestination::execute()
{
  auto source = getRequiredInput(Source);
  auto destination = getRequiredInput(Destination);

  if (needToExecute())
  {
    setAlgoOptionFromState(Parameters::MappingMethod);
    setAlgoDoubleFromState(Parameters::DefaultValue);
    setAlgoDoubleFromState(Parameters::MaxDistance);

    auto output = algo().run(withInputData((Source, source)(Destination, destination)));
    sendOutputFromAlgorithm(Remapped_Destination, output);
  }
}
