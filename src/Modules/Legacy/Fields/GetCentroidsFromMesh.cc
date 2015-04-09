/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
#include <Core/Algorithms/Fields/MeshDerivatives/GetCentroids.h>

// The module class
#include <Dataflow/Network/Module.h>

// We need to define the ports used
#include <Dataflow/Network/Ports/FieldPort.h>

namespace SCIRun {

/// @class GetCentroidsFromMesh
/// @brief This module computes a PointCloudField containing all of the element centers for a field. 

class GetCentroidsFromMesh : public Module {
  public:
    GetCentroidsFromMesh(GuiContext* ctx);
    virtual ~GetCentroidsFromMesh() {}
    
    virtual void execute();

  private:  
    SCIRunAlgo::GetCentroidsAlgo algo_;

    GuiString component_;
};


DECLARE_MAKER(GetCentroidsFromMesh)


GetCentroidsFromMesh::GetCentroidsFromMesh(GuiContext* ctx)
  : Module("GetCentroidsFromMesh", ctx, Filter, "NewField", "SCIRun"),
    component_(get_ctx()->subVar("component"),"elem")
{
  /// Forward errors to the module
  algo_.set_progress_reporter(this);
}

void
GetCentroidsFromMesh::execute()
{
  FieldHandle input, output;
  if (!get_input_handle("Field", input)) return;

  if (inputs_changed_ && !oport_cached("Field"))
  {
    update_state(Executing);
    /// Set the component we want to extract
    algo_.set_option("centroid",component_.get());

    /// Run the algorithm
    if(!(algo_.run(input,output))) return;
    
    send_output_handle("Field", output);
  }
}

} // End namespace SCIRun


