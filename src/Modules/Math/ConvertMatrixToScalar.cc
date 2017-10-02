/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
University of Utah.

License for the specific language governing rights and limitations under
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

#include <Modules/Math/ConvertMatrixToScalar.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Scalar.h>


using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(ConvertMatrixToScalar, Converters, SCIRun)

ConvertMatrixToScalar::ConvertMatrixToScalar() : Module(staticInfo_, false)
{
  INITIALIZE_PORT(Input);
  INITIALIZE_PORT(Output);
}

void ConvertMatrixToScalar::execute()
{
  auto input_matrix = getRequiredInput(Input);

  if (needToExecute())
  {
    if (!matrixIs::dense(input_matrix))
    {
      //TODO implement something with sparse
      error("Currently only works with dense matrices");
      return;
    }

    auto dense = castMatrix::toDense (input_matrix);

    if (dense->get_dense_size()!=1)
    {
      error("matrix must be size 1x1");
    }

    double* dataptr = 0;
    dataptr = dense->data();
    int datavalue= dataptr[0];

    boost::shared_ptr<Int32> out(new Int32(datavalue));

    sendOutput(Output,out);
  }
}
