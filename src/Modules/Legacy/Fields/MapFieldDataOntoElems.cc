/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Modules/Legacy/Fields/MapFieldDataOntoElems.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoElems.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

const ModuleLookupInfo MapFieldDataOntoElements::staticInfo_("MapFieldDataOntoElems", "ChangeFieldData", "SCIRun");

/// @class MapFieldDataOntoElems
/// @brief Maps data from one mesh to another mesh. The output mesh will have
/// the data located on the elements. 

MapFieldDataOntoElements::MapFieldDataOntoElements() : Module(staticInfo_)
{
  INITIALIZE_PORT(Source);
  INITIALIZE_PORT(Weights);
  INITIALIZE_PORT(Destination);
  INITIALIZE_PORT(OutputField);
}

void MapFieldDataOntoElements::setStateDefaults()
{
  setStateStringFromAlgoOption(Parameters::Quantity);
  setStateStringFromAlgoOption(Parameters::InterpolationModel);
  setStateStringFromAlgoOption(Parameters::SamplePoints);
  setStateStringFromAlgoOption(Parameters::SampleMethod);
  setStateDoubleFromAlgo(Parameters::OutsideValue);
  setStateDoubleFromAlgo(Parameters::MaxDistance);
}

void
MapFieldDataOntoElements::execute()
{
  auto source = getRequiredInput(Source);
  auto weights = getOptionalInput(Weights);

  auto destination = getOptionalInput(Destination);
  if (!destination)
  {
    //----------
    // Backwards compatibility:
    // Weights port did not always exist
    if (weights && *weights) 
      destination = weights;
    else 
    { 
      error("No destination input field."); 
      return;
    }
    //----------
  }
  
//   if (inputs_changed_ || !oport_cached("Output") ||
//     gui_quantity_.changed() || gui_value_.changed() ||
//     gui_sample_points_.changed() || gui_sample_method_.changed() ||
//     gui_outside_value_.changed() || gui_max_distance_.changed())
  if (needToExecute())
  {
    update_state(Executing);

    setAlgoOptionFromState(Parameters::Quantity);
    setAlgoOptionFromState(Parameters::InterpolationModel);
    setAlgoOptionFromState(Parameters::SampleMethod);
    setAlgoOptionFromState(Parameters::SamplePoints);
    setAlgoDoubleFromState(Parameters::OutsideValue);
    setAlgoDoubleFromState(Parameters::MaxDistance);

    auto output = algo().run_generic(make_input((Source, source)(Destination, optionalAlgoInput(destination))(Weights, optionalAlgoInput(weights))));

    sendOutputFromAlgorithm(OutputField, output);
  }
}
