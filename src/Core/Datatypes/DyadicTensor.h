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

#include <Core/Datatypes/TensorBase.h>
#include <Core/Datatypes/TensorFwd.h>
#include <Core/Utils/Exception.h>
#include <Eigen/Dense>
#include <algorithm>
#include <initializer_list>
#include <limits>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
  namespace Datatypes {
    // Dyadic tensors are also known as second-order tensors
    template <typename Number, size_t Dim>
    class DyadicTensorGeneric : public TensorBaseGeneric<Number, Eigen::Sizes<Dim, Dim>>
    {
     public:
      using parent = TensorBaseGeneric<Number, Eigen::Sizes<Dim, Dim>>;
      using VectorType = Eigen::Matrix<Number, Dim, 1>;
      using MatrixType = Eigen::Matrix<Number, Dim, Dim>;

      DyadicTensorGeneric() : parent() { parent::setZero(); }

      explicit DyadicTensorGeneric(Number val) : parent()
      {
        for (size_t i = 0; i < Dim; ++i)
          for (size_t j = 0; j < Dim; ++j)
            (*this)(i, j) = (i == j) ? val : 0;
      }

      explicit DyadicTensorGeneric(const std::vector<VectorType>& eigvecs) : parent()
      {
        if (eigvecs.size() != Dim)
          THROW_INVALID_ARGUMENT("The number of input vectors must be " + Dim);
        setEigenVectors(eigvecs);
      }

      DyadicTensorGeneric(const std::initializer_list<VectorType>& eigvecs) : parent()
      {
        if (eigvecs.size() != Dim)
          THROW_INVALID_ARGUMENT("The number of input vectors must be " + Dim);
        setEigenVectors(eigvecs);
      }

      DyadicTensorGeneric(const DyadicTensorGeneric<Number, Dim>& other) : parent()
      {
        for (size_t i = 0; i < Dim; ++i)
          for (size_t j = 0; j < Dim; ++j)
            (*this)(index(i), index(j)) = other(index(i), index(j));
        eigvecs_ = other.getEigenvectors();
        eigvals_ = other.getEigenvalues();
        haveEigens_ = true;
      }

      DyadicTensorGeneric(DyadicTensorGeneric<Number, Dim>&& other) : parent()
      {
        auto otherData = other.data();
        std::move(otherData, otherData + other.size(), this->data());

        eigvecs_ = std::move(other.eigvecs_);
        eigvals_ = std::move(other.eigvals_);
        haveEigens_ = true;
      }

      DyadicTensorGeneric(const Eigen::TensorFixedSize<Number, Eigen::Sizes<Dim, Dim>>& other)
          : parent()
      {
        for (size_t i = 0; i < Dim; ++i)
          for (size_t j = 0; j < Dim; ++j)
            (*this)(index(i), index(j)) = other(index(i), index(j));
      }

      explicit DyadicTensorGeneric(const MatrixType& mat)
      {
        for (size_t i = 0; i < Dim; ++i)
          for (size_t j = 0; j < Dim; ++j)
            if (mat(i, j) != mat(j, i)) THROW_INVALID_ARGUMENT("Input matrix must be symmetric.");
        for (size_t i = 0; i < Dim; ++i)
          for (size_t j = 0; j < Dim; ++j)
            (*this)(index(i), index(j)) = mat(index(i), index(j));
      }

      using parent::operator=;
      using parent::contract;

      template <typename OtherDerived>
      bool operator!=(const OtherDerived& other) const
      {
        return !operator==(other);
      }

      template <typename OtherDerived>
      bool operator==(const OtherDerived& other) const
      {
        auto otherTensor = static_cast<DyadicTensorGeneric<Number, Dim>>(other);
        if (Dim != otherTensor.dimension(0) || Dim != otherTensor.dimension(1)) return false;
        for (size_t i = 0; i < Dim; ++i)
          for (size_t j = 0; j < Dim; ++j)
            if ((*this)(index(i), index(j)) != otherTensor(index(i), index(j))) return false;
        return true;
      }

      void setEigenVectors(const std::vector<VectorType>& eigvecs)
      {
        eigvecs_ = eigvecs;
        setEigenvaluesFromEigenvectors();
        normalizeEigenvectors();
        haveEigens_ = true;
        setTensorValues();
        reorderTensorValues();
      }

      // This function is listed as something that will be added to Eigen::Tensor in the future.
      // Usage of this function should be replaced with Eigen's asMatrix function when it is
      // implemented.
      MatrixType asMatrix() const
      {
        MatrixType mat;
        for (size_t i = 0; i < Dim; ++i)
          for (size_t j = 0; j < Dim; ++j)
            mat(index(i), index(j)) = (*this)(index(i), index(j));
        return mat;
      }

      VectorType getEigenvector(int index) const
      {
        if (!haveEigens_) buildEigens();
        return eigvecs_[index];
      }

      std::vector<VectorType> getEigenvectors() const
      {
        if (!haveEigens_) buildEigens();
        return eigvecs_;
      }

      std::vector<Number> getEigenvalues() const
      {
        if (!haveEigens_) buildEigens();
        return eigvals_;
      }

      SCISHARE friend std::ostream& operator<<(
          std::ostream& os, const DyadicTensorGeneric<Number, Dim>& other)
      {
        os << '[';
        for (size_t i = 0; i < other.getDimension1(); ++i)
          for (size_t j = 0; j < other.getDimension2(); ++j)
          {
            if (i + j != 0) os << ' ';
            os << other(other.index(j), other.index(i));
          }
        os << ']';

        return os;
      }

      SCISHARE friend std::istream& operator>>(
          std::istream& is, DyadicTensorGeneric<Number, Dim>& other)
      {
        char bracket;
        is >> bracket;
        for (size_t i = 0; i < other.getDimension1(); ++i)
          for (size_t j = 0; j < other.getDimension2(); ++j)
            is >> other(other.index(j), other.index(i));
        is >> bracket;

        return is;
      }

      DyadicTensorGeneric<Number, Dim>& operator=(const Number& v)
      {
        for (size_t i = 0; i < getDimension1(); ++i)
          for (size_t j = 0; j < getDimension2(); ++j)
            (*this)(index(i), index(j)) = v;
        haveEigens_ = false;
        return *this;
      }

      size_t getDimension1() const { return Dim; }
      size_t getDimension2() const { return Dim; }

      Number frobeniusNorm() const
      {
        if (!haveEigens_) buildEigens();
        return Eigen::Map<VectorType>(eigvals_.data()).norm();
      }

      Number maxNorm() const
      {
        if (!haveEigens_) buildEigens();
        auto maxVal = (std::numeric_limits<Number>::min)();
        for (const auto& e : eigvals_)
          maxVal = (std::max)(maxVal, e);
        return maxVal;
      }

      void setEigens(
          const std::vector<VectorType>& eigvecs, const std::vector<Number>& eigvals) const
      {
        if (eigvecs_.size() != eigvecs.size())
          THROW_INVALID_ARGUMENT("The number of input eigvecs must be " + eigvecs_.size());
        if (eigvals_.size() != eigvals.size())
          THROW_INVALID_ARGUMENT("The number of input eigvals must be " + eigvals_.size());
        eigvecs_ = eigvecs;
        eigvals_ = eigvals;
        haveEigens_ = true;
      }

      Number eigenValueSum() const
      {
        if (!haveEigens_) buildEigens();
        Number sum = 0;
        for (const auto& e : eigvals_)
          sum += e;
        return sum;
      }

     protected:
      const int RANK_ = 2;
      mutable std::vector<VectorType> eigvecs_;
      mutable std::vector<Number> eigvals_;
      mutable bool haveEigens_ = false;

      void buildEigens() const
      {
        if (haveEigens_) return;

        auto es = Eigen::EigenSolver<MatrixType>(this->asMatrix());
        auto vecs = es.eigenvectors();
        auto vals = es.eigenvalues();

        eigvals_.resize(Dim);
        eigvecs_.resize(Dim);

        for (size_t i = 0; i < Dim; ++i)
        {
          eigvals_[i] = vals(i).real();
          eigvecs_[i] = VectorType(Dim);
          for (size_t j = 0; j < Dim; ++j)
            eigvecs_[i](j, 0) = vecs(j, i).real();
        }

        haveEigens_ = true;
        reorderTensorValues();
      }

      void reorderTensorValues() const
      {
        if (!haveEigens_) return;
        typedef std::pair<Number, VectorType> EigPair;
        std::vector<EigPair> sortList(Dim);
        for (size_t i = 0; i < Dim; ++i)
          sortList[i] = std::make_pair(eigvals_[i], eigvecs_[i]);

        // sort by descending order of eigenvalues
        std::sort(sortList.begin(), sortList.end(),
            [](const EigPair& left, const EigPair& right) { return left.first > right.first; });

        auto sortedEigsIter = sortList.begin();

        for (size_t i = 0; i < Dim; ++i)
          std::tie(eigvals_[i], eigvecs_[i]) = *sortedEigsIter++;
      }

      void setEigenvaluesFromEigenvectors() const
      {
        eigvals_ = std::vector<Number>(eigvecs_.size());
        for (size_t i = 0; i < Dim; ++i)
          eigvals_[i] = eigvecs_[i].norm();
      }

      void normalizeEigenvectors() const
      {
        for (size_t i = 0; i < Dim; ++i)
          eigvecs_[i] /= eigvals_[i];
      }

      void setTensorValues()
      {
        for (size_t i = 0; i < Dim; ++i)
          for (size_t j = 0; j < Dim; ++j)
            (*this)(index(j), index(i)) = eigvecs_[i][j] * eigvals_[i];
      }

      long int index(size_t i) const { return static_cast<long int>(i); }
    };
  }
}
}

#endif
