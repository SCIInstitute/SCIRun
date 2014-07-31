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

#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>
#include <Core/Algorithms/Math/MathAlgo.h>

namespace SCIRun {

/// @class ReportMatrixColumnMeasure
/// @brief This module computes a measure on each column of the input matrix and
/// stores the result in the output matrix. 

class ReportMatrixColumnMeasure : public Module {
public:
  ReportMatrixColumnMeasure(GuiContext*);
  virtual void execute();

private:
  GuiString guimethod_;
};


DECLARE_MAKER(ReportMatrixColumnMeasure)
ReportMatrixColumnMeasure::ReportMatrixColumnMeasure(GuiContext* ctx)
  : Module("ReportMatrixColumnMeasure", ctx, Source, "Math", "SCIRun"),
    guimethod_(ctx->subVar("method"))
{
}


void
ReportMatrixColumnMeasure::execute()
{
  MatrixHandle input, output;
  
  if (!(get_input_handle("Matrix",input,true))) return;
  
  if (inputs_changed_ || guimethod_.changed() || !oport_cached("Vector"))
  {
    std::string method = guimethod_.get();
  
    SCIRunAlgo::MathAlgo algo(this);
  
    if (!(algo.ApplyColumnOperation(input,output,method))) return;
    send_output_handle("Vector", output);
  }
}

} // End namespace SCIRun

