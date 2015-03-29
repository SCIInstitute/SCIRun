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
///@file  ColumnMatrix.cc
///@brief for RHS and LHS
/// 
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  July 1994
/// 

#include <cmath>
#include <Core/Datatypes/ColumnMatrixFunctions.h>

namespace SCIRun {

namespace Private {

double linalg_norm2(size_type rows, const double* data)
{
  double norm=0;
  index_type i;
  double sum0=0;
  double sum1=0;
  double sum2=0;
  double sum3=0;
  index_type r3=rows-3;
  for(i=0;i<r3;i+=4)
  {
    double d0=data[i+0];
    double d1=data[i+1];
    double d2=data[i+2];
    double d3=data[i+3];
    sum0+=d0*d0;
    sum1+=d1*d1;
    sum2+=d2*d2;
    sum3+=d3*d3;
  }
  norm=(sum0+sum1)+(sum2+sum3);
  for(;i<rows;i++)
  {
    double d=data[i];
    norm+=d*d;
  }
  return sqrt(norm);
}

void linalg_mult(size_type rows, double* res, double* a, double* b)
{
  index_type i=0;
  for(;i<rows;i++)
  {
    res[i]=a[i]*b[i];
  }
}

void linalg_sub(size_type rows, double* res, double* a, double* b)
{
  index_type i=0;
  for(;i<rows;i++)
  {
    res[i]=a[i]-b[i];
  }
}

void linalg_add(size_type rows, double* res, double* a, double* b)
{
  index_type i=0;
  for(;i<rows;i++)
  {
    res[i]=a[i]+b[i];
  }
}

double linalg_dot(size_type rows, double* a, double* b)
{
  double dot=0;
  index_type i;
  double sum0=0;
  double sum1=0;
  double sum2=0;
  double sum3=0;
  index_type r3=rows-3;
  for(i=0;i<r3;i+=4)
  {
    double a0=a[i+0];
    double a1=a[i+1];
    double a2=a[i+2];
    double a3=a[i+3];
    double b0=b[i+0];
    double b1=b[i+1];
    double b2=b[i+2];
    double b3=b[i+3];
    sum0+=a0*b0;
    sum1+=a1*b1;
    sum2+=a2*b2;
    sum3+=a3*b3;
  }
  dot=(sum0+sum1)+(sum2+sum3);
  for(;i<rows;i++)
  {
    dot+=a[i]*b[i];
  }
  return dot;
}

void linalg_smadd(size_type rows, double* res, 
                  double s, double* a, double* b)
{
  for (index_type i = 0; i < rows; i++)
  {
    res[i] = s * a[i] + b[i];
  }
}

}
} // End namespace SCIRun

