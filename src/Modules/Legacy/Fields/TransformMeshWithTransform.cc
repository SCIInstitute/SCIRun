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

#include <Core/Algorithms/Fields/TransformMesh/TransformMeshWithTransform.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#include <Dataflow/Network/Module.h>

namespace SCIRun {

/// @class TransformMeshWithTransform
/// @brief Non-interactive geometric transform of a field. 

class TransformMeshWithTransform : public Module
{
  public:
    TransformMeshWithTransform(GuiContext* ctx);
    virtual ~TransformMeshWithTransform() {} 
    virtual void execute();

  private:
    SCIRunAlgo::TransformMeshWithTransformAlgo algo_;
};


DECLARE_MAKER(TransformMeshWithTransform)

TransformMeshWithTransform::TransformMeshWithTransform(GuiContext* ctx)
  : Module("TransformMeshWithTransform", ctx, Filter, "ChangeMesh", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

void
TransformMeshWithTransform::execute()
{
  // Get input field.
  FieldHandle ifield, ofield;
  get_input_handle("Input Field", ifield,true);

  MatrixHandle imatrix;
  get_input_handle("Transform Matrix", imatrix,true);

  if (inputs_changed_ || !oport_cached("Transformed Field"))
  {
    update_state(Executing);

    if(!(algo_.run(ifield,imatrix,ofield))) return;    
    send_output_handle("Transformed Field", ofield);
  }
}


} // End namespace SCIRun

