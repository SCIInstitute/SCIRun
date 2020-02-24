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
///@class Mat
///@brief Simple matrix calculations
///
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///
///@date  October 1994
///

#ifndef sci_Math_Mat_h
#define sci_Math_Mat_h 1

#include <Core/Math/share.h>

#ifdef __cplusplus
extern "C" {
#endif
    SCISHARE void matsolve3by3(double mat[3][3], double rhs[3]);
    SCISHARE void matsolve3by3_cond(double mat[3][3], double rhs[3], double* rcond);
    SCISHARE void min_norm_least_sq_3(double *A[3], double *b, double *x, double *bprime, int size);

#ifdef __cplusplus
}
#endif

#endif
