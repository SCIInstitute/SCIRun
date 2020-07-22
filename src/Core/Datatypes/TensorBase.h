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

#ifndef CORE_DATATYPES_TENSOR_BASE_H
#define CORE_DATATYPES_TENSOR_BASE_H

#include <unsupported/Eigen/CXX11/Tensor>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
  namespace Datatypes {
    template <typename Number, typename Sizes>
    class TensorBaseGeneric : public Eigen::TensorFixedSize<Number, Sizes>
    {
      using parent = Eigen::TensorFixedSize<Number, Sizes>;

     public:
      using parent::contract;
      using parent::parent;  // adding parent constructors

      template <typename OtherDerived>
      parent operator*(const OtherDerived& other) const
      {
        return static_cast<parent>(*this) * static_cast<parent>(other);
      }

      parent operator*(const Number& other) const { return static_cast<parent>(*this) * other; }

      template <typename OtherDerived>
      parent operator-(const OtherDerived& other) const
      {
        return static_cast<parent>(*this) - static_cast<parent>(other);
      }

      template <typename OtherDerived>
      TensorBaseGeneric contract(const OtherDerived& other)
      {
        Eigen::array<Eigen::IndexPair<int>, 1> product_dims = {Eigen::IndexPair<int>(1, 0)};
        return parent::contract(static_cast<parent>(other), product_dims);
      }
    };

    template <typename Number, typename Sizes>
    inline TensorBaseGeneric<Number, Sizes> operator*(
        Number n, const TensorBaseGeneric<Number, Sizes>& t)
    {
      return t * n;
    }
  }
}
}

#endif
