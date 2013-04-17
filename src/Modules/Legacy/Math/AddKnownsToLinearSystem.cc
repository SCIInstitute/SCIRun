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

#include <Core/Algorithms/Math/LinearSystem/AddKnownsToLinearSystem.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Datatypes/SparseRowMatrix.h>

namespace SCIRun {

using namespace SCIRun;

class AddKnownsToLinearSystem: public Module {
public:
  AddKnownsToLinearSystem(GuiContext*);
  virtual void execute();

private:
  SCIRunAlgo::AddKnownsToLinearSystemAlgo algo_;
};


DECLARE_MAKER(AddKnownsToLinearSystem)

AddKnownsToLinearSystem::AddKnownsToLinearSystem(GuiContext* ctx) :
  Module("AddKnownsToLinearSystem", ctx, Source, "Math", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

void
AddKnownsToLinearSystem::execute()
{
  MatrixHandle ain, bin, x, bout;
  SparseRowMatrixHandle aout;
  
  get_input_handle("Matrix",ain,true);
  get_input_handle("RHS",bin,false);
  get_input_handle("X",x,true);
  
  if (inputs_changed_ || !oport_cached("Matrix") ||!oport_cached("RHS"))
  {
    algo_.run(ain,bin,x,aout,bout);
    send_output_handle("Matrix",aout,true);
    send_output_handle("RHS",bout,true);
  }
}

} // End namespace SCIRun
