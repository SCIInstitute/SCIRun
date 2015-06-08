/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

const double matrix_convert::zero_threshold = 1.00000e-08f;

DenseMatrixHandle matrix_cast::as_dense(const MatrixHandle& mh)
{
  return to<DenseMatrix>(mh);
}

SparseRowMatrixHandle matrix_cast::as_sparse(const MatrixHandle& mh)
{
  return to<SparseRowMatrix>(mh);
}

DenseColumnMatrixHandle matrix_cast::as_column(const MatrixHandle& mh)
{
  return to<DenseColumnMatrix>(mh);
}

bool matrix_is::dense(const MatrixHandle& mh)
{
  return matrix_cast::as_dense(mh) != 0;
}

bool matrix_is::sparse(const MatrixHandle& mh)
{
  return matrix_cast::as_sparse(mh) != 0;
}

bool matrix_is::column(const MatrixHandle& mh)
{
  return matrix_cast::as_column(mh) != 0;
}

std::string matrix_is::whatType(const MatrixHandle& mh)
{
  if (!mh)
    return "<null>";
  if (matrix_is::column(mh))
    return "DenseColumnMatrix";
  else if (matrix_is::dense(mh))
    return "DenseMatrix";
  else if (matrix_is::sparse(mh))
    return "SparseRowMatrix";
  return typeid(*mh).name();
}

MatrixTypeCode matrix_is::typeCode(const MatrixHandle& mh)
{
  if (!mh)
    return NULL_MATRIX;
  if (matrix_is::column(mh))
    return COLUMN;
  else if (matrix_is::dense(mh))
    return DENSE;
  else if (matrix_is::sparse(mh))
    return SPARSE_ROW;
  return UNKNOWN;
}

/* Old Code - disabled
DenseColumnMatrixHandle matrix_convert::to_column(const MatrixHandle& mh)
{
auto col = matrix_cast::as_column(mh);
if (col)
return col;

auto dense = matrix_cast::as_dense(mh);
if (dense)
return boost::make_shared<DenseColumnMatrix>(dense->col(0));

return DenseColumnMatrixHandle();
}
*/

DenseColumnMatrixHandle matrix_convert::to_column(const MatrixHandle& mh)
{
  auto col = matrix_cast::as_column(mh);
  if (col)
    return col;

  auto dense = matrix_cast::as_dense(mh);
  if (dense)
    return boost::make_shared<DenseColumnMatrix>(dense->col(0));

  auto sparse = matrix_cast::as_sparse(mh);
  if (sparse)
  {
    DenseColumnMatrix dense_col(DenseColumnMatrix::Zero(sparse->nrows()));
    for (auto i = 0; i < sparse->nrows(); i++)
      dense_col(i, 0) = sparse->coeff(i, 0);

    return boost::make_shared<DenseColumnMatrix>(dense_col);
  }

  return DenseColumnMatrixHandle();
}

DenseMatrixHandle matrix_convert::to_dense(const MatrixHandle& mh)
{
  auto dense = matrix_cast::as_dense(mh);
  if (dense)
    return dense;

  auto col = matrix_cast::as_column(mh);
  if (col)
    return boost::make_shared<DenseMatrix>(*col);

  auto sparse = matrix_cast::as_sparse(mh);
  if (sparse)
  {
    //warn user or log a message?
    DenseMatrixHandle dense_matrix(new DenseMatrix(sparse->nrows(), sparse->ncols(), 0));

    for (index_type row = 0; row < sparse->outerSize(); row++)
    {
      for (SparseRowMatrix::InnerIterator it(*sparse, row); it; ++it)
        (*dense_matrix)(row, it.index()) = it.value();
    }
    return dense_matrix;
  }

  return DenseMatrixHandle();
}

SparseRowMatrixHandle matrix_convert::to_sparse(const MatrixHandle& mh)
{
  auto sparse = matrix_cast::as_sparse(mh);
  if (sparse)
    return sparse;

  auto col = matrix_cast::as_column(mh);
  if (col)
  {
    SparseRowMatrixFromMap::Values data;
    for (auto i = 0; i<col->nrows(); i++)
      if (fabs((*col)(i, 0)) > zero_threshold)
        data[i][0] = (*col)(i, 0);

    return SparseRowMatrixFromMap::make(col->nrows(), 1, data);
  }

  auto dense = matrix_cast::as_dense(mh);
  if (dense)
  {
    return denseToSparse(*dense);
  }

  return SparseRowMatrixHandle();
}

SparseRowMatrixHandle matrix_convert::denseToSparse(const DenseMatrix& dense)
{
  SparseRowMatrixFromMap::Values data;
  for (auto i = 0; i < dense.nrows(); i++)
    for (auto j = 0; j < dense.ncols(); j++)
      if (fabs(dense(i, j))>zero_threshold)
        data[i][j] = dense(i, j);

  return SparseRowMatrixFromMap::make(dense.nrows(), dense.ncols(), data);
}
