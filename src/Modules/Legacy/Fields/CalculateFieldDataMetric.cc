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


#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Algorithms/Fields/FieldData/CalculateFieldDataMetric.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/StringPort.h>

namespace SCIRun {

/// @class CalculateFieldDataMetric
/// @brief Reads in a field file(s), allows parser edits to the data, and
/// exports 1 of 7 metrics selected by the user as a matrix file.

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


DECLARE_MAKER(CalculateFieldDataMetric)
CalculateFieldDataMetric::CalculateFieldDataMetric(GuiContext* ctx)
  : Module("CalculateFieldDataMetric", ctx, Source, "ChangeFieldData", "SCIRun"),
  guimethod_(get_ctx()->subVar("method"),"value-mean"),
  guifunction_(get_ctx()->subVar("function"),"RESULT = DATA;"),
  guienable_function_(get_ctx()->subVar("enable-function"),0),
  guimetric_(get_ctx()->subVar("metric",false),"---")
{
  algo_.set_progress_reporter(this);
}

void
CalculateFieldDataMetric::execute()
{
  // Define local handles of data objects:
  StringHandle func;
  std::vector<FieldHandle> fields;

  // Get the new input data:
  if (get_input_handle("Function",func,false))
  {
    if (func.get_rep())
    {
      guifunction_.set(func->get());
      get_ctx()->reset();
    }
  }
  get_dynamic_input_handles("Fields",fields,false);

  TCLInterface::eval(get_id()+" update_text");

  if (inputs_changed_ || guimethod_.changed() ||
      guifunction_.changed() || guienable_function_.changed() ||
      !oport_cached("Metric"))
  {
    // Inform module that execution started
    update_state(Executing);

    std::vector<FieldHandle> mfields(fields.size(),0);
    // If not caching set the field and count to zero.

    if (guienable_function_.get())
    {
      for (size_t j=0;j<fields.size();j++)
      {
        if (fields[j].get_rep() && fields[j]->vmesh()->num_nodes() > 0)
        {
          NewArrayMathEngine engine;
          engine.set_progress_reporter(this);

          // Create the DATA object for the function
          // DATA is the data on the field
          if(!(engine.add_input_fielddata("DATA",fields[j]))) return;

          // Create the POS, X,Y,Z, data location objects.

          if(!(engine.add_input_fielddata_location("POS",fields[j]))) return;
          if(!(engine.add_input_fielddata_coordinates("X","Y","Z",fields[j]))) return;


          // Create the ELEMENT object describing element properties
          if(!(engine.add_input_fielddata_element("ELEMENT",fields[j]))) return;

          int basis_order = fields[j]->vfield()->basis_order();

          std::string function = guifunction_.get();

          if(!(engine.add_output_fielddata("RESULT",fields[j],basis_order,"double"))) return;

          // Add an object for getting the index and size of the array.

          if(!(engine.add_index("INDEX"))) return;
          if(!(engine.add_size("SIZE"))) return;

          if(!(engine.add_expressions(function))) return;

          // Actual engine call, which does the dynamic compilation, the creation of the
          // code for all the objects, as well as inserting the function and looping
          // over every data point

          if (!(engine.run()))
          {
            return;
          }
          // Get the result from the engine

          engine.get_field("RESULT",mfields[j]);
        }
      }
    }
    else
    {
      for (size_t j=0; j<fields.size(); j++) mfields[j] = fields[j];
    }

    MatrixHandle metric;
    algo_.set_option("method",guimethod_.get());
    if (!(algo_.run(mfields,metric))) return;

    guimetric_.set(to_string(metric));

    // send new output if there is any:
    send_output_handle("Metric", metric);
  }
}

void
CalculateFieldDataMetric::presave()
{
  // update gui_function_ before saving.
  TCLInterface::execute(get_id() + " update_text");
}

} // End namespace SCIRun
