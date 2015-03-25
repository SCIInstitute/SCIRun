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
	
#include <Modules/Legacy/Fields/ConvertFieldDataType.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

// Include the algorithm
#include <Core/Algorithms/Fields/FieldData/ConvertFieldDataType.h>

// The module class
#include <Dataflow/Network/Module.h>

// We need to define the ports used
#include <Dataflow/Network/Ports/FieldPort.h>

namespace SCIRun {

/// @class ConvertFieldDataType
/// @brief ConvertFieldDataType is used to change the type of data associated
/// with the field elements. 

class ConvertFieldDataType : public Module {
  public:
    ConvertFieldDataType(GuiContext* ctx);

    virtual ~ConvertFieldDataType();
    virtual void		execute();

  private:
    SCIRunAlgo::ConvertFieldDataTypeAlgo algo_;

  private:
    GuiString		outputdatatype_;   // the out field type
    GuiString		inputdatatype_;    // the input field type
    GuiString		fldname_;          // the input field name
  
};

DECLARE_MAKER(ConvertFieldDataType)

ConvertFieldDataType::ConvertFieldDataType(GuiContext* ctx)
  : Module("ConvertFieldDataType", ctx, Filter, "ChangeFieldData", "SCIRun"),
    outputdatatype_(get_ctx()->subVar("outputdatatype"), "double"),
    inputdatatype_(get_ctx()->subVar("inputdatatype", false), "---"),
    fldname_(get_ctx()->subVar("fldname", false), "---")
{
  /// Forward errors to the module
  algo_.set_progress_reporter(this);
}

ConvertFieldDataType::~ConvertFieldDataType()
{
  fldname_.set("---");
  inputdatatype_.set("---");
}

void
ConvertFieldDataType::execute()
{
  /// define input/output handles:
  FieldHandle input;
  FieldHandle output;
  get_input_handle("Input Field",input,true);


  // Only do work if needed:
  if (inputs_changed_ || outputdatatype_.changed() || 
      !oport_cached("Output Field"))
  {    
    update_state(Executing);
    /// Set the method to use
    algo_.set_option("datatype",outputdatatype_.get());

    if(!(algo_.run(input,output))) return;
    
    inputdatatype_.set(input->vfield()->get_data_type());

    /// Relay some information to user
    std::string name = input->get_name();
    if (name == "") name = "--- no name ---";
    fldname_.set(name);

    /// send data downstream:
    send_output_handle("Output Field", output, true);    
  }
}

} // End namespace SCIRun
#endif
