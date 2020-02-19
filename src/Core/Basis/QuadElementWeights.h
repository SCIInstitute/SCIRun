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


#ifndef CORE_BASIS_QUADWEIGHTS_H
#define CORE_BASIS_QUADWEIGHTS_H 1

namespace SCIRun {
namespace Core {
namespace Basis {

class QuadElementWeights {

public:
  template <class VECTOR>
  void get_linear_weights(const VECTOR& coords, double *w) const
  {
    const double x = static_cast<double>(coords[0]), y = static_cast<double>(coords[1]);
    w[0] = (-1 + x) * (-1 + y);
    w[1] = -x * (-1 + y);
    w[2] = x * y;
    w[3] = -(-1 + x) * y;
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_linear_derivate_weights(const VECTOR& coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]);
    w[0] = (-1 + y);
    w[1] = (1 - y);
    w[2] = y;
    w[3] = -y;
    w[4] = (-1 + x);
    w[5] =  -x;
    w[6] = x;
    w[7] = (1 - x);
  }

  /// get weight factors at parametric coordinate
  template< class VECTOR>
  void get_quadratic_weights(const VECTOR& coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]);

    w[0] = -((-1 + x)*(-1 + y)*(-1 + 2*x + 2*y));
    w[1] = -(x*(-1 + 2*x - 2*y)*(-1 +y));
    w[2] = +x*y*(-3 + 2*x + 2*y);
    w[3] = +(-1 + x)*(1 + 2*x - 2*y)*y;
    w[4] = +4*(-1 + x)*x*(-1 + y);
    w[5] = -4*x*(-1 + y)*y;
    w[6] = -4*(-1 + x)*x*y;
    w[7] = +4*(-1 + x)*(-1 + y)*y;
  }

  /// get weight factors of derivative at parametric coordinate
  template< class VECTOR>
  void get_quadratic_derivate_weights(const VECTOR& coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]);

    w[0]= -((-1 + y)*(-3 + 4*x + 2*y));
    w[1]= -((-1 + 4*x - 2*y)*(-1 + y));
    w[2]= +y*(-3+ 4*x + 2*y);
    w[3]= +(-1 + 4*x - 2*y)*y;
    w[4]= +4*(-1 + 2*x)*(-1 + y);
    w[5]= -4*(-1 + y)*y;
    w[6]= +(4 - 8*x)*y;
    w[7]= +4*(-1 + y)*y;
    w[8]= -((-1 + x)*(-3 + 2*x +4*y));
    w[9]= +x*(-1 - 2*x + 4*y);
    w[10]= +x*(-3 + 2*x + 4*y);
    w[11]= +(-1 + x)*(1 + 2*x -4*y);
    w[12]= +4*(-1 + x)*x;
    w[13]= +x*(4 -8*y);
    w[14]= -4*(-1 + x)*x;
    w[15]= +4*(-1 + x)*(-1 +2*y);
  }


  template <class VECTOR>
  void get_cubic_weights(const VECTOR &coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]);
    w[0]  = -((-1 + x)*(-1 + y)*(-1 - x + 2*x*x - y + 2*y*y));
    w[1]  = -((x-1)*(x-1)*x*(-1 + y));
    w[2]  = -((-1 + x)*(y-1)*(y-1)*y);
    w[3]  = +x*(-1 + y)*(-3*x + 2*x*x + y*(-1 + 2*y));
    w[4]  = +x*x*(-1 + x + y - x*y);
    w[5]  = +x*(y-1)*(y-1)*y;
    w[6]  = +x*y*(-1 + 3*x - 2*x*x + 3*y - 2*y*y);
    w[7]  = +(-1 + x)*x*x*y;
    w[8]  = +x*(-1 + y)*y*y;
    w[9]  = +(-1 + x)*y*(-x + 2*x*x + y*(-3 + 2*y));
    w[10] = +(x-1)*(x-1)*x*y;
    w[11] = +y*y*(-1 + x + y - x*y);
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_cubic_derivate_weights(const VECTOR &coords, double *w) const
  {
    const double x=static_cast<double>(coords[0]), y=static_cast<double>(coords[1]);
    w[0]= -((-1 + y)*(-6*x + 6*x*x + y*(-1 + 2*y)));
    w[1]= -((1 - 4*x + 3*x*x)*(-1 + y));
    w[2]= -((y-1)*(y-1)*y);
    w[3]= +(-1 + y)*(-6*x + 6*x*x + y*(-1 + 2*y));
    w[4]= -(x*(-2 + 3*x)*(-1 + y));
    w[5]= +(y-1)*(y-1)*y;
    w[6]= +y*(-1 + 6*x - 6*x*x + 3*y - 2*y*y);
    w[7]= +x*(-2 + 3*x)*y;
    w[8]= +(-1 + y)*y*y;
    w[9]= +y*(1 - 6*x + 6*x*x - 3*y + 2*y*y);
    w[10]= +(1 - 4*x + 3*x*x)*y;
    w[11]= -((-1 + y)*y*y);
    w[12]= -((-1 + x)*(-x + 2*x*x + 6*(-1 + y)*y));
    w[13]= -((x-1)*(x-1)*x);
    w[14]= -((-1 + x)*(1 - 4*y + 3*y*y));
    w[15]= +x*(1 - 3*x + 2*x*x - 6*y + 6*y*y);
    w[16]= -((-1 + x)*x*x);
    w[17]= +x*(1 - 4*y + 3*y*y);
    w[18]= +x*(-1 + 3*x - 2*x*x + 6*y - 6*y*y);
    w[19]= +(-1 + x)*x*x;
    w[20]= +x*y*(-2 + 3*y);
    w[21]= +(-1 + x)*(-x + 2*x*x + 6*(-1 + y)*y);
    w[22]= +(x-1)*(x-1)*x;
    w[23]= -((-1 + x)*y*(-2 + 3*y));
  }


  inline int num_linear_weights() { return 4; }
  inline int num_quadratic_weights() { return 8; }
  inline int num_cubic_weights() { return 12; }

  inline int num_linear_derivate_weights() { return 8; }
  inline int num_quadratic_derivate_weights() { return 16; }
  inline int num_cubic_derivate_weights() { return 24; }

  inline int num_derivs()  { return 2; }
  inline int num_hderivs() { return 2; }
};

}}}

#endif
