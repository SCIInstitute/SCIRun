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


#include <Core/Datatypes/Bundle.h>
#include <Core/Datatypes/MatrixTypeConverter.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/BundlePort.h>

using namespace SCIRun;

/// @class JoinBundles
/// @brief This module merges the contents of multiple bundles into one bundle.

class JoinBundles : public Module {
public:
  JoinBundles(GuiContext*);
  virtual void execute();
};


DECLARE_MAKER(JoinBundles)
  JoinBundles::JoinBundles(GuiContext* ctx)
    : Module("JoinBundles", ctx, Source, "Bundle", "SCIRun")
{
}

void JoinBundles::execute()
{
  /// vector of inputs
  std::vector<BundleHandle> inputs;

  /// Get the handles from the module
  get_dynamic_input_handles("bundle",inputs,false);

  /// If anything changed we need to reexecute
  if (inputs_changed_ || !oport_cached("bundle"))
  {
    update_state(Executing);
    /// Create output object
    BundleHandle output;
    output = new Bundle;

    /// In case output object could not be allocated
    if (output.get_rep() == 0)
    {
      error("Could not allocate new bundle");
      return;
    }

    /// Merge in all the different bundles
    for (size_t p=0; p<inputs.size();p++)
    {
      output->merge(inputs[p]);
    }

    send_output_handle("bundle",output);
  }
}
