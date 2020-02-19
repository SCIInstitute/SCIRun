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


// File:   ConvertBundleToField.cc
// Author: Fangxiang Jiao
// Date:   March 25 2010

#include <Core/Datatypes/Bundle.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>

#include <Core/Algorithms/Converter/ConvertBundleToField.h>
#include <Core/Algorithms/Fields/ConvertMeshType/ConvertMeshToPointCloudMesh.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Ports/FieldPort.h>

using namespace SCIRun;

class ConvertBundleToField : public Module {

public:
  ConvertBundleToField(GuiContext*);
  virtual ~ConvertBundleToField() {}
  virtual void execute();

private:
    GuiInt    guiclear_;
    GuiDouble guitolerance_;
    GuiInt    guimergenodes_;
    GuiInt    guiforcepointcloud_;
    GuiInt    guimatchval_;
    GuiInt    guimeshonly_;

    SCIRunAlgo::ConvertBundleToFieldAlgo algo_;
    SCIRunAlgo::ConvertMeshToPointCloudMeshAlgo calgo_;
};

DECLARE_MAKER(ConvertBundleToField)
ConvertBundleToField::ConvertBundleToField(GuiContext* ctx)
  : Module("ConvertBundleToField", ctx, Source, "Converters", "SCIRun"),
  guiclear_(get_ctx()->subVar("clear", false), 0),
  guitolerance_(get_ctx()->subVar("tolerance"), 0.0001),
  guimergenodes_(get_ctx()->subVar("force-nodemerge"), 1),
  guiforcepointcloud_(get_ctx()->subVar("force-pointcloud"), 0),
  guimatchval_(get_ctx()->subVar("matchval"), 0),
  guimeshonly_(get_ctx()->subVar("meshonly"), 0)
{
  algo_.set_progress_reporter(this);
  calgo_.set_progress_reporter(this);
}

void ConvertBundleToField::execute()
{
  // Define input handle:
  BundleHandle handle;

  // Get data from input port:
  if (!(get_input_handle("bundle", handle, true)))
  {
    return;
  }

  if (inputs_changed_ || !oport_cached("Output") )
  {
    update_state(Executing);

    // Some stuff for old power apps
    if (guiclear_.get())
    {
      guiclear_.set(0);

      // Sending 0 does not clear caches.
      FieldInformation fi("PointCloudMesh", 0, "double");
      FieldHandle fhandle = CreateField(fi);

      send_output_handle("Output", fhandle);
      return;
    }

    FieldHandle output;

    algo_.set_scalar("tolerance", guitolerance_.get());
    algo_.set_bool("merge_nodes", guimergenodes_.get());
    algo_.set_bool("match_node_values", guimatchval_.get());
    algo_.set_bool("make_no_data", guimeshonly_.get());

    if (! ( algo_.run(handle, output) ))
      return;

    // This option is here to be compatible with the old GatherFields module:
    // This is a separate algorithm now
    if (guiforcepointcloud_.get())
    {
      if(! ( calgo_.run(output, output) ))
        return;
    }

    // send new output if there is any:
    send_output_handle("Output", output);
  }
}
