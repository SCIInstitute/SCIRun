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

#include <Modules/Legacy/Fields/CalculateFieldDataMetric.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

/// @class CalculateFieldDataMetric
/// @brief Reads in a field file(s), allows parser edits to the data, and
/// exports 1 of 7 metrics selected by the user as a matrix file.
/*
class CalculateFieldDataMetric : public Module {
  public:
    CalculateFieldDataMetric(GuiContext*);
    virtual ~CalculateFieldDataMetric() {}

    virtual void execute();
    virtual void presave();

  private:
    GuiString guimethod_;
    GuiString guifunction_;         // function code
    GuiInt    guienable_function_;  // Whether to transform data
    GuiString guimetric_;
    SCIRunAlgo::CalculateFieldDataMetricAlgo algo_;

};
*/

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;

MODULE_INFO_DEF(CalculateFieldDataMetric, ChangeFieldData, SCIRun)
ALGORITHM_PARAMETER_DEF(Fields, EnableFunction)

CalculateFieldDataMetric::CalculateFieldDataMetric()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(InputFields);
  INITIALIZE_PORT(Function);
  INITIALIZE_PORT(Metric);
}

void CalculateFieldDataMetric::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Variables::FunctionString, std::string("RESULT = DATA;"));
  state->setValue(Parameters::EnableFunction, false);
  setStateStringFromAlgoOption(Variables::Method);
}

void
CalculateFieldDataMetric::execute()
{
  auto fields = getRequiredDynamicInputs(InputFields);
  auto func = getOptionalInput(Function);
  auto state = get_state();
  if (func)
  {
    if (*func)
    {
      get_state()->setValue(Variables::FunctionString, (*func)->value());
    }
  }

  if (needToExecute())
  {
    std::vector<FieldHandle> modifiedFields;
    // If not caching set the field and count to zero.

    if (state->getValue(Parameters::EnableFunction).toBool())
    {
      for (auto& field : fields)
      {
        if (field && field->vmesh()->num_nodes() > 0)
        {
          NewArrayMathEngine engine;
          engine.setLogger(this);

          // Create the DATA object for the function
          // DATA is the data on the field
          if(!(engine.add_input_fielddata("DATA",field))) return;

          // Create the POS, X,Y,Z, data location objects.

          if(!(engine.add_input_fielddata_location("POS",field))) return;
          if(!(engine.add_input_fielddata_coordinates("X","Y","Z",field))) return;


          // Create the ELEMENT object describing element properties
          if(!(engine.add_input_fielddata_element("ELEMENT",field))) return;

          int basis_order = field->vfield()->basis_order();

          std::string function = state->getValue(Variables::FunctionString).toString();

          if(!(engine.add_output_fielddata("RESULT",field,basis_order,"double"))) return;

          // Add an object for getting the index and size of the array.

          if(!(engine.add_index("INDEX"))) return;
          if(!(engine.add_size("SIZE"))) return;

          if(!(engine.add_expressions(function))) return;

          // Actual engine call, which does the dynamic compilation, the creation of the
          // code for all the objects, as well as inserting the function and looping
          // over every data point

          if (!engine.run())
          {
            error("Error in parser."); //todo: improve
            return;
          }
          // Get the result from the engine

          FieldHandle modified;
          engine.get_field("RESULT", modified);
          modifiedFields.push_back(modified);
        }
      }
    }
    else
    {
      modifiedFields = fields;
    }

    setAlgoOptionFromState(Variables::Method);
    auto output = algo().run(withInputData((InputFields, modifiedFields)));
    auto metric = output.get<DenseMatrix>(Core::Algorithms::AlgorithmParameterName(Variables::OutputMatrix));

    //TODO later
    //guimetric_.set(to_string(metric));

    sendOutput(Metric, metric);
  }
}
