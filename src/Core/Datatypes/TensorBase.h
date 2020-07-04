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

#include <unsupported/Eigen/CXX11/TensorSymmetry>
#include "unsupported/Eigen/CXX11/src/TensorSymmetry/Symmetry.h"
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
  namespace Datatypes {
    template <typename Number, int Rank>
    class TensorBaseGeneric : public Eigen::Tensor<Number, Rank>
    {
      typedef Eigen::Tensor<Number, Rank> parent;

     public:
      using parent::Tensor;  // adding parent constructors
      // using parent::contract;

      bool operator==(const TensorBaseGeneric<Number, Rank>& t) const
      {
        return dimensionsEqual(t) && valuesEqual(t);
      }

      bool operator!=(const TensorBaseGeneric<Number, Rank>& t) const { return !(*this == t); }

      TensorBaseGeneric& operator=(const parent& other)
      {
        parent::operator=(other);
        return *this;
      }

      template <typename OtherDerived>
      TensorBaseGeneric contract(const OtherDerived& other) const
      {
        Eigen::array<Eigen::IndexPair<int>, 1> product_dims = {Eigen::IndexPair<int>(1, 0)};
        return parent::contract(static_cast<parent>(other), product_dims);
      }

      template <typename OtherDerived>
      parent operator*(const OtherDerived& other) const
      {
        return static_cast<parent>(*this) * static_cast<parent>(other);
      }

      template <typename OtherDerived>
      parent operator+(const OtherDerived& other) const
      {
        return static_cast<parent>(*this) + static_cast<parent>(other);
      }

      parent operator*(const Number& other) const
      {
        return static_cast<parent>(*this) * other;
      }

      template <typename OtherDerived>
      parent operator-(const OtherDerived& other) const
      {
        return static_cast<parent>(*this) - static_cast<parent>(other);
      }

      bool dimensionsEqual(const TensorBaseGeneric<Number, Rank>& t) const
      {
        if (parent::NumDimensions != t.NumDimensions) return false;

        auto thisDim = parent::dimensions();
        auto thatDim = t.dimensions();

        for (int i = 0; i < parent::NumDimensions; ++i)
          if (thisDim[i] != thatDim[i]) return false;

        // TensorBase<double, 2> et(2, 2);
        // auto b = et + et;

        return true;
      }

      bool valuesEqual(const TensorBaseGeneric<Number, Rank>& t) const
      {
        auto dim = parent::dimensions();
        auto index = std::vector<int>(parent::NumDimensions, 0);
        return checkForEquals(index, dim, t);
      }

      bool checkForEquals(std::vector<int>& index, Eigen::DSizes<long int, Rank>& dim,
          const TensorBaseGeneric<Number, Rank>& t) const
      {
        if ((*this)(index) != t(index)) return false;

        if (incrementIndex(index, dim))
          return checkForEquals(index, dim, t);
        else
          return true;
      }

      bool incrementIndex(std::vector<int>& index, Eigen::DSizes<long int, Rank>& dim) const
      {
        long unsigned int d = index.size() - 1;

        while (d >= 0)
        {
          auto dIndex = index[d] + 1;
          if (dIndex >= dim[d])
            --d;
          else
          {
            index[d] = dIndex;
            break;
          }
        }

        if (d == 0 && (index[0] + 1 >= dim[0])) return false;
        if (d != index.size() - 1)
        {
          for (int i = d + 1; i < index.size(); ++i)
            index[i] = 0;
        }
        return true;
      }
    };

    template<typename Number, int Rank>
    inline TensorBaseGeneric<Number, Rank> operator*(Number n, const TensorBaseGeneric<Number, Rank>& t)
    {
      return t * n;
    }
  }
}
}

#endif
