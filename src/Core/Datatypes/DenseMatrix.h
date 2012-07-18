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


#ifndef CORE_DATATYPES_DENSE_MATRIX_H
#define CORE_DATATYPES_DENSE_MATRIX_H 

#include <Core/Datatypes/Matrix.h>

namespace SCIRun {
namespace Domain {
namespace Datatypes {

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric()
{
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(size_t nrows, size_t ncols) : matrix_(nrows, ncols)
{
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(size_t nrows, size_t ncols, const T& val) : matrix_(nrows, ncols, val)
{
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(const DenseMatrixGeneric<T>& copy) : matrix_(copy.matrix_)
{
}

template <typename T>
DenseMatrixGeneric<T>& DenseMatrixGeneric<T>::operator=(const DenseMatrixGeneric<T>& copy)
{
  if (this != &copy)
  {
    matrix_ = copy.matrix_;
  }
  return *this;
}

template <typename T>
size_t DenseMatrixGeneric<T>::nrows() const
{
  return matrix_.size1();
}

template <typename T>
size_t DenseMatrixGeneric<T>::ncols() const
{
  return matrix_.size2();
}

template <typename T>
DenseMatrixGeneric<T>* DenseMatrixGeneric<T>::clone() const
{
  return new DenseMatrixGeneric(*this);
}

template <typename T>
T& DenseMatrixGeneric<T>::operator()(size_t r, size_t c)
{
  return matrix_(r,c);
}

template <typename T>
const T& DenseMatrixGeneric<T>::operator()(size_t r, size_t c) const
{
  return matrix_(r,c);
}

template <typename T>
T DenseMatrixGeneric<T>::min() const
{
  return *std::min_element(matrix_.data().begin(), matrix_.data().end());
}

template <typename T>
T DenseMatrixGeneric<T>::max() const
{
  return *std::max_element(matrix_.data().begin(), matrix_.data().end());
}

template <typename T>
/*static*/ DenseMatrixGeneric<T> DenseMatrixGeneric<T>::zero_matrix(size_t nrows, size_t ncols)
{
  return DenseMatrixGeneric(nrows, ncols, 0);
}

template <typename T>
DenseMatrixGeneric<T>& DenseMatrixGeneric<T>::operator+=(const DenseMatrixGeneric<T>& rhs)
{
  matrix_ += rhs.matrix_;
  return *this;
}

template <typename T>
DenseMatrixGeneric<T>& DenseMatrixGeneric<T>::operator-=(const DenseMatrixGeneric<T>& rhs)
{
  matrix_ -= rhs.matrix_;
  return *this;
}

template <typename T>
DenseMatrixGeneric<T>& DenseMatrixGeneric<T>::operator*=(const DenseMatrixGeneric<T>& rhs)
{
  matrix_ = prod(matrix_, rhs.matrix_);
  return *this;
}

template <typename T>
DenseMatrixGeneric<T>& DenseMatrixGeneric<T>::operator*=(const T& scalar)
{
  matrix_ *= scalar;
  return *this;
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(const MatrixInternal& internals) : matrix_(internals)
{
}

template <typename T>
DenseMatrixGeneric<T>* DenseMatrixGeneric<T>::make_transpose() const
{
  return new DenseMatrixGeneric(trans(matrix_));
}

template <typename T>
void DenseMatrixGeneric<T>::resize(size_t nrows, size_t ncols)
{
  matrix_.resize(nrows, ncols);
}

template <typename T>
void DenseMatrixGeneric<T>::clear()
{
  matrix_.clear();
}

template <typename T>
void DenseMatrixGeneric<T>::transpose_in_place()
{
  matrix_ = trans(matrix_);
}

//operators
template <typename T>
DenseMatrixGeneric<T> operator+(const DenseMatrixGeneric<T>& lhs, const DenseMatrixGeneric<T>& rhs)
{
  DenseMatrixGeneric<T> sum(lhs);
  sum += rhs;
  return sum;
}

template <typename T>
DenseMatrixGeneric<T> operator*(const DenseMatrixGeneric<T>& lhs, const DenseMatrixGeneric<T>& rhs)
{
  DenseMatrixGeneric<T> product(lhs);
  product *= rhs;
  return product;
}

template <typename T, typename U>
DenseMatrixGeneric<T> operator*(const U& scalar, const DenseMatrixGeneric<T>& rhs)
{
  DenseMatrixGeneric<T> scaled(rhs);
  scaled *= scalar;
  return scaled;
}

template <typename T, typename U>
DenseMatrixGeneric<T> operator*(const DenseMatrixGeneric<T>& lhs, const U& scalar)
{
  return scalar * lhs;
}

template <typename T>
DenseMatrixGeneric<T> operator-(const DenseMatrixGeneric<T>& m)
{
  return -1 * m;
}

template <typename T>
DenseMatrixGeneric<T> operator-(const DenseMatrixGeneric<T>& lhs, const DenseMatrixGeneric<T>& rhs)
{
  DenseMatrixGeneric<T> diff(lhs);
  diff -= rhs;
  return diff;
}

template <typename T>
DenseMatrixGeneric<T> transpose(const DenseMatrixGeneric<T>& m)
{
  DenseMatrixGeneric<T> mt(m);
  mt.transpose_in_place();
  return mt;
}





}}}


#endif
