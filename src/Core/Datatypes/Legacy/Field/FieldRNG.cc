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


/// @todo Documentation Core/Datatypes/Legacy/Field/FieldRNG.cc

#include <Core/Datatypes/Legacy/Field/FieldRNG.h>

namespace SCIRun {

FieldRNG::FieldRNG( int seed )
{
  n[0]=4;
  n[1]=7;
  n[2]=10;
  n[3]=15;
  n[4]=6;
  n[5]=13;
  n[6]=1;
  n[7]=8;
  n[8]=12;
  n[9]=9;
  n[10]=0;
  n[11]=14;
  n[12]=5;
  n[13]=11;
  n[14]=2;
  n[15]=3;
  for(int i=0;i<32;i++){
    stab[0][i]=i;
    stab[1][i]=-i;
  }
  a1=0;
  a2=0;
  a3=0;
  a4=0;
  a5=(seed >> 28)&0x0f;
  a6=(seed >> 24)&0x0f;
  a7=(seed >> 20)&0x0f;
  a8=(seed >> 16)&0x0f;
  a9=(seed >> 12)&0x0f;
  a10=(seed >> 8)&0x0f;
  a11=(seed >> 4)&0x0f;
  a12=seed&0x0f;
  b1=b2=b3=b4=b5=b6=b7=b8=b9=b10=b11=b12=0;
  d1=d2=d3=d4=d5=d6=d7=d8=d9=d10=d11=d12=0;
  point=4;
  (*this)(); // the first value is *always* 0.770611 -- just throw it out
}

double
FieldRNG::operator()()
{
  point=(point+20+stab[d1&1][a1+b1])&0x0f;
  d1=a1;a1=b1;b1=n[point];
  point=(point+20+stab[d2&1][a2+b2])&0x0f;
  d2=a2;a2=b2;b2=n[point];
  point=(point+20+stab[d3&1][a3+b3])&0x0f;
  d3=a3;a3=b3;b3=n[point];
  point=(point+20+stab[d4&1][a4+b4])&0x0f;
  d4=a4;a4=b4;b4=n[point];
  point=(point+20+stab[d5&1][a5+b5])&0x0f;
  d5=a5;a5=b5;b5=n[point];
  point=(point+20+stab[d6&1][a6+b6])&0x0f;
  d6=a6;a6=b6;b6=n[point];
  point=(point+20+stab[d7&1][a7+b7])&0x0f;
  d7=a7;a7=b7;b7=n[point];
  point=(point+20+stab[d8&1][a8+b8])&0x0f;
  d8=a8;a8=b8;b8=n[point];
  point=(point+20+stab[d9&1][a9+b9])&0x0f;
  d9=a9;a9=b9;b9=n[point];
  point=(point+20+stab[d10&1][a10+b10])&0x0f;
  d10=a10;a10=b10;b10=n[point];
  point=(point+20+stab[d11&1][a11+b11])&0x0f;
  d11=a11;a11=b11;b11=n[point];
  point=(point+20+stab[d12&1][a12+b12])&0x0f;
  d12=a12;a12=b12;b12=n[point];

  double random=(((((((((((b12/16.0+b11)/16.0+b10)/16.0+b9)/16.0
			+b8)/16.0+b7)/16.0+b6)/16.0+b5)/16.0+b4)/16.0
		   +b3)/16.0+b2)/16.0+b1)/16.0;
  return random;
}

}
