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
#include <Core/Algorithms/Fields/FieldData/CalculateLatVolGradientsAtNodes.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

/// @class CalculateLatVolGradientsAtNodes
/// @brief Compute the derivative of a scalar lattice and output it as a vector lattice.

class CalculateLatVolGradientsAtNodes : public Module
{
  public:
    CalculateLatVolGradientsAtNodes(GuiContext* ctx);
    virtual ~CalculateLatVolGradientsAtNodes() {}

    virtual void execute();

  private:
    SCIRunAlgo::CalculateLatVolGradientsAtNodesAlgo algo_;
};


DECLARE_MAKER(CalculateLatVolGradientsAtNodes)

CalculateLatVolGradientsAtNodes::CalculateLatVolGradientsAtNodes(GuiContext* ctx)
  : Module("CalculateLatVolGradientsAtNodes", ctx, Filter, "ChangeFieldData", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

void
CalculateLatVolGradientsAtNodes::execute()
{
  FieldHandle input, output;
  get_input_handle("Input Field", input, true);

  if (inputs_changed_ || !oport_cached("Output Gradient"))
  {
    update_state(Executing);

    if (!(algo_.run(input,output))) return;
    send_output_handle("Output Gradient", output);
  }
}

} // End namespace SCIRun
