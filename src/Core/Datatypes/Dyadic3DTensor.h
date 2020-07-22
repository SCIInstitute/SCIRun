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
#include <unsupported/Eigen/CXX11/TensorSymmetry>
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
      Dyadic3DTensorGeneric() : parent() {}
      using parent::parent;
      using parent::operator=;
      using parent::operator==;

      explicit Dyadic3DTensorGeneric(
          const std::initializer_list<VectorType>& eigvecs)
          : parent()
      {
        if (eigvecs.size() != DIM_)
          THROW_INVALID_ARGUMENT("The number of input parameters must be " + DIM_);
        parent::setEigenVectors(eigvecs);
      }

      Dyadic3DTensorGeneric(const VectorType& eigvec0,
          const VectorType& eigvec1,
          const VectorType& eigvec2)
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
        parent::buildEigens();
      }

      Number linearCertainty()
      {
        auto eigvals = parent::getEigenvalues();
        return (eigvals[0] - eigvals[1]) / parent::eigenValueSum();
      }

      Number planarCertainty()
      {
        auto eigvals = parent::getEigenvalues();
        return 2.0 * (eigvals[1] - eigvals[2]) / parent::eigenValueSum();
      }

      Number sphericalCertainty()
      {
        auto eigvals = parent::getEigenvalues();
        return 3.0 * eigvals[2] / parent::eigenValueSum();
      }

      VectorType mandel()
      {
        auto eigvals = parent::getEigenvalues();
        auto eigvecs = parent::getEigenvectors();

        for (size_t i = 0; i < DIM_; ++i)
          eigvecs[i] *= eigvals[i];

        static const double sqrt2 = std::sqrt(2);
        VectorType mandel({eigvecs[0][0], eigvecs[1][1], eigvecs[2][2],
            eigvecs[0][1] * sqrt2, eigvecs[0][2] * sqrt2, eigvecs[1][2] * sqrt2});
        return mandel;
      }

     private:
      const size_t DIM_ = 3;
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

    template <typename Indexable>
    Dyadic3DTensor symmetricTensorFromMandel(const Indexable& array)
    {
      static const double sqrt2 = std::sqrt(2);
      return Dyadic3DTensor(
          array[0], array[3] / sqrt2, array[4] / sqrt2, array[1], array[5] / sqrt2, array[2]);
    }
  }
}
}

#endif
