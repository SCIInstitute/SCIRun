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


/// @todo Documentation Modules/Legacy/Math/RemoveZerosFromMatrix.cc

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Math/SelectMatrix/SelectMatrix.h>
#include <Core/Algorithms/Math/FindMatrix/FindMatrix.h>

namespace SCIRun {

using namespace SCIRun;

class RemoveZerosFromMatrix : public Module {
public:
  RemoveZerosFromMatrix(GuiContext*);

  virtual void execute();

private:
  GuiString  row_or_column_;

  SCIRunAlgo::SelectMatrixAlgo select_algo_;
  SCIRunAlgo::FindMatrixAlgo   find_algo_;
};


DECLARE_MAKER(RemoveZerosFromMatrix)

RemoveZerosFromMatrix::RemoveZerosFromMatrix(GuiContext* ctx) :
  Module("RemoveZerosFromMatrix", ctx, Source, "Math", "SCIRun"),
  row_or_column_(ctx->subVar("row_or_col"))
{
  select_algo_.set_progress_reporter(this);
  find_algo_.set_progress_reporter(this);
}

void
RemoveZerosFromMatrix::execute()
{
  MatrixHandle input, output;

  get_input_handle("Matrix",input,true);

  if (inputs_changed_ || !oport_cached("Matrix") || row_or_column_.changed())
  {
    if (row_or_column_.get() == "row")
    {
      std::vector<index_type> nz;
      find_algo_.set_option("method", "nonzero_rows");
      find_algo_.run(input, nz);
      select_algo_.set_option("method", "select_rows");
      select_algo_.run(input, output, nz);
    }
    else
    {
      std::vector<index_type> nz;
      find_algo_.set_option("method", "nonzero_columns");
      find_algo_.run(input, nz);
      select_algo_.set_option("method", "select_columns");
      select_algo_.run(input, output, nz);
    }

    send_output_handle("Matrix",output,true);
  }
}


} // End namespace SCIRun
