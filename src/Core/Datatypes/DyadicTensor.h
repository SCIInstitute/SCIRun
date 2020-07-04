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
#include <Core/Datatypes/TensorBase.h>
#include <Core/Datatypes/TensorFwd.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <iostream>  // TODO DELETE
#include <unsupported/Eigen/CXX11/TensorSymmetry>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
  namespace Datatypes {
    // Dyadic tensors are also known as second-order tensors
    template <typename Number>
    class DyadicTensorGeneric : public TensorBaseGeneric<Number, 2>
    // TODO need to add operator>>, type_name
    {
     public:
      typedef TensorBaseGeneric<Number, 2> parent;

      DyadicTensorGeneric(size_t dim1, size_t dim2) : parent(dim1, dim2), dim1_(dim1), dim2_(dim2)
      {
        parent::setZero();
        // buildEigens();
      }

      DyadicTensorGeneric(size_t dim1, size_t dim2, Number val)
          : parent(dim1, dim2), dim1_(dim1), dim2_(dim2)
      {
        for (size_t i = 0; i < dim1; ++i)
          for (size_t j = 0; j < dim2; ++j)
            (*this)(i, j) = (i == j) ? val : 0;
        // buildEigens();
      }

      DyadicTensorGeneric(const std::vector<DenseColumnMatrixGeneric<Number>>& eigvecs)
          : parent(eigvecs.size(), eigvecs[0].size()), dim1_(eigvecs.size()),
            dim2_(eigvecs[0].size())
      {
        setEigenVectors(eigvecs);
      }

      DyadicTensorGeneric(DyadicTensorGeneric<Number>& other)
          : parent(other.dimension(0), other.dimension(1)), dim1_(other.dimension(0)),
            dim2_(other.dimension(1))
      {
        for (size_t i = 0; i < dim1_; ++i)
          for (size_t j = 0; j < dim1_; ++j)
            (*this)(i, j) = other(i, j);
        eigvecs_ = other.getEigenvectors();
        eigvals_ = other.getEigenvalues();
        haveEigens_ = true;
        reorderTensorValues();
      }

      DyadicTensorGeneric(const Eigen::Tensor<Number, 2>& other)
          : parent(other.dimension(0), other.dimension(1)), dim1_(other.dimension(0)),
            dim2_(other.dimension(1))
      {
        for (size_t i = 0; i < dim1_; ++i)
          for (size_t j = 0; j < dim1_; ++j)
            (*this)(i, j) = other(i, j);
        // buildEigens();
      }

      using parent::operator=;
      using parent::operator!=;
      using parent::contract;

      void setEigenVectors(const std::vector<DenseColumnMatrixGeneric<Number>>& eigvecs)
      {
        eigvecs_ = eigvecs;
        setEigenvaluesFromEigenvectors();
        normalizeEigenvectors();
        haveEigens_ = true;
        setTensorValues();
        reorderTensorValues();
      }

      Eigen::Matrix<Number, Eigen::Dynamic, Eigen::Dynamic> asMatrix()
      {
        Eigen::Matrix<Number, Eigen::Dynamic, Eigen::Dynamic> mat;
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

        auto es = Eigen::EigenSolver<Eigen::Matrix<Number, Eigen::Dynamic, Eigen::Dynamic>>(mat);
        auto vecs = es.eigenvectors();
        auto vals = es.eigenvalues();

        eigvals_.resize(dim1_);
        eigvecs_.resize(dim1_);

        for (size_t i = 0; i < dim1_; ++i)
        {
          eigvals_[i] = real(vals(i));
          eigvecs_[i] = DenseColumnMatrixGeneric<Number>(dim2_);
          for (size_t j = 0; j < dim2_; ++j)
            eigvecs_[i].put(j, 0, real(vecs(i, j)));
        }

        haveEigens_ = true;
        reorderTensorValues();
      }

      void reorderTensorValues()
      {
        typedef std::pair<Number, DenseColumnMatrixGeneric<Number>> EigPair;
        if (!haveEigens_) return;
        std::vector<EigPair> sortList(dim1_);
        for (size_t i = 0; i < dim1_; ++i)
          sortList[i] = std::make_pair(eigvals_[i], eigvecs_[i]);

        std::sort(sortList.begin(), sortList.end(),
            [](const EigPair& left, const EigPair& right) { return left.first > right.first; });

        auto sortedEigsIter = sortList.begin();

        for (size_t i = 0; i < dim1_; ++i)
          std::tie(eigvals_[i], eigvecs_[i]) = *sortedEigsIter++;
      }

      DenseColumnMatrixGeneric<Number> getEigenvector(int index)
      {
        if (!haveEigens_) buildEigens();
        return eigvecs_[index];
      }

      std::vector<DenseColumnMatrixGeneric<Number>> getEigenvectors()
      {
        if (!haveEigens_) buildEigens();
        return eigvecs_;
      }

      std::vector<Number> getEigenvalues()
      {
        if (!haveEigens_) buildEigens();
        return eigvals_;
      }

      SCISHARE friend std::ostream& operator<<(std::ostream& os, const DyadicTensorGeneric<Number>& t)
      {
        os << '[';
        for (size_t i = 0; i < t.getDimension1(); ++i)
          for (size_t j = 0; j < t.getDimension2(); ++j)
          {
            if (i + j != 0) os << ' ';
            os << t(j, i);
          }
        os << ']';

        return os;
      }

      SCISHARE friend std::istream& operator>>(std::istream& is, DyadicTensorGeneric<Number>& t)
      {
        for (size_t i = 0; i < t.getDimension1(); ++i)
          for (size_t j = 0; j < t.getDimension2(); ++j)
            is >> t(j, i);

        return is;
      }

      DyadicTensorGeneric<Number>& operator=(const Number& v)
      {
        for (size_t i = 0; i < getDimension1(); ++i)
          for (size_t j = 0; j < getDimension2(); ++j)
            (*this)(i, j) = v;
        haveEigens_ = false;
        return *this;
      }

      size_t getDimension1() const { return dim1_; }
      size_t getDimension2() const { return dim2_; }

      Number frobeniusNorm()
      {
        if (!haveEigens_) buildEigens();
        auto eigvals = DenseColumnMatrix(eigvals_);
        return eigvals.norm();
      }

      Number maxNorm()
      {
        if (!haveEigens_) buildEigens();
        auto maxVal = std::numeric_limits<Number>::min();
        for (const auto& e : eigvals_)
          maxVal = std::max(maxVal, e);
        return maxVal;
      }

      void setEigens(const std::vector<DenseColumnMatrix>& eigvecs, const std::vector<Number>& eigvals)
      {
        assert(eigvecs_.size() == eigvecs.size());
        assert(eigvals_.size() == eigvals.size());
        eigvecs_ = eigvecs;
        eigvals_ = eigvals;
        haveEigens_ = true;
      }

      Number eigenValueSum()
      {
        if (!haveEigens_) buildEigens();
        Number sum = 0;
        for (const auto& e : eigvals_)
          sum += e;
        return sum;
      }

     protected:
      const int RANK_ = 2;
      size_t dim1_ = 0;
      size_t dim2_ = 0;
      std::vector<DenseColumnMatrixGeneric<Number>> eigvecs_;
      std::vector<Number> eigvals_;
      bool haveEigens_ = false;

      void setEigenvaluesFromEigenvectors()
      {
        eigvals_ = std::vector<Number>(eigvecs_.size());
        for (size_t i = 0; i < dim1_; ++i)
          eigvals_[i] = eigvecs_[i].norm();
      }

      void normalizeEigenvectors()
      {
        for (size_t i = 0; i < dim1_; ++i)
          eigvecs_[i] /= eigvals_[i];
      }

      void setTensorValues()
      {
        for (size_t i = 0; i < dim1_; ++i)
          for (size_t j = 0; j < dim2_; ++j)
            (*this)(j, i) = eigvecs_[i][j] * eigvals_[i];
      }
    };
  }
}
}

#endif
