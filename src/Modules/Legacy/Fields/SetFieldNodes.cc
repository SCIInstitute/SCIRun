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


/*
 *  SetFieldNodes.cc:  Set the field data using either a matrix or a NRRD.
 *
 *  Written by:
 *   SCI Institute
 *   University of Utah
 *   July 2007
 *
 */

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Fields/MeshData/SetMeshNodes.h>

namespace SCIRun {

class SetFieldNodes : public Module {
  public:
    SetFieldNodes(GuiContext*);
    virtual ~SetFieldNodes() {}
    virtual void execute();
    
  private:
    SCIRunAlgo::SetMeshNodesAlgo algo_;
};


DECLARE_MAKER(SetFieldNodes)
SetFieldNodes::SetFieldNodes(GuiContext* ctx)
  : Module("SetFieldNodes", ctx, Source, "ChangeMesh", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

void
SetFieldNodes::execute()
{
  FieldHandle field_input_handle,field_output_handle;
  if(!(get_input_handle("Field",field_input_handle,true))) return;

  MatrixHandle matrix_input_handle;
  get_input_handle("Matrix Nodes",matrix_input_handle,true);

  if (inputs_changed_ ||
      !oport_cached("Field"))
  {
    update_state(Executing);

    if(!(algo_.run(field_input_handle,matrix_input_handle,field_output_handle))) return;
    send_output_handle("Field", field_output_handle);
  }
}

} // End namespace SCIRun
