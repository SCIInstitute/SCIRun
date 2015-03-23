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

///@brief 
///@file  DenseMatrixMultiplication.h

#ifndef CORE_DATATYPES_DENSEMATRIXMULTIPLICATION_H
#define CORE_DATATYPES_DENSEMATRIXMULTIPLICATION_H 

#if defined(HAVE_CBLAS)
#if defined(__APPLE__)
#include <vecLib/cblas.h>
#else
extern "C"{
#include <cblas.h>
}
#endif
#endif

namespace SCIRun {

template <typename T>
void
DenseMatrixGeneric<T>::mult(const ColumnMatrix& x, ColumnMatrix& b,
                            index_type beg, index_type end,
                            int spVec) const
{
  ASSERTEQ(x.nrows(), this->ncols_);
  ASSERTEQ(b.nrows(), this->nrows_);

  if (beg == -1) beg = 0;
  if (end == -1) end = this->nrows_;

#if defined(HAVE_CBLAS)
  double ALPHA = 1.0;
  double BETA = 0.0;
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, (end-beg),
    1, this->ncols_, ALPHA, dataptr_+(beg*this->ncols_), this->ncols_,
    x.get_data_pointer(), 1, BETA,
    b.get_data_pointer()+beg, 1);
#else

  double* xdata = x.get_data_pointer();
  double* bdata = b.get_data_pointer();

  size_type m8 = (this->ncols_)/8;
  size_type m = (this->ncols_)-m8*8;

  index_type i, j;
  if(!spVec)
  {
    for (i=beg; i<end; i++)
    {
      double sum=0;
      double* row=data[i];
      double* xd=xdata;

      for (j=0; j<m8; j++)
      {
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
      }

      for (j=0; j<m; j++)
      {
        sum+=(*row)*(*xd);
        row++; xd++;
      }
      (*bdata)=sum;
      bdata++;
    }
  }
  else
  {
    for (i=beg; i<end; i++) b[i]=0;
    for (j=0; j<this->ncols_; j++)
    {
      if (x[j])
        for (i=beg; i<end; i++)
        {
          b[i]+=data[i][j]*x[j];
        }
    }
  }

#endif
}

template <typename T>
void
DenseMatrixGeneric<T>::multiply(ColumnMatrix& x, ColumnMatrix& b) const
{
  index_type i, j;

  double* xdata = x.get_data_pointer();
  double* bdata = b.get_data_pointer();

  size_type m8 = (this->ncols_)/8;
  size_type m = (this->ncols_)-m8*8;

  for (i=0; i<this->nrows_; i++)
  {
    double sum=0;
    double* row = data[i];
    double* xd = xdata;
    for (j=0; j<m8; j++)
    {
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
    }

    for (j=0; j<m; j++)
    {
      sum+=(*row)*(*xd);
      row++; xd++;
    }

    *bdata=sum;
    bdata++;
  }
}

template <typename T>
void
DenseMatrixGeneric<T>::mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
                                      index_type beg, index_type end,
                                      int spVec) const
{
  // Compute At*x=b
  ASSERT(x.nrows() == this->nrows_);
  ASSERT(b.nrows() == this->ncols_);
  if (beg == -1) beg = 0;
  if (end == -1) end = this->ncols_;
  index_type i, j;
  if (!spVec)
  {
    for (i=beg; i<end; i++)
    {
      double sum=0;
      for (j=0; j<this->nrows_; j++)
      {
        sum+=data[j][i]*x[j];
      }
      b[i]=sum;
    }
  }
  else
  {
    for (i=beg; i<end; i++) b[i]=0;
    for (j=0; j<this->nrows_; j++)
      if (x[j])
      {
        double *row=data[j];
        for (i=beg; i<end; i++)
          b[i]+=row[i]*x[j];
      }
  }
}

template <typename T>
DenseMatrix*
DenseMatrixGeneric<T>::make_diagonal_from_column(const ColumnMatrix& column, size_type rows, size_type cols)
{
  DenseMatrix* result = zero_matrix(rows, cols);
  for (size_type i = 0; i < column.nrows(); ++i)
    (*result)[i][i] = column[i];

  return result;
}

} // End namespace SCIRun

#endif
