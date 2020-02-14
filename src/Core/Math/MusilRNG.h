/*/*
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
///@class MusilRNG
///@brief Musil random number generator
///
///@author
///       Steve Parker
///       Department of Computer Science
///       University of Utah
///
///@date  November 1994
///

#ifndef sci_Math_MusilRNG_h
#define sci_Math_MusilRNG_h 1

#include <Core/Math/share.h>

class SCISHARE MusilRNG {
  int n[16];
  int stab[2][32];
  int point;
  int d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12;
  int a1,b1;
  int a2,b2;
  int a3,b3;
  int a4,b4;
  int a5,b5;
  int a6,b6;
  int a7,b7;
  int a8,b8;
  int a9,b9;
  int a10,b10;
  int a11,b11;
  int a12,b12;
public:
  MusilRNG( int seed = 0 );
  double operator()();
};

#endif
