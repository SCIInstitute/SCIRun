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

// Include the algorithm
#include <Core/Algorithms/Fields/ConvertMeshType/ConvertMeshToPointCloudMesh.h>

// Base class for the module
#include <Dataflow/Network/Module.h>

// Ports included in the module
#include <Dataflow/Network/Ports/FieldPort.h>

// For Windows support
#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

/// @class ConvertMeshToPointCloud
/// @brief Convert a structured field into an unstructured field for editing. 

class ConvertMeshToPointCloud : public Module
{
  public:
    ConvertMeshToPointCloud(GuiContext* ctx);
    virtual ~ConvertMeshToPointCloud() {}
    virtual void execute();
  
  private:
    SCIRunAlgo::ConvertMeshToPointCloudMeshAlgo algo_;
      
  private:
      GuiInt datalocation_;
};

DECLARE_MAKER(ConvertMeshToPointCloud)
ConvertMeshToPointCloud::ConvertMeshToPointCloud(GuiContext* ctx)
  : Module("ConvertMeshToPointCloud", ctx, Filter, "ChangeMesh", "SCIRun"),
    datalocation_(get_ctx()->subVar("datalocation"))
{
  /// Forward errors to the module
  algo_.set_progress_reporter(this);
}

void
ConvertMeshToPointCloud::execute()
{
  // Define fieldhandles
  FieldHandle ifield, ofield;
  
  // Get data from input port
  get_input_handle("Field",ifield,true);

  // We only execute if something changed
  if (inputs_changed_ || datalocation_.changed() || !oport_cached("Field"))
  {
    update_state(Executing);
    
    if(datalocation_.get()) 
    {
      algo_.set_option("location","data");
    }
    else
    {
      algo_.set_option("location","node");
    }
    // Run the algorithm
    if (!(algo_.run(ifield,ofield))) return;
    // Send to output to the output port
    send_output_handle("Field", ofield);
  }
}

} // End namespace SCIRun

