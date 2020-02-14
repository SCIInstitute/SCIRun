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
#include <Core/Datatypes/DenseMatrix.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>


namespace SCIRun {

using namespace SCIRun;

class CreateScalarMatrix : public Module {
  public:
    CreateScalarMatrix(GuiContext*);
    virtual void execute();

  public:
    GuiDouble value_;
    GuiDouble value_min_;
    GuiDouble value_max_;
    GuiDouble value_step_;
};


DECLARE_MAKER(CreateScalarMatrix)

CreateScalarMatrix::CreateScalarMatrix(GuiContext* ctx) :
  Module("CreateScalarMatrix", ctx, Source, "Math", "SCIRun"),
    value_(get_ctx()->subVar("value"),0.0),
    value_min_(get_ctx()->subVar("value-min"),0.0),
    value_max_(get_ctx()->subVar("value-max"),1.0),
    value_step_(get_ctx()->subVar("value-step"),0.01)
{
}


void
CreateScalarMatrix::execute()
{
  TCLInterface::eval("set "+get_id()+"-init 1");

  if (value_.changed() || !oport_cached("Scalar"))
  {
    MatrixHandle matrix = new DenseMatrix(value_.get());
    send_output_handle("Scalar",matrix);
  }
}

} // End namespace SCIRun
