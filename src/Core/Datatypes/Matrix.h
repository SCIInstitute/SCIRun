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


#ifndef CORE_DATATYPES_MATRIX_H
#define CORE_DATATYPES_MATRIX_H 

#include <boost/numeric/ublas/matrix.hpp> //TODO
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/MatrixFwd.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  template <typename T>
  class DenseMatrixGeneric : public Datatype
  {
  public:
    typedef T value_type;
    typedef DenseMatrixGeneric<T> this_type;

    DenseMatrixGeneric();
    DenseMatrixGeneric(size_t nrows, size_t ncols);
    DenseMatrixGeneric(size_t nrows, size_t ncols, const T& val);
    DenseMatrixGeneric(const DenseMatrixGeneric& rhs);
    DenseMatrixGeneric& operator=(const DenseMatrixGeneric& rhs);

    size_t nrows() const;
    size_t ncols() const;
    T& operator()(size_t r, size_t c);
    const T& operator()(size_t r, size_t c) const;

    T min() const;
    T max() const;

    DenseMatrixGeneric& operator+=(const DenseMatrixGeneric& rhs);
    DenseMatrixGeneric& operator-=(const DenseMatrixGeneric& rhs);
    DenseMatrixGeneric& operator*=(const DenseMatrixGeneric& rhs);
    DenseMatrixGeneric& operator*=(const T& scalar);

    DenseMatrixGeneric* make_transpose() const;
    void transpose_in_place();
    void clear();
    void resize(size_t nrows, size_t ncols);

    virtual DenseMatrixGeneric* clone() const;
    
    static DenseMatrixGeneric zero_matrix(size_t nrows, size_t ncols);

  private:
    typedef boost::numeric::ublas::matrix<T> MatrixInternal;
    DenseMatrixGeneric(const MatrixInternal& internals);
    MatrixInternal matrix_;
  };

  typedef DenseMatrixGeneric<double> DenseMatrix;


}}}


#endif
