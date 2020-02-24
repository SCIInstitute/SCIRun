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


#ifndef CORE_BASIS_NOWEIGHTS_H
#define CORE_BASIS_NOWEIGHTS_H 1

namespace SCIRun {
namespace Core {
namespace Basis {

class NoElementWeights {

public:
  template <class VECTOR>
  void get_linear_weights(const VECTOR&, double*) const
  {
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_linear_derivate_weights(const VECTOR&, double*) const
  {
  }

  /// get weight factors at parametric coordinate
  template< class VECTOR>
  void get_quadratic_weights(const VECTOR&, double*) const
  {
  }

  /// get weight factors of derivative at parametric coordinate
  template< class VECTOR>
  void get_quadratic_derivate_weights(const VECTOR&, double*) const
  {
  }

  template <class VECTOR>
  void get_cubic_weights(const VECTOR&, double*) const
  {
  }

  /// get derivative weight factors at parametric coordinate
  template <class VECTOR>
  void get_cubic_derivate_weights(const VECTOR &, double*) const
  {
  }

  inline int num_linear_weights() { return 0; }
  inline int num_quadratic_weights() { return 0; }
  inline int num_cubic_weights() { return 0; }

  inline int num_linear_derivate_weights() { return 0; }
  inline int num_quadratic_derivate_weights() { return 0; }
  inline int num_cubic_derivate_weights() { return 0; }

  inline int num_derivs()  { return 0; }
  inline int num_hderivs() { return 0; }
};

}}}

#endif
