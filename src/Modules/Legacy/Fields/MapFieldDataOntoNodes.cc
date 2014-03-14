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

///TODO Documentation
#include <Core/Algorithms/Fields/Mapping/MapFieldDataOntoNodes.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <float.h>

namespace SCIRun {

using namespace SCIRun;

class MapFieldDataOntoNodes : public Module {
  public:
    MapFieldDataOntoNodes(GuiContext*);
    virtual ~MapFieldDataOntoNodes() {}

    virtual void execute();
    virtual void post_read();

  private:
    GuiString gui_quantity_;
    GuiString gui_value_;
    GuiDouble gui_outside_value_;
    GuiDouble gui_max_distance_;
    
    SCIRunAlgo::MapFieldDataOntoNodesAlgo algo_;    
};


DECLARE_MAKER(MapFieldDataOntoNodes)

MapFieldDataOntoNodes::MapFieldDataOntoNodes(GuiContext* ctx) :
  Module("MapFieldDataOntoNodes", ctx, Source, "ChangeFieldData", "SCIRun"),
  gui_quantity_(get_ctx()->subVar("quantity"),"value"),
  gui_value_(get_ctx()->subVar("value"),"interpolateddata"),
  gui_outside_value_(get_ctx()->subVar("outside-value"),0.0),
  gui_max_distance_(get_ctx()->subVar("max-distance"),DBL_MAX)
{
  algo_.set_progress_reporter(this);
}

void
MapFieldDataOntoNodes::execute()
{
  FieldHandle source, destination, weights, output;
  
  get_input_handle("Source",source,true);
  get_input_handle("Destination",destination,true);
  get_input_handle("Weights",weights,false);
  
  if (inputs_changed_ || !oport_cached("Output") ||
    gui_quantity_.changed() || gui_value_.changed() ||
    gui_outside_value_.changed() || gui_max_distance_.changed())
  {
    update_state(Executing);
    
    algo_.set_option("quantity",gui_quantity_.get());
    algo_.set_option("value",gui_value_.get());
    algo_.set_scalar("outside_value",gui_outside_value_.get());
    algo_.set_scalar("max_distance",gui_max_distance_.get());
  
    if(!(algo_.run(source,weights,destination,output))) return;
    send_output_handle("Output",output,true);
  }
}

void
MapFieldDataOntoNodes::post_read()
{
  std::string old_module_name = get_old_modulename();
  const std::string modName = get_ctx()->getfullname() + "-";
  std::string val;
  
  // Backwards compatibility
  
  if (old_module_name == "MapFieldDataOntoFieldNodes")
  {
    // Convert from ModalMapping module
    if( TCLInterface::get(modName+"mappingmethod", val, get_ctx()) )
    {
      if (val == "ClosestNodalData") val = "closestnodedata";
      TCLInterface::set(modName+"value", val, get_ctx());
    }

    if( TCLInterface::get(modName+"def-value", val, get_ctx()) )
      TCLInterface::set(modName+"outside-value", val, get_ctx());
 
    IPortHandle wport;
    get_iport_handle("Weights",wport);
    
    if (wport->nconnections() == 1)
    {
      // Swap connections
      ConnectionHandle con = wport->connection(0);
      std::string imod = con->imod->get_id();
      std::string omod = con->omod->get_id();
      int iport = con->iport->get_which_port();
      int oport = con->oport->get_which_port();
      
      // Free handle
      con = 0;
      std::string command = "addConnection "+omod+" "+to_string(oport)+" "+imod+" "+to_string(iport+1);
      TCLInterface::eval(command);
      command = "deleteConnection "+omod+" "+to_string(oport)+" "+imod+" "+to_string(iport);
      TCLInterface::eval(command);
    }
  }
}

} // End namespace SCIRun

