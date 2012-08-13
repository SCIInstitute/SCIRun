/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Math/AppendMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun::Algorithms;
using namespace SCIRun::Algorithms::Math;
using namespace SCIRun::Domain::Datatypes;

AlgorithmParameterName AppendMatrixAlgorithm::OptionName("RowsOrColumns");

AppendMatrixAlgorithm::Outputs AppendMatrixAlgorithm::run(const AppendMatrixAlgorithm::Inputs& input, const AppendMatrixAlgorithm::Parameters& params) const
{
  DenseMatrixConstHandle lhs = input.get<0>();
  DenseMatrixConstHandle rhs = input.get<1>();
  if (!lhs || !rhs)
    return Outputs(); //TODO: error

  if (params == ROWS)
  {
    if (lhs->ncols() != rhs->ncols())
      return Outputs(); //TODO: error

    DenseMatrixHandle output(new DenseMatrix(lhs->nrows() + rhs->nrows(), lhs->ncols()));
    for (int i = 0; i < lhs->nrows(); ++i)
      for (int j = 0; j < lhs->ncols(); ++j)
        (*output)(i,j) = (*lhs)(i,j);
    for (int i = 0; i < rhs->nrows(); ++i)
      for (int j = 0; j < rhs->ncols(); ++j)
        (*output)(i + lhs->nrows(), j) = (*rhs)(i,j);
    return output;
  }
  else // columns
  {
    if (lhs->nrows() != rhs->nrows())
      return Outputs(); //TODO: error

    DenseMatrixHandle output(new DenseMatrix(lhs->nrows(), lhs->ncols() + rhs->ncols()));
    for (int i = 0; i < lhs->nrows(); ++i)
      for (int j = 0; j < lhs->ncols(); ++j)
        (*output)(i,j) = (*lhs)(i,j);
    for (int i = 0; i < rhs->nrows(); ++i)
      for (int j = 0; j < rhs->ncols(); ++j)
        (*output)(i, j + lhs->ncols()) = (*rhs)(i,j);
    return output;
  }
}