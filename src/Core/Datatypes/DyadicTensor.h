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
#include <unsupported/Eigen/CXX11/Tensor>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
  namespace Datatypes {
    // Dyadic tensors are also known as second-order tensors
    template <typename Number, size_t Dim0, size_t Dim1>
    class DyadicTensorGeneric : public TensorBaseGeneric<Number, Eigen::Sizes<Dim0, Dim1>>
    {
     public:
      typedef TensorBaseGeneric<Number, Eigen::Sizes<Dim0, Dim1>> parent;

      DyadicTensorGeneric() : parent() { parent::setZero(); }

      DyadicTensorGeneric(Number val) : parent()
      {
        for (size_t i = 0; i < Dim0; ++i)
          for (size_t j = 0; j < Dim1; ++j)
            (*this)(i, j) = (i == j) ? val : 0;
      }

      DyadicTensorGeneric(const std::vector<DenseColumnMatrixGeneric<Number>>& eigvecs) : parent()
      {
        if (eigvecs.size() != Dim0)
          THROW_INVALID_ARGUMENT("The number of input vectors must be " + Dim0);
        if (eigvecs[0].size() != Dim1)
          THROW_INVALID_ARGUMENT("The length of input vectors must be " + Dim1);
        setEigenVectors(eigvecs);
      }

      DyadicTensorGeneric(const DyadicTensorGeneric<Number, Dim0, Dim1>& other) : parent()
      {
        for (size_t i = 0; i < Dim0; ++i)
          for (size_t j = 0; j < Dim1; ++j)
            (*this)(index(i), index(j)) = other(index(i), index(j));
        eigvecs_ = other.getEigenvectors();
        eigvals_ = other.getEigenvalues();
        haveEigens_ = true;
      }

      DyadicTensorGeneric(DyadicTensorGeneric<Number, Dim0, Dim1>&& other) : parent()
      {
        for (size_t i = 0; i < Dim0; ++i)
          for (size_t j = 0; j < Dim1; ++j)
            (*this)(index(i), index(j)) = std::move(other(index(i), index(j)));
        eigvecs_ = std::move(other.eigvecs_);
        eigvals_ = std::move(other.eigvals_);
        haveEigens_ = true;
      }

      DyadicTensorGeneric(const Eigen::TensorFixedSize<Number, Eigen::Sizes<Dim0, Dim1>>& other)
          : parent()
      {
        for (size_t i = 0; i < Dim0; ++i)
          for (size_t j = 0; j < Dim1; ++j)
            (*this)(index(i), index(j)) = other(index(i), index(j));
      }

      explicit DyadicTensorGeneric(const DenseMatrixGeneric<Number>& mat)
      {
        if (mat.nrows() != Dim0)
          THROW_INVALID_ARGUMENT("The number of rows of the input matrix must be " + Dim0);
        if (mat.ncols() != Dim1)
          THROW_INVALID_ARGUMENT("The length of rows of the input matrix must be " + Dim1);
        for (size_t i = 0; i < Dim0; ++i)
          for (size_t j = 0; j < Dim1; ++j)
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
        auto otherTensor = static_cast<DyadicTensorGeneric<Number, Dim0, Dim1>>(other);
        if (Dim0 != otherTensor.dimension(0) || Dim1 != otherTensor.dimension(1)) return false;
        for (size_t i = 0; i < Dim0; ++i)
          for (size_t j = 0; j < Dim1; ++j)
            if ((*this)(index(i), index(j)) != otherTensor(index(i), index(j))) return false;
        return true;
      }

      void setEigenVectors(const std::vector<DenseColumnMatrixGeneric<Number>>& eigvecs)
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
      Eigen::Matrix<Number, Dim0, Dim1> asMatrix() const
      {
        Eigen::Matrix<Number, Dim0, Dim1> mat;
        for (size_t i = 0; i < Dim0; ++i)
          for (size_t j = 0; j < Dim1; ++j)
            mat(index(i), index(j)) = (*this)(index(i), index(j));
        return mat;
      }

      DenseColumnMatrixGeneric<Number> getEigenvector(int index) const
      {
        if (!haveEigens_) buildEigens();
        return eigvecs_[index];
      }

      std::vector<DenseColumnMatrixGeneric<Number>> getEigenvectors() const
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
          std::ostream& os, const DyadicTensorGeneric<Number, Dim0, Dim1>& other)
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
          std::istream& is, DyadicTensorGeneric<Number, Dim0, Dim1>& other)
      {
        char bracket;
        is >> bracket;
        for (size_t i = 0; i < other.getDimension1(); ++i)
          for (size_t j = 0; j < other.getDimension2(); ++j)
            is >> other(other.index(j), other.index(i));
        is >> bracket;

        return is;
      }

      DyadicTensorGeneric<Number, Dim0, Dim1>& operator=(const Number& v)
      {
        for (size_t i = 0; i < getDimension1(); ++i)
          for (size_t j = 0; j < getDimension2(); ++j)
            (*this)(index(i), index(j)) = v;
        haveEigens_ = false;
        return *this;
      }

      size_t getDimension1() const { return Dim0; }
      size_t getDimension2() const { return Dim1; }

      Number frobeniusNorm() const
      {
        if (!haveEigens_) buildEigens();
        auto eigvals = DenseColumnMatrix(eigvals_);
        return eigvals.norm();
      }

      Number maxNorm() const
      {
        if (!haveEigens_) buildEigens();
        auto maxVal = std::numeric_limits<Number>::min();
        for (const auto& e : eigvals_)
          maxVal = std::max(maxVal, e);
        return maxVal;
      }

      void setEigens(
          const std::vector<DenseColumnMatrix>& eigvecs, const std::vector<Number>& eigvals) const
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
      mutable std::vector<DenseColumnMatrixGeneric<Number>> eigvecs_;
      mutable std::vector<Number> eigvals_;
      mutable bool haveEigens_ = false;

      void buildEigens() const
      {
        if (haveEigens_) return;

        auto es = Eigen::EigenSolver<Eigen::Matrix<Number, Dim0, Dim1>>(this->asMatrix());
        auto vecs = es.eigenvectors();
        auto vals = es.eigenvalues();

        eigvals_.resize(Dim0);
        eigvecs_.resize(Dim0);

        for (size_t i = 0; i < Dim0; ++i)
        {
          eigvals_[i] = real(vals(i));
          eigvecs_[i] = DenseColumnMatrixGeneric<Number>(Dim1);
          for (size_t j = 0; j < Dim1; ++j)
            eigvecs_[i].put(j, 0, real(vecs(j, i)));
        }

        haveEigens_ = true;
        reorderTensorValues();
      }

      void reorderTensorValues() const
      {
        if (!haveEigens_) return;
        typedef std::pair<Number, DenseColumnMatrixGeneric<Number>> EigPair;
        std::vector<EigPair> sortList(Dim0);
        for (size_t i = 0; i < Dim0; ++i)
          sortList[i] = std::make_pair(eigvals_[i], eigvecs_[i]);

        // sort by descending order of eigenvalues
        std::sort(sortList.begin(), sortList.end(),
            [](const EigPair& left, const EigPair& right) { return left.first > right.first; });

        auto sortedEigsIter = sortList.begin();

        for (size_t i = 0; i < Dim0; ++i)
          std::tie(eigvals_[i], eigvecs_[i]) = *sortedEigsIter++;
      }

      void setEigenvaluesFromEigenvectors() const
      {
        eigvals_ = std::vector<Number>(eigvecs_.size());
        for (size_t i = 0; i < Dim0; ++i)
          eigvals_[i] = eigvecs_[i].norm();
      }

      void normalizeEigenvectors() const
      {
        for (size_t i = 0; i < Dim0; ++i)
          eigvecs_[i] /= eigvals_[i];
      }

      void setTensorValues()
      {
        for (size_t i = 0; i < Dim0; ++i)
          for (size_t j = 0; j < Dim1; ++j)
            (*this)(index(j), index(i)) = eigvecs_[i][j] * eigvals_[i];
      }

      long int index(size_t i) const { return static_cast<long int>(i); }
    };
  }
}
}

#endif
