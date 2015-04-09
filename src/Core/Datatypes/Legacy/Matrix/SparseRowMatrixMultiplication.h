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



///
///@file  SparseRowMatrix.h
///@brief Sparse Row Matrices
/// 
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  November 1994
/// 
///@details See http://math.nist.gov/MatrixMarket/formats.html#MMformat
/// for more information about this sparse matrix format.
/// 


#ifndef CORE_DATATYPES_SPARSEROWMATRIXMULTIPLICATION_H
#define CORE_DATATYPES_SPARSEROWMATRIXMULTIPLICATION_H 

namespace SCIRun {

template <typename T>
void
SparseRowMatrixGeneric<T>::mult(const ColumnMatrix& x, ColumnMatrix& b,
                                index_type beg, index_type end,
                                int) const
{
  // Compute A*x=b
  ASSERT(x.nrows() == this->ncols_);
  ASSERT(b.nrows() == this->nrows_);
  if (beg==-1) beg = 0;
  if (end==-1) end = this->nrows_;
  T* xp=&x[0];
  T* bp=&b[0];


  index_type i, j;
  for(i=beg;i<end;i++)
  {
    T sum = 0;
    index_type row_idx=rows_[i];
    index_type next_idx=rows_[i+1];
    for(j=row_idx;j<next_idx;j++)
    {
      sum+=data_[j]*xp[columns_[j]];
    }
    bp[i]=sum;
  }  
}

template <typename T>
void
SparseRowMatrixGeneric<T>::mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
                                          index_type beg, index_type end, int) const
{
  // Compute At*x=b
  ASSERT(x.nrows() == this->nrows_);
  ASSERT(b.nrows() == this->ncols_);
  if (beg==-1) beg = 0;
  if (end==-1) end = this->nrows_;
  T* bp=&b[0];
  for (index_type i=beg; i<end; i++)
    bp[i] = 0;
  for (index_type j=0; j<this->nrows_; j++)
  {
    if (!x[j]) continue;
    T xj = x[j];
    index_type row_idx = rows_[j];
    index_type next_idx = rows_[j+1];
    index_type i=row_idx;
    for (; i<next_idx && columns_[i] < beg; i++);
    for (; i<next_idx && columns_[i] < end; i++)
      bp[columns_[i]] += data_[i]*xj;
  }
}

} // End namespace SCIRun

#endif
