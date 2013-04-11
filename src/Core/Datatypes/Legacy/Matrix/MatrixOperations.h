/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
 *  MatrixOperations.h: Matrix Operations
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   August 2003
 *
 */

#ifndef CORE_DATATYPES_MATRIXOPERATIONS_H
#define CORE_DATATYPES_MATRIXOPERATIONS_H 1

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {


SCISHARE MatrixHandle operator+(MatrixHandle A, MatrixHandle B);
SCISHARE ColumnMatrix operator+(const ColumnMatrix& A, const ColumnMatrix& B);
SCISHARE ColumnMatrix operator-(const ColumnMatrix& A, const ColumnMatrix& B);

SCISHARE MatrixHandle operator-(MatrixHandle A, MatrixHandle B);
SCISHARE MatrixHandle operator-(MatrixHandle A);
SCISHARE ColumnMatrix operator-(const ColumnMatrix& A);

SCISHARE MatrixHandle operator*(MatrixHandle A, MatrixHandle B);
SCISHARE ColumnMatrix operator*(const DenseMatrix& A, const ColumnMatrix& B);
SCISHARE DenseMatrix operator*(const DenseMatrix& A, const DenseMatrix& B);
SCISHARE MatrixHandle operator*(double a, MatrixHandle B);
SCISHARE MatrixHandle operator*(MatrixHandle A, double b);
SCISHARE ColumnMatrix operator*(double a, const ColumnMatrix& B);
SCISHARE ColumnMatrix operator*(const ColumnMatrix& A, double b);
SCISHARE SparseRowMatrix operator*(double a, const SparseRowMatrix& B);
SCISHARE SparseRowMatrix operator*(const SparseRowMatrix& A, double b);
SCISHARE MatrixHandle inv(MatrixHandle A);


} // End namespace SCIRun

#endif
