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
/// @todo Documentation Modules/Legacy/Fields/MapFieldDataOntoElems.cc

#include <Core/Algorithms/Fields/Mapping/MapFieldDataOntoElems.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <float.h>

namespace SCIRun {

using namespace SCIRun;

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


DECLARE_MAKER(MapFieldDataOntoElems)

MapFieldDataOntoElems::MapFieldDataOntoElems(GuiContext* ctx) :
  Module("MapFieldDataOntoElems", ctx, Source, "ChangeFieldData", "SCIRun"),
  gui_quantity_(get_ctx()->subVar("quantity"),"value"),
  gui_value_(get_ctx()->subVar("value"),"interpolateddata"),
  gui_sample_points_(get_ctx()->subVar("sample-points"),"regular2"),
  gui_sample_method_(get_ctx()->subVar("sample-method"),"average"),
  gui_outside_value_(get_ctx()->subVar("outside-value"),0.0),
  gui_max_distance_(get_ctx()->subVar("max-distance"),DBL_MAX)  
{
  algo_.set_progress_reporter(this);
}

void
MapFieldDataOntoElems::execute()
{
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
}

void
MapFieldDataOntoElems::post_read()
{
  std::string old_module_name = get_old_modulename();
  const std::string modName = get_ctx()->getfullname() + "-";
  std::string val;
  
  // Backwards compatibility
  
  if (old_module_name == "ModalMapping")
  {
    // Convert from ModalMapping module
    if( TCLInterface::get(modName+"mappingmethod", val, get_ctx()) )
    {
      if (val == "ClosestNodalData") val = "closestnodedata";
      TCLInterface::set(modName+"value", val, get_ctx());
    }
    if( TCLInterface::get(modName+"integrationmethod", val, get_ctx()) )
      TCLInterface::set(modName+"sample-points", val, get_ctx());
      
    if( TCLInterface::get(modName+"integrationfilter", val, get_ctx()) )
    {
      // This option no longer exists
      if (val == "WeightedAverage") val = "average";
      if (val == "maximum" || val == "Maximum" ) val = "max";
      if (val == "minimum" || val == "Minimum") val = "min";

      TCLInterface::set(modName+"sample-method", val, get_ctx());
    }
    if( TCLInterface::get(modName+"def-value", val, get_ctx()) )
      TCLInterface::set(modName+"outside-value", val, get_ctx());
 
    FieldIPortHandle wport;
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

  if (old_module_name == "MapFieldDataGradientOntoField")
  {
    // Convert from ModalMapping module
    if( TCLInterface::get(modName+"mappingmethod", val, get_ctx()) )
    {
      if (val == "ClosestNodalData") val = "closestnodedata";
      TCLInterface::set(modName+"value", val, get_ctx());
    }
    if( TCLInterface::get(modName+"integrationmethod", val, get_ctx()) )
    {
      TCLInterface::set(modName+"sample-points", val, get_ctx());
    }
    if( TCLInterface::get(modName+"integrationfilter", val, get_ctx()) )
    {
      // This option no longer exists
      if (val == "WeightedAverage") val = "average";
      if (val == "maximum" || val == "Maximum" ) val = "max";
      if (val == "minimum" || val == "Minimum") val = "min";

      TCLInterface::set(modName+"sample-method", val, get_ctx());
    }
    if( TCLInterface::get(modName+"def-value", val, get_ctx()) )
    {
      TCLInterface::set(modName+"outside-value", val, get_ctx());
    }
    
    TCLInterface::set(modName+"quantity","gradient",get_ctx());
    if( TCLInterface::get(modName+"calcnorm", val, get_ctx()) )
    {
      if (val == "1") TCLInterface::set(modName+"quantity","gradientnorm",get_ctx());
    }
    
    FieldIPortHandle wport;
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


  if (old_module_name == "MapCurrentDensityOntoField")
  {
    // Convert from ModalMapping module
    if( TCLInterface::get(modName+"mappingmethod", val, get_ctx()) )
    {
      if (val == "ClosestNodalData") val = "closestnodedata";
      TCLInterface::set(modName+"value", val, get_ctx());
    }
    if( TCLInterface::get(modName+"integrationmethod", val, get_ctx()) )
    {
      TCLInterface::set(modName+"sample-points", val, get_ctx());
    }
    if( TCLInterface::get(modName+"integrationfilter", val, get_ctx()) )
    {
      // This option no longer exists
      if (val == "WeightedAverage") val = "average";
      if (val == "maximum" || val == "Maximum" ) val = "max";
      if (val == "minimum" || val == "Minimum") val = "min";
      
      TCLInterface::set(modName+"sample-method", val, get_ctx());
    }
    if( TCLInterface::get(modName+"def-value", val, get_ctx()) )
    {
      TCLInterface::set(modName+"outside-value", val, get_ctx());
    }
    
    TCLInterface::set(modName+"value","gradient",get_ctx());
    if( TCLInterface::get(modName+"calcnorm", val, get_ctx()) )
    {
      if (val == "1") TCLInterface::set(modName+"quantity","gradientnorm",get_ctx());
    }
    if( TCLInterface::get(modName+"multiply-with-normal", val, get_ctx()) )
    {
      if (val == "1") TCLInterface::set(modName+"quantity","flux",get_ctx());
    }
  }
}


} // End namespace SCIRun


