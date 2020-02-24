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


#ifndef CORE_BASIS_TRIWEIGHTS_H
#define CORE_BASIS_TRIWEIGHTS_H 1

namespace SCIRun {
namespace Core {
namespace Basis {

class TriElementWeights {

public:
  template <class VECTOR>
  void get_linear_weights(const VECTOR& coords, double *w) const
  {
    const double x = static_cast<double>(coords[0]), y = static_cast<double>(coords[1]);

    w[0] = (1 - x - y);
    w[1] = x;
    w[2] = y;
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_linear_derivate_weights(const VECTOR& /*coords*/, double *w) const
  {
    w[0] = -1;
    w[1] = 1;
    w[2] = 0;
    w[3] = -1;
    w[4] = 0;
    w[5] = 1;
  }

  /// get weight factors at parametric coordinate
  template< class VECTOR>
  void get_quadratic_weights(const VECTOR& coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]);
    w[0] = (1 + 2*x*x - 3*y + 2*y*y + x*(-3 + 4*y));
    w[1] = +x*(-1 + 2*x);
    w[2] = +y*(-1+ 2*y);
    w[3] = -4*x*(-1 + x + y);
    w[4] = +4*x*y;
    w[5] = -4*y*(-1 + x + y);
  }

  /// get weight factors of derivative at parametric coordinate
  template< class VECTOR>
  void get_quadratic_derivate_weights(const VECTOR& coords, double *w) const
  {
   const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]);
    w[0] = (-3 + 4*x + 4*y);
    w[1] = (-1 + 4*x);
    w[2] = 0;
    w[3] = -4*(-1 + 2*x + y);
    w[4] = +4*y;
    w[5] = -4*y;
    w[6] = (-3 + 4*x + 4*y);
    w[7] = 0;
    w[8] = (-1 +4*y);
    w[9] = -4*x;
    w[10] = 4*x;
    w[11] = -4*(-1 + x +2*y);
  }


  template <class VECTOR>
  void get_cubic_weights(const VECTOR &coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]);
    const double x2=x*x, x3=x2*x, y2=y*y, y3=y2*y;

    w[0]  = (-1 + x + y)*(-1 - x + 2*x2 - y - 2*x*y + 2*y2);
    w[1]  = x*(1 - 2*x + x2 - 3*y2 + 2*y3);
    w[2]  = y*(1 - 3*x2 + 2*x3 - 2*y + y2);
    w[3]  = x*y*(1 - 2*x + x2 - 2*y + y2);
    w[4]  = -(x2*(-3 + 2*x));
    w[5]  = (-1 + x)*x2;
    w[6]  = -(x2*(-3 + 2*x)*y);
    w[7]  = (-1 + x)*x2*y;
    w[8]  = -y2*(-3 + 2*y);
    w[9]  = -(x*y2*(-3 + 2*y));
    w[10] = (-1 + y)*y2;
    w[11] = x*(-1 + y)*y2;
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_cubic_derivate_weights(const VECTOR &coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]);
    const double x2=x*x, x3=x2*x, y2=y*y;
    const double y12=(y-1)*(y-1);

    w[0] = 6*(-1 + x)*x;
    w[1] = (-4*x + 3*x2 + y12*(1 + 2*y));
    w[2] = 6*(-1 + x)*x*y;
    w[3] = (-4*x + 3*x2 + y12)*y;
    w[4] = -6*(-1 + x)*x;
    w[5] = x*(-2 + 3*x);
    w[6] = -6*(-1 + x)*x*y;
    w[7] = x*(-2 + 3*x)*y;
    w[8] = 0;
    w[9] = (3 - 2*y)*y2;
    w[10] = 0;
    w[11] = (-1 + y)*y2;

    w[12] = 6*(-1 + y)*y;
    w[13] = 6*x*(-1 + y)*y;
    w[14] = (1 - 3*x2 + 2*x3 - 4*y + 3*y2);
    w[15] = x*(1 - 2*x + x2 - 4*y + 3*y2);
    w[16] = 0;
    w[17] = 0;
    w[18] = (3 - 2*x)*x2;
    w[19] = (-1 + x)*x2;
    w[20] = -6*(-1 + y)*y;
    w[21] = -6*x*(-1 + y);
    w[22] = y*(-2 + 3*y);
    w[23] = x*y*(-2 + 3*y);
  }

  inline int num_linear_weights() { return 3; }
  inline int num_quadratic_weights() { return 6; }
  inline int num_cubic_weights() { return 12; }

  inline int num_linear_derivate_weights() { return 6; }
  inline int num_quadratic_derivate_weights() { return 12; }
  inline int num_cubic_derivate_weights() { return 24; }

  inline int num_derivs()  { return 2; }
  inline int num_hderivs() { return 3; }

};

}}}

#endif
