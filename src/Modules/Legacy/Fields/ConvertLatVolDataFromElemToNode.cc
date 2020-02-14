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


#include <Core/Algorithms/Fields/ConvertMeshType/ConvertLatVolDataFromElemToNode.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

/// @class ConvertLatVolDataFromElemToNode
/// @brief Make a new field with data at nodes instead of elements.

class ConvertLatVolDataFromElemToNode : public Module
{
  public:
    ConvertLatVolDataFromElemToNode(GuiContext* ctx);
    virtual ~ConvertLatVolDataFromElemToNode() {}

    virtual void execute();

  private:
    SCIRunAlgo::ConvertLatVolDataFromElemToNodeAlgo algo_;
};


DECLARE_MAKER(ConvertLatVolDataFromElemToNode)

ConvertLatVolDataFromElemToNode::ConvertLatVolDataFromElemToNode(GuiContext* ctx)
  : Module("ConvertLatVolDataFromElemToNode", ctx, Filter, "ChangeFieldData", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

void
ConvertLatVolDataFromElemToNode::execute()
{
  // Get input field.
  FieldHandle input, output;
  get_input_handle("Elem Field", input, true);

  if (inputs_changed_ || !oport_cached("Node Field"))
  {
    update_state(Executing);

    /// Run algorithm
    if(!(algo_.run(input,output))) return;

    /// Send output
    send_output_handle("Node Field", output);
  }
}

} // End namespace SCIRun
