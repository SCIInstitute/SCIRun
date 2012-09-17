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


#ifndef CORE_DATATYPES_SPARSE_MATRIX_H
#define CORE_DATATYPES_SPARSE_MATRIX_H 

#include <Core/Datatypes/Matrix.h>
#include <Eigen/SparseCore>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  template <typename T>
  class SparseRowMatrixGeneric : public MatrixBase<T>, public Eigen::SparseMatrix<T, Eigen::RowMajor>
  {
  public:
    typedef T value_type;
    typedef SparseRowMatrixGeneric<T> this_type;
    typedef Eigen::SparseMatrix<T, Eigen::RowMajor> Base;

    //TODO: need C++11
    //using Base::Base;

    SparseRowMatrixGeneric() : Base() {}
    SparseRowMatrixGeneric(int nrows, int ncols) : Base(nrows, ncols) {}

    // This constructor allows you to construct SparseRowMatrixGeneric from Eigen expressions
    template<typename OtherDerived>
    SparseRowMatrixGeneric(const Eigen::SparseMatrixBase<OtherDerived>& other)
      : Base(other)
    { }

    // This method allows you to assign Eigen expressions to SparseRowMatrixGeneric
    template<typename OtherDerived>
    SparseRowMatrixGeneric& operator=(const Eigen::SparseMatrixBase<OtherDerived>& other)
    {
      this->Base::operator=(other);
      return *this;
    }

    virtual SparseRowMatrixGeneric* clone() const 
    {
      return new SparseRowMatrixGeneric(*this);
    }
  };

  typedef SparseRowMatrixGeneric<double> SparseRowMatrix;

}}}


#endif
