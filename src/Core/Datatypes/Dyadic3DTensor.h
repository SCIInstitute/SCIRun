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
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
  namespace Datatypes {
    template <typename Number>
    class Dyadic3DTensorGeneric : public DyadicTensorGeneric<Number>
    {
     public:
      typedef DyadicTensorGeneric<Number> parent;
      Dyadic3DTensorGeneric() : parent(3, 3) {}

      Dyadic3DTensorGeneric(const std::vector<Core::Geometry::Vector>& eigvecs) : parent(3, 3)
      {
        assert(eigvecs.size() == DIM_);
        parent::setEigenVectors(convertNativeVectorsToEigen(eigvecs));
      }

      Dyadic3DTensorGeneric(const Core::Geometry::Vector& eigvec0,
          const Core::Geometry::Vector& eigvec1, const Core::Geometry::Vector& eigvec2)
          : parent(3, 3)
      {
        parent::setEigenVectors(convertNativeVectorsToEigen({eigvec0, eigvec1, eigvec2}));
      }

      Dyadic3DTensorGeneric(const std::vector<DenseColumnMatrixGeneric<Number>>& eigvecs) : parent(3, 3)
      {
        assert(eigvecs.size() == DIM_);
        parent::setEigenVectors(eigvecs);
      }

      Dyadic3DTensorGeneric(const DenseColumnMatrixGeneric<Number>& eigvec0,
          const DenseColumnMatrixGeneric<Number>& eigvec1, const DenseColumnMatrixGeneric<Number>& eigvec2)
          : parent(3, 3)
      {
        parent::setEigenVectors({eigvec0, eigvec1, eigvec2});
      }

      Dyadic3DTensorGeneric(Number v1, Number v2, Number v3, Number v4, Number v5, Number v6) : parent(3, 3)
      {
        (*this)(0, 0) = v1;
        (*this)(1, 1) = v4;
        (*this)(2, 2) = v6;
        (*this)(0, 1) = (*this)(1, 0) = v2;
        (*this)(0, 2) = (*this)(2, 0) = v3;
        (*this)(1, 2) = (*this)(2, 1) = v5;
        parent::buildEigens();
      }

      Dyadic3DTensorGeneric(const std::vector<Number>& v) : parent(3, 3)
      {
        assert(v.size() == 6);
        (*this)(0, 0) = v[0];
        (*this)(1, 1) = v[3];
        (*this)(2, 2) = v[5];
        (*this)(0, 1) = (*this)(1, 0) = v[1];
        (*this)(0, 2) = (*this)(2, 0) = v[2];
        (*this)(1, 2) = (*this)(2, 1) = v[4];
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

     private:
      const int DIM_ = 3;

      std::vector<DenseColumnMatrixGeneric<Number>> convertNativeVectorsToEigen(
          const std::vector<Core::Geometry::Vector>& vecs)
      {
        std::vector<DenseColumnMatrixGeneric<Number>> outVecs(vecs.size());
        for (int i = 0; i < vecs.size(); ++i)
        {
          outVecs[i] = DenseColumnMatrixGeneric<Number>(DIM_);
          for (int j = 0; j < DIM_; ++j)
            outVecs[i][j] = vecs[i][j];
        }
        return outVecs;
      }
    };

    template <typename Indexable>
    Dyadic3DTensor symmetricTensorFromSixElementArray(const Indexable& array)
    {
      return Dyadic3DTensor(array[0], array[1], array[2], array[3], array[4], array[5]);
    }

    template <typename Indexable>
    Dyadic3DTensor symmetricTensorFromNineElementArray(const Indexable& array)
    {
      static int sixElementTensorMatrixIndices[] = {0, 1, 2, 4, 5, 8};
      return Dyadic3DTensor(array[sixElementTensorMatrixIndices[0]],
          array[sixElementTensorMatrixIndices[1]], array[sixElementTensorMatrixIndices[2]],
          array[sixElementTensorMatrixIndices[3]], array[sixElementTensorMatrixIndices[4]],
          array[sixElementTensorMatrixIndices[5]]);
    }
  }
}
}

#endif
