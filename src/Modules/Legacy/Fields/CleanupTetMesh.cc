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

#include <Core/Datatypes/Field.h>
#include <Core/Algorithms/Fields/Cleanup/CleanupTetMesh.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

using namespace SCIRun;

/// @class CleanupTetMesh
/// @brief This module tidies up node orientation and removes degenerate
/// tetrahedral elements on a given tetrahedral mesh. 

class CleanupTetMesh : public Module {
  public:
    CleanupTetMesh(GuiContext*);
    virtual ~CleanupTetMesh() {}
    virtual void execute();

  private:
    GuiInt  fix_orientation_;
    GuiInt  remove_degenerate_;
    
    SCIRunAlgo::CleanupTetMeshAlgo algo_;
};


DECLARE_MAKER(CleanupTetMesh)

CleanupTetMesh::CleanupTetMesh(GuiContext* ctx) :
  Module("CleanupTetMesh", ctx, Source, "ChangeMesh", "SCIRun"),
    fix_orientation_(get_ctx()->subVar("fix-orientation"),1),
    remove_degenerate_(get_ctx()->subVar("remove-degenerate"),1)
{
  algo_.set_progress_reporter(this);
}

void
CleanupTetMesh::execute()
{
  FieldHandle input, output;
  get_input_handle("Field",input,true);
  
  if (inputs_changed_ || fix_orientation_.changed() || 
      remove_degenerate_.changed() || !oport_cached("Field"))
  {
    update_state(Executing);
    
    algo_.set_bool("fix_orientation",fix_orientation_.get());
    algo_.set_bool("remove_degenerate",remove_degenerate_.get());
    if(!(algo_.run(input,output))) return;
    
    send_output_handle("Field",output);
  }
}

} // End namespace SCIRun


