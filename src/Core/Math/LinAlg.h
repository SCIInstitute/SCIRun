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


///
///@class LinAlg
///@brief Tuned linear algebra routines
///
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///
///@date  November 1995
///

#ifndef Math_LinAlg_h
#define Math_LinAlg_h 1

#include <Core/Math/share.h>

#ifdef __cplusplus
extern "C" {
#endif
    SCISHARE double linalg_norm2(int n, const double* data);
    SCISHARE void linalg_mult(int n, double* result, double* a, double* b);
    SCISHARE void linalg_sub(int n, double* result, double* a, double* b);
    SCISHARE void linalg_add(int n, double* result, double* a, double* b);
    SCISHARE double linalg_dot(int n, double* a, double* b);
    SCISHARE void linalg_smadd(int n, double* result, double s, double* a, double* b);
    SCISHARE typedef double LinAlg_TriDiagRow[3];
    SCISHARE void linalg_tridiag(int n, LinAlg_TriDiagRow* data, double* c);
#ifdef __cplusplus
}
#endif

#endif
