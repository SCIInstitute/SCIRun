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


#include <Modules/Legacy/Fields/ClipFieldByFunction3.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

#include <Core/Algorithms/Legacy/Fields/ClipMesh/ClipMeshBySelection.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;

/// @class ClipFieldByFunction3
/// @brief This module selects a subset of one or more (up to three) fields
/// using a function.

MODULE_INFO_DEF(ClipFieldByFunction, NewField, SCIRun)

const AlgorithmParameterName ClipFieldByFunction::FunctionString("FunctionString");

ClipFieldByFunction::ClipFieldByFunction()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(InputFields);
  INITIALIZE_PORT(Function);
  INITIALIZE_PORT(InputArrays);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(Mapping);
}

void ClipFieldByFunction::setStateDefaults()
{
  auto state = get_state();
  state->setValue(FunctionString, std::string("DATA1 < 0;"));
  setStateStringFromAlgoOption(Parameters::ClipMethod);
}

void ClipFieldByFunction::execute()
{
  auto fields = getRequiredDynamicInputs(InputFields);
  auto func = getOptionalInput(Function);
  auto state = get_state();
  auto matrices = getOptionalDynamicInputs(InputArrays);

  if (needToExecute())
  {
    if (func && *func)
    {
      state->setValue(FunctionString, (*func)->value());
    }

    // Get number of matrix ports with data (the last one is always empty)
    size_t numinputs = matrices.size();
    if (numinputs > 23)
    {
      error("This module cannot handle more than 23 input matrices.");
      return;
    }

    auto field = fields[0];
    std::string method;
    if (field->vmesh()->is_pointcloudmesh())
      method = "Element Center";
    else
      method = state->getValue(Parameters::ClipMethod).toString();

    const int basis_order = method == "Element Center" ? 0 : 1;

    if (field->vmesh()->is_empty())
    {
      warning("Input mesh does not contain any nodes: copying input to output");
      sendOutput(OutputField, field);
      sendOutput(Mapping, MatrixHandle());
      return;
    }

    const int field_basis_order = field->vfield()->basis_order();

    NewArrayMathEngine engine;
    engine.setLogger(this);

    // Create the DATA object for the function
    // DATA is the data on the field
    if (basis_order == field_basis_order)
    {
      if(!(engine.add_input_fielddata("DATA",field))) return;
      if(!(engine.add_input_fielddata("DATA1",field))) return;
    }
    else
    {
      std::ostringstream oss;
      oss << "Input field's basis order (";
      if (field_basis_order == 0)
        oss << "constant";
      else if (field_basis_order == 1)
        oss << "linear";
      else
        oss << "other";
      oss << ") does not match basis order supported by method '" << method << "'. Using DATA variables will not be supported.";
      warning(oss.str());
    }

    // Create the POS, X,Y,Z, data location objects.
    if(!(engine.add_input_fielddata_location("POS",field,basis_order))) return;
    if(!(engine.add_input_fielddata_coordinates("X","Y","Z",field,basis_order))) return;

    // Create the ELEMENT object describing element properties
    if(!(engine.add_input_fielddata_element("ELEMENT",field,basis_order))) return;

    //TODO: increase this past 5
    if (!addFieldVariableIfPresent(fields, engine, basis_order, 2))
      return;
    if (!addFieldVariableIfPresent(fields, engine, basis_order, 3))
      return;
    if (!addFieldVariableIfPresent(fields, engine, basis_order, 4))
      return;
    if (!addFieldVariableIfPresent(fields, engine, basis_order, 5))
      return;

    // Loop through all matrices and add them to the engine as well
    char mname = 'A';
    std::string matrixname("A");

    for (size_t p = 0; p < numinputs; p++)
    {
      if (!matrices[p])
      {
        error("No matrix was found on input port.");
        return;
      }

      matrixname[0] = mname++;
      if (!(engine.add_input_matrix(matrixname,matrices[p]))) return;
    }

    if(!(engine.add_output_fielddata("RESULT",field,basis_order,"char"))) return;

    // Add an object for getting the index and size of the array.

    if(!(engine.add_index("INDEX"))) return;
    if(!(engine.add_size("SIZE"))) return;

    // Define the function we are using for clipping:
    std::string function = state->getValue(FunctionString).toString();
    if (function.find("RESULT") == std::string::npos)
      function = "RESULT = " + function;

    if (!engine.add_expressions(function)) return;

    // Actual engine call, which does the dynamic compilation, the creation of the
    // code for all the objects, as well as inserting the function and looping
    // over every data point

    if (!engine.run()) return;

    // Get the result from the engine
    FieldHandle sfield;
    engine.get_field("RESULT",sfield);

    algo().setOption(Parameters::ClipMethod, method);
    auto output = algo().run(withInputData((Variables::InputField, field)(ClipMeshBySelectionAlgo::SelectionField, sfield)));

    sendOutputFromAlgorithm(OutputField, output);
    sendOutputFromAlgorithm(Mapping, output);
  }
}

bool ClipFieldByFunction::addFieldVariableIfPresent(const FieldList& fields, NewArrayMathEngine& engine, int basis_order, int index) const
{
  if (fields.size() >= index)
  {
    auto field = fields[index-1];
    if (field) //this should always be true since dynamic values are filtered for nulls
    {
      auto indexStr = boost::lexical_cast<std::string>(index);
      auto vStr = boost::lexical_cast<std::string>(index - 1);

      int field_basis_order = field->vfield()->basis_order();
      if (basis_order == field_basis_order)
      {
        if(!engine.add_input_fielddata("DATA" + indexStr, field))
          return false;
        if(!engine.add_input_fielddata("v" + vStr, field))
          return false;
      }
      // Create the POS, X,Y,Z, data location objects.

      if(!engine.add_input_fielddata_location("POS" + indexStr, field, basis_order))
        return false;
      if(!engine.add_input_fielddata_coordinates("X" + indexStr,"Y" + indexStr,"Z" + indexStr,field, basis_order))
        return false;

      // Create the ELEMENT object describing element properties
      if(!engine.add_input_fielddata_element("ELEMENT" + indexStr, field, basis_order))
        return false;
    }
  }
  return true;
}
