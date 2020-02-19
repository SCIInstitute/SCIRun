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


#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Converter/ConverterAlgo.h>


namespace SCIRun {

class ConvertMatrixToField : public Module {
public:
  ConvertMatrixToField(GuiContext*);

  virtual void execute();

private:
  GuiString guidatalocation_;
};


DECLARE_MAKER(ConvertMatrixToField)
ConvertMatrixToField::ConvertMatrixToField(GuiContext* ctx)
  : Module("ConvertMatrixToField", ctx, Source, "Converters", "SCIRun"),
    guidatalocation_(get_ctx()->subVar("datalocation"))
{
}

void ConvertMatrixToField::execute()
{
  // Define local handles of data objects:
  MatrixHandle imatrix;
  FieldHandle ofield;

  // Get the new input data:
  get_input_handle("Matrix",imatrix,true);

  // Only reexecute if the input changed. SCIRun uses simple scheduling
  // that executes every module downstream even if no data has changed:
  if (inputs_changed_ || guidatalocation_.changed() || !oport_cached("Field"))
  {
    update_state(Executing);
    std::string datalocation = guidatalocation_.get();
    SCIRunAlgo::ConverterAlgo algo(this);
    if (!(algo.MatrixToField(imatrix,ofield,datalocation))) return;

    // send new output if there is any:
    send_output_handle("Field",ofield);
  }
}

} // End namespace SCIRun
