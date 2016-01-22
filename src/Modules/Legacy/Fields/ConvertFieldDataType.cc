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
#include <Core/Algorithms/Legacy/Fields/FieldData/ConvertFieldBasisType.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

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

void ConvertFieldDataType::execute()
{
  auto input = getRequiredInput(InputField);

  if (needToExecute())
  {    
    update_state(Executing);
    setAlgoOptionFromState(Parameters::FieldDatatype);

    auto output = algo().run_generic(withInputData((InputField, input)));
    
    auto state = get_state();
    state->setValue(Parameters::InputType, input->vfield()->get_data_type());

    /// Relay some information to user
    std::string name = input->properties().get_name();
    if (name.empty()) 
      name = "--- no name ---";
    state->setValue(Parameters::InputFieldName, name);

    sendOutputFromAlgorithm(OutputField, output);
  }
}
