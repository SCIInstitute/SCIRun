/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


/// @todo Documentation Modules/Legacy/Math/SortMatrix.cc

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Math/SortMatrix/SortMatrix.h>

namespace SCIRun {

using namespace SCIRun;

class SortMatrix : public Module {
public:
  SortMatrix(GuiContext*);
  virtual void execute();

private:
  GuiString  row_or_column_;
  SCIRunAlgo::SortMatrixAlgo algo_;
};


DECLARE_MAKER(SortMatrix)

SortMatrix::SortMatrix(GuiContext* ctx) :
  Module("SortMatrix", ctx, Source, "Math", "SCIRun"),
  row_or_column_(ctx->subVar("row_or_col"))
{
  algo_.set_progress_reporter(this);
}

void
SortMatrix::execute()
{
  MatrixHandle input, output;

  get_input_handle("Matrix",input,true);

  if (inputs_changed_ || !oport_cached("Matrix") || row_or_column_.changed())
  {
    algo_.set_option("method","sort_columns");
    if (row_or_column_.get() == "row")
      algo_.set_option("method","sort_rows");
    algo_.run(input,output);

    send_output_handle("Matrix",output,true);
  }
}



} // End namespace SCIRun
