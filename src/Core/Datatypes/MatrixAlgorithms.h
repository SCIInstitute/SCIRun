/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


/// @todo Documentation Core/Datatypes/Legacy/Matrix/MatrixAlgorithms.h

#ifndef CORE_DATATYPES_MATRIX_ALGORITHMS_H
#define CORE_DATATYPES_MATRIX_ALGORITHMS_H

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {

  namespace MatrixAlgorithms
  {
#if SCIRUN4_TO_BE_ENABLED_LATER
    int cg_solve(const Matrix<double>& matrix, const ColumnMatrix& rhs, ColumnMatrix& lhs,
      double &err, int &niter,
      double max_error=1.e-6, int toomany=0,
      int useLhsAsGuess=0);
    int cg_solve(const Matrix<double>& matrix, const ColumnMatrix& rhs, ColumnMatrix& lhs);
    int cg_solve(const Matrix<double>& matrix, const DenseMatrix& rhs, DenseMatrix& lhs,
      double &err, int &niter,
      double max_error=1.e-6, int toomany=0,
      int useLhsAsGuess=0);
    int cg_solve(const Matrix<double>& matrix, const DenseMatrix& rhs, DenseMatrix& lhs);

    int bicg_solve(const Matrix<double>& matrix, const ColumnMatrix& rhs, ColumnMatrix& lhs,
      double &err, int &niter,
      double max_error=1.e-6, int toomany=0,
      int useLhsAsGuess=0);
    int bicg_solve(const Matrix<double>& matrix, const ColumnMatrix& rhs, ColumnMatrix& lhs);
    int bicg_solve(const Matrix<double>& matrix, const DenseMatrix& rhs, DenseMatrix& lhs,
      double &err, int &niter,
      double max_error=1.e-6, int toomany=0,
      int useLhsAsGuess=0);
    int bicg_solve(const Matrix<double>& matrix, const DenseMatrix& rhs, DenseMatrix& lhs);
#endif

    SCISHARE Core::Geometry::Transform matrix_to_transform(const Core::Datatypes::Matrix& matrix);
  }
}

#endif
