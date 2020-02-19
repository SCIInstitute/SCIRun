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


#include <teem/air.h>
#include <teem/ten.h>


#include <Core/Parser/ArrayMathFunctionCatalog.h>
#include <Core/Math/MiscMath.h>

#include <cmath>

namespace ArrayMathFunctions {

using namespace SCIRun;


//--------------------------------------------------------------------------
// Simple Vector functions

bool tensor_ssssss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);
  double* data4 = pc.get_variable(4);
  double* data5 = pc.get_variable(5);
  double* data6 = pc.get_variable(6);
  double* data1_end = data1 + pc.get_size();

  while (data1 != data1_end)
  {
    *data0 = *data1; data0++; data1++;
    *data0 = *data2; data0++; data2++;
    *data0 = *data3; data0++; data3++;
    *data0 = *data4; data0++; data4++;
    *data0 = *data5; data0++; data5++;
    *data0 = *data6; data0++; data6++;
  }

  return (true);
}


bool tensor_(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = 0.0; data0++;
  }

  return (true);
}


bool tensor_vvsss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);
  double* data4 = pc.get_variable(4);
  double* data5 = pc.get_variable(5);
  double* data4_end = data4 + pc.get_size();

  double a0,a1,a2,s1,s2,s12,s22;

  while (data4 != data4_end)
  {
    s12 = data1[0]*data1[0]+data1[1]*data1[1]+data1[2]*data1[2];
    s22 = data2[0]*data2[0]+data2[1]*data2[1]+data2[2]*data2[2];
    s1 = ::sqrt(s12);
    s2 = ::sqrt(s22);
    if (s1 > 0.0) s1 = 1.0/s1; else s1 = 1.0;
    if (s2 > 0.0) s2 = 1.0/s2; else s2 = 1.0;
    if (s12 > 0.0) s12 = 1.0/s12; else s12 = 1.0;
    if (s22 > 0.0) s22 = 1.0/s22; else s22 = 1.0;

    a0 = (data1[1]*data2[2]-data1[2]*data2[1])*s1*s2;
    a1 = (data1[2]*data2[0]-data1[0]*data2[2])*s1*s2;
    a2 = (data1[0]*data2[1]-data1[1]*data2[0])*s1*s2;

    data0[0] = s12*data3[0]*data1[0]*data1[0]+s22*data4[0]*data2[0]*data2[0]+data5[0]*a0*a0;
    data0[1] = s12*data3[0]*data1[1]*data1[0]+s22*data4[0]*data2[1]*data2[0]+data5[0]*a1*a0;
    data0[2] = s12*data3[0]*data1[2]*data1[0]+s22*data4[0]*data2[2]*data2[0]+data5[0]*a2*a0;
    data0[3] = s12*data3[0]*data1[1]*data1[1]+s22*data4[0]*data2[1]*data2[1]+data5[0]*a1*a1;
    data0[4] = s12*data3[0]*data1[1]*data1[2]+s22*data4[0]*data2[1]*data2[2]+data5[0]*a1*a2;
    data0[5] = s12*data3[0]*data1[2]*data1[2]+s22*data4[0]*data2[2]*data2[2]+data5[0]*a2*a2;

    data0 += 6; data1 += 3; data2 += 3; data3++; data4++; data5++;
  }

  return (true);
}

bool tensor_vss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);
  double* data3_end = data3 + pc.get_size();

  double a1,a2,a0,s1,s2,s12,s22;
  double dataN[3];


  while (data3 != data3_end)
  {
    // determine a normal vector
    if (data1[1] == 0 && data1[2] == 0)
    {
      dataN[0] = data1[2];
      dataN[1] = 0;
      dataN[2] = -data1[0];
    }
    else
    {
      dataN[0] = 0;
      dataN[1] = -data1[2];
      dataN[2] = data1[1];
    }


    s12 = data1[0]*data1[0]+data1[1]*data1[1]+data1[2]*data1[2];
    s22 = dataN[0]*dataN[0]+dataN[1]*dataN[1]+dataN[2]*dataN[2];
    s1 = ::sqrt(s12);
    s2 = ::sqrt(s22);
    if (s1 > 0.0) s1 = 1.0/s1; else s1 = 1.0;
    if (s2 > 0.0) s2 = 1.0/s2; else s2 = 1.0;
    if (s12 > 0.0) s12 = 1.0/s12; else s12 = 1.0;
    if (s22 > 0.0) s22 = 1.0/s22; else s22 = 1.0;


    a0 = (data1[1]*dataN[2]-data1[2]*dataN[1])*s1*s2;
    a1 = (data1[2]*dataN[0]-data1[0]*dataN[2])*s1*s2;
    a2 = (data1[0]*dataN[1]-data1[1]*dataN[0])*s1*s2;

    data0[0] = s12*data2[0]*data1[0]*data1[0]+s22*data3[0]*dataN[0]*dataN[0]+data3[0]*a0*a0;
    data0[1] = s12*data2[0]*data1[1]*data1[0]+s22*data3[0]*dataN[1]*dataN[0]+data3[0]*a1*a0;
    data0[2] = s12*data2[0]*data1[2]*data1[0]+s22*data3[0]*dataN[2]*dataN[0]+data3[0]*a2*a0;
    data0[3] = s12*data2[0]*data1[1]*data1[1]+s22*data3[0]*dataN[1]*dataN[1]+data3[0]*a1*a1;
    data0[4] = s12*data2[0]*data1[1]*data1[2]+s22*data3[0]*dataN[1]*dataN[2]+data3[0]*a1*a2;
    data0[5] = s12*data2[0]*data1[2]*data1[2]+s22*data3[0]*dataN[2]*dataN[2]+data3[0]*a2*a2;

    data0 += 6; data1 += 3; data2++; data3++;
  }

  return (true);
}


bool tensor_vvss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);
  double* data4 = pc.get_variable(4);
  double* data4_end = data4 + pc.get_size();

  double a1,a2,a0,s1,s2,s12,s22;

  while (data4 != data4_end)
  {
    s12 = data1[0]*data1[0]+data1[1]*data1[1]+data1[2]*data1[2];
    s22 = data2[0]*data2[0]+data2[1]*data2[1]+data2[2]*data2[2];

    s1 = ::sqrt(s12);
    s2 = ::sqrt(s22);
    if (s1 > 0.0) s1 = 1.0/s1; else s1 = 1.0;
    if (s2 > 0.0) s2 = 1.0/s2; else s2 = 1.0;
    if (s12 > 0.0) s12 = 1.0/s12; else s12 = 1.0;
    if (s22 > 0.0) s22 = 1.0/s22; else s22 = 1.0;


    a0 = (data1[1]*data2[2]-data1[2]*data2[1])*s1*s2;
    a1 = (data1[2]*data2[0]-data1[0]*data2[2])*s1*s2;
    a2 = (data1[0]*data2[1]-data1[1]*data2[0])*s1*s2;

    data0[0] = s12*data3[0]*data1[0]*data1[0]+s22*data4[0]*data2[0]*data2[0]+data4[0]*a0*a0;
    data0[1] = s12*data3[0]*data1[1]*data1[0]+s22*data4[0]*data2[1]*data2[0]+data4[0]*a1*a0;
    data0[2] = s12*data3[0]*data1[2]*data1[0]+s22*data4[0]*data2[2]*data2[0]+data4[0]*a2*a0;
    data0[3] = s12*data3[0]*data1[1]*data1[1]+s22*data4[0]*data2[1]*data2[1]+data4[0]*a1*a1;
    data0[4] = s12*data3[0]*data1[1]*data1[2]+s22*data4[0]*data2[1]*data2[2]+data4[0]*a1*a2;
    data0[5] = s12*data3[0]*data1[2]*data1[2]+s22*data4[0]*data2[2]*data2[2]+data4[0]*a2*a2;

    data0 += 6; data1 += 3; data2 += 3; data3++; data4++;
  }

  return (true);
}

bool tensor_vvvsss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);
  double* data4 = pc.get_variable(4);
  double* data5 = pc.get_variable(5);
  double* data6 = pc.get_variable(6);
  double* data4_end = data4 + pc.get_size();

  double s1,s2,s3;

  while (data4 != data4_end)
  {
    s1 = (data1[0]*data1[0]+data1[1]*data1[1]+data1[2]*data1[2]);
    s2 = (data2[0]*data2[0]+data2[1]*data2[1]+data2[2]*data2[2]);
    s3 = (data3[0]*data3[0]+data3[1]*data3[1]+data3[2]*data3[2]);
    if (s1 > 0.0) s1 = 1.0/s1; else s1 = 1.0;
    if (s2 > 0.0) s2 = 1.0/s2; else s2 = 1.0;
    if (s3 > 0.0) s3 = 1.0/s3; else s3 = 1.0;

    data0[0] = s1*data4[0]*data1[0]*data1[0]+s2*data5[0]*data2[0]*data2[0]+data6[0]*data3[0]*data3[0];
    data0[1] = s1*data4[0]*data1[1]*data1[0]+s2*data5[0]*data2[1]*data2[0]+data6[0]*data3[1]*data3[0];
    data0[2] = s1*data4[0]*data1[2]*data1[0]+s2*data5[0]*data2[2]*data2[0]+data6[0]*data3[2]*data3[0];
    data0[3] = s1*data4[0]*data1[1]*data1[1]+s2*data5[0]*data2[1]*data2[1]+data6[0]*data3[1]*data3[1];
    data0[4] = s1*data4[0]*data1[1]*data1[2]+s2*data5[0]*data2[1]*data2[2]+data6[0]*data3[1]*data3[2];
    data0[5] = s1*data4[0]*data1[2]*data1[2]+s2*data5[0]*data2[2]*data2[2]+data6[0]*data3[2]*data3[2];

    data0 += 6; data1 += 3; data2 += 3; data3+=3; data4++; data5++; data6++;
  }

  return (true);
}

bool tensor_s(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + pc.get_size();

  while (data1 != data1_end)
  {
    data0[0] = *data1;
    data0[1] = 0.0;
    data0[2] = 0.0;
    data0[3] = *data1;
    data0[4] = 0.0;
    data0[5] = *data1;
    data0 +=6; data1++;
  }
  return (true);
}

bool eigvec1_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3]; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);

    data0[0] = evec[0];
    data0[1] = evec[1];
    data0[2] = evec[2];

    data0 +=3; data1 +=6;
  }

  return (true);
}

bool eigvec2_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3]; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);

    data0[0] = evec[3];
    data0[1] = evec[4];
    data0[2] = evec[5];

    data0 +=3; data1 +=6;
  }

  return (true);
}

bool eigvec3_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3]; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);


    data0[0] = evec[6];
    data0[1] = evec[7];
    data0[2] = evec[8];

    data0 +=3; data1 +=6;
  }

  return (true);
}

bool eigval1_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3]; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);


    data0[0] = eval[0];

    data0++; data1 +=6;
  }

  return (true);
}

bool eigval2_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3]; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);


    data0[0] = eval[1];

    data0 ++; data1 +=6;
  }

  return (true);
}

bool eigval3_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3]; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);


    data0[0] = eval[2];

    data0 ++; data1 +=6;
  }

  return (true);
}
/*
  bool trace_t_bug(SCIRun::ArrayMathProgramCode& pc)
  {

  double eval[3];// = {0};
  while (true)
    {

  double* data0 = pc.get_variable(0);
     double result = eval[0]+eval[1]+eval[2];
    *data0 = result;
    }
  return (true);
  }
*/

bool trace_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3] = {0}; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {

    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];


    tenEigensolve_d(eval, evec, ten);


    data0[0] = eval[0]+eval[1]+eval[2];

    data0 ++; data1 +=6;
  }

  return (true);
}

bool det_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3] = {0}; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);


    data0[0] = eval[0]*eval[1]*eval[2];

    data0 ++; data1 +=6;
  }

  return (true);
}


bool B_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3] = {0}; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);


    data0[0] = eval[0]*eval[1]+eval[0]*eval[2]+eval[2]*eval[1];

    data0 ++; data1 +=6;
  }

  return (true);
}

bool S_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3] = {0}; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);


    data0[0] = eval[0]*eval[0]+eval[1]*eval[1]+eval[2]*eval[2];

    data0 ++; data1 +=6;
  }

  return (true);
}

bool quality_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3] = {0}; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);


    data0[0] = ((eval[0]*eval[0]+eval[1]*eval[1]+eval[2]*eval[2]) -
      (eval[0]*eval[1]+eval[0]*eval[2]+eval[2]*eval[1]))/9.0;

    data0 ++; data1 +=6;
  }

  return (true);
}

bool frobenius_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3] = {0}; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);


    data0[0] = ::sqrt(eval[0]*eval[0]+eval[1]*eval[1]+eval[2]*eval[2]);

    data0 ++; data1 +=6;
  }

  return (true);
}

bool frobenius2_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3] = {0}; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);


    data0[0] = eval[0]*eval[0]+eval[1]*eval[1]+eval[2]*eval[2];

    data0++; data1 +=6;
  }

  return (true);
}


bool fracanisotropy_t(SCIRun::ArrayMathProgramCode& pc)
{
  double ten[7]; double eval[3] = {0}; double evec[9];

  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    ten[0] = 1.0; ten[1] = data1[0]; ten[2] = data1[1];
    ten[3] = data1[2]; ten[4] = data1[3];
    ten[5] = data1[4]; ten[6] = data1[5];

    tenEigensolve_d(eval, evec, ten);


    double S = eval[0]*eval[0]+eval[1]*eval[1]+eval[2]*eval[2];
    double B = eval[0]*eval[1]+eval[0]*eval[2]+eval[2]*eval[1];
    data0[0] = ::sqrt((S-B)/S);

    data0++; data1 +=6;
  }

  return (true);
}

bool vec1_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    data0[0] = data1[0];
    data0[1] = data1[1];
    data0[2] = data1[2];
    data0+=3; data1 +=6;
  }

  return (true);
}

bool vec2_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    data0[0] = data1[1];
    data0[1] = data1[3];
    data0[2] = data1[4];
    data0+=3; data1 +=6;
  }

  return (true);
}


bool vec3_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);

  double* data1_end = data1 + 6*pc.get_size();

  while (data1 != data1_end)
  {
    data0[0] = data1[2];
    data0[1] = data1[4];
    data0[2] = data1[5];
    data0+=3; data1 +=6;
  }

  return (true);
}

bool eq_tt(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if ((data1[0] == data2[0])&&(data1[1] == data2[1])&&
        (data1[2] == data2[2])&&(data1[3] == data2[3])&&
        (data1[4] == data2[4])&&(data1[5] == data2[5])) *data0 = 1.0;
    else *data0 = 0.0;
    data0++; data1+=6; data2+=6;
  }

  return (true);
}


bool neq_tt(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if ((data1[0] == data2[0])&&(data1[1] == data2[1])&&
        (data1[2] == data2[2])&&(data1[3] == data2[3])&&
        (data1[4] == data2[4])&&(data1[5] == data2[5])) *data0 = 0.0;
    else *data0 = 1.0;
    data0++; data1+=6; data2+=6;
  }

  return (true);
}


bool xx_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = data1[0]; data0++; data1 += 6;
  }

  return (true);
}

bool xy_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = data1[1]; data0++; data1 += 6;
  }

  return (true);
}

bool xz_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = data1[2]; data0++; data1 += 6;
  }

  return (true);
}

bool yy_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = data1[3]; data0++; data1 += 6;
  }

  return (true);
}

bool yz_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = data1[4]; data0++; data1 += 6;
  }

  return (true);
}

bool zz_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = data1[5]; data0++; data1 += 6;
  }

  return (true);
}


bool isfinite_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if (IsFinite(data1[0])&&IsFinite(data1[1])&&IsFinite(data1[2])&&
        IsFinite(data1[3])&&IsFinite(data1[4])&&IsFinite(data1[5]))
      *data0 = 1.0; else *data0 = 0.0;
    data0++; data1+=6;
  }

  return (true);
}

bool isinfinite_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if (IsInfinite(data1[0])||IsInfinite(data1[1])||IsInfinite(data1[2])||
        IsInfinite(data1[3])||IsInfinite(data1[4])||IsInfinite(data1[5]))
      *data0 = 1.0; else *data0 = 0.0;
    data0++; data1+=6;
  }

  return (true);
}

bool isnan_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if (IsNan(data1[0])||IsNan(data1[1])||IsNan(data1[2])||
        IsNan(data1[3])||IsNan(data1[4])||IsNan(data1[5]))
      *data0 = 1.0; else *data0 = 0.0;
    data0++; data1+=6;
  }

  return (true);
}

bool max_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    double a = data1[0];
    if (a < data1[1]) a = data1[1];
    if (a < data1[2]) a = data1[2];
    if (a < data1[3]) a = data1[3];
    if (a < data1[4]) a = data1[4];
    if (a < data1[5]) a = data1[5];
    data0++; data1+=6;
  }

  return (true);
}

bool min_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    double a = data1[0];
    if (a > data1[1]) a = data1[1];
    if (a > data1[2]) a = data1[2];
    if (a > data1[3]) a = data1[3];
    if (a > data1[4]) a = data1[4];
    if (a > data1[5]) a = data1[5];
    data0++; data1+=6;
  }

  return (true);
}

bool boolean_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if(data1[0] || data1[1] || data1[2] ||data1[3] || data1[4] || data1[5])
      *data0 = 1.0; else *data0 = 0.0;
    data0++; data1+=6;
  }

  return (true);
}


bool inv_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = 1.0/(*data1); data0++; data1++;
  }

  return (true);
}



bool abs_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    if(*data1<0) *data0 = -*data1; else *data0 = *data1; data0++; data1++;
  }

  return (true);
}


bool round_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = static_cast<double>(static_cast<int>(*data1+0.5)); data0++; data1++;
  }

  return (true);
}

bool floor_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::floor(*data1); data0++; data1++;
  }

  return (true);
}

bool ceil_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::ceil(*data1); data0++; data1++;
  }

  return (true);
}

bool exp_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::exp(*data1); data0++; data1++;
  }

  return (true);
}

bool pow_ts(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::pow(*data0,*data2); data0++; data1++;
    *data0 = ::pow(*data0,*data2); data0++; data1++;
    *data0 = ::pow(*data0,*data2); data0++; data1++; data2++;
  }

  return (true);
}

bool sqrt_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::sqrt(*data1); data0++; data1++;
  }

  return (true);
}

bool log_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::log(*data1); data0++; data1++;
  }

  return (true);
}

bool ln_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::log(*data1); data0++; data1++;
  }

  return (true);
}

bool log2_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  double s = 1.0/log(2.0);
  while (data0 != data0_end)
  {
    *data0 = ::log(*data1)*s; data0++; data1++;
  }

  return (true);
}

bool log10_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  double s = 1.0/log(10.0);
  while (data0 != data0_end)
  {
    *data0 = ::log(*data1)*s; data0++; data1++;
  }

  return (true);
}

bool cbrt_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::pow(*data1,1.0/3.0); data0++; data1++;
  }

  return (true);
}

bool sin_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::sin(*data1); data0++; data1++;
  }

  return (true);
}

bool cos_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::cos(*data1); data0++; data1++;
  }

  return (true);
}

bool tan_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::tan(*data1); data0++; data1++;
  }

  return (true);
}

bool sinh_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::sinh(*data1); data0++; data1++;
  }

  return (true);
}

bool cosh_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::cosh(*data1); data0++; data1++;
  }

  return (true);
}

bool asin_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::asin(*data1); data0++; data1++;
  }

  return (true);
}

bool acos_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::acos(*data1); data0++; data1++;
  }

  return (true);
}

bool atan_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::atan(*data1); data0++; data1++;
  }

  return (true);
}

bool asinh_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    double d = *data1;
    *data0 = (d==0?0:(d>0?1:-1)) * ::log((d<0?-d:d) + ::sqrt(1+d*d)); data0++; data1++;
  }

  return (true);
}

bool acosh_t(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 6*pc.get_size();

  while (data0 != data0_end)
  {
    double d = *data1;
    *data0 = ::log(d + ::sqrt(d*d-1)); data0++; data1++;
  }

  return (true);
}

} // end namsespace

namespace SCIRun {

void
InsertTensorArrayMathFunctionCatalog(ArrayMathFunctionCatalogHandle& catalog)
{
  // Constructors for vectors (or points for some SCIRun applications)
  catalog->add_function(ArrayMathFunctions::tensor_ssssss,"tensor$S:S:S:S:S:S","T");
  catalog->add_function(ArrayMathFunctions::tensor_vvvsss,"tensor$V:V:V:S:S:S","T");
  catalog->add_function(ArrayMathFunctions::tensor_vvsss,"tensor$V:V:S:S:S","T");
  catalog->add_function(ArrayMathFunctions::tensor_vvss,"tensor$V:V:S:S","T");
  catalog->add_function(ArrayMathFunctions::tensor_vss,"tensor$V:S:S","T");
  catalog->add_function(ArrayMathFunctions::tensor_s,"tensor$S","T");

  catalog->add_function(ArrayMathFunctions::tensor_ssssss,"Tensor$S:S:S:S:S:S","T");
  catalog->add_function(ArrayMathFunctions::tensor_vvvsss,"Tensor$V:V:V:S:S:S","T");
  catalog->add_function(ArrayMathFunctions::tensor_vvsss,"Tensor$V:V:S:S:S","T");
  catalog->add_function(ArrayMathFunctions::tensor_vvss,"Tensor$V:V:S:S","T");
  catalog->add_function(ArrayMathFunctions::tensor_vss,"Tensor$V:S:S","T");
  catalog->add_function(ArrayMathFunctions::tensor_s,"Tensor$S","T");
  // Function always returns the same tensor
  catalog->add_cst_function(ArrayMathFunctions::tensor_,"Tensor$","T");
  catalog->add_cst_function(ArrayMathFunctions::tensor_,"tensor$","T");

  // Test nan inf
  catalog->add_function(ArrayMathFunctions::isnan_t,"isnan$T","S");
  catalog->add_function(ArrayMathFunctions::isfinite_t,"isfinite$T","S");
  catalog->add_function(ArrayMathFunctions::isinfinite_t,"isinfinite$T","S");
  catalog->add_function(ArrayMathFunctions::isinfinite_t,"isinf$T","S");

  // Getting components e.g. x(Vector(0,0,0))
  catalog->add_function(ArrayMathFunctions::xx_t,"xx$T","S");
  catalog->add_function(ArrayMathFunctions::xy_t,"xy$T","S");
  catalog->add_function(ArrayMathFunctions::xz_t,"xz$T","S");
  catalog->add_function(ArrayMathFunctions::yy_t,"yy$T","S");
  catalog->add_function(ArrayMathFunctions::yz_t,"yz$T","S");
  catalog->add_function(ArrayMathFunctions::zz_t,"zz$T","S");
  catalog->add_function(ArrayMathFunctions::xx_t,"uu$T","S");
  catalog->add_function(ArrayMathFunctions::xy_t,"uv$T","S");
  catalog->add_function(ArrayMathFunctions::xz_t,"uw$T","S");
  catalog->add_function(ArrayMathFunctions::yy_t,"vv$T","S");
  catalog->add_function(ArrayMathFunctions::yz_t,"vw$T","S");
  catalog->add_function(ArrayMathFunctions::zz_t,"ww$T","S");

  // Get the first, second and third column of the matrix
  catalog->add_function(ArrayMathFunctions::vec1_t,"vec1$T","V");
  catalog->add_function(ArrayMathFunctions::vec2_t,"vec2$T","V");
  catalog->add_function(ArrayMathFunctions::vec3_t,"vec3$T","V");

  // Get eigen vectors and values
  catalog->add_function(ArrayMathFunctions::eigvec1_t,"eigvec1$T","V");
  catalog->add_function(ArrayMathFunctions::eigvec2_t,"eigvec2$T","V");
  catalog->add_function(ArrayMathFunctions::eigvec3_t,"eigvec3$T","V");
  catalog->add_function(ArrayMathFunctions::eigval1_t,"eigval1$T","S");
  catalog->add_function(ArrayMathFunctions::eigval2_t,"eigval2$T","S");
  catalog->add_function(ArrayMathFunctions::eigval3_t,"eigval3$T","S");

  // Tensor properties
  catalog->add_function(ArrayMathFunctions::trace_t,"trace$T","S");
  catalog->add_function(ArrayMathFunctions::det_t,"det$T","S");
  catalog->add_function(ArrayMathFunctions::S_t,"S$T","S");
  catalog->add_function(ArrayMathFunctions::B_t,"B$T","S");
  catalog->add_function(ArrayMathFunctions::quality_t,"Q$T","S");
  catalog->add_function(ArrayMathFunctions::quality_t,"quality$T","S");

  catalog->add_function(ArrayMathFunctions::frobenius_t,"frobenius$T","S");
  catalog->add_function(ArrayMathFunctions::frobenius2_t,"frobenius2$T","S");
  catalog->add_function(ArrayMathFunctions::fracanisotropy_t,"fracanisotropy$T","S");

  catalog->add_function(ArrayMathFunctions::min_t,"min$T","S");
  catalog->add_function(ArrayMathFunctions::max_t,"max$T","S");

  // Converting to boolean    == Vector(0,0,0)
  catalog->add_function(ArrayMathFunctions::boolean_t,"boolean$T","S");

  // Vectorized operations
  catalog->add_function(ArrayMathFunctions::inv_t,"inv$T","T");
  catalog->add_function(ArrayMathFunctions::abs_t,"abs$T","T");
  catalog->add_function(ArrayMathFunctions::round_t,"round$T","T");
  catalog->add_function(ArrayMathFunctions::floor_t,"floor$T","T");
  catalog->add_function(ArrayMathFunctions::ceil_t,"ceil$T","T");
  catalog->add_function(ArrayMathFunctions::exp_t,"exp$T","T");
  catalog->add_function(ArrayMathFunctions::pow_ts,"pow$T:S","T");
  catalog->add_function(ArrayMathFunctions::sqrt_t,"sqrt$T","T");
  catalog->add_function(ArrayMathFunctions::log_t,"log$T","T");
  catalog->add_function(ArrayMathFunctions::ln_t,"ln$T","T");
  catalog->add_function(ArrayMathFunctions::log2_t,"log2$T","T");
  catalog->add_function(ArrayMathFunctions::log10_t,"log10$T","T");
  catalog->add_function(ArrayMathFunctions::cbrt_t,"cbrt$T","T");
  catalog->add_function(ArrayMathFunctions::sin_t,"sin$T","T");
  catalog->add_function(ArrayMathFunctions::cos_t,"cos$T","T");
  catalog->add_function(ArrayMathFunctions::tan_t,"tan$T","T");
  catalog->add_function(ArrayMathFunctions::sinh_t,"sinh$T","T");
  catalog->add_function(ArrayMathFunctions::cosh_t,"cosh$T","T");
  catalog->add_function(ArrayMathFunctions::asin_t,"asin$T","T");
  catalog->add_function(ArrayMathFunctions::acos_t,"acos$T","T");
  catalog->add_function(ArrayMathFunctions::atan_t,"atan$T","T");
  catalog->add_function(ArrayMathFunctions::asinh_t,"asinh$T","T");
  catalog->add_function(ArrayMathFunctions::acosh_t,"acosh$T","T");

  catalog->add_sym_function(ArrayMathFunctions::eq_tt,"eq$T:T","S");
  catalog->add_sym_function(ArrayMathFunctions::neq_tt,"neq$T:T","S");
}

} // end namespace
