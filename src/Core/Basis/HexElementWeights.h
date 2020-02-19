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


#ifndef CORE_BASIS_HEXWEIGHTS_H
#define CORE_BASIS_HEXWEIGHTS_H 1

namespace SCIRun {
namespace Core {
namespace Basis {

class HexElementWeights
{
public:
  template <class VECTOR>
  void get_linear_weights(const VECTOR& coords, double *w) const
  {
    const double x = static_cast<double>(coords[0]), y = static_cast<double>(coords[1]), z = static_cast<double>(coords[2]);
    w[0] = -((-1 + x)*(-1 + y)*(-1 + z));
    w[1] = x*(-1 + y)*(-1 + z);
    w[2] = -(x*y*(-1 + z));
    w[3] = (-1 + x)*y*(-1 + z);
    w[4] = (-1 + x)*(-1 + y)*z;
    w[5] = -(x*(-1 + y)*z);
    w[6] = x*y*z;
    w[7] = -((-1 + x)*y*z);
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_linear_derivate_weights(const VECTOR& coords, double *w) const
  {
    const double x = static_cast<double>(coords[0]), y = static_cast<double>(coords[1]), z = static_cast<double>(coords[2]);
    w[0]=(-1 + y + z - y * z);
    w[1]= (-1 + y) * (-1 + z);
    w[2]= (y - y * z);
    w[3]= y * (-1 + z);
    w[4]= (-1 + y) * z;
    w[5]= (z - y * z);
    w[6]=  y * z;
    w[7]=  -(y * z);
    w[8]=(-1 + x + z - x * z);
    w[9]= x * (-1 + z);
    w[10]=  (x - x * z);
    w[11]=  (-1 + x) * (-1 + z);
    w[12]=  (-1 + x) * z;
    w[13]=  -(x * z);
    w[14]=  x * z;
    w[15]=  (z - x * z);
    w[16]=(-1 + x + y - x * y);
    w[17]= x * (-1 + y);
    w[18]=  -(x * y);
    w[19]= (-1 + x) * y;
    w[20]= (-1 + x) * (-1 + y);
    w[21]=  (x - x * y);
    w[22]=  x * y;
    w[23]=  (y - x * y);

  }

  /// get weight factors at parametric coordinate
  template< class VECTOR>
  void get_quadratic_weights(const VECTOR& coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
    w[0]  = (-1 + x)*(-1 + y)*(-1 + z)*(-1 + 2*x + 2*y + 2*z);
    w[1]  = +x*(-1 + y)*(-1 + 2*x - 2*y - 2*z)*(-1 + z);
    w[2]  = -x*y*(-3 + 2*x + 2*y - 2*z)*(-1 + z);
    w[3]  = -(-1 + x)*y*(-1 + z)*(1 + 2*x - 2*y + 2*z);
    w[4]  = -(-1 + x)*(-1 + y)*(1 + 2*x + 2*y - 2*z)*z;
    w[5]  = +x*(-1 + y)*(3 - 2*x + 2*y - 2*z)*z;
    w[6]  = +x*y*z*(-5 + 2*x + 2*y + 2*z);
    w[7]  = +(-1 + x)*y*(3 + 2*x - 2*y - 2*z)*z;
    w[8]  = -4*(-1 + x)*x*(-1 + y)*(-1 + z);
    w[9]  = +4*x*(-1 +y)*y*(-1 + z);
    w[10] = +4*(-1 + x)*x*y*(-1 + z);
    w[11] = -4*(-1 + x)*(-1 + y)*y*(-1 + z);
    w[12] = -4*(-1 + x)*(-1 + y)*(-1 + z)*z;
    w[13] = +4*x*(-1 + y)*(-1 + z)*z;
    w[14] = +4*(-1 + x)*y*(-1 + z)*z;
    w[15] = -4*x*y*(-1 + z)*z;
    w[16] = +4*(-1 + x)*x*(-1 + y)*z;
    w[17] = -4*x*(-1 + y)*y*z;
    w[18] = -4*(-1 + x)*x*y*z;
    w[19] = +4*(-1 + x)*(-1 + y)*y*z;
  }

  /// get weight factors of derivative at parametric coordinate
  template< class VECTOR>
  void get_quadratic_derivate_weights(const VECTOR& coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
   w[0]=(-1 + y)*(-1 + z)*(-3 + 4*x + 2*y + 2*z);
    w[1]=-((-1 + y)*(-1 + z)*(1 - 4*x + 2*y + 2*z));
    w[2]=-(y*(-3 + 4*x + 2*y - 2*z)*(-1 + z));
    w[3]=+y*(1 - 4*x + 2*y - 2*z)*(-1 + z);
    w[4]=-((-1 + y)*(-1 + 4*x + 2*y - 2*z)*z);
    w[5]=+(-1 + y)*(3 - 4*x + 2*y - 2*z)*z;
    w[6]=+y*z*(-5 + 4*x + 2*y + 2*z);
    w[7]=+y*(1 + 4*x - 2*y - 2*z)*z;
    w[8]=-4*(-1 + 2*x)*(-1 + y)*(-1 + z);
    w[9]=+4*(-1 + y)*y*(-1 + z);
    w[10]=+4*(-1 + 2*x)*y*(-1 + z);
    w[11]=+4*y*(-1 + y + z - y*z);
    w[12]=+4*z*(-1 + y + z - y*z);
    w[13]=+4*(-1 + y)*(-1 + z)*z;
    w[14]=+4*y*(-1 + z)*z;
    w[15]=-4*y*(-1 + z)*z;
    w[16]=+4*(-1 + 2*x)*(-1 + y)*z;
    w[17]=-4*(-1 + y)*y*z;
    w[18]=+4*(1 - 2*x)*y*z;
    w[19]=+4*(-1 + y)*y*z;

    w[20]=(-1 + x)*(-1 + z)*(-3 + 2*x + 4*y + 2*z);
    w[21]=+x*(1 + 2*x - 4*y - 2*z)*(-1 + z);
    w[22]=-(x*(-3 + 2*x + 4*y - 2*z)*(-1 + z));
    w[23]=-((-1 + x)*(-1 + z)*(1 + 2*x - 4*y + 2*z));
    w[24]=-((-1 + x)*(-1 + 2*x + 4*y - 2*z)*z);
    w[25]=+x*(1 - 2*x + 4*y - 2*z)*z;
    w[26]=+x*z*(-5 + 2*x + 4*y + 2*z);
    w[27]=+(-1 + x)*(3 + 2*x - 4*y - 2*z)*z;
    w[28]=+4*x*(-1 + x + z - x*z);
    w[29]=+4*x*(-1 + 2*y)*(-1 + z);
    w[30]=+4*(-1 + x)*x*(-1 + z);
    w[31]=-4*(-1 + x)*(-1 + 2*y)*(-1 + z);
    w[32]=+4*z*(-1 + x + z - x*z);
    w[33]=+4*x*(-1 + z)*z;
    w[34]=+4*(-1 + x)*(-1 + z)*z;
    w[35]=-4*x*(-1 + z)*z;
    w[36]=+4*(-1 + x)*x*z;
    w[37]=+4*x*(1 - 2*y)*z;
    w[38]=-4*(-1 + x)*x*z;
    w[39]=+4*(-1 + x)*(-1 + 2*y)*z;

    w[40]=(-1 + x)*(-1 + y)*(-3 + 2*x + 2*y + 4*z);
    w[41]=+x*(-1 + y)*(1 + 2*x - 2*y - 4*z);
    w[42]=+x*y*(1 - 2*x - 2*y + 4*z);
    w[43]=-((-1 + x)*y*(-1 + 2*x - 2*y + 4*z));
    w[44]=-((-1 + x)*(-1 + y)*(1 + 2*x + 2*y - 4*z));
    w[45]=+x*(-1 + y)*(3 - 2*x + 2*y - 4*z);
    w[46]=+x*y*(-5 + 2*x + 2*y + 4*z);
    w[47]=+(-1 + x)*y*(3 + 2*x - 2*y - 4*z);
    w[48]=+4*x*(-1 + x + y - x*y);
    w[49]=+4*x*(-1 + y)*y;
    w[50]=+4*(-1 + x)*x*y;
    w[51]=+4*y*(-1 + x + y - x*y);
    w[52]=-4*(-1 + x)*(-1 + y)*(-1 + 2*z);
    w[53]=+4*x*(-1 + y)*(-1 + 2*z);
    w[54]=+4*(-1 + x)*y*(-1 + 2*z);
    w[55]=+4*x*y*(1 - 2*z);
    w[56]=+4*(-1 + x)*x*(-1 + y);
    w[57]=-4*x*(-1 + y)*y;
    w[58]=-4*(-1 + x)*x*y;
    w[59]=+4*(-1 + x)*(-1 + y)*y;

  }


  template <class VECTOR>
  void get_cubic_weights(const VECTOR &coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
    const double x2=x*x;
    const double y2=y*y;
    const double z2=z*z;
    const double x12=(x-1)*(x-1);
    const double y12=(y-1)*(y-1);
    const double z12=(z-1)*(z-1);

    w[0]  = x12*(1 + 2*x)*y12*(1 + 2*y)*z12*(1 + 2*z);
    w[1]  = +x12*x*y12*(1 + 2*y)*z12*(1 + 2*z);
    w[2]  = +x12*(1 + 2*x)*y12*y*z12*(1 + 2*z);
    w[3]  = +x12*(1 + 2*x)*y12*(1 + 2*y)*z12*z;
    w[4]  = +x12*x*y12*y*z12*(1 + 2*z);
    w[5]  = +x12*(1 + 2*x)*y12*y*z12*z;
    w[6]  = +x12*x*y12*(1 + 2*y)*z12*z;
    w[7]  = +x12*x*y12*y*z12*z;
    w[8]  = -(x2*(-3 + 2*x)*y12*(1 + 2*y)*z12*(1 + 2*z));
    w[9]  = +(-1 + x)*x2*y12*(1 + 2*y)*z12*(1 + 2*z);
    w[10] = -(x2*(-3 + 2*x)*y12*y*z12*(1 + 2*z));
    w[11] = -(x2*(-3 + 2*x)*y12*(1 + 2*y)*z12*z);
    w[12] = +(-1 + x)*x2*y12*y*z12*(1 + 2*z);
    w[13] = -(x2*(-3 + 2*x)*y12*y*z12*z) ;
    w[14] = +(-1 + x)*x2*y12*(1 + 2*y)*z12*z;
    w[15] = +(-1 + x)*x2*y12*y*z12*z;
    w[16] = +x2*(-3 + 2*x)*y2*(-3 + 2*y)*z12*(1 + 2*z);
    w[17] = -((-1 + x)*x2*y2*(-3 + 2*y)*z12*(1 + 2*z));
    w[18] = -(x2*(-3 + 2*x)*(-1 + y)*y2*z12*(1 + 2*z));
    w[19] = +x2*(-3 + 2*x)*y2*(-3 + 2*y)*z12*z;
    w[20] = +(-1 + x)*x2*(-1 + y)*y2*z12*(1 + 2*z);
    w[21] = -(x2*(-3 + 2*x)*(-1 + y)*y2*z12*z);
    w[22] = -((-1 + x)*x2*y2*(-3 + 2*y)*z12*z);
    w[23] = +(-1 + x)*x2*(-1 + y)*y2*z12*z;
    w[24] = -(x12*(1 + 2*x)*y2*(-3 + 2*y)*z12*(1 + 2*z));
    w[25] = -(x12*x*y2*(-3 + 2*y)*z12*(1 + 2*z));
    w[26] = +x12*(1 + 2*x)*(-1 + y)*y2*z12*(1 + 2*z);
    w[27] = -(x12*(1 + 2*x)*y2*(-3 + 2*y)*z12*z);
    w[28] = +x12*x*(-1 + y)*y2*z12*(1 + 2*z);
    w[29] = +x12*(1 + 2*x)*(-1 + y)*y2*z12*z;
    w[30] = -(x12*x*y2*(-3 + 2*y)*z12*z);
    w[31] = +x12*x*(-1 + y)*y2*z12*z;
    w[32] = -(x12*(1 + 2*x)*y12*(1 + 2*y)*z2*(-3 + 2*z));
    w[33] = -(x12*x*y12*(1 + 2*y)*z2*(-3 + 2*z));
    w[34] = -(x12*(1 + 2*x)*y12*y*z2*(-3 + 2*z));
    w[35] = +x12*(1 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z2;
    w[36] = -(x12*x*y12*y*z2*(-3 + 2*z));
    w[37] = +x12*(1 + 2*x)*y12*y*(-1 + z)*z2;
    w[38] = +x12*x*y12*(1 + 2*y)*(-1 + z)*z2;
    w[39] = +x12*x*y12*y*(-1 + z)*z2;
    w[40] = +x2*(-3 + 2*x)*y12*(1 + 2*y)*z2*(-3 + 2*z);
    w[41] = -((-1 + x)*x2*y12*(1 + 2*y)*z2*(-3 + 2*z));
    w[42] = +x2*(-3 + 2*x)*y12*y*z2*(-3 + 2*z);
    w[43] = -(x2*(-3 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z2);
    w[44] = -((-1 + x)*x2*y12*y*z2*(-3 + 2*z));
    w[45] = -(x2*(-3 + 2*x)*y12*y*(-1 + z)*z2);
    w[46] = +(-1 + x)*x2*y12*(1 + 2*y)*(-1 + z)*z2;
    w[47] = +(-1 + x)*x2*y12*y*(-1 + z)*z2;
    w[48] = -(x2*(-3 + 2*x)*y2*(-3 + 2*y)*z2*(-3 + 2*z));
    w[49] = +(-1 + x)*x2*y2*(-3 + 2*y)*z2*(-3 + 2*z);
    w[50] = +x2*(-3 + 2*x)*(-1 + y)*y2*z2*(-3 + 2*z);
    w[51] = +x2*(-3 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z2;
    w[52] = -((-1 + x)*x2*(-1 + y)*y2*z2*(-3 + 2*z));
    w[53] = -(x2*(-3 + 2*x)*(-1 + y)*y2*(-1 + z)*z2);
    w[54] = -((-1 + x)*x2*y2*(-3 + 2*y)*(-1 + z)*z2);
    w[55] = +(-1 + x)*x2*(-1 + y)*y2*(-1 + z)*z2;
    w[56] = +x12*(1 + 2*x)*y2*(-3 + 2*y)*z2*(-3 + 2*z);
    w[57] = +x12*x*y2*(-3 + 2*y)*z2*(-3 + 2*z);
    w[58] = -(x12*(1 + 2*x)*(-1 + y)*y2*z2*(-3 + 2*z));
    w[59] = -(x12*(1 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z2);
    w[60] = -(x12*x*(-1 + y)*y2*z2*(-3 + 2*z));
    w[61] = +x12*(1 + 2*x)*(-1 + y)*y2*(-1 + z)*z2;
    w[62] = -(x12*x*y2*(-3 + 2*y)*(-1 + z)*z2);
    w[63] = +x12*x*(-1 + y)*y2*(-1 + z)*z2;
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_cubic_derivate_weights(const VECTOR &coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
    const double x2=x*x;
    const double y2=y*y;
    const double z2=z*z;
    const double x12=(x-1)*(x-1);
    const double y12=(y-1)*(y-1);
    const double z12=(z-1)*(z-1);

    w[0]=6*(-1 + x)*x*y12*(1 + 2*y)*z12*(1 + 2*z);
    w[1]=+(1 - 4*x + 3*x2)*y12*(1 + 2*y)*z12*(1 + 2*z);
    w[2]=+6*(-1 + x)*x*y12*y*z12*(1 + 2*z);
    w[3]=+6*(-1 + x)*x*y12*(1 + 2*y)*z12*z;
    w[4]=+(1 - 4*x + 3*x2)*y12*y*z12*(1 + 2*z);
    w[5]=+6*(-1 + x)*x*y12*y*z12*z;
    w[6]=+(1 - 4*x + 3*x2)*y12*(1 + 2*y)*z12*z;
    w[7]=+(1 - 4*x + 3*x2)*y12*y*z12*z;
    w[8]=-6*(-1 + x)*x*y12*(1 + 2*y)*z12*(1 + 2*z);
    w[9]=+x*(-2 + 3*x)*y12*(1 + 2*y)*z12*(1 + 2*z);
    w[10]=-6*(-1 + x)*x*y12*y*z12*(1 + 2*z);
    w[11]=-6*(-1 + x)*x*y12*(1 + 2*y)*z12*z;
    w[12]=+x*(-2 + 3*x)*y12*y*z12*(1 + 2*z);
    w[13]=-6*(-1 + x)*x*y12*y*z12*z;
    w[14]=+x*(-2 + 3*x)*y12*(1 + 2*y)*z12*z;
    w[15]=+x*(-2 + 3*x)*y12*y*z12*z;
    w[16]=+6*(-1 + x)*x*y2*(-3 + 2*y)*z12*(1 + 2*z);
    w[17]=-(x*(-2 + 3*x)*y2*(-3 + 2*y)*z12*(1 + 2*z));
    w[18]=-6*(-1 + x)*x*(-1 + y)*y2*z12*(1 + 2*z);
    w[19]=+6*(-1 + x)*x*y2*(-3 + 2*y)*z12*z;
    w[20]=+x*(-2 + 3*x)*(-1 + y)*y2*z12*(1 + 2*z);
    w[21]=-6*(-1 + x)*x*(-1 + y)*y2*z12*z;
    w[22]=-(x*(-2 + 3*x)*y2*(-3 + 2*y)*z12*z);
    w[23]=+x*(-2 + 3*x)*(-1 + y)*y2*z12*z;
    w[24]=-6*(-1 + x)*x*y2*(-3 + 2*y)*z12*(1 + 2*z);
    w[25]=-((1 - 4*x + 3*x2)*y2*(-3 + 2*y)*z12*(1 + 2*z));
    w[26]=+6*(-1 + x)*x*(-1 + y)*y2*z12*(1 + 2*z);
    w[27]=-6*(-1 + x)*x*y2*(-3 + 2*y)*z12*z;
    w[28]=+(1 - 4*x + 3*x2)*(-1 + y)*y2*z12*(1 + 2*z);
    w[29]=+6*(-1 + x)*x*(-1 + y)*y2*z12*z;
    w[30]=-((1 - 4*x + 3*x2)*y2*(-3 + 2*y)*z12*z);
    w[31]=+(1 - 4*x + 3*x2)*(-1 + y)*y2*z12*z;
    w[32]=-6*(-1 + x)*x*y12*(1 + 2*y)*z2*(-3 + 2*z);
    w[33]=-((1 - 4*x + 3*x2)*y12*(1 + 2*y)*z2*(-3 + 2*z));
    w[34]=-6*(-1 + x)*x*y12*y*z2*(-3 + 2*z);
    w[35]=+6*(-1 + x)*x*y12*(1 + 2*y)*(-1 + z)*z2;
    w[36]=-((1 - 4*x + 3*x2)*y12*y*z2*(-3 + 2*z));
    w[37]=+6*(-1 + x)*x*y12*y*(-1 + z)*z2;
    w[38]=+(1 - 4*x + 3*x2)*y12*(1 + 2*y)*(-1 + z)*z2;
    w[39]=+(1 - 4*x + 3*x2)*y12*y*(-1 + z)*z2;
    w[40]=+6*(-1 + x)*x*y12*(1 + 2*y)*z2*(-3 + 2*z);
    w[41]=-(x*(-2 + 3*x)*y12*(1 + 2*y)*z2*(-3 + 2*z));
    w[42]=+6*(-1 + x)*x*y12*y*z2*(-3 + 2*z);
    w[43]=-6*(-1 + x)*x*y12*(1 + 2*y)*(-1 + z)*z2;
    w[44]=-(x*(-2 + 3*x)*y12*y*z2*(-3 + 2*z));
    w[45]=-6*(-1 + x)*x*y12*y*(-1 + z)*z2;
    w[46]=+x*(-2 + 3*x)*y12*(1 + 2*y)*(-1 + z)*z2;
    w[47]=+x*(-2 + 3*x)*y12*y*(-1 + z)*z2;
    w[48]=-6*(-1 + x)*x*y2*(-3 + 2*y)*z2*(-3 + 2*z);
    w[49]=+x*(-2 + 3*x)*y2*(-3 + 2*y)*z2*(-3 + 2*z);
    w[50]=+6*(-1 + x)*x*(-1 + y)*y2*z2*(-3 + 2*z);
    w[51]=+6*(-1 + x)*x*y2*(-3 + 2*y)*(-1 + z)*z2;
    w[52]=-(x*(-2 + 3*x)*(-1 + y)*y2*z2*(-3 + 2*z));
    w[53]=-6*(-1 + x)*x*(-1 + y)*y2*(-1 + z)*z2;
    w[54]=-(x*(-2 + 3*x)*y2*(-3 + 2*y)*(-1 + z)*z2);
    w[55]=+x*(-2 + 3*x)*(-1 + y)*y2*(-1 + z)*z2;
    w[56]=+6*(-1 + x)*x*y2*(-3 +  2*y)*z2*(-3 + 2*z);
    w[57]=+(1 - 4*x + 3*x2)*y2*(-3 + 2*y)*z2*(-3 + 2*z);
    w[58]=-6*(-1 + x)*x*(-1 + y)*y2*z2*(-3 + 2*z);
    w[59]=-6*(-1 + x)*x*y2*(-3 + 2*y)*(-1 + z)*z2;
    w[60]=-((1 - 4*x + 3*x2)*(-1 + y)*y2*z2*(-3 + 2*z));
    w[61]=+6*(-1 + x)*x*(-1 + y)*y2*(-1 + z)*z2;
    w[62]=-((1 - 4*x + 3*x2)*y2*(-3 + 2*y)*(-1 + z)*z2);
    w[63]=+(1 - 4*x + 3*x2)*(-1 + y)*y2*(-1 + z)*z2;

    w[64]=6*x12*(1 + 2*x)*(-1 + y)*y*z12*(1 + 2*z);
    w[65]=+6*x12*x*(-1 + y)*y*z12*(1 + 2*z);
    w[66]=+x12*(1 + 2*x)*(1 - 4*y + 3*y2)*z12*(1 + 2*z);
    w[67]=+6*x12*(1 + 2*x)*(-1 + y)*y*z12*z;
    w[68]=+x12*x*(1 - 4*y + 3*y2)*z12*(1 + 2*z);
    w[69]=+x12*(1 + 2*x)*(1 - 4*y + 3*y2)*z12*z;
    w[70]=+6*x12*x*(-1 + y)*y*z12*z;
    w[71]=+x12*x*(1 - 4*y + 3*y2)*z12*z;
    w[72]=-6*x2*(-3 + 2*x)*(-1 + y)*y*z12*(1 + 2*z);
    w[73]=+6*(-1 + x)*x2*(-1 + y)*y*z12*(1 + 2*z);
    w[74]=-(x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*z12*(1 + 2*z));
    w[75]=-6*x2*(-3 + 2*x)*(-1 + y)*y*z12*z;
    w[76]=+(-1 + x)*x2*(1 - 4*y + 3*y2)*z12*(1 + 2*z);
    w[77]=-(x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*z12*z);
    w[78]=+6*(-1 + x)*x2*(-1 + y)*y*z12*z;
    w[79]=+(-1 + x)*x2*(1 - 4*y + 3*y2)*z12*z;
    w[80]=+6*x2*(-3 + 2*x)*(-1 + y)*y*z12*(1 + 2*z);
    w[81]=-6*(-1 + x)*x2*(-1 + y)*y*z12*(1 + 2*z);
    w[82]=-(x2*(-3 + 2*x)*y*(-2 + 3*y)*z12*(1 + 2*z));
    w[83]=+6*x2*(-3 + 2*x)*(-1 + y)*y*z12*z;
    w[84]=+(-1 + x)*x2*y*(-2 + 3*y)*z12*(1 + 2*z);
    w[85]=-(x2*(-3 + 2*x)*y*(-2 + 3*y)*z12*z);
    w[86]=-6*(-1 + x)*x2*(-1 + y)*y*z12*z;
    w[87]=+(-1 + x)*x2*y*(-2 + 3*y)*z12*z;
    w[88]=-6*x12*(1 + 2*x)*(-1 + y)*y*z12*(1 + 2*z);
    w[89]=-6*x12*x*(-1 + y)*y*z12*(1 + 2*z);
    w[90]=+x12*(1 + 2*x)*y*(-2 + 3*y)*z12*(1 + 2*z);
    w[91]=-6*x12*(1 + 2*x)*(-1 + y)*y*z12*z;
    w[92]=+x12*x*y*(-2 + 3*y)*z12*(1 + 2*z);
    w[93]=+x12*(1 + 2*x)*y*(-2 + 3*y)*z12*z;
    w[94]=-6*x12*x*(-1 + y)*y*z12*z;
    w[95]=+x12*x*y*(-2 + 3*y)*z12*z;
    w[96]=-6*x12*(1 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z);
    w[97]=-6*x12*x*(-1 + y)*y*z2*(-3 + 2*z);
    w[98]=-(x12*(1 + 2*x)*(1 - 4*y + 3*y2)*z2*(-3 + 2*z));
    w[99]=+6*x12*(1 + 2*x)*(-1 + y)*y*(-1 + z)*z2;
    w[100]=-(x12*x*(1 - 4*y + 3*y2)*z2*(-3 + 2*z));
    w[101]=+x12*(1 + 2*x)*(1 - 4*y + 3*y2)*(-1 + z)*z2;
    w[102]=+6*x12*x*(-1 + y)*y*(-1 + z)*z2;
    w[103]=+x12*x*(1 - 4*y + 3*y2)*(-1 + z)*z2;
    w[104]=+6*x2*(-3 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z);
    w[105]=-6*(-1 + x)*x2*(-1 + y)*y*z2*(-3 + 2*z);
    w[106]=+x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*z2*(-3 + 2*z);
    w[107]=-6*x2*(-3 + 2*x)*(-1 + y)*y*(-1 + z)*z2;
    w[108]=-((-1 + x)*x2*(1 - 4*y + 3*y2)*z2*(-3 + 2*z));
    w[109]=-(x2*(-3 + 2*x)*(1 - 4*y + 3*y2)*(-1 + z)*z2);
    w[110]=+6*(-1 + x)*x2*(-1 + y)*y*(-1 + z)*z2;
    w[111]=+(-1 + x)*x2*(1 - 4*y + 3*y2)*(-1 + z)*z2;
    w[112]=-6*x2*(-3 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z);
    w[113]=+6*(-1 + x)*x2*(-1 + y)*y*z2*(-3 + 2*z);
    w[114]=+x2*(-3 + 2*x)*y*(-2 + 3*y)*z2*(-3 + 2*z);
    w[115]=+6*x2*(-3 + 2*x)*(-1 + y)*y*(-1 + z)*z2;
    w[116]=-((-1 + x)*x2*y*(-2 + 3*y)*z2*(-3 + 2*z));
    w[117]=-(x2*(-3 + 2*x)*y*(-2 + 3*y)*(-1 + z)*z2);
    w[118]=-6*(-1 + x)*x2*(-1 + y)*y*(-1 + z)*z2;
    w[119]=+(-1 + x)*x2*y*(-2 + 3*y)*(-1 + z)*z2;
    w[120]=+6*x12*(1 + 2*x)*(-1 + y)*y*z2*(-3 + 2*z);
    w[121]=+6*x12*x*(-1 + y)*y*z2*(-3 + 2*z);
    w[122]=-(x12*(1 + 2*x)*y*(-2 + 3*y)*z2*(-3 + 2*z));
    w[123]=-6*x12*(1 + 2*x)*(-1 + y)*y*(-1 + z)*z2;
    w[124]=-(x12*x*y*(-2 + 3*y)*z2*(-3 + 2*z));
    w[125]=+x12*(1 + 2*x)*y*(-2 + 3*y)*(-1 + z)*z2;
    w[126]=-6*x12*x*(-1 + y)*y*(-1 + z)*z2;
    w[127]=+x12*x*y*(-2 + 3*y)*(-1 + z)*z2;

    w[128]=6*x12*(1 + 2*x)*y12*(1 + 2*y)*(-1 + z);
    w[129]=+6*x12*x*y12*(1 + 2*y)*(-1 + z)*z;
    w[130]=+6*x12*(1 + 2*x)*y12*y*(-1 + z)*z;
    w[131]=+x12*(1 + 2*x)*y12*(1 + 2*y)*(1 - 4*z + 3*z2);
    w[132]=+6*x12*x*y12*y*(-1 + z)*z;
    w[133]=+x12*(1 + 2*x)*y12*y*(1 - 4*z + 3*z2);
    w[134]=+x12*x*y12*(1 + 2*y)*(1 - 4*z + 3*z2);
    w[135]=+x12*x*y12*y*(1 - 4*z + 3*z2);
    w[136]=-6*x2*(-3 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z;
    w[137]=+6*(-1 + x)*x2*y12*(1 + 2*y)*(-1 + z)*z;
    w[138]=-6*x2*(-3 + 2*x)*y12*y*(-1 + z)*z;
    w[139]=-(x2*(-3 + 2*x)*y12*(1 + 2*y)*(1 - 4*z + 3*z2));
    w[140]=+6*(-1 + x)*x2*y12*y*(-1 + z)*z;
    w[141]=-(x2*(-3 + 2*x)*y12*y*(1 - 4*z + 3*z2));
    w[142]=+(-1 + x)*x2*y12*(1 + 2*y)*(1 - 4*z + 3*z2);
    w[143]=+(-1 + x)*x2*y12*y*(1 - 4*z + 3*z2);
    w[144]=+6*x2*(-3 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z;
    w[145]=-6*(-1 + x)*x2*y2*(-3 + 2*y)*(-1 + z)*z;
    w[146]=-6*x2*(-3 + 2*x)*(-1 + y)*y2*(-1 + z)*z;
    w[147]=+x2*(-3 + 2*x)*y2*(-3 + 2*y)*(1 - 4*z + 3*z2);
    w[148]=+6*(-1 + x)*x2*(-1 + y)*y2*(-1 + z)*z;
    w[149]=-(x2*(-3 + 2*x)*(-1 + y)*y2*(1 - 4*z + 3*z2));
    w[150]=-((-1 + x)*x2*y2*(-3 + 2*y)*(1 - 4*z + 3*z2));
    w[151]=+(-1 + x)*x2*(-1 + y)*y2*(1 - 4*z + 3*z2);
    w[152]=-6*x12*(1 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z;
    w[153]=-6*x12*x*y2*(-3 + 2*y)*(-1 + z)*z;
    w[154]=+6*x12*(1 + 2*x)*(-1 + y)*y2*(-1 + z)*z;
    w[155]=-(x12*(1 + 2*x)*y2*(-3 + 2*y)*(1 - 4*z + 3*z2));
    w[156]=+6*x12*x*(-1 + y)*y2*(-1 + z)*z;
    w[157]=+x12*(1 + 2*x)*(-1 + y)*y2*(1 - 4*z + 3*z2);
    w[158]=-(x12*x*y2*(-3 + 2*y)*(1 - 4*z + 3*z2));
    w[159]=+x12*x*(-1 + y)*y2*(1 - 4*z + 3*z2);
    w[160]=-6*x12*(1 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z;
    w[161]=-6*x12*x*y12*(1 + 2*y)*(-1 + z)*z;
    w[162]=-6*x12*(1 + 2*x)*y12*y*(-1 + z)*z;
    w[163]=+x12*(1 + 2*x)*y12*(1 + 2*y)*z*(-2 + 3*z);
    w[164]=-6*x12*x*y12*y*(-1 + z)*z;
    w[165]=+x12*(1 + 2*x)*y12*y*z*(-2 + 3*z);
    w[166]=+x12*x*y12*(1 + 2*y)*z*(-2 + 3*z);
    w[167]=+x12*x*y12*y*z*(-2 + 3*z);
    w[168]=+6*x2*(-3 + 2*x)*y12*(1 + 2*y)*(-1 + z)*z;
    w[169]=-6*(-1 + x)*x2*y12*(1 + 2*y)*(-1 + z)*z;
    w[170]=+6*x2*(-3 + 2*x)*y12*y*(-1 + z)*z;
    w[171]=-(x2*(-3 + 2*x)*y12*(1 + 2*y)*z*(-2 + 3*z));
    w[172]=-6*(-1 + x)*x2*y12*y*(-1 + z)*z;
    w[173]=-(x2*(-3 + 2*x)*y12*y*z*(-2 + 3*z));
    w[174]=+(-1 + x)*x2*y12*(1 + 2*y)*z*(-2 + 3*z);
    w[175]=+(-1 + x)*x2*y12*y*z*(-2 + 3*z);
    w[176]=-6*x2*(-3 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z;
    w[177]=+6*(-1 + x)*x2*y2*(-3 + 2*y)*(-1 + z)*z;
    w[178]=+6*x2*(-3 + 2*x)*(-1 + y)*y2*(-1 + z)*z;
    w[179]=+x2*(-3 + 2*x)*y2*(-3 + 2*y)*z*(-2 + 3*z);
    w[180]=-6*(-1 + x)*x2*(-1 + y)*y2*(-1 + z)*z;
    w[181]=-(x2*(-3 + 2*x)*(-1 + y)*y2*z*(-2 + 3*z));
    w[182]=-((-1 + x)*x2*y2*(-3 + 2*y)*z*(-2 + 3*z));
    w[183]=+(-1 + x)*x2*(-1 + y)*y2*z*(-2 + 3*z);
    w[184]=+6*x12*(1 + 2*x)*y2*(-3 + 2*y)*(-1 + z)*z;
    w[185]=+6*x12*x*y2*(-3 + 2*y)*(-1 + z)*z;
    w[186]=-6*x12*(1 + 2*x)*(-1 + y)*y2*(-1 + z)*z;
    w[187]=-(x12*(1 + 2*x)*y2*(-3 + 2*y)*z*(-2 + 3*z));
    w[188]=-6*x12*x*(-1 + y)*y2*(-1 + z)*z;
    w[189]=+x12*(1 + 2*x)*(-1 + y)*y2*z*(-2 + 3*z);
    w[190]=-(x12*x*y2*(-3 + 2*y)*z*(-2 + 3*z));
    w[191]=+x12*x*(-1 + y)*y2*z*(-2 + 3*z);
  }

  inline int num_linear_weights() { return 8; }
  inline int num_quadratic_weights() { return 20; }
  inline int num_cubic_weights() { return 64; }

  inline int num_linear_derivate_weights() { return 24; }
  inline int num_quadratic_derivate_weights() { return 60; }
  inline int num_cubic_derivate_weights() { return 192; }

  inline int num_derivs()  { return 3; }
  inline int num_hderivs() { return 7; }
};

}}}

#endif
