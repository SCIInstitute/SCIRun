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

author: Moritz Dannhauer & Kimia Shayestehfard
last change: 02/16/17
*/

#include <Modules/Math/ConvertComplexToRealMatrix.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(ConvertComplexToRealMatrix, Converters, SCIRun)

ConvertComplexToRealMatrix::ConvertComplexToRealMatrix() : Module(staticInfo_, false)
{
  INITIALIZE_PORT(InputComplexMatrix);
  INITIALIZE_PORT(OutputRealPartMatrix);
  INITIALIZE_PORT(OutputComplexPartMatrix);
}

namespace
{
  template <class M, class T1, class T2>
  std::tuple<boost::shared_ptr<M>, boost::shared_ptr<M>> moveToHeap(std::tuple<T1, T2>&& mats)
  {
    return std::make_tuple(boost::make_shared<M>(std::get<0>(mats)), boost::make_shared<M>(std::get<1>(mats)));
  }
}

void ConvertComplexToRealMatrix::execute()
{

  auto input_complex_matrix = getRequiredInput(InputComplexMatrix);

  if (needToExecute())
  {
    if (input_complex_matrix->nrows() == 0 || input_complex_matrix->ncols() == 0)
    {
      error("Number of Rows or Columns are zero");
      return;
    }

    auto input_dense = castMatrix::toDense(input_complex_matrix);
    auto input_sparse = castMatrix::toSparse(input_complex_matrix);
    auto input_column = castMatrix::toColumn(input_complex_matrix);

    if (!input_dense && !input_sparse && !input_column)
    {
      error("Unknown matrix type");
      return;
    }

    MatrixHandle output_realH, output_imagH;

    if (input_dense)
    {
      std::tie(output_realH, output_imagH) = moveToHeap<DenseMatrix>(splitByComponents(*input_dense));
    }
    else if (input_column)
    {
      std::tie(output_realH, output_imagH) = moveToHeap<DenseColumnMatrix>(splitByComponents(*input_column));
    }
    else
    {
      std::tie(output_realH, output_imagH) = moveToHeap<SparseRowMatrix>(splitByComponents(*input_sparse));
    }

    sendOutput(OutputRealPartMatrix, output_realH);
    sendOutput(OutputComplexPartMatrix, output_imagH);
  }
}
