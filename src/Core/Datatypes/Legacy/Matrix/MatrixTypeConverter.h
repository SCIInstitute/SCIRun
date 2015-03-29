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
/// @todo Documentation Core/Datatypes/Legacy/Matrix/MatrixTypeConverter.h

#ifndef CORE_DATATYPES_MATRIX_TYPE_CONVERTER_H
#define CORE_DATATYPES_MATRIX_TYPE_CONVERTER_H

#include <limits>
#include <functional>

#include <Core/Datatypes/share.h>

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseColMajMatrix.h>
#include <Core/Datatypes/DenseColMajMatrixMultiplication.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseMatrixMultiplication.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixMultiplication.h>

#include <Core/Math/MiscMath.h>

namespace SCIRun {

// No conversion is done.
// NULL is returned if the matrix is not of the appropriate type.
class SCISHARE matrix_cast
{
public:
  template <class ToType>
  static ToType* to(const MatrixHandle& matrix, typename boost::enable_if<boost::is_base_of<MatrixBase, ToType> >::type* = 0)
  {
    return SCI_DATATYPE_CAST<ToType*>(matrix.get_rep());
  }

  static inline DenseMatrix* as_dense(const MatrixHandle& mh)
  {
    return to<DenseMatrix>(mh);
  }

  static inline SparseRowMatrix* as_sparse(const MatrixHandle& mh)
  {
    return to<SparseRowMatrix>(mh);
  }

  static inline ColumnMatrix* as_column(const MatrixHandle& mh)
  {
    return to<ColumnMatrix>(mh);
  }

  static inline DenseColMajMatrix* as_dense_col_maj(const MatrixHandle& mh)
  {
    return to<DenseColMajMatrix>(mh);
  }
private:
  matrix_cast();
};

class SCISHARE matrix_is
{
public:
  // Test to see if the matrix is this subtype.
  static inline bool dense(const MatrixHandle& mh) { return matrix_cast::as_dense(mh) != 0; }
  static inline bool sparse(const MatrixHandle& mh) { return matrix_cast::as_sparse(mh) != 0; }
  static inline bool column(const MatrixHandle& mh) { return matrix_cast::as_column(mh) != 0; }
  static inline bool dense_col_maj(const MatrixHandle& mh) { return matrix_cast::as_dense_col_maj(mh) != 0; }
private:
  matrix_is();
};

/// @todo: these will be moved to a separate class to break compilation dependency on the subclasses of Matrix.

template <typename T>
DenseMatrix *
DenseColMajMatrixGeneric<T>::dense()
{
  DenseMatrix *m = new DenseMatrix(this->nrows_, this->ncols_);
  for (index_type i = 0; i < this->nrows_; i++)
    for (index_type j = 0; j < this->ncols_; j++)
      (*m)[i][j] = iget(i, j);
  return m;
}

template <typename T>
ColumnMatrix *
DenseColMajMatrixGeneric<T>::column()
{
  ColumnMatrix *cm = new ColumnMatrix(this->nrows_);
  for (index_type i=0; i < this->nrows_; i++)
    (*cm)[i] = iget(i, 0);
  return cm;
}

template <typename T>
SparseRowMatrix *
DenseColMajMatrixGeneric<T>::sparse()
{
  size_type nnz = this->number_of_nonzero_elements();

  SparseRowMatrix::Data sparseData(this->nrows_ + 1, nnz);

  const SparseRowMatrix::Rows& rows = sparseData.rows();
  const SparseRowMatrix::Columns& columns = sparseData.columns();
  const SparseRowMatrix::Storage& a = sparseData.data();

  index_type count = 0;
  for (index_type r=0; r<this->nrows_; r++)
  {
    rows[r] = count;
    for (index_type c=0; c<this->ncols_; c++)
      if ( nonzero(iget(r, c)) )
      {
        columns[count] = c;
        a[count] = iget(r, c);
        count++;
      }
  }
  rows[this->nrows_] = count;

  return new SparseRowMatrix(this->nrows_, this->ncols_, sparseData, nnz);
}

template <typename T>
DenseColMajMatrixGeneric<T>*
DenseColMajMatrixGeneric<T>::dense_col_maj()
{
  return this;
}


template <typename T>
SparseRowMatrixGeneric<T>*
SparseRowMatrixGeneric<T>::sparse()
{
  return this;
}


template <typename T>
DenseMatrix *
SparseRowMatrixGeneric<T>::dense()
{
  DenseMatrix *dm = new DenseMatrix(this->nrows_, this->ncols_);
  if (this->nrows_ == 0) return dm;

  dm->zero();
  index_type count=0;
  index_type nextRow;
  for (index_type r=0; r<this->nrows_; r++)
  {
    nextRow = rows_[r+1];
    while (count < nextRow)
    {
      (*dm)[r][columns_[count]]=data_[count];
      count++;
    }
  }
  return dm;
}

template <typename T>
DenseColMajMatrix*
SparseRowMatrixGeneric<T>::dense_col_maj()
{
  DenseColMajMatrix* dm = new DenseColMajMatrix(this->nrows_, this->ncols_);
  if (this->nrows_ == 0) return dm;
  dm->zero();
  index_type count = 0;
  index_type nextRow;
  for (index_type r = 0; r<this->nrows_; r++)
  {
    nextRow = rows_[r+1];
    while (count < nextRow)
    {
      dm->iget(r, columns_[count]) = data_[count];
      count++;
    }
  }
  return dm;
}

template <typename T>
ColumnMatrix*
SparseRowMatrixGeneric<T>::column()
{
  ColumnMatrix *cm = new ColumnMatrix(this->nrows_);
  if (this->nrows_)
  {
    cm->zero();
    for (int i=0; i<this->nrows_; i++)
    {
      // If the first column entry for the row is a zero.
      if (columns_[rows_[i]] == 0)
      {
        (*cm)[i] = data_[rows_[i]];
      }
      else
      {
        (*cm)[i] = 0;
      }
    }
  }
  return cm;
}

template <typename T>
ColumnMatrixGeneric<T> *
ColumnMatrixGeneric<T>::column()
{
  return this;
}

template <typename T>
DenseMatrix *
ColumnMatrixGeneric<T>::dense()
{
  DenseMatrix *dm = new DenseMatrix(this->nrows_, 1);
  for (index_type i = 0; i < this->nrows_; i++)
  {
    (*dm)[i][0] = data_[i];
  }
  return dm;
}

template <typename T>
DenseColMajMatrixGeneric<T>*
ColumnMatrixGeneric<T>::dense_col_maj()
{
  DenseColMajMatrixGeneric<T>* dm = new DenseColMajMatrixGeneric<T>(this->nrows_, 1);
  for (index_type i = 0; i < this->nrows_; i++)
  {
    dm->iget(i, 0) = data_[i];
  }
  return dm;
}

template <typename T>
SparseRowMatrixGeneric<T>*
ColumnMatrixGeneric<T>::sparse()
{
  size_type nnz = this->number_of_nonzero_elements();

  typename SparseRowMatrixGeneric<T>::Data sparseData(this->nrows_ + 1, nnz);

  index_type count = 0;
  const SparseRowMatrix::Rows& rows = sparseData.rows();
  const SparseRowMatrix::Columns& columns = sparseData.columns();
  const SparseRowMatrix::Storage& a = sparseData.data();
  for (index_type r = 0; r < this->nrows_; r++) 
  {
    rows[r] = count;
    if ( nonzero(data_[r]) ) 
    {
      columns[count]=0;
      a[count]=data_[r];
      count++;
    }
  }
  rows[this->nrows_] = count;
  return new SparseRowMatrixGeneric<T>(this->nrows_, 1, sparseData, nnz);
}


template <typename T>
DenseMatrix*
DenseMatrixGeneric<T>::dense()
{
  return this;
}

template <typename T>
DenseColMajMatrix*
DenseMatrixGeneric<T>::dense_col_maj()
{
  DenseColMajMatrix* dm = new DenseColMajMatrix(this->nrows_, this->ncols_);
  if (dm == 0) return (0);

  index_type m1 = static_cast<index_type>(this->ncols_);
  index_type m2 = static_cast<index_type>(this->nrows_);

  T *v1 = dataptr_;
  T *v2 = dm->get_data_pointer();
  for (index_type i = 0; i < this->nrows_; i++)
  {
    for (index_type j = 0; j < this->ncols_; j++)
    {
      v2[i+j*m2] = v1[i*m1+j];
    }
  }
  return dm;
}

template <typename T>
ColumnMatrix *
DenseMatrixGeneric<T>::column()
{
  ColumnMatrix *cm = new ColumnMatrix(this->nrows_);
  for (index_type i=0; i<static_cast<index_type>(this->nrows_); i++)
    (*cm)[i] = data[i][0];
  return cm;
}

template <typename T>
SparseRowMatrix *
DenseMatrixGeneric<T>::sparse()
{
  size_type nnz = this->number_of_nonzero_elements();
  SparseRowMatrix::Data sparseData(this->nrows_ + 1, nnz);

  if (!sparseData.allocated())
  {
    std::cerr << "Could not allocate memory for rows, columns, or non-zero elements buffer "
      << __FILE__ << ": " << __LINE__ << std::endl;

    return 0;
  }

  index_type count = 0;
  const SparseRowMatrix::Rows& rows = sparseData.rows();
  const SparseRowMatrix::Columns& columns = sparseData.columns();
  const SparseRowMatrix::Storage& a = sparseData.data();
  for (index_type r = 0; r < this->nrows_; r++)
  {
    rows[r] = count;
    for (index_type c = 0; c < this->ncols_; c++)
      if ( nonzero(data[r][c]) )
      {
        columns[count]=c;
        a[count]=data[r][c];
        count++;
      }
  }
  rows[this->nrows_] = count;

  return new SparseRowMatrix(this->nrows_, this->ncols_, sparseData, nnz);
}


} // End namespace SCIRun

#endif
