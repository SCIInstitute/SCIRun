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
/// @todo Documentation Modules/Legacy/Fields/MapFieldDataFromSourceToDestination.cc

#include <Core/Algorithms/Fields/Mapping/MapFieldDataFromSourceToDestination.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {


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


DECLARE_MAKER(MapFieldDataFromSourceToDestination)

MapFieldDataFromSourceToDestination::MapFieldDataFromSourceToDestination(GuiContext* ctx) : 
  Module("MapFieldDataFromSourceToDestination", ctx, Filter, "ChangeFieldData", "SCIRun"),
  gui_interpolation_basis_(get_ctx()->subVar("interpolation_basis"), "linear"),
  gui_map_source_to_single_dest_(get_ctx()->subVar("map_source_to_single_dest"), 0),
  gui_exhaustive_search_max_dist_(get_ctx()->subVar("exhaustive_search_max_dist"), -1.0),
  gui_default_value_(get_ctx()->subVar("default-value"),0.0)
{
}


void
MapFieldDataFromSourceToDestination::execute()
{
  FieldHandle source,destination,output;

  get_input_handle( "Source",source, true );
  get_input_handle( "Destination", destination, true );

  if( inputs_changed_  ||
      gui_interpolation_basis_.changed() ||
      gui_map_source_to_single_dest_.changed()  ||
      gui_exhaustive_search_max_dist_.changed() ||
      gui_default_value_.changed() ||
      !oport_cached("Remapped Destination") ) 
  {
    update_state(Executing);
    
    std::string interpolation_basis = gui_interpolation_basis_.get();
    if (interpolation_basis == "linear") 
    {
      algo_.set_option("method","interpolateddata");
    }
    else
    {
      if (gui_map_source_to_single_dest_.get())
        algo_.set_option("method","singledestination");
      else
        algo_.set_option("method","closestdata");
    }
    
    algo_.set_scalar("default_value",gui_default_value_.get());
    algo_.set_scalar("max_distance",gui_exhaustive_search_max_dist_.get());
    algo_.run(source,destination,output);
    
    // Send the data downstream
    send_output_handle("Remapped Destination", output);
  }
}

} // End namespace SCIRun
