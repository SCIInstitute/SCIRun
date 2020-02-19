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
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Math/MathAlgo.h>
#include <Core/Algorithms/Converter/ConverterAlgo.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

/// @class ResizeMatrix
/// This module resizes a matrix to a different size.

class ResizeMatrix : public Module {
public:
  ResizeMatrix(GuiContext*);
  virtual void execute();

private:
  GuiInt guim_;
  GuiInt guin_;

};


DECLARE_MAKER(ResizeMatrix)
ResizeMatrix::ResizeMatrix(GuiContext* ctx)
  : Module("ResizeMatrix", ctx, Source, "Math", "SCIRun"),
  guim_(get_ctx()->subVar("dim-m")),
  guin_(get_ctx()->subVar("dim-n"))
{
}


void ResizeMatrix::execute()
{
  MatrixHandle Mat, M, N;

  if(!(get_input_handle("Matrix",Mat,true))) return;
  get_input_handle("M",M,false);
  get_input_handle("N",N,false);

  if (inputs_changed_ || guim_.changed() || guin_.changed() ||
      !oport_cached("Matrix"))
  {
    SCIRunAlgo::MathAlgo nalgo(this);
    SCIRunAlgo::ConverterAlgo calgo(this);
    int n,m;

    if (M.get_rep())
    {
    if(calgo.MatrixToInt(M,m)) guim_.set(m);
    get_ctx()->reset();
    }

    if (N.get_rep())
    {
    if(calgo.MatrixToInt(N,n)) guin_.set(n);
    get_ctx()->reset();
    }

    m = guim_.get();
    n = guin_.get();

    if(!(nalgo.ResizeMatrix(Mat,Mat,m,n))) return;

    send_output_handle("Matrix",Mat,true);
  }
}

} // End namespace SCIRun
