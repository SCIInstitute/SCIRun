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
/// @todo Documentation Core/Datatypes/Legacy/Matrix/MatrixFwd.h
#ifndef CORE_DATATYPES_MATRIX_FWD_H
#define CORE_DATATYPES_MATRIX_FWD_H

#include <Core/Containers/LockingHandle.h>

namespace SCIRun {

template <typename T>
class Matrix;

template <typename T>
class SparseRowMatrixGeneric;
typedef SparseRowMatrixGeneric<double> SparseRowMatrix;

template <typename T>
class DenseMatrixGeneric;
typedef DenseMatrixGeneric<double> DenseMatrix;

template <typename T>
class ColumnMatrixGeneric;
typedef ColumnMatrixGeneric<double> ColumnMatrix;

template <typename T>
class DenseColMajMatrixGeneric;
typedef DenseColMajMatrixGeneric<double> DenseColMajMatrix;

typedef LockingHandle< Matrix<double> > MatrixHandle;
typedef LockingHandle<DenseMatrix> DenseMatrixHandle;
typedef LockingHandle<ColumnMatrix> ColumnMatrixHandle;
typedef LockingHandle<SparseRowMatrix> SparseRowMatrixHandle;
typedef LockingHandle<DenseColMajMatrix> DenseColMajMatrixHandle;

}

#endif
