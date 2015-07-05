/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Dataflow/Network/Module.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>

#include <Core/Algorithms/Fields/DistanceField/CalculateInsideWhichField.h>

namespace SCIRun {

/// @class CalculateInsideWhichField
/// @brief Calculate inside which field of a number of given fields a certain
/// element is located. 

class CalculateInsideWhichField : public Module 
{
  public:
    CalculateInsideWhichField(GuiContext*);
    virtual ~CalculateInsideWhichField() {}
    virtual void execute();
    
  private:
    GuiString gui_sampling_scheme_;
    GuiDouble gui_outside_value_;
    GuiDouble gui_start_value_;
    GuiString gui_method_;
    GuiString gui_change_outside_values_;
    GuiString gui_output_type_;  
    GuiString gui_datalocation_;
    
    SCIRunAlgo::CalculateInsideWhichFieldAlgo algo_;
};


DECLARE_MAKER(CalculateInsideWhichField)
CalculateInsideWhichField::CalculateInsideWhichField(GuiContext* ctx)
  : Module("CalculateInsideWhichField", ctx, Source, "ChangeFieldData", "SCIRun"),
    gui_sampling_scheme_(ctx->subVar("sampling-scheme"),"regular2"),
    gui_outside_value_(ctx->subVar("outside-value"),0.0),
    gui_start_value_(ctx->subVar("start-value"),1.0),
    gui_method_(ctx->subVar("method"),"one"),
    gui_change_outside_values_(ctx->subVar("change-output-values"),"true"), 
    gui_output_type_(ctx->subVar("outputtype"),"same as input"),
    gui_datalocation_(ctx->subVar("datalocation"),"elem")
{
  algo_.set_progress_reporter(this);
}


void CalculateInsideWhichField::execute()
{
  SCIRun::FieldHandle input, output;
  std::vector<SCIRun::FieldHandle> objectfields;
  
  get_input_handle("Field",input,true);
  get_dynamic_input_handles("Object",objectfields,true);

  if (objectfields.size() == 0) return;
  
  if (inputs_changed_ || gui_output_type_.changed() || 
      gui_sampling_scheme_.changed() ||
      gui_outside_value_.changed() ||
      gui_start_value_.changed() ||
      gui_method_.changed() ||
      gui_change_outside_values_.changed() || 
      gui_datalocation_.changed() || 
      !oport_cached("Field"))
  {
    update_state(Executing);
    
    algo_.set_option("sampling_scheme",gui_sampling_scheme_.get());
    algo_.set_scalar("outside_value",gui_outside_value_.get());
    algo_.set_scalar("start_value",gui_start_value_.get());
    algo_.set_option("method",gui_method_.get());
    algo_.set_bool("change_outside_values",(gui_change_outside_values_.get()=="true"));
    algo_.set_option("output_type",gui_output_type_.get());
    algo_.set_option("data_location",gui_datalocation_.get());

    if(!(algo_.run(input,objectfields,output))) return; 
    send_output_handle("Field", output);
  }
}

} // End namespace SCIRun

