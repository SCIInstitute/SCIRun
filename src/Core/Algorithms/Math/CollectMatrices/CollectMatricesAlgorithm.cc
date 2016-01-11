/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Math/CollectMatrices/CollectMatricesAlgorithm.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

ALGORITHM_PARAMETER_DEF(Math, CollectAppendIndicator);
ALGORITHM_PARAMETER_DEF(Math, CollectRowIndicator);
ALGORITHM_PARAMETER_DEF(Math, CollectPrependIndicator);
ALGORITHM_PARAMETER_DEF(Math, ClearCollectMatricesOutput);

MatrixHandle
CollectDenseMatricesAlgorithm::concat_cols(MatrixHandle m1H, MatrixHandle m2H) const
{
  DenseMatrixHandle out(new DenseMatrix(m1H->nrows(), m1H->ncols()+m2H->ncols()));
  copy_matrix(m1H, *out);

  for (index_type r = 0; r < m2H->nrows(); r++)
  {
    for (index_type c = m1H->ncols(); c < m1H->ncols()+m2H->ncols(); c++)
    {
      (*out)(r, c) = m2H->get(r,c - m1H->ncols());
    }
  }
  return out;
}

MatrixHandle
CollectDenseMatricesAlgorithm::concat_rows(MatrixHandle m1H, MatrixHandle m2H) const
{
  DenseMatrixHandle out(new DenseMatrix(m1H->nrows()+m2H->nrows(), m1H->ncols()));
  copy_matrix(m1H, *out);

  for (index_type r = m1H->nrows(); r < m1H->nrows()+m2H->nrows(); r++)
  {
    for (index_type c = 0; c < m2H->ncols(); c++)
    {
      (*out)(r, c) = m2H->get(r - m1H->nrows(), c);
    }
  }
  return out;
}

void
CollectDenseMatricesAlgorithm::copy_matrix(MatrixHandle mh, DenseMatrix& out) const
{
  for (index_type r = 0; r < mh->nrows(); r++)
    for (index_type c = 0; c < mh->ncols(); c++)
      out(r, c) = mh->get(r,c);
}

MatrixHandle
CollectSparseRowMatricesAlgorithm::concat_cols(MatrixHandle m1H, MatrixHandle m2H) const
{
  check_args(m1H, m2H);

  const size_type newRows = m1H->nrows();
  const size_type newCols = m1H->ncols() + m2H->ncols();

  SparseRowMatrixFromMap::Values shiftedValues;
  auto m1sparse = matrix_cast::as_sparse(m1H);
  auto m2sparse = matrix_cast::as_sparse(m2H);

  copy_shifted_contents(m2sparse, shiftedValues, 0, m1sparse->ncols());

  return SparseRowMatrixFromMap::appendToSparseMatrix(newRows, newCols, *m1sparse, shiftedValues);
}

MatrixHandle
CollectSparseRowMatricesAlgorithm::concat_rows(MatrixHandle m1H, MatrixHandle m2H) const
{
  check_args(m1H, m2H);

  auto newRows = m1H->nrows() + m2H->nrows();
  auto newCols = m1H->ncols();

  SparseRowMatrixFromMap::Values shiftedValues;
  auto m1sparse = matrix_cast::as_sparse(m1H);
  auto m2sparse = matrix_cast::as_sparse(m2H);

  copy_shifted_contents(m2sparse, shiftedValues, m1sparse->nrows(), 0);

  return SparseRowMatrixFromMap::appendToSparseMatrix(newRows, newCols, *m1sparse, shiftedValues);
}

void
CollectSparseRowMatricesAlgorithm::check_args(MatrixHandle m1H, MatrixHandle m2H) const
{
  if (!matrix_is::sparse(m1H) || !matrix_is::sparse(m2H))
    THROW_ALGORITHM_INPUT_ERROR("Both matrices to concatenate must be sparse.");
}

void
CollectSparseRowMatricesAlgorithm::copy_shifted_contents(SparseRowMatrixHandle sparse, SparseRowMatrixFromMap::Values& shiftedValues,
  size_t rowShift, size_t columnShift) const
{
  size_t count = 0;
  size_t nextRow;
  for (size_t r = 0; r < sparse->nrows(); ++r)
  {
    nextRow = sparse->get_rows()[r+1];
    while (count < nextRow)
    {
      shiftedValues[r + rowShift][sparse->get_cols()[count] + columnShift] = sparse->valuePtr()[count];
      count++;
    }
  }
}
