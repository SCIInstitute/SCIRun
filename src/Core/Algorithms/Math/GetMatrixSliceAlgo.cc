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

#include <Core/Algorithms/Math/GetMatrixSliceAlgo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <boost/lexical_cast.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

ALGORITHM_PARAMETER_DEF(Math, IsSliceColumn);
ALGORITHM_PARAMETER_DEF(Math, SliceIndex);

GetMatrixSliceAlgo::GetMatrixSliceAlgo()
{
  addParameter(Parameters::IsSliceColumn, true);
  addParameter(Parameters::SliceIndex, 0);
}

AlgorithmOutput GetMatrixSliceAlgo::run_generic(const AlgorithmInput& input) const
{
  auto inputMatrix = input.get<Matrix>(Variables::InputMatrix);
  auto outputMatrix = runImpl(inputMatrix, get(Parameters::SliceIndex).toInt(), get(Parameters::IsSliceColumn).toBool());

  AlgorithmOutput output;
  output[Variables::OutputMatrix] = outputMatrix;

  return output;
}

MatrixHandle GetMatrixSliceAlgo::runImpl(MatrixHandle matrix, int index, bool getColumn) const
{
  ENSURE_ALGORITHM_INPUT_NOT_NULL(matrix, "Input matrix");
  if (getColumn)
  {
    checkIndex(index, matrix->ncols());

    // dense case only now
    auto dense = matrix_cast::as_dense(matrix);
    if (dense)
      return boost::make_shared<DenseMatrix>(dense->col(index));
    else
    {
      auto sparse = matrix_cast::as_sparse(matrix);
      if (sparse)
        return boost::make_shared<SparseRowMatrix>(sparse->col(index));
      return nullptr;
    }
  }
  else
  {
    checkIndex(index, matrix->nrows());

    // dense case only now
    auto dense = matrix_cast::as_dense(matrix);
    if (dense)
      return boost::make_shared<DenseMatrix>(dense->row(index));
    else
    {
      auto sparse = matrix_cast::as_sparse(matrix);
      if (sparse)
        return boost::make_shared<SparseRowMatrix>(sparse->row(index));
      return nullptr;
    }
  }
}

void GetMatrixSliceAlgo::checkIndex(int index, int max) const
{
  if (index < 0 || index >= max)
    THROW_ALGORITHM_INPUT_ERROR("Slice index out of range: " + boost::lexical_cast<std::string>(index));
}
