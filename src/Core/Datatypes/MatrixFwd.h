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
/// @todo Documentation Core/Datatypes/MatrixFwd.h

#ifndef CORE_DATATYPES_MATRIX_FWD_H
#define CORE_DATATYPES_MATRIX_FWD_H

#include <Core/Datatypes/Legacy/Base/TypeName.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <boost/shared_ptr.hpp>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  template <typename T>
  class MatrixBase;

  typedef MatrixBase<double> Matrix;
  using ComplexMatrix = MatrixBase<complex>;

  template <typename T>
  using SharedPointer = boost::shared_ptr<T>;

  typedef SharedPointer<Matrix> MatrixHandle;
  typedef SharedPointer<const Matrix> MatrixConstHandle;

  typedef SharedPointer<ComplexMatrix> ComplexMatrixHandle;

  template <typename T>
  class DenseMatrixGeneric;

  typedef DenseMatrixGeneric<double> DenseMatrix;
  using ComplexDenseMatrix = DenseMatrixGeneric<complex>;

  typedef SharedPointer<DenseMatrix> DenseMatrixHandle;
  typedef SharedPointer<const DenseMatrix> DenseMatrixConstHandle;
  typedef SharedPointer<ComplexDenseMatrix> ComplexDenseMatrixHandle;

  template <typename T>
  class DenseColumnMatrixGeneric;

  typedef DenseColumnMatrixGeneric<double> DenseColumnMatrix;

  typedef SharedPointer<DenseColumnMatrix> DenseColumnMatrixHandle;
  typedef SharedPointer<const DenseColumnMatrix> DenseColumnMatrixConstHandle;

  template <typename T>
  class SparseRowMatrixGeneric;

  typedef SparseRowMatrixGeneric<double> SparseRowMatrix;
  using ComplexSparseRowMatrix = SparseRowMatrixGeneric<complex>;

  typedef SharedPointer<SparseRowMatrix> SparseRowMatrixHandle;
  typedef SharedPointer<const SparseRowMatrix> SparseRowMatrixConstHandle;

  typedef SharedPointer<ComplexSparseRowMatrix> ComplexSparseRowMatrixHandle;

}}}


#endif
