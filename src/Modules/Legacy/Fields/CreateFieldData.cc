/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


#include <Modules/Legacy/Fields/CreateFieldData.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;

MODULE_INFO_DEF(CreateFieldData, ChangeFieldData, SCIRun)

const AlgorithmParameterName CreateFieldData::BasisString("BasisString");

CreateFieldData::CreateFieldData() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(Function);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(DataArray);
}

void CreateFieldData::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Variables::FunctionString, std::string("RESULT = 1;"));
  state->setValue(Variables::FormatString, std::string("Scalar"));
  state->setValue(BasisString, std::string("Linear"));
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    virtual void presave();

  private:
    GuiString guifunction_;     // function code
    GuiString guiformat_;       // scalar, vector, or tensor ?
    GuiString guibasis_;       // constant, linear, quadratic, ....

    guifunction_(get_ctx()->subVar("function")),
  guiformat_(get_ctx()->subVar("format")),
  guibasis_(get_ctx()->subVar("basis"))
#endif

void
CreateFieldData::execute()
{
  auto field = getRequiredInput(InputField);
  auto func = getOptionalInput(Function);

  if (func && *func)
  {
    get_state()->setValue(Variables::FunctionString, (*func)->value());
  }
  auto matrices = getOptionalDynamicInputs(DataArray);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  TCLInterface::eval(get_id()+" update_text");
#endif

  if (needToExecute())
  {
    // Get number of matrix ports with data (the last one is always empty)
    size_t numinputs = matrices.size();
    if (numinputs > 23)
    {
      error("This module cannot handle more than 23 input matrices.");
      return;
    }
    auto state = get_state();

    NewArrayMathEngine engine;
    engine.setLogger(this);

    std::string format = state->getValue(Variables::FormatString).toString();
    if (format.empty()) format = "double";
    std::string basis = state->getValue(BasisString).toString();
    if (basis.empty()) basis = "Linear";

    // Add as well the output object
    int basis_order = 0;
    if (basis == "Linear") basis_order = 1;

    if(!(engine.add_input_fielddata_location("POS",field,basis_order))) return;

    if(!(engine.add_input_fielddata_coordinates("X","Y","Z",field,basis_order))) return;

    if(!(engine.add_input_fielddata_element("ELEMENT",field,basis_order))) return;

    // Loop through all matrices and add them to the engine as well
    char mname = 'A';
    std::string matrixname("A");

    for (size_t p = 0; p < matrices.size(); p++)
    {
      if (!matrices[p])
      {
        error("No matrix was found on input port.");
        return;
      }

      matrixname[0] = mname++;
      if (!(engine.add_input_matrix(matrixname,matrices[p]))) return;
    }

    if(!(engine.add_output_fielddata("RESULT",field,basis_order,format))) return;

    // Add an object for getting the index and size of the array.

    if(!(engine.add_index("INDEX"))) return;
    if(!(engine.add_size("SIZE"))) return;

    std::string function = state->getValue(Variables::FunctionString).toString();
    if(!(engine.add_expressions(function))) return;

    // Actual engine call, which does the dynamic compilation, the creation of the
    // code for all the objects, as well as inserting the function and looping
    // over every data point

    if (!(engine.run())) return;

    // Get the result from the engine
    FieldHandle ofield;
    engine.get_field("RESULT",ofield);

    // send new output if there is any:
    sendOutput(OutputField, ofield);
  }
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
void
CreateFieldData::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("CreateDataField needs a minor command");
    return;
  }

  if( args[1] == "gethelp" )
  {
    return;
  }
  else
  {
    Module::tcl_command(args, userdata);
  }
}

void
CreateFieldData::presave()
{
  // update gui_function_ before saving.
  TCLInterface::execute(get_id() + " update_text");
}


} // End namespace SCIRun


#endif
