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

#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/DyadicTensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <unsupported/Eigen/CXX11/TensorSymmetry>
#include "Core/Datatypes/MatrixFwd.h"
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
  namespace Datatypes {
    template <typename T>
    class Dyadic3DTensor : public DyadicTensor<T>
    {
     public:
      Dyadic3DTensor() : DyadicTensor<T>(3, 3) {}

      Dyadic3DTensor(const std::vector<Core::Geometry::Vector>& eigvecs) : DyadicTensor<T>(3, 3)
      {
        assert(eigvecs.size() == DIM_);
        DyadicTensor<T>::setEigenVectors(convertNativeVectorsToEigen(eigvecs));
        DyadicTensor<T>::setTensorValues();
      }

      Dyadic3DTensor(const Core::Geometry::Vector& eigvec0, const Core::Geometry::Vector& eigvec1,
          const Core::Geometry::Vector& eigvec2)
          : DyadicTensor<T>(3, 3)
      {
        DyadicTensor<T>::setEigenVectors(convertNativeVectorsToEigen({eigvec0, eigvec1, eigvec2}));
        DyadicTensor<T>::setTensorValues();
      }

      Dyadic3DTensor(const std::vector<DenseColumnMatrixGeneric<T>>& eigvecs)
          : DyadicTensor<T>(3, 3)
      {
        assert(eigvecs.size() == DIM_);
        DyadicTensor<T>::setEigenVectors(eigvecs);
        DyadicTensor<T>::setTensorValues();
      }

      Dyadic3DTensor(const DenseColumnMatrixGeneric<T>& eigvec0,
          const DenseColumnMatrixGeneric<T>& eigvec1, const DenseColumnMatrixGeneric<T>& eigvec2)
          : DyadicTensor<T>(3, 3)
      {
        DyadicTensor<T>::setEigenVectors({eigvec0, eigvec1, eigvec2});
        DyadicTensor<T>::setTensorValues();
      }

      Dyadic3DTensor(double v1, double v2, double v3, double v4, double v5, double v6)
          : DyadicTensor<T>(3, 3)
      {
        (*this)(0, 0) = v1;
        (*this)(1, 1) = v4;
        (*this)(2, 2) = v6;
        (*this)(0, 1) = (*this)(1, 0) = v2;
        (*this)(0, 2) = (*this)(2, 0) = v3;
        (*this)(1, 2) = (*this)(2, 1) = v5;
      }

      Dyadic3DTensor(const std::vector<double>& v) : DyadicTensor<T>(3, 3)
      {
        assert(v.size() == 6);
        (*this)(0, 0) = v[0];
        (*this)(1, 1) = v[3];
        (*this)(2, 2) = v[5];
        (*this)(0, 1) = (*this)(1, 0) = v[1];
        (*this)(0, 2) = (*this)(2, 0) = v[2];
        (*this)(1, 2) = (*this)(2, 1) = v[4];
      }

     private:
      const int DIM_ = 3;

      std::vector<DenseColumnMatrixGeneric<T>> convertNativeVectorsToEigen(
          const std::vector<Core::Geometry::Vector>& vecs)
      {
        std::vector<DenseColumnMatrixGeneric<T>> outVecs(vecs.size());
        for (int i = 0; i < vecs.size(); ++i)
        {
          outVecs[i] = DenseColumnMatrixGeneric<T>(DIM_);
          for (int j = 0; j < DIM_; ++j)
            outVecs[i][j] = vecs[i][j];
        }
        return outVecs;
      }
    };
  }
}
}

#endif
