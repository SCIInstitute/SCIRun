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

#include <Core/Algorithms/Math/ReorderMatrix/ReorderMatrix.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

/// @class ReorderMatrixByReverseCuthillMcKee
/// @brief This module reorders a matrix to reduce the average bandwidth of the matrix. 

class ReorderMatrixByReverseCuthillMcKee : public Module {
  public:
    ReorderMatrixByReverseCuthillMcKee(GuiContext*);
    virtual void execute();

  private:
    SCIRunAlgo::ReorderMatrixAlgo algo_;
};


DECLARE_MAKER(ReorderMatrixByReverseCuthillMcKee)
ReorderMatrixByReverseCuthillMcKee::ReorderMatrixByReverseCuthillMcKee(GuiContext* ctx)
  : Module("ReorderMatrixByReverseCuthillMcKee", ctx, Source, "Math", "SCIRun")
{
  algo_.set_progress_reporter(this);
  algo_.set_option("method","reversecuthillmckee");
}

void ReorderMatrixByReverseCuthillMcKee::execute()
{
  MatrixHandle Mat, Mapping, InverseMapping;
  if(!(get_input_handle("Matrix",Mat,true))) return;

  if (inputs_changed_ || !oport_cached("Matrix") || !oport_cached("Mapping") || 
        !oport_cached("InverseMapping"))
  {
    if(!(algo_.run(Mat,Mat,InverseMapping))) return;
    Mapping = InverseMapping->make_transpose();
        
    send_output_handle("Matrix",Mat,true);
    send_output_handle("Mapping",Mapping,true);
    send_output_handle("InverseMapping",InverseMapping,true);
  }
}

} // End namespace SCIRun


