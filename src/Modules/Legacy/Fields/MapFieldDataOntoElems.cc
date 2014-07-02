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

//#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoElems.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
//using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

const ModuleLookupInfo MapFieldDataOntoElements::staticInfo_("MapFieldDataOntoElems", "ChangeFieldData", "SCIRun");

/// @class MapFieldDataOntoElems
/// @brief Maps data from one mesh to another mesh. The output mesh will have
/// the data located on the elements. 

/*
class MapFieldDataOntoElems : public Module {
  public:
    MapFieldDataOntoElems(GuiContext*);
    virtual ~MapFieldDataOntoElems() {}

    virtual void execute();
    virtual void post_read();
  
  private:
    GuiString gui_quantity_;
    GuiString gui_value_;
    GuiString gui_sample_points_;
    GuiString gui_sample_method_;
    GuiDouble gui_outside_value_;
    GuiDouble gui_max_distance_;
    
    SCIRunAlgo::MapFieldDataOntoElemsAlgo algo_;
};
*/

MapFieldDataOntoElements::MapFieldDataOntoElements() : Module(staticInfo_)
//   gui_quantity_(get_ctx()->subVar("quantity"),"value"),
//   gui_value_(get_ctx()->subVar("value"),"interpolateddata"),
//   gui_sample_points_(get_ctx()->subVar("sample-points"),"regular2"),
//   gui_sample_method_(get_ctx()->subVar("sample-method"),"average"),
//   gui_outside_value_(get_ctx()->subVar("outside-value"),0.0),
//   gui_max_distance_(get_ctx()->subVar("max-distance"),DBL_MAX)  
{
  INITIALIZE_PORT(Source);
  INITIALIZE_PORT(Weights);
  INITIALIZE_PORT(Destination);
  INITIALIZE_PORT(OutputField);
}

void MapFieldDataOntoElements::setStateDefaults()
{

}

void
MapFieldDataOntoElements::execute()
{
  #if 0
  FieldHandle source, destination, weights, output;
  
  get_input_handle("Source",source,true);
  get_input_handle("Weights",weights,false);
  if(!(get_input_handle("Destination",destination,false)))
  {
    //----------
    // Backwards compatibility:
    // Weights port did not always exist
    if (weights.get_rep()) destination = weights;
    else { error("No destination input field."); return; }
    //----------
  }
  
  if (inputs_changed_ || !oport_cached("Output") ||
    gui_quantity_.changed() || gui_value_.changed() ||
    gui_sample_points_.changed() || gui_sample_method_.changed() ||
    gui_outside_value_.changed() || gui_max_distance_.changed())
  {
    update_state(Executing);

    algo_.set_option("quantity",gui_quantity_.get());
    algo_.set_option("value",gui_value_.get());
    algo_.set_option("sample_points",gui_sample_points_.get());
    algo_.set_option("sample_method",gui_sample_method_.get());
    algo_.set_scalar("outside_value",gui_outside_value_.get());
    algo_.set_scalar("max_distance",gui_max_distance_.get());
  
    if(!(algo_.run(source,weights,destination,output))) return;
    send_output_handle("Output",output,true);
  }
#endif
}
