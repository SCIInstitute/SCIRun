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

#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldDataToConstantValue.h>

#include <Modules/Legacy/Fields/SetFieldDataToConstantValue.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

const ModuleLookupInfo SetFieldDataToConstantValue::staticInfo_("SetFieldDataToConstantValue", "ChangeFieldData", "SCIRun");

/// @class SetFieldDataToConstantValue
/// @brief Sets field data to a given scalar value on a new output field based
/// on the input field geometry. 
//
//class SetFieldDataToConstantValue : public Module {
//  public:
//    SetFieldDataToConstantValue(GuiContext*);
//    virtual ~SetFieldDataToConstantValue() {}
//
//    virtual void execute();
//
//  private:
//    GuiString gui_data_type_;
//    GuiString gui_basis_order_;
//    GuiDouble gui_value_;
//    
//    SCIRunAlgo::SetFieldDataToConstantValueAlgo algo_;
//};

SetFieldDataToConstantValue::SetFieldDataToConstantValue() :
  Module(staticInfo_)
//     gui_data_type_(ctx->subVar("data-type"),"same as input"),
//     gui_basis_order_(ctx->subVar("basis-order"),"same as input"),
//     gui_value_(ctx->subVar("value"),0.0)
{
}

void
SetFieldDataToConstantValue::execute()
{
  FieldHandle input, output;
  
  get_input_handle("Field",input,true);
  
  if ( inputs_changed_ || gui_data_type_.changed() || 
       gui_basis_order_.changed() || gui_value_.changed() ||
      !oport_cached("Field"))
  {
    update_state(Executing);

    algo_.set_option("data_type",gui_data_type_.get());
    algo_.set_option("basis_order",gui_basis_order_.get());
    algo_.set_scalar("value",gui_value_.get());
    if(!(algo_.run(input,output))) return;
    
    send_output_handle("Field",output,true);
  }

}
