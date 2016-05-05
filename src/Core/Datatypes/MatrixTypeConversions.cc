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

const double convertMatrix::zero_threshold = 1.00000e-08f;

DenseMatrixHandle castMatrix::toDense(const MatrixHandle& mh)
{
  return to<DenseMatrix>(mh);
}

SparseRowMatrixHandle castMatrix::toSparse(const MatrixHandle& mh)
{
  return to<SparseRowMatrix>(mh);
}

DenseColumnMatrixHandle castMatrix::toColumn(const MatrixHandle& mh)
{
  return to<DenseColumnMatrix>(mh);
}

bool matrixIs::dense(const MatrixHandle& mh)
{
  return castMatrix::toDense(mh) != nullptr;
}

bool matrixIs::sparse(const MatrixHandle& mh)
{
  return castMatrix::toSparse(mh) != nullptr;
}

bool matrixIs::column(const MatrixHandle& mh)
{
  return castMatrix::toColumn(mh) != nullptr;
}

std::string matrixIs::whatType(const MatrixHandle& mh)
{
  if (!mh)
    return "<null>";
  if (column(mh))
    return "DenseColumnMatrix";
  if (dense(mh))
    return "DenseMatrix";
  if (sparse(mh))
    return "SparseRowMatrix";
  return typeid(*mh).name();
}

std::string matrixIs::whatType(const ComplexDenseMatrixHandle&)
{
  return "ComplexDenseMatrix";
}

MatrixTypeCode matrixIs::typeCode(const MatrixHandle& mh)
{
  if (!mh)
    return NULL_MATRIX;
  if (column(mh))
    return COLUMN;
  if (dense(mh))
    return DENSE;
  if (sparse(mh))
    return SPARSE_ROW;
  return UNKNOWN;
}

/* Old Code - disabled
DenseColumnMatrixHandle convertMatrix::toColumn(const MatrixHandle& mh)
{
auto col = castMatrix::toColumn(mh);
if (col)
return col;

auto dense = castMatrix::toDense(mh);
if (dense)
return boost::make_shared<DenseColumnMatrix>(dense->col(0));

return DenseColumnMatrixHandle();
}
*/

DenseColumnMatrixHandle convertMatrix::toColumn(const MatrixHandle& mh)
{
  auto col = castMatrix::toColumn(mh);
  if (col)
    return col;

  auto dense = castMatrix::toDense(mh);
  if (dense)
    return boost::make_shared<DenseColumnMatrix>(dense->col(0));

  auto sparse = castMatrix::toSparse(mh);
  if (sparse)
  {
    DenseColumnMatrix dense_col(DenseColumnMatrix::Zero(sparse->nrows()));
    for (auto i = 0; i < sparse->nrows(); i++)
      dense_col(i, 0) = sparse->coeff(i, 0);

    return boost::make_shared<DenseColumnMatrix>(dense_col);
  }

  return DenseColumnMatrixHandle();
}

DenseMatrixHandle convertMatrix::toDense(const MatrixHandle& mh)
{
  auto dense = castMatrix::toDense(mh);
  if (dense)
    return dense;

  auto col = castMatrix::toColumn(mh);
  if (col)
    return boost::make_shared<DenseMatrix>(*col);

  auto sparse = castMatrix::toSparse(mh);
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

SparseRowMatrixHandle convertMatrix::toSparse(const MatrixHandle& mh)
{
  auto sparse = castMatrix::toSparse(mh);
  if (sparse)
    return sparse;

  auto col = castMatrix::toColumn(mh);
  if (col)
  {
    SparseRowMatrixFromMap::Values data;
    for (auto i = 0; i<col->nrows(); i++)
      if (fabs((*col)(i, 0)) > zero_threshold)
        data[i][0] = (*col)(i, 0);

    return SparseRowMatrixFromMap::make(col->nrows(), 1, data);
  }

  auto dense = castMatrix::toDense(mh);
  if (dense)
  {
    return fromDenseToSparse(*dense);
  }

  return SparseRowMatrixHandle();
}

SparseRowMatrixHandle convertMatrix::fromDenseToSparse(const DenseMatrix& dense)
{
  SparseRowMatrixFromMap::Values data;
  for (auto i = 0; i < dense.nrows(); i++)
    for (auto j = 0; j < dense.ncols(); j++)
      if (fabs(dense(i, j))>zero_threshold)
        data[i][j] = dense(i, j);

  return SparseRowMatrixFromMap::make(dense.nrows(), dense.ncols(), data);
}
