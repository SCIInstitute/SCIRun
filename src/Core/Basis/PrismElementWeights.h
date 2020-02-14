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


#ifndef CORE_BASIS_PRISMWEIGHTS_H
#define CORE_BASIS_PRISMWEIGHTS_H 1

namespace SCIRun {
namespace Core {
namespace Basis {

class PrismElementWeights {

public:
  template <class VECTOR>
  void get_linear_weights(const VECTOR& coords, double *w) const
  {
    const double x = static_cast<double>(coords[0]), y = static_cast<double>(coords[1]), z = static_cast<double>(coords[2]);
    w[0] = (-1 + x + y) * (-1 + z);
    w[1] = - (x * (-1 + z));
    w[2] = - (y * (-1 + z));
    w[3] = - ((-1 + x + y) * z);
    w[4] = +x * z;
    w[5] = +y * z;
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_linear_derivate_weights(const VECTOR& coords, double *w) const
  {
    const double x = static_cast<double>(coords[0]), y = static_cast<double>(coords[1]), z = static_cast<double>(coords[2]);
    w[0]= (-1 + z);
    w[1]= (1 - z);
    w[2]= 0;
    w[3]= - z;
    w[4]= +z;
    w[5]= 0;
    w[6]= (-1 + z);
    w[7]= 0;
    w[8]= (1 - z);
    w[9]= - z;
    w[10]= 0;
    w[11]= z;
    w[12]= (-1 + x + y);
    w[13]= - x;
    w[14]= - y;
    w[15]= (1 - x - y);
    w[16]= x;
    w[17]= y;
  }

  /// get weight factors at parametric coordinate
  template< class VECTOR>
  void get_quadratic_weights(const VECTOR& coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
    w[0] = -((-1 + x + y)*(-1 + z)*(-1 + 2*x + 2*y + 2*z));
    w[1] = -(x*(-1 + 2*x -2*z)*(-1 + z));
    w[2] = -(y*(-1 + 2*y - 2*z)*(-1 + z));
    w[3] = (-1 + x + y)*(1 + 2*x + 2*y - 2*z)*z;
    w[4] = +x*z*(-3 + 2*x + 2*z);
    w[5] = y*z*(-3 + 2*y + 2*z);
    w[6] = +4*x*(-1 + x + y)*(-1 + z);
    w[7] = -4*x*y*(-1 + z);
    w[8] = +4*y*(-1 + x + y)*(-1 + z);
    w[9] = +4*(-1 + x + y)*(-1 + z)*z;
    w[10] = -4*x*(-1 + z)*z;
    w[11] = -4*y*(-1 + z)*z;
    w[12] = -4*x*(-1 + x +y)*z;
    w[13] = +4*x*y*z;
    w[14] = -4*y*(-1 + x + y)*z;
  }

  /// get weight factors of derivative at parametric coordinate
  template< class VECTOR>
  void get_quadratic_derivate_weights(const VECTOR& coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
    w[0]=-(-1 + z)*(-3 + 4*x + 4*y + 2*z);
    w[1]=-(-1 + 4*x - 2*z)*(-1 + z);
    w[2]=0;
    w[3]=+(-1 + 4*x + 4*y - 2*z)*z;
    w[4]=+z*(-3 + 4*x + 2*z);
    w[5]=0;
    w[6]=+4*(-1 + 2*x + y)*(-1 +z);
    w[7]=-4*y*(-1 + z);
    w[8]=+4*y*(-1 + z);
    w[9]=+4*(-1 + z)*z;
    w[10]=-4*(-1 + z)*z;
    w[11]=0;
    w[12]=-4*(-1 + 2*x + y)*z;
    w[13]=+4*y*z;
    w[14]=-4*y*z;
    w[15]=-((-1 + z)*(-3 + 4*x + 4*y + 2*z));
    w[16]=0;
    w[17]=-((-1 + 4*y - 2*z)*(-1 + z));
    w[18]=+(-1 + 4*x + 4*y - 2*z)*z;
    w[19]=0;
    w[20]=+z*(-3 + 4*y + 2*z);
    w[21]=+4*x*(-1 + z);
    w[22]=-4*x*(-1 + z);
    w[23]=+4*(-1 + x + 2*y)*(-1 + z);
    w[24]=4*(-1 + z)*z;
    w[25]=0;
    w[26]=-4*(-1 + z)*z;
    w[27]=-4*x*z;
    w[28]=+4*x*z;
    w[29]=-4*(-1 + x + 2*y)*z;
    w[30]=-((-1 + x + y)*(-3 + 2*x + 2*y + 4*z));
    w[31]=+x*(-1 - 2*x + 4*z);
    w[32]=+y*(-1 - 2*y + 4*z);
    w[33]=+(-1 + x + y)*(1 + 2*x + 2*y - 4*z);
    w[34]=+x*(-3 + 2*x + 4*z);
    w[35]=+y*(-3 + 2*y + 4*z);
    w[36]=+4*x*(-1 + x + y);
    w[37]=-4*x*y;
    w[38]=+4*y*(-1 + x + y);
    w[39]=+4*(-1 + x + y)*(-1 + 2*z);
    w[40]=+x*(4 - 8*z);
    w[41]=+y*(4 - 8*z);
    w[42]=-4*x*(-1 + x + y);
    w[43]=+4*x*y;
    w[44]=-4*y*(-1 + x + y);
  }


  template <class VECTOR>
  void get_cubic_weights(const VECTOR &coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
    const double x2=x*x;
    const double y2=y*y;
    const double z2=z*z;
    const double y12=(y-1)*(y-1);
    const double z12=(z-1)*(z-1);

    w[0]  = -((-1 + x + y)*(-1 + z)*(-1 + 2*x2 - y + 2*y2 - x*(1 + 2*y) - z + 2*z2));
    w[1]  = -(x*(1 - 2*x + x2 - y2)*(-1 + z));
    w[2]  = +(x2 - y12)*y*(-1 + z);
    w[3]  = -((-1 + x + y)*z12*z);
    w[4]  = +x*(-1 + z)*(-3*x + 2*x2 + z*(-1 + 2*z));
    w[5]  = +x2*(-1 + x + z - x*z);
    w[6]  = -(x2*y*(-1 + z));
    w[7]  = +x*z12*z;
    w[8]  = +y*(-1 + z)*(-3*y + 2*y2 + z*(-1 + 2*z));
    w[9]  = -(x*y2*(-1 + z));
    w[10] = +y2*(-1 + y + z - y*z);
    w[11] = +y*z12*z;
    w[12] = +(-1 + x + y)*z*(2*x2 - y + 2*y2 - x*(1 + 2*y) + z*(-3 + 2*z));
    w[13] = +x*(1 - 2*x + x2 - y2)*z;
    w[14] = +(-x2 + y12)*y*z;
    w[15] = -((-1 + x + y)*(-1 + z)*z2);
    w[16] = +x*z*(-1 + 3*x - 2*x2 + 3*z - 2*z2);
    w[17] = +(-1 + x)*x2*z;
    w[18] = +x2*y*z;
    w[19] = x*(-1 + z)*z2;
    w[20] = +y*z*(-1 + 3*y - 2*y2 + 3*z - 2*z2);
    w[21] = +x*y2*z;
    w[22] = +(-1 + y)*y2*z;
    w[23] = +y*(-1 + z)*z2;
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_cubic_derivate_weights(const VECTOR &coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
    const double x2=x*x;
    const double y2=y*y;
    const double z2=z*z;
    const double y12=(y-1)*(y-1);
    const double z12=(z-1)*(z-1);
    w[0]=-((-1 + z)*(-6*x + 6*x2 + z*(-1 + 2*z)));
    w[1]=-((1 - 4*x + 3*x2 - y2)*(-1 + z));
    w[2]=+2*x*y*(-1 + z);
    w[3]=-(z12*z);
    w[4]=+(-1 + z)*(-6*x + 6*x2 + z*(-1 + 2*z));
    w[5]=-(x*(-2 + 3*x)*(-1 + z));
    w[6]=-2*x*y*(-1 + z);
    w[7]=+z12*z;
    w[8]=0;
    w[9]=-(y2*(-1 + z));
    w[10]=0;
    w[11]=0;
    w[12]=z*(1- 6*x + 6*x2 - 3*z + 2*z2);
    w[13]=+(1 - 4*x + 3*x2 - y2)*z;
    w[14]=-2*x*y*z;
    w[15]=-((-1 + z)*z2);
    w[16]=+z*(-1 + 6*x - 6*x2 + 3*z - 2*z2);
    w[17]=+x*(-2 + 3*x)*z;
    w[18]=+2*x*y*z;
    w[19]=+(-1 + z)*z2;
    w[20]=0;
    w[21]=+y2*z;
    w[22]=0;
    w[23]=0;
    w[24]=-((-1 + z)*(-6*y + 6*y2 + z*(-1 + 2*z)));
    w[25]=+2*x*y*(-1 + z);
    w[26]=+(-1 + x2 + 4*y - 3*y2)*(-1 + z);
    w[27]=-(z12*z);
    w[28]=0;
    w[29]=0;
    w[30]=-(x2*(-1 + z));
    w[31]=0;
    w[32]=+(-1 + z)*(-6*y + 6*y2 + z*(-1 + 2*z));
    w[33]=-2*x*y*(-1 + z);
    w[34]=-(y*(-2 + 3*y)*(-1 + z));
    w[35]=+z12*z;
    w[36]=+z*(1 - 6*y + 6*y2 - 3*z + 2*z2);
    w[37]=-2*x*y*z;
    w[38]=-((-1 + x2 + 4*y - 3*y2)*z);
    w[39]=-((-1 + z)*z2);
    w[40]=0;
    w[41]=0;
    w[42]=+x2*z;
    w[43]=0;
    w[44]=z*(-1 + 6*y - 6*y2 + 3*z - 2*z2);
    w[45]=2*x*y*z;
    w[46]=y*(-2 + 3*y)*z;
    w[47]=(-1 + z)*z2;
    w[48]=-((-1 + x + y)*(2*x2 - y + 2*y2 - x*(1 + 2*y) + 6*(-1 + z)*z));
    w[49]=x*(-1 + 2*x - x2 + y2);
    w[50]=+(x2 - y12)*y;
    w[51]=-((-1 + x + y)*(1 - 4*z + 3*z2));
    w[52]=+x*(1 - 3*x + 2*x2 - 6*z + 6*z2);
    w[53]=-((-1 + x)*x2);
    w[54]=-(x2*y);
    w[55]=+x*(1 - 4*z + 3*z2);
    w[56]=+y*(1 - 3*y + 2*y2 - 6*z + 6*z2);
    w[57]=-(x*y2);
    w[58]=-((-1 + y)*y2);
    w[59]=+y*(1 - 4*z + 3*z2);
    w[60]=+(-1 + x + y)*(2*x2 - y + 2*y2 - x*(1 + 2*y) + 6*(-1 + z)*z);
    w[61]=+x*(1 - 2*x + x2 - y2);
    w[62]=+(-x2 + y12)*y;
    w[63]=-((-1 + x + y)*z*(-2 + 3*z));
    w[64]=+x*(-1 + 3*x - 2*x2 + 6*z - 6*z2);
    w[65]=+(-1 + x)*x2;
    w[66]=+x2*y;
    w[67]=+x*z*(-2 + 3*z);
    w[68]=+y*(-1 + 3*y - 2*y2 + 6*z - 6*z2);
    w[69]=+x*y2;
    w[70]=+(-1 + y)*y2;
    w[71]=+y*z*(-2 + 3*z);
  }

  inline int num_linear_weights() { return 6; }
  inline int num_quadratic_weights() { return 15; }
  inline int num_cubic_weights() { return 24; }

  inline int num_linear_derivate_weights() { return 18; }
  inline int num_quadratic_derivate_weights() { return 45; }
  inline int num_cubic_derivate_weights() { return 72; }


  inline int num_derivs()  { return 3; }
  inline int num_hderivs() { return 3; }
};

}}}

#endif
