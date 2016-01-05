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
#include <Core/Algorithms/Legacy/Fields/FieldData/ConvertFieldDataType.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace Core::Algorithms::Fields;

/// @class ConvertFieldDataType
/// @brief ConvertFieldDataType is used to change the type of data associated
/// with the field elements. 

const ModuleLookupInfo ConvertFieldDataType::staticInfo_("ConvertFieldDataType", "ChangeFieldData", "SCIRun");

ConvertFieldDataType::ConvertFieldDataType() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
}

void ConvertFieldDataType::setStateDefaults()
{
  setStateStringFromAlgoOption(Parameters::FieldDatatype);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

namespace SCIRun {


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
#endif

void ConvertFieldDataType::execute()
{
  auto input = getRequiredInput(InputField);

  if (needToExecute())
  {    
    update_state(Executing);
    /// Set the method to use
    setAlgoOptionFromState(Parameters::FieldDatatype);

    auto output = algo().run_generic(withInputData((InputField, input)));
    
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    inputdatatype_.set(input->vfield()->get_data_type());

    /// Relay some information to user
    std::string name = input->get_name();
    if (name == "") name = "--- no name ---";
    fldname_.set(name);
#endif

    sendOutputFromAlgorithm(OutputField, output);
  }
}
