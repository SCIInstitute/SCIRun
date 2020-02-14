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


#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Fields/DistanceField/CalculateIsInsideField.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

/// @class CalculateIsInsideField
/// @brief Calculate whether an element of one field is inside the domain of another field.

class CalculateIsInsideField : public Module
{
  public:
    CalculateIsInsideField(GuiContext*);
    virtual ~CalculateIsInsideField() {}
    virtual void execute();

  private:
    GuiString outputtype_;
    GuiDouble outval_;
    GuiDouble inval_;

    SCIRunAlgo::CalculateIsInsideFieldAlgo algo_;
};


DECLARE_MAKER(CalculateIsInsideField)
CalculateIsInsideField::CalculateIsInsideField(GuiContext* ctx)
  : Module("CalculateIsInsideField", ctx, Source, "ChangeFieldData", "SCIRun"),
    outputtype_(ctx->subVar("outputtype")),
    outval_(ctx->subVar("outval")),
    inval_(ctx->subVar("inval"))
{
  algo_.set_progress_reporter(this);
}

void CalculateIsInsideField::execute()
{
  // Define local handles of data objects:
  FieldHandle input, output;
  FieldHandle object;

  // Get the new input data:
  if (!(get_input_handle("Field",input,true))) return;
  if (!(get_input_handle("ObjectField",object,true))) return;

    // Only reexecute if the input changed. SCIRun uses simple scheduling
  // that executes every module downstream even if no data has changed:
  if (inputs_changed_ || outputtype_.changed() || outval_.changed() ||
      inval_.changed() || !oport_cached("Field"))
  {
    update_state(Executing);

    algo_.set_scalar("outside_value",outval_.get());
    algo_.set_scalar("inside_value",inval_.get());
    algo_.set_option("output_type",outputtype_.get());
    if(!(algo_.run(input,object,output))) return;

    // send new output if there is any:
    send_output_handle("Field", output);
  }
}

} // End namespace SCIRun
