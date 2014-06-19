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

#include <Modules/Legacy/Fields/ConvertFieldBasis.h>



//#include <Core/Algorithms/Fields/FieldData/ConvertFieldBasisType.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
//using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;


/// @class ConvertFieldBasis
/// @brief ConvertFieldBasis can modify the location of data in the input field.

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
namespace SCIRun {

class ConvertFieldBasis : public Module {
  public:
    ConvertFieldBasis(GuiContext* ctx);
    virtual ~ConvertFieldBasis();
    virtual void execute();
    
  private:
    SCIRunAlgo::ConvertFieldBasisTypeAlgo algo_;    
    
  private:  
    GuiString outputbasis_;    // the out data at
    GuiString inputbasis_;     // the in data at
    GuiString fldname_;         // the field name

};
#endif

ModuleLookupInfo ConvertFieldBasis::staticInfo_("ConvertFieldBasis", "ChangeFieldData", "SCIRun");

ConvertFieldBasis::ConvertFieldBasis()
  : Module(staticInfo_)
    //outputbasis_(get_ctx()->subVar("output-basis"), "Linear"),
    //inputbasis_(get_ctx()->subVar("inputdataat", false), "---"),
    //fldname_(get_ctx()->subVar("fldname", false), "---")
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(Mapping);
}

void ConvertFieldBasis::setStateDefaults()
{

}

void
ConvertFieldBasis::execute()
{
#if 0
  /// Get the input field handle from the port.
  FieldHandle input_field_handle;
  get_input_handle( "Input",  input_field_handle, true );

  bool need_mapping = oport_connected("Mapping");

  // Only do work if needed:
  if (inputs_changed_ || outputbasis_.changed() || 
      !oport_cached("Output") ||
      (need_mapping & !oport_cached("Mapping")))
  {
    update_state(Executing);
    /// Relay some information to user
    std::string name = input_field_handle->get_name();
    if (name == "") name = "--- no name ---";
    fldname_.set(name);

    if (input_field_handle->vfield()->is_nodata()) inputbasis_.set("NoData");
    if (input_field_handle->vfield()->is_constantdata()) inputbasis_.set("ConstantData");
    if (input_field_handle->vfield()->is_lineardata()) inputbasis_.set("LinearData");
    if (input_field_handle->vfield()->is_quadraticdata()) inputbasis_.set("QuadraticData");
    if (input_field_handle->vfield()->is_cubicdata()) inputbasis_.set("CubicData");


    // Set the method to use
    std::string basistype = outputbasis_.get();
    
    // For backwards compatibility
    if (basistype == "None") basistype = "nodata";
    algo_.set_option("basistype",basistype);

    FieldHandle output_field_handle;
    MatrixHandle mapping_matrix_handle;

    if (need_mapping)
    {
      if (!(algo_.run(input_field_handle,output_field_handle,mapping_matrix_handle))) return;
    }
    else
    {
      if (!(algo_.run(input_field_handle,output_field_handle))) return;    
    }
    /// send data downstream:
    send_output_handle("Output", output_field_handle);    
    send_output_handle("Mapping", mapping_matrix_handle);    
  }
#endif
}
