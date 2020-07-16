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


/// @todo Documentation Core/Datatypes/MatrixFwd.h

#ifndef CORE_DATATYPES_TENSOR_FWD_H
#define CORE_DATATYPES_TENSOR_FWD_H

#include <Core/Datatypes/Legacy/Base/TypeName.h>
#include <unsupported/Eigen/CXX11/Tensor>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  template <typename Number, size_t Dim>
  class DyadicTensorGeneric;

  template <size_t Dim>
  using DyadicTensor = DyadicTensorGeneric<double, Dim>;

  typedef DyadicTensorGeneric<double, 2> Dyadic2DTensor;
  typedef DyadicTensorGeneric<double, 4> Dyadic4DTensor;

  template <typename Number>
  class Dyadic3DTensorGeneric;

  typedef Dyadic3DTensorGeneric<double> Dyadic3DTensor;
}}}


#endif
