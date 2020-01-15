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


#ifndef CORE_BASIS_TETWEIGHTS_H
#define CORE_BASIS_TETWEIGHTS_H 1

namespace SCIRun {
namespace Core {
namespace Basis {

class TetElementWeights {

public:
  template <class VECTOR>
  void get_linear_weights(const VECTOR& coords, double *w) const
  {
    const double x = static_cast<double>(coords[0]), y = static_cast<double>(coords[1]), z = static_cast<double>(coords[2]);
    w[0] = (1. - x - y - z);
    w[1] = x;
    w[2] = y;
    w[3] = z;
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_linear_derivate_weights(const VECTOR& /*coords*/, double *w) const
  {
    w[0] = -1;
    w[1] = 1;
    w[2] = 0;
    w[3] = 0;
    w[4] = -1;
    w[5] = 0;
    w[6] = 1;
    w[7] = 0;
    w[8] = -1;
    w[9] = 0;
    w[10] = 0;
    w[11] = 1;
  }

  /// get weight factors at parametric coordinate
  template< class VECTOR>
  void get_quadratic_weights(const VECTOR& coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
    w[0] = (1 + 2*x*x + 2*y*y - 3*z + 2*z*z + y*(-3 + 4*z) + x*(-3 + 4*y + 4*z));
    w[1] = +x*(-1 + 2*x);
    w[2] = +y*(-1 + 2*y);
    w[3] = +z*(-1 + 2*z);
    w[4] = -4*x*(-1 + x + y + z);
    w[5] = +4*x*y;
    w[6] = -4*y*(-1 + x + y + z);
    w[7] = -4*z*(-1 + x + y + z);
    w[8] = +4*x*z;
    w[9] = +4*y*z;
  }

  /// get weight factors of derivative at parametric coordinate
  template< class VECTOR>
  void get_quadratic_derivate_weights(const VECTOR& coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
    w[0] = (-3 + 4*x + 4*y + 4*z);
    w[1] = +(-1 + 4*x);
    w[2] = 0;
    w[3] = 0;
    w[4] = -4*(-1 + 2*x + y + z);
    w[5] = 4*y;
    w[6] = -4*y;
    w[7] = -4*z;
    w[8] = 4*z;
    w[9] = 0;
    w[10] = (-3 + 4*x + 4*y + 4*z);
    w[11] = 0;
    w[12] = (-1 + 4*y);
    w[13] = 0;
    w[14] = -4*x;
    w[15] = +4*x;
    w[16] = -4*(-1 + x + 2*y + z);
    w[17] = -4*z;
    w[18] = 0;
    w[19] = +4*z;
    w[20] = (-3 + 4*x + 4*y + 4*z);
    w[21] = 0;
    w[22] = 0;
    w[23] = +(-1 + 4*z);
    w[24] = -4*x;
    w[25] = 0;
    w[26] = -4*y;
    w[27] = -4*(-1 + x + y + 2*z);
    w[28] = +4*x;
    w[29] = +4*y;
  }


  template <class VECTOR>
  void get_cubic_weights(const VECTOR &coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
    w[0]  = (-3*x*x + 2*x*x*x - 3*y*y + 2*y*y*y + (z-1)*(z-1)*(1 + 2*z));
    w[1]  = +x*(1 + x*x + x*(-2 + y) - y - z*z);
    w[2]  = +y*(-x*x + (-1 + y)*(-1 + y + z));
    w[3]  = +(-y*y + x*(-1 + z) + (z-1)*(z-1))*z;
    w[4]  = +(3 - 2*x)*x*x;
    w[5]  = +(-1 + x)*x*x;
    w[6]  = +x*x*y;
    w[7]  = -(x*(-1 + z)*z);
    w[8]  = +(3 - 2*y)*y*y;
    w[9]  = -((-1 + x)*x*y);
    w[10] = +(-1 + y)*y*y;
    w[11] = +y*y*z;
    w[12] = +(3 - 2*z)*z*z;
    w[13] = +x*z*z;
    w[14] = -((-1 + y)*y*z);
    w[15] = +(-1 + z)*z*z;
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_cubic_derivate_weights(const VECTOR &coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]), z=static_cast<double>(coords[2]);
    w[0] = 6*(-1 + x)*x;
    w[1] = +(1 + 3*x*x + 2*x*(-2 + y) - y - z*z);
    w[2] = -2*x*y;
    w[3] = +(-1 + z)*z;
    w[4] = -6*(-1 + x)*x;
    w[5] = +x*(-2 + 3*x);
    w[6] = +2*x*y;
    w[7] = -((-1 + z)*z);
    w[8] = (y - 2*x*y);
    w[9] = 0;
    w[10] = 0;
    w[11] = 0;
    w[12] = z*z;
    w[13] = 0;
    w[14] = 0;
    w[15] = 0;
    w[16] = 6*(-1 + y)*y;
    w[17] = (-1 + x)*x;
    w[18] = (1 - x*x + 3*y*y + 2*y*(-2 + z) - z);
    w[19] = -2*y*z;
    w[20] = 0;
    w[21] = 0;
    w[22] = x*x;
    w[23] = 0;
    w[24] = -6*(-1 + y)*y;
    w[25] = -((-1 + x)*x);
    w[26] = +y*(-2 + 3*y);
    w[27] = +2*y*z;
    w[28] = 0;
    w[29] = 0;
    w[30] = (z - 2*y*z);
    w[31] = 0;
    w[32] = 6*(-1 + z)*z;
    w[33] = -2*x*z;
    w[34] = (-1 + y)*y;
    w[35] = (1 - x - y*y - 4*z + 2*x*z + 3*z*z);
    w[36] = 0;
    w[37] = 0;
    w[38] = 0;
    w[39] = (x - 2*x*z);
    w[40] = 0;
    w[41] = 0;
    w[42] = 0;
    w[43] = y*y;
    w[44] = -6*(-1 + z)*z;
    w[45] = 2*x*z;
    w[46] = -((-1 + y)*y);
    w[47] = z*(-2 + 3*z);
  }

  inline int num_linear_weights() { return 4; }
  inline int num_quadratic_weights() { return 10; }
  inline int num_cubic_weights() { return 16; }

  inline int num_linear_derivate_weights() { return 12; }
  inline int num_quadratic_derivate_weights() { return 30; }
  inline int num_cubic_derivate_weights() { return 48; }

  inline int num_derivs()  { return 3; }
  inline int num_hderivs() { return 3; }
};

}}}

#endif
