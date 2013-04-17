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


#include <Core/Algorithms/Fields/Mapping/BuildMappingMatrix.h>

#include <Dataflow/GuiInterface/GuiVar.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

namespace SCIRun {

class BuildMappingMatrix : public Module
{
  public:
    BuildMappingMatrix(GuiContext* ctx);
    virtual ~BuildMappingMatrix() {}
    virtual void execute();
   
  private:   
    GuiString  interpolation_basis_;
    GuiInt     map_source_to_single_dest_;
    GuiDouble  exhaustive_search_max_dist_;

    SCIRunAlgo::BuildMappingMatrixAlgo algo_;
    
};

DECLARE_MAKER(BuildMappingMatrix)
BuildMappingMatrix::BuildMappingMatrix(GuiContext* ctx) : 
  Module("BuildMappingMatrix", ctx, Filter, "MiscField", "SCIRun"),
  interpolation_basis_(get_ctx()->subVar("interpolation_basis"), "linear"),
  map_source_to_single_dest_(get_ctx()->subVar("map_source_to_single_dest"), 0),
  exhaustive_search_max_dist_(get_ctx()->subVar("exhaustive_search_max_dist"), -1.0)
{
  algo_.set_progress_reporter(this);
}

void
BuildMappingMatrix::execute()
{
  FieldHandle source, destination;
  MatrixHandle output;
  
  get_input_handle("Source",source,true);
  get_input_handle("Destination", destination,true);
  
  if (inputs_changed_ || interpolation_basis_.changed() ||
      map_source_to_single_dest_.changed() ||
      exhaustive_search_max_dist_.changed() || !oport_cached("Mapping"))
  {
    // Inform module that execution started
    update_state(Executing);
      
    std::string interpolation_basis = interpolation_basis_.get();
    if (interpolation_basis == "linear") 
    {
      algo_.set_option("method","interpolateddata");
    }
    else
    {
      if (map_source_to_single_dest_.get())
        algo_.set_option("method","singledestination");
      else
        algo_.set_option("method","closestdata");
    }
    
    // Core algorithm of the module
    algo_.set_scalar("max_distance",exhaustive_search_max_dist_.get());
    if (!(algo_.run(source,destination,output))) return;
   
    // Send output to output ports
     send_output_handle("Mapping", output);
  }
}

} // End namespace SCIRun
