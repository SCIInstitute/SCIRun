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

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#include <Core/Algorithms/Fields/ClipMesh/ClipMeshBySelection.h>
#include <Core/Algorithms/Fields/DistanceField/CalculateIsInsideField.h>

namespace SCIRun {

/// @class ClipFieldByMesh
/// @brief Clip a mesh to another mesh. 

class ClipFieldByMesh : public Module {
  public:
    ClipFieldByMesh(GuiContext*);
    virtual void execute();

  private:
    SCIRunAlgo::ClipMeshBySelectionAlgo clip_algo_;
    SCIRunAlgo::CalculateIsInsideFieldAlgo inside_algo_;

};


DECLARE_MAKER(ClipFieldByMesh)
ClipFieldByMesh::ClipFieldByMesh(GuiContext* ctx)
  : Module("ClipFieldByMesh", ctx, Source, "NewField", "SCIRun")
{
  clip_algo_.set_progress_reporter(this);
  inside_algo_.set_progress_reporter(this);
}


void
ClipFieldByMesh::execute()
{
  // Define local handles of data objects:
  FieldHandle input;
  FieldHandle output;
  FieldHandle object;
  FieldHandle selection;
  MatrixHandle interpolant;
  
  // Get the new input data is:  
  get_input_handle("Field",input,true);
  get_input_handle("Object",object,true);

  // Only reexecute if the input changed. SCIRun uses simple scheduling
  // that executes every module downstream even if no data has changed:  
  if (inputs_changed_ || !oport_cached("Field") || !oport_cached("Mapping"))
  {
    update_state(Executing);
    inside_algo_.set_option("output_type","char");
    inside_algo_.set_option("sampling_scheme","regular2");
    
    if(!(inside_algo_.run(input,object,selection))) return;
    
    if (input->vmesh()->num_elems() == selection->vfield()->num_values())
      clip_algo_.set_option("method","element");
    if(!(clip_algo_.run(input,selection,output,interpolant))) return;

    // send new output if there is any:      
    send_output_handle("Field", output);
    send_output_handle("Mapping", interpolant);
  }
}


} // End namespace SCIRun


