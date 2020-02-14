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


#include <Core/Parser/ArrayMathFunctionCatalog.h>
#include <Core/Math/MiscMath.h>

#include <math.h>

namespace ArrayMathFunctions {

using namespace SCIRun;

//--------------------------------------------------------------------------
// Simple Vector functions

bool vector_sss(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data3 = pc.get_variable(3);
  double* data1_end = data1 + pc.get_size();

  while (data1 != data1_end)
  {
    *data0 = *data1; data0++; data1++;
    *data0 = *data2; data0++; data2++;
    *data0 = *data3; data0++; data3++;
  }

  return (true);
}


bool vector_(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = 0.0; data0++;
  }

  return (true);
}


bool eq_vv(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if ((data1[0] == data2[0])&&(data1[1] == data2[1])
              &&(data1[2] == data2[2])) *data0 = 1.0;
    else *data0 = 0.0;
    data0++; data1+=3; data2+=3;
  }

  return (true);
}


bool neq_vv(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if ((data1[0] == data2[0])&&(data1[1] == data2[1])
              &&(data1[2] == data2[2])) *data0 = 0.0;
    else *data0 = 1.0;
    data0++; data1+=3; data2+=3;
  }

  return (true);
}


bool x_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = data1[0]; data0++; data1 += 3;
  }

  return (true);
}

bool y_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = data1[1]; data0++; data1 += 3;
  }

  return (true);
}

bool z_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = data1[2]; data0++; data1 += 3;
  }

  return (true);
}



bool isfinite_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if (IsFinite(data1[0])&&IsFinite(data1[1])&&IsFinite(data1[2]))
      *data0 = 1.0; else *data0 = 0.0;

    data0++; data1+=3;
  }

  return (true);
}

bool isinfinite_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if (IsInfinite(data1[0])||IsInfinite(data1[1])||IsInfinite(data1[2]))
      *data0 = 1.0; else *data0 = 0.0;
    data0++; data1+=3;
  }

  return (true);
}

bool isnan_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if (IsNan(data1[0])||IsNan(data1[1])||IsNan(data1[2]))
      *data0 = 1.0; else *data0 = 0.0;
    data0++; data1+=3;
  }

  return (true);
}

bool length2_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    data0[0] = data1[0]*data1[0]+data1[1]*data1[1]+data1[2]*data1[2];
    data0++; data1+=3;
  }

  return (true);
}



bool norm_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    data0[0] = ::sqrt(data1[0]*data1[0]+data1[1]*data1[1]+data1[2]*data1[2]);
    data0++; data1+=3;
  }

  return (true);
}


bool maxnorm_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    double a = data1[0];
    if (a < std::fabs(data1[1])) a = std::fabs(data1[1]);
    if (a < std::fabs(data1[2])) a = std::fabs(data1[2]);
    data0++; data1+=3;
  }

  return (true);
}


bool max_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    double a = data1[0];
    if (a < data1[1]) a = data1[1];
    if (a < data1[2]) a = data1[2];
    data0++; data1+=3;
  }

  return (true);
}

bool min_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    double a = data1[0];
    if (a > data1[1]) a = data1[1];
    if (a > data1[2]) a = data1[2];
    data0++; data1+=3;
  }

  return (true);
}

bool boolean_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    if(data1[0] || data1[1] || data1[2]) *data0 = 1.0; else *data0 = 0.0;
    data0++; data1+=3;
  }

  return (true);
}


bool inv_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = 1.0/(*data1); data0++; data1++;
  }

  return (true);
}



bool abs_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    if(*data1<0) *data0 = -*data1; else *data0 = *data1; data0++; data1++;
  }

  return (true);
}


bool round_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = static_cast<double>(static_cast<int>(*data1+0.5)); data0++; data1++;
  }

  return (true);
}

bool floor_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::floor(*data1); data0++; data1++;
  }

  return (true);
}

bool ceil_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::ceil(*data1); data0++; data1++;
  }

  return (true);
}

bool exp_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::exp(*data1); data0++; data1++;
  }

  return (true);
}

bool pow_vs(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::pow(*data0,*data2); data0++; data1++;
    *data0 = ::pow(*data0,*data2); data0++; data1++;
    *data0 = ::pow(*data0,*data2); data0++; data1++; data2++;
  }

  return (true);
}

bool sqrt_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::sqrt(*data1); data0++; data1++;
  }

  return (true);
}

bool log_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::log(*data1); data0++; data1++;
  }

  return (true);
}

bool ln_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::log(*data1); data0++; data1++;
  }

  return (true);
}

bool log2_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  double s = 1.0/log(2.0);
  while (data0 != data0_end)
  {
    *data0 = ::log(*data1)*s; data0++; data1++;
  }

  return (true);
}

bool log10_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  double s = 1.0/log(10.0);
  while (data0 != data0_end)
  {
    *data0 = ::log(*data1)*s; data0++; data1++;
  }

  return (true);
}

bool cbrt_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::pow(*data1,1.0/3.0); data0++; data1++;
  }

  return (true);
}

bool sin_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::sin(*data1); data0++; data1++;
  }

  return (true);
}

bool cos_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::cos(*data1); data0++; data1++;
  }

  return (true);
}

bool tan_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::tan(*data1); data0++; data1++;
  }

  return (true);
}

bool sinh_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::sinh(*data1); data0++; data1++;
  }

  return (true);
}

bool cosh_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::cosh(*data1); data0++; data1++;
  }

  return (true);
}

bool asin_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::asin(*data1); data0++; data1++;
  }

  return (true);
}

bool acos_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::acos(*data1); data0++; data1++;
  }

  return (true);
}

bool atan_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = ::atan(*data1); data0++; data1++;
  }

  return (true);
}

bool asinh_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    double d = *data1;
    *data0 = (d==0?0:(d>0?1:-1)) * ::log((d<0?-d:d) + ::sqrt(1+d*d)); data0++; data1++;
  }

  return (true);
}

bool acosh_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    double d = *data1;
    *data0 = ::log(d + ::sqrt(d*d-1)); data0++; data1++;
  }

  return (true);
}


bool dot_vv(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data0_end = data0 + pc.get_size();

  while (data0 != data0_end)
  {
    *data0 = data1[0]*data2[0] + data1[1]*data2[1] + data1[2]*data2[2];
    data0++; data1+=3; data2 +=3;
  }

  return (true);
}

bool mult_vv(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data0_end = data0 + pc.get_size()*3;

  while (data0 != data0_end)
  {
    data0[0] = data1[0]*data2[0];
    data0[1] = data1[1]*data2[1];
    data0[2] = data1[2]*data2[2];
    data0+=3; data1+=3; data2 +=3;
  }

  return (true);
}



bool cross_vv(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data2 = pc.get_variable(2);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    data0[0] = data1[1]*data2[2] - data1[2]*data2[1];
    data0[1] = data1[2]*data2[0] - data1[0]*data2[2];
    data0[2] = data1[0]*data2[1] - data1[1]*data2[0];
    data0+=3; data1+=3; data2 +=3;
  }

  return (true);
}


bool normalize_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    double s = 0.0;
    double len = ::sqrt(data1[0]*data1[0]+data1[1]*data1[1]+data1[2]*data1[2]);
    if (len > 0.0) s = 1.0/len;

    data0[0] = s*data1[0];
    data0[1] = s*data1[1];
    data0[2] = s*data1[2];

    data0+=3; data1+=3;
  }

  return (true);
}

bool find_normal1_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    double a0,a1,a2;
    double b0,b1,b2;
    a0 = std::fabs(data1[0]);
    a1 = std::fabs(data1[1]);
    a2 = std::fabs(data1[2]);

    if ((a0==0.0)&&(a1==0.0)&&(a2==0.0))
    {
      data0[0] = 0.0; data0[1] = 0.0; data0[2] = 0.0;
    }
    else
    {
      // determine the two other normal directions
      if ((a0 >= a1)&&(a0 >= a2))
      {
       b0 = data1[1]+data1[2]; b1 = -data1[0]; b2 = -data1[1];
      }
      else if ((a1 >= a0)&&(a1 >= a2))
      {
       b0 = -data1[1]; b1 = data1[0]+data1[2]; b2 = -data1[1];
      }
      else
      {
       b0 = -data1[2]; b1 = -data1[2]; b2 = data1[0]+data1[1];
      }

      double s = 1.0/::sqrt(b0*b0+b1*b1+b2*b2);
      data0[0] = b0*s; data0[1] = b1*s; data0[2] = b2*s;
    }
    data0+=3; data1+=3;
  }

  return (true);
}

bool find_normal2_v(SCIRun::ArrayMathProgramCode& pc)
{
  double* data0 = pc.get_variable(0);
  double* data1 = pc.get_variable(1);
  double* data0_end = data0 + 3*pc.get_size();

  while (data0 != data0_end)
  {
    double a0,a1,a2;
    double b0,b1,b2;
    double c0,c1,c2;
    a0 = std::fabs(data1[0]);
    a1 = std::fabs(data1[1]);
    a2 = std::fabs(data1[2]);

    if ((a0==0.0)&&(a1==0.0)&&(a2==0.0))
    {
      data0[0] = 0.0; data0[1] = 0.0; data0[2] = 0.0;
    }
    else
    {
      // determine the two other normal directions
      if ((a0 >= a1)&&(a0 >= a2))
      {
       b0 = data1[1]+data1[2]; b1 = -data1[0]; b2 = -data1[1];
      }
      else if ((a1 >= a0)&&(a1 >= a2))
      {
       b0 = -data1[1]; b1 = data1[0]+data1[2]; b2 = -data1[1];
      }
      else
      {
       b0 = -data1[2]; b1 = -data1[2]; b2 = data1[0]+data1[1];
      }

      double s = 1.0/::sqrt(b0*b0+b1*b1+b2*b2);
      c0 = b0*s; c1 = b1*s; c2 = b2*s;

      data0[0] = a1*b2-a2*b1;
      data0[1] = a2*b0-a0*b2;
      data0[2] = a0*b1-a1*b0;
    }
    data0+=3; data1+=3;
  }

  return (true);
}

} // end namsespace

namespace SCIRun {

void
InsertVectorArrayMathFunctionCatalog(ArrayMathFunctionCatalogHandle& catalog)
{
  // Constructors for vectors (or points for some SCIRun applications)
  catalog->add_function(ArrayMathFunctions::vector_sss,"vector$S:S:S","V");
  catalog->add_function(ArrayMathFunctions::vector_sss,"Vector$S:S:S","V");
  catalog->add_function(ArrayMathFunctions::vector_sss,"point$S:S:S","V");
  catalog->add_function(ArrayMathFunctions::vector_sss,"Point$S:S:S","V");
  catalog->add_cst_function(ArrayMathFunctions::vector_,"vector","V");
  catalog->add_cst_function(ArrayMathFunctions::vector_,"Vector","V");
  catalog->add_cst_function(ArrayMathFunctions::vector_,"point","V");
  catalog->add_cst_function(ArrayMathFunctions::vector_,"Point","V");

  // Test nan inf
  catalog->add_function(ArrayMathFunctions::isnan_v,"isnan$V","S");
  catalog->add_function(ArrayMathFunctions::isfinite_v,"isfinite$V","S");
  catalog->add_function(ArrayMathFunctions::isinfinite_v,"isinfinite$V","S");
  catalog->add_function(ArrayMathFunctions::isinfinite_v,"isinf$V","S");

  // Getting components e.g. x(Vector(0,0,0))
  catalog->add_function(ArrayMathFunctions::x_v,"x$V","S");
  catalog->add_function(ArrayMathFunctions::y_v,"y$V","S");
  catalog->add_function(ArrayMathFunctions::z_v,"z$V","S");
  catalog->add_function(ArrayMathFunctions::x_v,"u$V","S");
  catalog->add_function(ArrayMathFunctions::y_v,"v$V","S");
  catalog->add_function(ArrayMathFunctions::z_v,"w$V","S");

  catalog->add_function(ArrayMathFunctions::maxnorm_v,"maxnorm$V","S");
  catalog->add_function(ArrayMathFunctions::norm_v,"norm$V","S");
  catalog->add_function(ArrayMathFunctions::norm_v,"length$V","S");
  catalog->add_function(ArrayMathFunctions::length2_v,"length2$V","S");
  catalog->add_function(ArrayMathFunctions::min_v,"min$V","S");
  catalog->add_function(ArrayMathFunctions::max_v,"max$V","S");

  // Converting to boolean    == Vector(0,0,0)
  catalog->add_function(ArrayMathFunctions::boolean_v,"boolean$V","S");

  // Vectorized operations
  catalog->add_function(ArrayMathFunctions::inv_v,"inv$V","V");
  catalog->add_function(ArrayMathFunctions::abs_v,"abs$V","V");
  catalog->add_function(ArrayMathFunctions::round_v,"round$V","V");
  catalog->add_function(ArrayMathFunctions::floor_v,"floor$V","V");
  catalog->add_function(ArrayMathFunctions::ceil_v,"ceil$V","V");
  catalog->add_function(ArrayMathFunctions::exp_v,"exp$V","V");
  catalog->add_function(ArrayMathFunctions::pow_vs,"pow$V:S","V");
  catalog->add_function(ArrayMathFunctions::sqrt_v,"sqrt$V","V");
  catalog->add_function(ArrayMathFunctions::log_v,"log$V","V");
  catalog->add_function(ArrayMathFunctions::ln_v,"ln$V","V");
  catalog->add_function(ArrayMathFunctions::log2_v,"log2$V","V");
  catalog->add_function(ArrayMathFunctions::log10_v,"log10$V","V");
  catalog->add_function(ArrayMathFunctions::cbrt_v,"cbrt$V","V");
  catalog->add_function(ArrayMathFunctions::sin_v,"sin$V","V");
  catalog->add_function(ArrayMathFunctions::cos_v,"cos$V","V");
  catalog->add_function(ArrayMathFunctions::tan_v,"tan$V","V");
  catalog->add_function(ArrayMathFunctions::sinh_v,"sinh$V","V");
  catalog->add_function(ArrayMathFunctions::cosh_v,"cosh$V","V");
  catalog->add_function(ArrayMathFunctions::asin_v,"asin$V","V");
  catalog->add_function(ArrayMathFunctions::acos_v,"acos$V","V");
  catalog->add_function(ArrayMathFunctions::atan_v,"atan$V","V");
  catalog->add_function(ArrayMathFunctions::asinh_v,"asinh$V","V");
  catalog->add_function(ArrayMathFunctions::acosh_v,"acosh$V","V");

  catalog->add_sym_function(ArrayMathFunctions::dot_vv,"dot$V:V","S");
  catalog->add_function(ArrayMathFunctions::cross_vv,"cross$V:V","V");
  catalog->add_function(ArrayMathFunctions::mult_vv,"mult$V:V","V");
  catalog->add_function(ArrayMathFunctions::normalize_v,"normalize$V","V");
  catalog->add_function(ArrayMathFunctions::find_normal1_v,"find_normal$V","V");
  catalog->add_function(ArrayMathFunctions::find_normal1_v,"find_normal1$V","V");
  catalog->add_function(ArrayMathFunctions::find_normal2_v,"find_normal2$V","V");
  catalog->add_function(ArrayMathFunctions::find_normal1_v,"findnormal$V","V");
  catalog->add_function(ArrayMathFunctions::find_normal1_v,"findnormal1$V","V");
  catalog->add_function(ArrayMathFunctions::find_normal2_v,"findnormal2$V","V");

  catalog->add_sym_function(ArrayMathFunctions::eq_vv,"eq$V:V","S");
  catalog->add_sym_function(ArrayMathFunctions::neq_vv,"neq$V:V","S");

}



} // end namespace
