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

#ifndef CORE_DATATYPES_DYADIC_3D_TENSOR_H
#define CORE_DATATYPES_DYADIC_3D_TENSOR_H

#include <Core/Datatypes/DyadicTensor.h>
#include <Core/Utils/Exception.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
  namespace Datatypes {
    template <typename Number>
    class Dyadic3DTensorGeneric : public DyadicTensorGeneric<Number, 3>
    {
     public:
      using parent = DyadicTensorGeneric<Number, 3>;
      using VectorType = Eigen::Matrix<Number, 3, 1>;
      using SizeType = long;
      Dyadic3DTensorGeneric() : parent() {}
      using parent::parent;
      using parent::operator=;
      using parent::operator==;
      using parent::reorderTensorValues;
      using parent::mandel;
      using parent::trace;

      explicit Dyadic3DTensorGeneric(const std::initializer_list<VectorType>& eigvecs) : parent()
      {
        if (eigvecs.size() != DIM_)
          THROW_INVALID_ARGUMENT("The number of input parameters must be " + std::to_string(DIM_));
        parent::setEigenVectors(eigvecs);
      }

      Dyadic3DTensorGeneric(const Dyadic3DTensorGeneric<Number>& other) : parent()
      {
        for (SizeType i = 0; i < DIM_; ++i)
          for (SizeType j = 0; j < DIM_; ++j)
            (*this)(i, j) = other(i, j);
        if (other.haveEigens_)
        {
          parent::eigvecs_ = other.eigvecs_;
          parent::eigvals_ = other.eigvals_;
          parent::haveEigens_ = true;
        }
      }

      Dyadic3DTensorGeneric(const Dyadic3DTensorGeneric<Number>&& other) : parent()
      {
        this->m_storage = std::move(other.m_storage);
        parent::eigvecs_ = std::move(other.eigvecs_);
        parent::eigvals_ = std::move(other.eigvals_);
        parent::haveEigens_ = true;
      }

      Dyadic3DTensorGeneric(const parent& other) : parent(other) {}

      Dyadic3DTensorGeneric(
          const VectorType& eigvec0, const VectorType& eigvec1, const VectorType& eigvec2)
          : parent()
      {
        parent::setEigenVectors({eigvec0, eigvec1, eigvec2});
      }

      Dyadic3DTensorGeneric(Number v1, Number v2, Number v3, Number v4, Number v5, Number v6)
          : parent()
      {
        (*this)(0, 0) = v1;
        (*this)(1, 1) = v4;
        (*this)(2, 2) = v6;
        (*this)(0, 1) = (*this)(1, 0) = v2;
        (*this)(0, 2) = (*this)(2, 0) = v3;
        (*this)(1, 2) = (*this)(2, 1) = v5;
      }

      Number linearCertainty() const
      {
        auto eigvals = parent::getEigenvalues();
        return (eigvals[0] - eigvals[1]) / parent::eigenvalueSum();
      }

      Number planarCertainty() const
      {
        auto eigvals = parent::getEigenvalues();
        return 2.0 * (eigvals[1] - eigvals[2]) / parent::eigenvalueSum();
      }

      Number sphericalCertainty() const
      {
        auto eigvals = parent::getEigenvalues();
        return 3.0 * eigvals[2] / parent::eigenvalueSum();
      }

      Dyadic3DTensorGeneric<Number> operator=(Dyadic3DTensorGeneric<Number>&& other)
      {
        parent::operator=(other);
        return *this;
      }

      Dyadic3DTensorGeneric<Number> operator=(const Dyadic3DTensorGeneric<Number>& other)
      {
        parent::operator=(static_cast<parent>(other));
        return *this;
      }

      template <typename OtherDerived>
      Dyadic3DTensorGeneric<Number> operator*(const OtherDerived& other) const
      {
        Dyadic3DTensorGeneric<Number> newTensor(parent::operator*(other));
        return newTensor;
      }

      template <typename OtherDerived>
      Dyadic3DTensorGeneric<Number> operator+(const OtherDerived& other) const
      {
        Dyadic3DTensorGeneric<Number> newTensor(parent::operator+(other));
        return newTensor;
      }

      template <typename OtherDerived>
      Dyadic3DTensorGeneric<Number> operator-(const OtherDerived& other) const
      {
        Dyadic3DTensorGeneric<Number> newTensor(parent::operator-(other));
        return newTensor;
      }

      using parent::makePositive;
      Number fractionalAnisotropy() const
      {
        const static Number sqrtHalf = std::sqrt(0.5);
        const auto eigvals = parent::getEigenvalues();
        return sqrtHalf * std::sqrt(std::pow(eigvals[0] - eigvals[1], 2) + std::pow(eigvals[1] - eigvals[2], 2) + std::pow(eigvals[2] - eigvals[0], 2)) / parent::frobeniusNorm();
      }

     private:
      const SizeType DIM_ = 3;
    };

    template <typename Indexable>
    Dyadic3DTensor symmetricTensorFromSixElementArray(const Indexable& array)
    {
      if (array.size() != 6) THROW_INVALID_ARGUMENT("This function requires 6 values.");
      return Dyadic3DTensor(array[0], array[1], array[2], array[3], array[4], array[5]);
    }

    template <typename Number>
    Dyadic3DTensor symmetricTensorFromSixElementArray(const std::initializer_list<Number>& array)
    {
      std::vector<Number> vec = array;
      if (vec.size() != 6) THROW_INVALID_ARGUMENT("This function requires 6 values.");
      return Dyadic3DTensorGeneric<double>(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);
    }
  }
}
}

#endif
