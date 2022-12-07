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


#include <Modules/Legacy/Fields/CalculateFieldData5.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;

/// @brief Calculate new data for the field given a function that uses field
/// data, node location and element properties as input.

/*
  private:
    GuiInt    gui_cache_;
    GuiInt    gui_count_;

  private:
    bool old_version_;   // Is this converted from an old version

    MatrixHandle count_;
    FieldHandle  cache_;
};
*/

MODULE_INFO_DEF(CalculateFieldData, ChangeFieldData, SCIRun)

CalculateFieldData::CalculateFieldData()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(InputFields);
  INITIALIZE_PORT(Function);
  INITIALIZE_PORT(InputArrays);
  INITIALIZE_PORT(OutputField);
}

void CalculateFieldData::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Variables::FunctionString, std::string("RESULT = abs(DATA1);"));
  state->setValue(Variables::FormatString, std::string("Scalar"));
/*
gui_cache_(get_ctx()->subVar("cache"), 0),
gui_count_(get_ctx()->subVar("count", 0), 0),
*/
}

void CalculateFieldData::execute()
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

  auto matrices = getOptionalDynamicInputs(InputArrays);

//   if (inputs_changed_ || guifunction_.changed() ||
//       guiformat_.changed() || !oport_cached("Field"))
  if (needToExecute())
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    // If not caching set the field and count to zero.
    if( !(gui_cache_.get()) )
    {
      cache_ = 0;
      gui_count_.set(0);
      gui_count_.reset();
    }

    if (!cache_)
    {
      // Create a dummy field with one value as default
      FieldInformation fi(field);
      FieldInformation fid("PointCloudMesh",1,fi.get_data_type());
      if (guiformat_.get() != "Same as Input") fid.set_data_type(guiformat_.get());
      cache_ = CreateField(fid);
      cache_->vmesh()->add_point(Point(0,0,0));
      cache_->vfield()->resize_values();
    }

    count_->put(0,0,static_cast<double>(gui_count_.get()));
#endif

    // Get number of matrix ports with data (the last one is always empty)
    size_t numinputs = matrices.size();

    if (numinputs > 23)
    {
      error("This module cannot handle more than 23 input matrices.");
      return;
    }

    NewArrayMathEngine engine;
    engine.setLogger(this);

    auto field = fields[0];
    // Create the DATA object for the function
    // DATA is the data on the field
    if(!(engine.add_input_fielddata("DATA",field))) return;
    if(!(engine.add_input_fielddata("DATA1",field))) return;

    ///-----------------------
    // Backwards compatibility with version 3.0.2
    if(!(engine.add_input_fielddata("v0",field))) return;
    if(!(engine.add_input_fielddata("v",field))) return;
    ///-----------------------

    // Create the POS, X,Y,Z, data location objects.

    if(!(engine.add_input_fielddata_location("POS",field))) return;
    if(!(engine.add_input_fielddata_location("POS1",field))) return;
    if(!(engine.add_input_fielddata_coordinates("X","Y","Z",field))) return;
    if(!(engine.add_input_fielddata_coordinates("X1","Y1","Z1",field))) return;

    ///-----------------------
    // Backwards compatibility with version 3.0.2
    if(!(engine.add_input_fielddata_coordinates("x","y","z",field))) return;
    ///-----------------------

    // Create the ELEMENT object describing element properties
    if(!(engine.add_input_fielddata_element("ELEMENT",field))) return;
    if(!(engine.add_input_fielddata_element("ELEMENT1",field))) return;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    // Caching method
    if(!(engine.add_input_matrix("COUNT",count_))) return;
    if(!(engine.add_input_fielddata("RESULT",cache_))) return;
    ///-----------------------
    // Backwards compatibility with intermediate version between 3.0.2 and 4.0
    if(!(engine.add_input_matrix("count",count_))) return;
    if(!(engine.add_input_fielddata("result",cache_))) return;
    ///-----------------------
#endif

    //TODO: increase this past 5
    if (!addFieldVariableIfPresent(fields, engine, 2))
      return;
    if (!addFieldVariableIfPresent(fields, engine, 3))
      return;
    if (!addFieldVariableIfPresent(fields, engine, 4))
      return;
    if (!addFieldVariableIfPresent(fields, engine, 5))
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

    int basis_order = field->vfield()->basis_order();

    std::string format = state->getValue(Variables::FormatString).toString();
    if (format.empty()) format = "double";

    std::string function = state->getValue(Variables::FunctionString).toString();
    bool has_RESULT = true;
    if (function.find("RESULT") != std::string::npos)
    {
      if(!(engine.add_output_fielddata("RESULT",field,basis_order,format))) return;
    }
    else
    {
      ///-----------------------
      // Backwards compatibility with version 3.0.2
      if(!(engine.add_output_fielddata("result",field,basis_order,format))) return;
      has_RESULT = false;
      ///-----------------------
    }

    // Add an object for getting the index and size of the array.

    if(!(engine.add_index("INDEX"))) return;
    if(!(engine.add_size("SIZE"))) return;


    if(!(engine.add_expressions(function))) return;

    // Actual engine call, which does the dynamic compilation, the creation of the
    // code for all the objects, as well as inserting the function and looping
    // over every data point

    if (!(engine.run()))
    {
      error("Error in parser."); //todo: improve
      return;
    }

    // Get the result from the engine
    FieldHandle ofield;
    if (has_RESULT)
    {
      engine.get_field("RESULT",ofield);
    }
    else
    {
      ///-----------------------
      // Backwards compatibility with version 3.0.2
      engine.get_field("result",ofield);
      ///-----------------------
    }

    // send new output if there is any:
    sendOutput(OutputField, ofield);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if( (gui_cache_.get()) )
    {
      int count = static_cast<int>(count_->get(0,0)); count++;
      gui_count_.set(count);
      gui_count_.reset();
      cache_ = ofield;
    }
    else
    {
      cache_ = 0;
    }
#endif

  }
}

bool CalculateFieldData::addFieldVariableIfPresent(const FieldList& fields, NewArrayMathEngine& engine, int index) const
{
  if (fields.size() >= index)
  {
    auto field = fields[index-1];
    if (field) //this should always be true since dynamic values are filtered for nulls
    {
      auto indexStr = boost::lexical_cast<std::string>(index);
      auto vStr = boost::lexical_cast<std::string>(index - 1);

      if(!engine.add_input_fielddata("DATA" + indexStr, field))
        return false;
      if(!engine.add_input_fielddata("v" + vStr, field))
        return false;

      // Create the POS, X,Y,Z, data location objects.

      if(!engine.add_input_fielddata_location("POS" + indexStr, field))
        return false;
      if(!engine.add_input_fielddata_coordinates("X" + indexStr,"Y" + indexStr,"Z" + indexStr,field))
        return false;

      // Create the ELEMENT object describing element properties
      if(!engine.add_input_fielddata_element("ELEMENT" + indexStr, field))
        return false;
    }
  }
  return true;
}
