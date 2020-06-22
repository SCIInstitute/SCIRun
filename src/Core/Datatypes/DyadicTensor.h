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

#ifndef CORE_DATATYPES_DYADIC_TENSOR_H
#define CORE_DATATYPES_DYADIC_TENSOR_H

#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/TensorBase.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <iostream>  // TODO DELETE
#include <unsupported/Eigen/CXX11/TensorSymmetry>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
  namespace Datatypes {
    // Dyadic tensors are also known as second-order tensors
    template <typename T>
    class DyadicTensor : public Eigen::Tensor<T, 2>
    {
     public:
      DyadicTensor(size_t dim1, size_t dim2)
        : Eigen::Tensor<T, 2>(dim1, dim2), dim1_(dim1), dim2_(dim2)
      {
        Eigen::Tensor<T, 2>::setZero();
      }

      DyadicTensor(size_t dim1, size_t dim2, T val)
        : Eigen::Tensor<T, 2>(dim1, dim2), dim1_(dim1), dim2_(dim2)
      {
        for (size_t i = 0; i < dim1; ++i)
          for (size_t j = 0; j < dim2; ++j)
            (*this)(i, j) = (i == j) ? val : 0;
      }

      DyadicTensor(const std::vector<DenseColumnMatrixGeneric<T>>& eigvecs)
        : Eigen::Tensor<T, 2>(eigvecs.size(), eigvecs[0].size()), dim1_(eigvecs.size()), dim2_(eigvecs[0].size())
      {
        setEigenVectors(eigvecs);
        setTensorValues();
        haveEigens_ = true;
      }

      // template <class OtherDerived>
      // DyadicTensor<T>& operator==(const OtherDerived& other)
      // {
        // TensorBase<T, 2>::operator==(other);
        // return *this;
      // }

      void setEigenVectors(const std::vector<DenseColumnMatrixGeneric<T>>& eigvecs)
      {
        eigvecs_ = eigvecs;
        setEigenvaluesFromEigenvectors();
        normalizeEigenvectors();
        haveEigens_ = true;
      }

      Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> asMatrix()
      {
        Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> mat;
        mat.resize(dim1_, dim2_);
        for (size_t i = 0; i < dim1_; ++i)
          for (size_t j = 0; j < dim2_; ++j)
            mat(i, j) = (*this)(i, j);
        return mat;
      }

      void buildEigens()
      {
        if (haveEigens_) return;
        auto mat = this->asMatrix();

        auto es = Eigen::EigenSolver<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>(mat);
        auto vecs = es.eigenvectors();
        auto vals = es.eigenvalues();

        eigvals_.resize(dim1_);
        eigvecs_.resize(dim1_);

        for (size_t i = 0; i < dim1_; ++i)
        {
          eigvals_[i] = real(vals(i));
          eigvecs_[i] = DenseColumnMatrixGeneric<T>(dim2_);
          for (size_t j = 0; j < dim2_; ++j)
            eigvecs_[i].put(j, 0, real(vecs(i, j)));
        }

        haveEigens_ = true;
        reorderTensorValues();
      }

      void reorderTensorValues()
      {
        if (!haveEigens_) return;
        std::map<T, DenseColumnMatrixGeneric<T>> sorted;
        for (int i = 0; i < dim1_; ++i)
          sorted[eigvals_[i]] = eigvecs_[i];

        auto sortedEigsIter = sorted.begin();

        for (int i = 0; i < dim1_; ++i)
          std::tie(eigvals_[i], eigvecs_[i]) = *sortedEigsIter++;
      }

      std::vector<DenseColumnMatrixGeneric<T>> getEigenvectors()
      {
        if (!haveEigens_) buildEigens();
        return eigvecs_;
      }
      std::vector<T> getEigenvalues()
      {
        if (!haveEigens_) buildEigens();
        return eigvals_;
      }

      SCISHARE friend std::ostream& operator<<(std::ostream& os, const DyadicTensor<T>& t)
      {
        os << '[';
        for (int i = 0; i < t.getDimension1(); ++i)
          for (int j = 0; j < t.getDimension2(); ++j)
          {
            if (i + j != 0) os << ' ';
            os << t(i, j);
          }
        os << ']';

        return os;
      }

      SCISHARE friend std::istream& operator>>(std::istream& is, DyadicTensor<T>& t)
      {
        for (int i = 0; i < t.getDimension1(); ++i)
          for (int j = 0; j < t.getDimension2(); ++j)
            is >> t(i, j);

        return is;
      }

      DyadicTensor<T>& operator=(const T& v)
      {
        for (int i = 0; i < getDimension1(); ++i)
          for (int j = 0; j < getDimension2(); ++j)
            (*this)(i, j) = v;
        haveEigens_ = false;
        return *this;
      }

      size_t getDimension1() const { return dim1_; }
      size_t getDimension2() const { return dim2_; }

     protected:
      const int RANK_ = 2;
      size_t dim1_ = 0;
      size_t dim2_ = 0;
      std::vector<DenseColumnMatrixGeneric<T>> eigvecs_;
      std::vector<T> eigvals_;
      bool haveEigens_ = false;

      void setEigenvaluesFromEigenvectors()
      {
        eigvals_ = std::vector<T>(eigvecs_.size());
        for (int i = 0; i < dim1_; ++i)
          eigvals_[i] = eigvecs_[i].norm();
      }

      void normalizeEigenvectors()
      {
        for (int i = 0; i < dim1_; ++i)
          eigvecs_[i] /= eigvals_[i];
      }

      void setTensorValues()
      {
        for (int i = 0; i < dim1_; ++i)
          for (int j = 0; j < dim2_; ++j)
            (*this)(i, j) = eigvecs_[i][j] * eigvals_[i];
      }
    };
  }
}
}

#endif
