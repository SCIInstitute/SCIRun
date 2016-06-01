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

#include <Core/Algorithms/Math/AppendMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

AppendMatrixAlgorithm::AppendMatrixAlgorithm()
{
  addParameter(Variables::RowsOrColumns, 0);
}

AppendMatrixAlgorithm::Outputs AppendMatrixAlgorithm::run(const AppendMatrixAlgorithm::Inputs& input, const AppendMatrixAlgorithm::Parameters& params) const
{
  DenseMatrixConstHandle lhsPtr = input.get<0>();
  DenseMatrixConstHandle rhsPtr = input.get<1>();
  if (!lhsPtr || !rhsPtr)
    return Outputs(); /// @todo: error

  const DenseMatrix& lhs = *lhsPtr;
  const DenseMatrix& rhs = *rhsPtr;

  if (params == ROWS)
  {
    if (lhs.cols() != rhs.cols())
      return Outputs(); /// @todo: error

    DenseMatrixHandle output(boost::make_shared<DenseMatrix>(lhs.rows() + rhs.rows(), lhs.cols()));
    for (int i = 0; i < lhs.rows(); ++i)
      for (int j = 0; j < lhs.cols(); ++j)
        (*output)(i,j) = lhs(i,j);
    for (int i = 0; i < rhs.rows(); ++i)
      for (int j = 0; j < rhs.cols(); ++j)
        (*output)(i + lhs.rows(), j) = rhs(i,j);
    return output;
  }
  else // columns
  {
    if (lhs.rows() != rhs.rows())
      return Outputs(); /// @todo: error

    DenseMatrixHandle output(boost::make_shared<DenseMatrix>(lhs.rows(), lhs.cols() + rhs.cols()));
    for (int i = 0; i < lhs.rows(); ++i)
      for (int j = 0; j < lhs.cols(); ++j)
        (*output)(i,j) = lhs(i,j);
    for (int i = 0; i < rhs.rows(); ++i)
      for (int j = 0; j < rhs.cols(); ++j)
        (*output)(i, j + lhs.cols()) = rhs(i,j);
    return output;
  }
}

AlgorithmOutput AppendMatrixAlgorithm::run(const AlgorithmInput& input) const
{
  auto lhs = input.get<DenseMatrix>(Variables::FirstMatrix);
  auto rhs = input.get<DenseMatrix>(Variables::SecondMatrix);

  auto outputs = run(boost::make_tuple(lhs, rhs), Option(get(Variables::RowsOrColumns).toInt()));

  AlgorithmOutput output;
  output[Variables::ResultMatrix] = outputs;
  return output;
}
