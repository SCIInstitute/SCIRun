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


#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

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
  return mh->dynamic_type_name();
}

std::string matrixIs::whatType(const ComplexMatrixHandle& cmh)
{
  if (!cmh)
    return "<null>";
  // if (column(mh))
  //   return "DenseColumnMatrix";
  if (boost::dynamic_pointer_cast<ComplexDenseMatrix>(cmh))
    return "ComplexDenseMatrix";
  if (boost::dynamic_pointer_cast<ComplexSparseRowMatrix>(cmh))
    return "ComplexSparseRowMatrix";
  return cmh->dynamic_type_name();
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
