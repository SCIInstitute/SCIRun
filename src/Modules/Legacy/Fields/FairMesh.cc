//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2009 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.

///TODO Documentation

#include <Core/Algorithms/Fields/SmoothMesh/FairMesh.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

class FairMesh : public Module {
  public:
    FairMesh(GuiContext*);
    virtual ~FairMesh() {}
    virtual void execute();
    
  private:
    GuiInt       iterations_;
    GuiString    method_;
    GuiDouble    lambda_;
    GuiDouble    mu_;
    
    SCIRunAlgo::FairMeshAlgo algo_;
};


DECLARE_MAKER(FairMesh)
FairMesh::FairMesh(GuiContext* ctx) : 
  Module("FairMesh", ctx, Source, "NewField", "SCIRun"),
  iterations_(get_ctx()->subVar("iterations"), 50),
  method_(get_ctx()->subVar("method"), "fast"),
  lambda_(get_ctx()->subVar("lambda"),0.6307),
  mu_(get_ctx()->subVar("mu"),0.1)
{
  algo_.set_progress_reporter(this);
}


void FairMesh::execute()
{
  FieldHandle input, output;
  
  get_input_handle("Input Mesh", input);

  //! If it is a new field get appropriate algorithm, 
  //! otherwise the cached algorithm is still good.
  if (inputs_changed_ || iterations_.changed() ||
      method_.changed() || lambda_.changed() ||
      mu_.changed() || !oport_cached("Faired Mesh"))
  {
    update_state(Executing);
    algo_.set_int("num_iterations",iterations_.get());
    algo_.set_option("method",method_.get());
    algo_.set_scalar("lambda",lambda_.get());
    algo_.set_scalar("filter_cutoff",mu_.get());
    if(!(algo_.run(input,output))) return;
    
    send_output_handle("Faired Mesh", output);
  }
}

} // End namespace SCIRun
