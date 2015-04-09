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
///@file  ColumnMatrixFunctions.h
///@brief for RHS and LHS
/// 
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  July 1994
/// 

#ifndef CORE_DATATYPES_COLUMNMATRIXFUNCTIONS_H
#define CORE_DATATYPES_COLUMNMATRIXFUNCTIONS_H 

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Util/Assert.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {

  namespace Private
  {
    SCISHARE double linalg_norm2(size_type n, const double* data);
    SCISHARE void linalg_mult(size_type n, double* result, double* a, double* b);
    SCISHARE void linalg_sub(size_type n, double* result, double* a, double* b);
    SCISHARE void linalg_add(size_type n, double* result, double* a, double* b);
    SCISHARE double linalg_dot(size_type n, double* a, double* b);
    SCISHARE void linalg_smadd(size_type n, double* result, double s, double* a, double* b);
  }

  template <typename T>
  T ColumnMatrixGeneric<T>::vector_norm() const
  {
    return Private::linalg_norm2(this->nrows_, data_);
  }

  template <typename T>
  T ColumnMatrixGeneric<T>::vector_norm( index_type beg, index_type end) const
  {
    ASSERTRANGE(end, 0, this->nrows_+1);
    ASSERTRANGE(beg, 0, end);
    return Private::linalg_norm2((end-beg), data_+beg);
  }

  struct AbsoluteValue
  {
    double operator()(double x) const
    {
      return std::fabs(x);
    }
  };

  template <typename T>
  T ColumnMatrixGeneric<T>::infinity_norm() const
  {
    std::vector<T> absoluteValues(this->nrows());
    std::transform(this->begin(), this->end(), absoluteValues.begin(), AbsoluteValue());
    return *std::max_element(absoluteValues.begin(), absoluteValues.end());
  }

  template <typename T>
  void 
    Mult(ColumnMatrixGeneric<T>& result, const ColumnMatrixGeneric<T>& a, const ColumnMatrixGeneric<T>& b)
  {
    ASSERTEQ(result.nrows(), a.nrows());
    ASSERTEQ(result.nrows(), b.nrows());
    Private::linalg_mult(result.nrows(), result.get_data_pointer(), a.get_data_pointer(), b.get_data_pointer());
  }

  template <typename T>
  void 
    Mult(ColumnMatrixGeneric<T>& result, const ColumnMatrixGeneric<T>& a, const ColumnMatrixGeneric<T>& b,
    index_type beg, index_type end)
  {
    ASSERTEQ(result.nrows(), a.nrows());
    ASSERTEQ(result.nrows(), b.nrows());
    ASSERTRANGE(end, 0, result.nrows()+1);
    ASSERTRANGE(beg, 0, end);
    Private::linalg_mult(end-beg, result.get_data_pointer()+beg, a.get_data_pointer()+beg, b.get_data_pointer()+beg);
  }

  template <typename T>
  void 
    Sub(ColumnMatrixGeneric<T>& result, const ColumnMatrixGeneric<T>& a, const ColumnMatrixGeneric<T>& b)
  {
    ASSERTEQ(result.nrows(), a.nrows());
    ASSERTEQ(result.nrows(), b.nrows());
    Private::linalg_sub(result.nrows(), result.get_data_pointer(), a.get_data_pointer(), b.get_data_pointer());
  }

  template <typename T>
  void 
    Sub(ColumnMatrixGeneric<T>& result, const ColumnMatrixGeneric<T>& a, const ColumnMatrixGeneric<T>& b,
    index_type beg, index_type end)
  {
    ASSERTEQ(result.nrows(), a.nrows());
    ASSERTEQ(result.nrows(), b.nrows());
    Private::linalg_sub(end-beg, result.get_data_pointer()+beg, a.get_data_pointer()+beg, b.get_data_pointer()+beg);
  }

  template <typename T>
  void 
    ScMult_Add(ColumnMatrix& result, T s, const ColumnMatrix& a,
    const ColumnMatrix& b)
  {
    ASSERTEQ(result.nrows(), a.nrows());
    ASSERTEQ(result.nrows(), b.nrows());
    Private::linalg_smadd(result.nrows(), result.get_data_pointer(), s, a.get_data_pointer(), b.get_data_pointer());
  }

  template <typename T>
  void 
    ScMult_Add(ColumnMatrix& result, T s, const ColumnMatrix& a,
    const ColumnMatrix& b, 
    index_type beg, index_type end)
  {
    ASSERTEQ(result.nrows(), a.nrows());
    ASSERTEQ(result.nrows(), b.nrows());
    ASSERTRANGE(end, 0, result.nrows()+1);
    ASSERTRANGE(beg, 0, end);
    Private::linalg_smadd(end-beg, result.get_data_pointer()+beg, s, a.get_data_pointer()+beg, b.get_data_pointer()+beg);
  }

  template <typename T>
  T 
    Dot(const ColumnMatrixGeneric<T>& a, const ColumnMatrixGeneric<T>& b)
  {
    ASSERTEQ(a.nrows(), b.nrows());
    return Private::linalg_dot(a.nrows(), a.get_data_pointer(), b.get_data_pointer());
  }

  template <typename T>
  T 
    Dot(const ColumnMatrixGeneric<T>& a, const ColumnMatrixGeneric<T>& b,
    index_type beg, index_type end)
  {
    ASSERTEQ(a.nrows(), b.nrows());
    ASSERTRANGE(end, 0, a.nrows()+1);
    ASSERTRANGE(beg, 0, end);
    return Private::linalg_dot((end-beg), a.get_data_pointer()+beg, b.get_data_pointer()+beg);
  }

  template <typename T>
  void 
    Copy(ColumnMatrixGeneric<T>& out, const ColumnMatrixGeneric<T>& in)
  {
    ASSERTEQ(out.nrows(), in.nrows());
    for(index_type i=0; i<out.nrows(); i++)
    {
      out[i] = in[i];
    }
  }

  template <typename T>
  void 
    Copy(ColumnMatrixGeneric<T>& out, const ColumnMatrixGeneric<T>& in,
    index_type beg, index_type end)
  {
    ASSERTEQ(out.nrows(), in.nrows());
    ASSERTRANGE(end, 0, out.nrows()+1);
    ASSERTRANGE(beg, 0, end);
    for(index_type i=beg;i<end;i++)
    {
      out[i]=in[i];
    }
  }

  template <typename T>
  void 
    AddScMult(ColumnMatrixGeneric<T>& result, const ColumnMatrixGeneric<T>& a,
    T s, const ColumnMatrixGeneric<T>& b)
  {
    ASSERTEQ(result.nrows(), a.nrows());
    ASSERTEQ(result.nrows(), b.nrows());
    Private::linalg_smadd(result.nrows(), result.get_data_pointer(), s, b.get_data_pointer(), a.get_data_pointer());
  }

  template <typename T>
  void 
    Add(ColumnMatrixGeneric<T>& result, const ColumnMatrixGeneric<T>& a, const ColumnMatrixGeneric<T>& b)
  {
    ASSERTEQ(result.nrows(), a.nrows());
    ASSERTEQ(result.nrows(), b.nrows());
    Private::linalg_add(result.nrows(), result.get_data_pointer(), a.get_data_pointer(), b.get_data_pointer());
  }

  template <typename T>
  void Add(ColumnMatrixGeneric<T>& result, const ColumnMatrixGeneric<T>& a, const ColumnMatrixGeneric<T>& b,
    const ColumnMatrixGeneric<T>& c)
  {
    ASSERTEQ(result.nrows(), a.nrows());
    ASSERTEQ(result.nrows(), b.nrows());
    ASSERTEQ(result.nrows(), c.nrows());
    for(index_type i=0;i<result.nrows();i++)
    {
      result[i]=a[i]+b[i]+c[i];
    }
  }

  template <typename T>
  void Mult(ColumnMatrixGeneric<T>& result, const ColumnMatrixGeneric<T>& a, T s)
  {
    ASSERTEQ(result.nrows(), a.nrows());
    for(index_type i=0; i<result.nrows(); i++)
      result[i] = a[i]*s;
  }




} // End namespace SCIRun

#endif
