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

#include <Core/Algorithms/Fields/MeshData/FlipSurfaceNormals.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Module.h>
//#include <Core/Malloc/Allocator.h>

namespace SCIRun {

//using namespace SCIRun;

/// @class FlipSurfaceNormals
/// @brief Changes the normal of the face of an element on a surface mesh by
/// reordering how the nodes are ordered in the face definition.

class FlipSurfaceNormals : public Module 
{
  public:

    FlipSurfaceNormals(GuiContext* ctx);
    virtual ~FlipSurfaceNormals();
    virtual void execute();
    virtual void tcl_command(GuiArgs&, void*);
  
  private:
    SCIRunAlgo::FlipSurfaceNormalsAlgo algo_;

};


DECLARE_MAKER(FlipSurfaceNormals)

FlipSurfaceNormals::FlipSurfaceNormals(GuiContext* ctx) :
  Module("FlipSurfaceNormals", ctx, Filter, "ChangeMesh", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

FlipSurfaceNormals::~FlipSurfaceNormals(){
}

void
FlipSurfaceNormals::execute()
{
  FieldHandle input, output;

  //get_input_handle( "vector_median_filt", input, true );
  get_input_handle("InputSurface",input,true);

  // If no data or a changed recalcute.
  if( inputs_changed_ || !oport_cached("OutputSurface") )
  {
    update_state(Executing);

    if (!(algo_.run(input,output))) return;
    send_output_handle( "OutputSurface", output );
  }

}

void
FlipSurfaceNormals::tcl_command(GuiArgs& args, void* userdata)
{
  Module::tcl_command(args, userdata);
}

} // End namespace SCIRun


