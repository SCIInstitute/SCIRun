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

#include <Core/Datatypes/Field.h>
#include <Core/Algorithms/Fields/FieldData/CalculateVectorMagnitudes.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>


namespace SCIRun {

class CalculateVectorMagnitudes : public Module
{
  public:
    CalculateVectorMagnitudes(GuiContext* ctx);
    virtual ~CalculateVectorMagnitudes() {}
    virtual void execute();

  private:
    SCIRunAlgo::CalculateVectorMagnitudesAlgo algo_;
};


DECLARE_MAKER(CalculateVectorMagnitudes)

CalculateVectorMagnitudes::CalculateVectorMagnitudes(GuiContext* ctx)
  : Module("CalculateVectorMagnitudes", ctx, Filter, "ChangeFieldData", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

void
CalculateVectorMagnitudes::execute()
{
  FieldHandle input, output;

  get_input_handle( "Field", input, true );

  // If no data or a changed recalcute.
  if( inputs_changed_ || !oport_cached("Field") )
  {
    update_state(Executing);
    if (!(algo_.run(input,output))) return;
    send_output_handle( "Field", output );
  }
}

} // End namespace SCIRun










