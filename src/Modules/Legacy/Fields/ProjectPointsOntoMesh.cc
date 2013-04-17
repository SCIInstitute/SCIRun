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

#include <Core/Algorithms/Fields/TransformMesh/ProjectPointsOntoMesh.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>

namespace SCIRun {

using namespace SCIRun;

class ProjectPointsOntoMesh : public Module {
  public:
    ProjectPointsOntoMesh(GuiContext*);
    virtual ~ProjectPointsOntoMesh() {}
    virtual void execute();
    
  private:
    GuiString guimethod_;
    SCIRunAlgo::ProjectPointsOntoMeshAlgo algo_;
};


DECLARE_MAKER(ProjectPointsOntoMesh)

ProjectPointsOntoMesh::ProjectPointsOntoMesh(GuiContext* ctx) :
  Module("ProjectPointsOntoMesh", ctx, Source, "ChangeMesh", "SCIRun"),
  guimethod_(ctx->subVar("method"),"nodes")
{
  algo_.set_progress_reporter(this);
}

void
ProjectPointsOntoMesh::execute()
{
  FieldHandle input, object, output;
  get_input_handle("Field",input,true);
  get_input_handle("Object",object,true);

  if (inputs_changed_ || guimethod_.changed() || !oport_cached("Field"))
  {
    update_state(Executing);

    algo_.set_option("method",guimethod_.get());
    if (!(algo_.run(input,object,output))) return;
    
    send_output_handle("Field",output);
  }
}


} // End namespace SCIRun


