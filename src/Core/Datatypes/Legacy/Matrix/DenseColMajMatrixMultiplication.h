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



/*
 *@file  DenseColMajMatrixMultiplication.h
 *@brief DenseColMaj matrices
 *
 *@author
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *@date  October 1994
 *
 */

#ifndef CORE_DATATYPES_DENSECOLMAJMATRIXMULTIPLICATION_H
#define CORE_DATATYPES_DENSECOLMAJMATRIXMULTIPLICATION_H 

namespace SCIRun {

template <typename T>
void
DenseColMajMatrixGeneric<T>::mult(const ColumnMatrix& x, ColumnMatrix& b,
                           index_type beg, index_type end,
                           int spVec) const
{
  // Compute A*x=b
  ASSERTEQ(x.nrows(), this->ncols_);
  ASSERTEQ(b.nrows(), this->nrows_);
  if (beg == -1) beg = 0;
  if (end == -1) end = this->nrows_;
  index_type i, j;
  if (!spVec)
  {
    for (i=beg; i<end; i++)
    {
      double sum = 0.0;
      for (j=0; j<this->ncols_; j++)
      {
        sum += iget(i, j) * x[j];
      }
      b[i] = sum;
    }
  }
  else
  {
    for (i=beg; i<end; i++) b[i] = 0.0;
    for (j=0; j<this->ncols_; j++)
    {
      if (x[j])
      {
        for (i=beg; i<end; i++)
        {
          b[i] += iget(i, j) * x[j];
        }
      }
    }
  }
}

template <typename T>
void
DenseColMajMatrixGeneric<T>::mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
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
      double sum = 0.0;
      for (j=0; j<this->nrows_; j++)
      {
        sum += iget(j, i) * x[j];
      }
      b[i] = sum;
    }
  }
  else
  {
    for (i=beg; i<end; i++) b[i] = 0.0;
    for (j=0; j<this->nrows_; j++)
    {
      if (x[j])
      {
        for (i=beg; i<end; i++)
        {
          b[i] += iget(j, i) * x[j];
        }
      }
    }
  }
}

} // End namespace SCIRun

#endif
