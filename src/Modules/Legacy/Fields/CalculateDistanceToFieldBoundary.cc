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

#include <Modules/Legacy/Fields/CalculateDistanceToFieldBoundary.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

#include <Core/Algorithms/Fields/DistanceField/CalculateDistanceField.h>
#include <Core/Algorithms/Fields/MeshDerivatives/GetFieldBoundary.h>

namespace SCIRun {

/// @class CalculateDistanceToFieldBoundary
/// @brief Calculate the distance field to a boundary of a given mesh. 

class CalculateDistanceToFieldBoundary : public Module 
{
  public:
    CalculateDistanceToFieldBoundary(GuiContext*);
    virtual ~CalculateDistanceToFieldBoundary() {}
    virtual void execute();
    
  private:
    GuiInt      gui_truncate_;
    GuiDouble   gui_truncate_distance_;
    GuiString   gui_datatype_;
    GuiString   gui_basistype_;
      
    SCIRunAlgo::CalculateDistanceFieldAlgo algo_;
    SCIRunAlgo::GetFieldBoundaryAlgo falgo_;    
};


DECLARE_MAKER(CalculateDistanceToFieldBoundary)
CalculateDistanceToFieldBoundary::CalculateDistanceToFieldBoundary(GuiContext* ctx)
  : Module("CalculateDistanceToFieldBoundary", ctx, Source, "ChangeFieldData", "SCIRun"),
    gui_truncate_(get_ctx()->subVar("truncate"),0),
    gui_truncate_distance_(get_ctx()->subVar("truncate-distance"),1.0),
    gui_datatype_(get_ctx()->subVar("datatype"),"double"),
    gui_basistype_(get_ctx()->subVar("basistype"),"same as input")
{
  algo_.set_progress_reporter(this);
  falgo_.set_progress_reporter(this);
}


void
CalculateDistanceToFieldBoundary::execute()
{
  // Define handles:
  FieldHandle input,output, value;
  FieldHandle object;

  // Get input from ports:
  get_input_handle("Field",input,true);  

  bool value_connected = oport_connected("ValueField");
  
  // Only do work if it is needed:
  if (inputs_changed_ || !oport_cached("DistanceField") ||
      (!oport_cached("ValueField") && value_connected))
  {
    // Inform module that execution started
    update_state(Executing);

    // Set parameters
    algo_.set_bool("truncate",gui_truncate_.get());
    algo_.set_scalar("truncate_distance",gui_truncate_distance_.get());
    algo_.set_option("datatype",gui_datatype_.get());
    algo_.set_option("basistype",gui_basistype_.get());
        
    if(!(falgo_.run(input,object))) return;    
    
    if (value_connected)
    {
      if(!(algo_.run(input,object,output,value))) return;

      // Send data downstream:
      send_output_handle("DistanceField", output);
      send_output_handle("ValueField", value);
    }
    else
    {
      if(!(algo_.run(input,object,output))) return;

      // Send data downstream:
      send_output_handle("DistanceField", output);
    };
  }
}

} // End namespace SCIRun


#endif
