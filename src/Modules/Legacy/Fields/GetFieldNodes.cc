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

//! Include the algorithm
#include <Core/Algorithms/Fields/MeshData/GetMeshNodes.h>

//! The module class
#include <Dataflow/Network/Module.h>

//! We need to define the ports used
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/NrrdPort.h>

namespace SCIRun {

class GetFieldNodes : public Module {
  public:
    //! constructor and execute function
    GetFieldNodes(GuiContext*);
    virtual ~GetFieldNodes() {}
    virtual void execute();
  
  private:
    //! Define algorithms needed
    SCIRunAlgo::GetMeshNodesAlgo algo_;
  
};


DECLARE_MAKER(GetFieldNodes)
GetFieldNodes::GetFieldNodes(GuiContext* ctx)
  : Module("GetFieldNodes", ctx, Source, "ChangeMesh", "SCIRun")
{
  //! Forward error messages;
  algo_.set_progress_reporter(this);
}

void GetFieldNodes::execute()
{
  //! Define dataflow handles:
  FieldHandle input;
  MatrixHandle matrixdata(0);
  
  //! Get data from port:
  if(!(get_input_handle("Field",input,true))) return;

  //! Only do work if needed:
  if (inputs_changed_ || !oport_cached("Matrix Nodes"))
  {    
    update_state(Executing);

    //! Run algorithm
    if(!(algo_.run(input,matrixdata))) return;

    //! If port is not connected at time of execute, send down a null handle
    //! send data downstream:
    send_output_handle("Matrix Nodes", matrixdata);
  }
}

} // End namespace SCIRun

