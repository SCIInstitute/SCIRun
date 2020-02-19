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


#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Math/AppendMatrix/AppendMatrix.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

namespace SCIRun {

/// @class AppendMatrix
/// @brief Append the rows or columns of a matrix to a base matrix.

class AppendMatrix : public Module {
  public:
    AppendMatrix(GuiContext*);
    virtual void execute();

  private:
    GuiString guiroc_;
    SCIRunAlgo::AppendMatrixAlgo algo_;
};


DECLARE_MAKER(AppendMatrix)
AppendMatrix::AppendMatrix(GuiContext* ctx)
  : Module("AppendMatrix", ctx, Source, "Math", "SCIRun"),
    guiroc_(ctx->subVar("row-or-column"))
{
  algo_.set_progress_reporter(this);
}


void
AppendMatrix::execute()
{
  MatrixHandle base;
  std::vector<MatrixHandle> matrices;

  get_input_handle("BaseMatrix",base,true);
  get_dynamic_input_handles("AppendMatrix",matrices,false);

  if (inputs_changed_ || guiroc_.changed() || !oport_cached("Matrix"))
  {
    std::string roc = guiroc_.get();
    MatrixHandle matrix;

    if (roc == "column")
    {
      algo_.set_option("method","append_columns");
      matrix = base;
      for (size_t p=0; p<matrices.size();p++)
      {
        if (!(algo_.run(matrix,matrix,matrices[p]))) return;
      }
    }
    else
    {
      algo_.set_option("method","append_rows");
      matrix = base;
      for (size_t p=0; p<matrices.size();p++)
      {
        if (!(algo_.run(matrix,matrix,matrices[p]))) return;
      }
    }

    send_output_handle("Matrix", matrix);
  }
}

} // End namespace SCIRun
