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
///@file  MatrixOperations.h
///@brief Matrix Operations
/// 
///@author
///    Michael Callahan
///    Department of Computer Science
///    University of Utah
///@date  August 2003
/// 
 

#ifndef CORE_DATATYPES_MATRIXOPERATIONS_H
#define CORE_DATATYPES_MATRIXOPERATIONS_H 1

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {


SCISHARE Core::Datatypes::MatrixHandle operator+(Core::Datatypes::MatrixHandle A, Core::Datatypes::MatrixHandle B);
SCISHARE Core::Datatypes::DenseColumnMatrix operator+(const Core::Datatypes::DenseColumnMatrix& A, const Core::Datatypes::DenseColumnMatrix& B);
SCISHARE Core::Datatypes::DenseColumnMatrix operator-(const Core::Datatypes::DenseColumnMatrix& A, const Core::Datatypes::DenseColumnMatrix& B);

SCISHARE Core::Datatypes::MatrixHandle operator-(Core::Datatypes::MatrixHandle A, Core::Datatypes::MatrixHandle B);
SCISHARE Core::Datatypes::MatrixHandle operator-(Core::Datatypes::MatrixHandle A);
SCISHARE Core::Datatypes::DenseColumnMatrix operator-(const Core::Datatypes::DenseColumnMatrix& A);

SCISHARE Core::Datatypes::MatrixHandle operator*(Core::Datatypes::MatrixHandle A, Core::Datatypes::MatrixHandle B);
SCISHARE Core::Datatypes::DenseColumnMatrix operator*(const Core::Datatypes::DenseMatrix& A, const Core::Datatypes::DenseColumnMatrix& B);
SCISHARE Core::Datatypes::DenseMatrix operator*(const Core::Datatypes::DenseMatrix& A, const Core::Datatypes::DenseMatrix& B);
SCISHARE Core::Datatypes::MatrixHandle operator*(double a, Core::Datatypes::MatrixHandle B);
SCISHARE Core::Datatypes::MatrixHandle operator*(Core::Datatypes::MatrixHandle A, double b);
SCISHARE Core::Datatypes::DenseColumnMatrix operator*(double a, const Core::Datatypes::DenseColumnMatrix& B);
SCISHARE Core::Datatypes::DenseColumnMatrix operator*(const Core::Datatypes::DenseColumnMatrix& A, double b);
SCISHARE Core::Datatypes::SparseRowMatrix operator*(double a, Core::Datatypes::SparseRowMatrix& B);
SCISHARE Core::Datatypes::SparseRowMatrix operator*(Core::Datatypes::SparseRowMatrix& A, double b);
SCISHARE Core::Datatypes::MatrixHandle inv(Core::Datatypes::MatrixHandle A);


} // End namespace SCIRun

#endif
