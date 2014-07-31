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

#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromSourceToDestination.h>

#include <Modules/Legacy/Fields/MapFieldDataFromSourceToDestination.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

/// @class MapFieldDataFromSourceToDestination
/// @brief MapFieldDataFromSourceToDestination takes a field and finds data
/// values for the destination geometry and outputs the resulting Field.
///
/// @detail The MapFieldDataFromSourceToDestination module takes two Fields as
/// input, the first of which, Source, contains geometry and data values;
/// the second, Destination, contains geometry only.

  /*
class MapFieldDataFromSourceToDestination : public Module
{
  public:
    MapFieldDataFromSourceToDestination(GuiContext* ctx);
    virtual ~MapFieldDataFromSourceToDestination() {}
    virtual void execute();

  private:
    GuiString  gui_interpolation_basis_;
    GuiInt     gui_map_source_to_single_dest_;
    GuiDouble  gui_exhaustive_search_max_dist_;
    GuiDouble  gui_default_value_;

    SCIRunAlgo::MapFieldDataFromSourceToDestinationAlgo algo_;
};
*/

const ModuleLookupInfo MapFieldDataFromSourceToDestination::staticInfo_("MapFieldDataFromSourceToDestination", "ChangeFieldData", "SCIRun");

MapFieldDataFromSourceToDestination::MapFieldDataFromSourceToDestination() :
  Module(staticInfo_)
//  gui_interpolation_basis_(get_ctx()->subVar("interpolation_basis"), "linear"),
//  gui_map_source_to_single_dest_(get_ctx()->subVar("map_source_to_single_dest"), 0),
//  gui_exhaustive_search_max_dist_(get_ctx()->subVar("exhaustive_search_max_dist"), -1.0),
{
  INITIALIZE_PORT(Source);
  INITIALIZE_PORT(Destination);
  INITIALIZE_PORT(Remapped_Destination);
}

void MapFieldDataFromSourceToDestination::setStateDefaults()
{
  #ifdef SCIRUN4_CODE_TO_BE_IMPLEMENTED_LATER
  setStateDoubleFromAlgo(Parameters::DefaultValue);
  auto state = get_state();
  state->setValue(InterpolationBasis, std::string("linear"));
  setStateDoubleFromAlgo(Parameters::MaxDistance);
  state->setValue(map_source_to_single_dest, false);
  #endif
}

void MapFieldDataFromSourceToDestination::execute()
{
  throw "not implemented";
  auto source = getRequiredInput(Source);
  auto destination = getRequiredInput(Destination);

//  if( inputs_changed_  ||
//      gui_interpolation_basis_.changed() ||
//      gui_map_source_to_single_dest_.changed()  ||
//      gui_exhaustive_search_max_dist_.changed() ||
//      gui_default_value_.changed() ||
//      !oport_cached("Remapped Destination") )
  if (needToExecute())
  {
    update_state(Executing);

    auto state = get_state();
    #ifdef SCIRUN4_CODE_TO_BE_IMPLEMENTED_LATER

    std::string interpolation_basis = state->getValue(InterpolationBasis).getString();
    if (interpolation_basis == "linear")
    {
      algo().set_option(Parameters::MappingMethod, "interpolateddata");
    }
    else
    {
      if (state->getValue(map_source_to_single_dest).getBool())
        algo().set_option(Parameters::MappingMethod, "singledestination");
      else
        algo().set_option(Parameters::MappingMethod, "closestdata");
    }

    setAlgoDoubleFromState(Parameters::DefaultValue);
    setAlgoDoubleFromState(Parameters::MaxDistance);

    auto output = algo().run_generic(make_input((Source, source)(Destination, destination)));
    sendOutputFromAlgorithm(Remapped_Destination, output);
    #endif
  }
}
