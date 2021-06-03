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
      using TensorType = DyadicTensorGeneric<Number, Dim>;
      using VectorType = Eigen::Matrix<Number, Dim, 1>;
      using MatrixType = Eigen::Matrix<Number, Dim, Dim>;
      static const size_t MANDEL_SIZE_ = Dim + (Dim*Dim-Dim)/2;
      using MandelVector = Eigen::Matrix<Number, MANDEL_SIZE_, 1>;
      using SizeType = long;

      DyadicTensorGeneric() : parent() { parent::setZero(); }

      explicit DyadicTensorGeneric(Number val) : parent()
      {
        ordering_ = OrderState::NONE;
        for (SizeType i = 0; i < Dim; ++i)
          for (SizeType j = 0; j < Dim; ++j)
            (*this)(i, j) = (i == j) ? val : 0;
      }

      explicit DyadicTensorGeneric(const std::vector<VectorType>& eigvecs) : parent()
      {
        ordering_ = OrderState::NONE;
        if (eigvecs.size() != Dim)
          THROW_INVALID_ARGUMENT("The number of input vectors must be " + std::to_string(Dim));
        setEigenVectors(eigvecs);
      }

      DyadicTensorGeneric(const std::initializer_list<VectorType>& eigvecs) : parent()
      {
        ordering_ = OrderState::NONE;
        if (eigvecs.size() != Dim)
          THROW_INVALID_ARGUMENT("The number of input vectors must be " + std::to_string(Dim));
        setEigenVectors(eigvecs);
      }

       DyadicTensorGeneric(const VectorType& eigvals) : parent()
      {
        eigvals_ = eigvals;
        eigvalConstructor();
      }

       DyadicTensorGeneric(VectorType&& eigvals) : parent()
      {
        eigvals_ = std::move(eigvals);
        eigvalConstructor();
      }

      DyadicTensorGeneric(
          const std::vector<VectorType>& eigvecs, const VectorType& eigvals)
          : parent()
      {
        ordering_ = OrderState::NONE;
        setEigens(eigvecs, eigvals);
      }

      DyadicTensorGeneric(const TensorType& other) : parent()
      {
        ordering_ = OrderState::NONE;
        this->m_storage = other.m_storage;
        eigvecs_ = other.getEigenvectors();
        eigvals_ = other.getEigenvalues();
        haveEigens_ = other.haveEigens_;
      }

      DyadicTensorGeneric(TensorType&& other) : parent()
      {
        this->m_storage = std::move(other.m_storage);
        ordering_ = OrderState::NONE;
        eigvecs_ = std::move(other.eigvecs_);
        eigvals_ = std::move(other.eigvals_);
        haveEigens_ = other.haveEigens_;
      }

      DyadicTensorGeneric(const parent& other) : parent()
      {
        ordering_ = OrderState::NONE;
        for (SizeType i = 0; i < Dim; ++i)
          for (SizeType j = 0; j < Dim; ++j)
            (*this)(i, j) = other(i, j);
      }

      explicit DyadicTensorGeneric(const MatrixType& mat)
      {
        ordering_ = OrderState::NONE;
        for (SizeType i = 0; i < Dim; ++i)
          for (SizeType j = 0; j < Dim; ++j)
            (*this)(i, j) = mat(i, j);
      }
      explicit DyadicTensorGeneric(MandelVector man)
      {
        static const Number sqrt2 = std::sqrt(2);
        for (SizeType i = Dim; i < MANDEL_SIZE_; ++i)
          man(i) /= sqrt2;

        SizeType topLeftRow = 0;
        SizeType topLeftColumn = 0;
        SizeType bottomRightRow = Dim-1;
        SizeType bottomRightColumn = Dim-1;
        SizeType topRightRow = 0;
        SizeType topRightColumn = Dim-1;

        SizeType index = 0;
        while (index < (MANDEL_SIZE_-1))
        {
          SizeType i, j;
          // Add diagonal
          i = topLeftRow;
          j = topLeftColumn;
          while (i <= bottomRightRow && j <= bottomRightColumn)
          {
            (*this)(j, i) = man(index);
            (*this)(i++, j++) = man(index++);
          }

          // Add last column except value from diagonal
          i = bottomRightRow - 1;
          j = bottomRightColumn;
          while (i >= topRightRow)
          {
            (*this)(j, i) = man(index);
            (*this)(i--, j) = man(index++);
          }

          // Add first row except values from last 2 loops
          i = topRightRow;
          j = topRightColumn - 1;
          while (j > topLeftColumn)
          {
            (*this)(j, i) = man(index);
            (*this)(i, j--) = man(index++);
          }

          topLeftRow++;
          topLeftColumn += 2;
          bottomRightRow -= 2;
          bottomRightColumn--;
          topRightRow++;
          topRightColumn--;
        }
      }

      using parent::operator=;
      using parent::contract;

      TensorType& operator=(const TensorType& other)
      {
        parent::operator=(other);
        return *this;
      }

      TensorType& operator=(const parent other)
      {
        parent::operator=(other);
        return *this;
      }

      template <typename OtherDerived>
      bool operator!=(const OtherDerived& other) const
      {
        return !operator==(other);
      }

      template <typename OtherDerived>
      bool operator==(const OtherDerived& other) const
      {
        auto otherTensor = static_cast<TensorType>(other);
        if (Dim != otherTensor.dimension(0) || Dim != otherTensor.dimension(1)) return false;
        for (SizeType i = 0; i < Dim; ++i)
          for (SizeType j = 0; j < Dim; ++j)
            if ((*this)(i, j) != otherTensor(i, j)) return false;
        return true;
      }

      template <typename OtherDerived>
      TensorType operator*(const OtherDerived& other) const
      {
        TensorType newTensor(parent::operator*(other));
        return newTensor;
      }

      template <typename OtherDerived>
      TensorType operator/(const OtherDerived& other) const
      {
        TensorType newTensor(parent::operator/(other));
        return newTensor;
      }

      TensorType operator/(Number val) const
        {
          TensorType newTensor(parent::operator/(val));
          return newTensor;
        }

      template <typename OtherDerived>
      TensorType operator+(const OtherDerived& other) const
      {
        TensorType newTensor(parent::operator+(other));
        return newTensor;
      }

      template <typename OtherDerived>
      TensorType operator-(const OtherDerived& other) const
      {
        TensorType newTensor(parent::operator-(other));
        return newTensor;
      }

      void setEigenVectors(const std::vector<VectorType>& eigvecs)
      {
        eigvecs_ = eigvecs;
        setEigenvaluesFromEigenvectors();
        normalizeEigenvectors();
        haveEigens_ = true;
        ordering_ = OrderState::NONE;
        setTensorValues();
      }

      // This function is listed as something that will be added to Eigen::Tensor in the future.
      // Usage of this function should be replaced with Eigen's asMatrix function when it is
      // implemented.
      MatrixType asMatrix() const
      {
        MatrixType mat;
        for (SizeType i = 0; i < Dim; ++i)
          for (SizeType j = 0; j < Dim; ++j)
            mat(i, j) = (*this)(i, j);
        return mat;
      }

      VectorType getEigenvector(SizeType index) const
      {
        if (!haveEigens_) buildEigens();
        return eigvecs_[index];
      }

      std::vector<VectorType> getEigenvectors() const
      {
        if (!haveEigens_) buildEigens();
        return eigvecs_;
      }

      Number getEigenvalue(SizeType index) const
      {
        if (!haveEigens_) buildEigens();
        return eigvals_[index];
      }

      VectorType getEigenvalues() const
      {
        if (!haveEigens_) buildEigens();
        return eigvals_;
      }

      SCISHARE friend std::ostream& operator<<(
          std::ostream& os, const TensorType& other)
      {
        os << '[';
        for (SizeType i = 0; i < other.getDimension1(); ++i)
          for (SizeType j = 0; j < other.getDimension2(); ++j)
          {
            if (i + j != 0) os << ' ';
            os << other(j, i);
          }
        os << ']';

        return os;
      }

      SCISHARE friend std::istream& operator>>(
          std::istream& is, TensorType& other)
      {
        char bracket;
        is >> bracket;
        for (SizeType i = 0; i < other.getDimension1(); ++i)
          for (SizeType j = 0; j < other.getDimension2(); ++j)
            is >> other(j, i);
        is >> bracket;

        return is;
      }

      TensorType& operator=(const Number& v)
      {
        for (SizeType i = 0; i < getDimension1(); ++i)
          for (SizeType j = 0; j < getDimension2(); ++j)
            (*this)(i, j) = v;
        haveEigens_ = false;
        return *this;
      }

      SizeType getDimension1() const { return Dim; }
      SizeType getDimension2() const { return Dim; }

      VectorType getNormalizedEigenvalues() const
      {
        auto eigvals = getEigenvalues();
        auto fro = frobeniusNorm();
        for (SizeType i = 0; i < Dim; ++i)
          eigvals[i] /= fro;
        return eigvals;
      }

      Number frobeniusNorm() const
      {
        if (!haveEigens_) buildEigens();
        return eigvals_.norm();
      }

      Number maxNorm() const
      {
        if (!haveEigens_) buildEigens();
        auto maxVal = (std::numeric_limits<Number>::min)();
        for (SizeType i = 0; i < Dim; ++i)
          maxVal = (std::max)(maxVal, eigvals_[i]);
        return maxVal;
      }

      void setEigens(const std::vector<VectorType>& eigvecs, const std::initializer_list<Number>& eigvals)
      {
        assertEigenSize(eigvecs.size(), eigvals.size());
        setEigens(eigvecs, std::vector<Number>(eigvals));
      }

      void setEigens(const std::vector<VectorType>& eigvecs, const std::vector<Number>& eigvals)
      {
        assertEigenSize(eigvecs.size(), eigvals.size());
        setEigens(eigvecs, VectorType::Map(eigvals.data()));
      }

      void setEigens(const std::vector<VectorType>& eigvecs, const VectorType& eigvals)
      {
        assertEigenSize(eigvecs.size(), eigvals.size());
        eigvecs_ = eigvecs;
        eigvals_ = eigvals;
        haveEigens_ = true;
        setTensorValues();
      }

      void normalize()
      {
        eigvals_ = getNormalizedEigenvalues();
        setTensorValues();
      }

      Number eigenvalueSum() const
      {
        if (!haveEigens_) buildEigens();
        return eigvals_.sum();
      }

      MatrixType getEigenvectorsAsMatrix() const
      {
        if (!haveEigens_) buildEigens();
        MatrixType V;
        for (SizeType i = 0; i < Dim; ++i)
          V.col(i) = eigvecs_[i];
        return V;
      }

      Number trace() const
      {
        double sum = 0.0;
        for (auto i = 0; i < Dim; ++i)
          sum += (*this)(i, i);
        return sum;
      }

      TensorType transpose() const
      {
        return TensorType(this->asMatrix().transpose());
      }

     protected:
      const SizeType RANK_ = 2;
      mutable std::vector<VectorType> eigvecs_;
      mutable VectorType eigvals_;
      mutable bool haveEigens_ = false;

      void buildEigens() const
      {
        if (haveEigens_) return;

        auto es = Eigen::SelfAdjointEigenSolver<MatrixType>(this->asMatrix());
        auto vecs = es.eigenvectors();
        auto vals = es.eigenvalues();

        eigvals_.resize(Dim);
        eigvecs_.resize(Dim);

        for (SizeType i = 0; i < Dim; ++i)
        {
          eigvals_[i] = vals(Dim-1-i);
          eigvecs_[i] = VectorType(Dim);
          for (SizeType j = 0; j < Dim; ++j)
            eigvecs_[i][j] = vecs(j, Dim-1-i);
        }

        haveEigens_ = true;
      }

      void reorderTensorValues() const
      {
        if (!haveEigens_) buildEigens();
        typedef std::pair<Number, VectorType> EigPair;
        std::vector<EigPair> sortList(Dim);
        for (SizeType i = 0; i < Dim; ++i)
          sortList[i] = std::make_pair(eigvals_[i], eigvecs_[i]);

        // sort by descending order of eigenvalues
        std::sort(sortList.begin(), sortList.end(),
            [](const EigPair& left, const EigPair& right) { return left.first > right.first; });

        auto sortedEigsIter = sortList.begin();

        for (SizeType i = 0; i < Dim; ++i)
          std::tie(eigvals_[i], eigvecs_[i]) = *sortedEigsIter++;
      }

      void setEigenvaluesFromEigenvectors() const
      {
        eigvals_ = VectorType(Dim);
        for (SizeType i = 0; i < Dim; ++i)
          eigvals_[i] = eigvecs_[i].norm();
      }

      void normalizeEigenvectors() const
      {
        for (SizeType i = 0; i < Dim; ++i)
          eigvecs_[i] /= eigvals_[i];
      }

    public:
     // An arbitrary eigenvector is flipped if the coordinate system is left handed
     void forceRightHandedCoordinateSystem() const
     {
       if (!haveEigens_) buildEigens();
       const static double epsilon = std::pow(2, -52);
       auto rightHandedEigvec2 = eigvecs_[0].cross(eigvecs_[1]);
       if ((rightHandedEigvec2).dot(eigvecs_[2]) < epsilon) eigvecs_[2] = rightHandedEigvec2;
       ordering_ = OrderState::DESCENDING_RHS;
     }

     void makePositive()
     {
       static const double zeroThreshold = 0.000001;

       auto eigvals = getEigenvalues();
       auto eigvecs = getEigenvectors();
       for (SizeType i = 0; i < Dim; ++i)
       {
         eigvals[i] = std::abs(eigvals[i]);
         if (eigvals[i] <= zeroThreshold) eigvals[i] = 0;
       }

       setEigens(eigvecs, eigvals);
       ordering_ = OrderState::NONE;
     }

     void setDescendingOrder()
     {
       if (ordering_ == OrderState::DESCENDING) return;
       reorderTensorValues();
       setTensorValues();
       ordering_ = OrderState::DESCENDING;
     }

     void setDescendingRHSOrder()
     {
       if (ordering_ == OrderState::DESCENDING_RHS) return;
       reorderTensorValues();
       forceRightHandedCoordinateSystem();
       setTensorValues();
       ordering_ = OrderState::DESCENDING_RHS;
     }

     void setTensorValues()
     {
       auto D = eigvals_.asDiagonal();
       auto V = this->getEigenvectorsAsMatrix();
       auto mat = V * (D * V.transpose());
       for (SizeType i = 0; i < Dim; ++i)
         for (SizeType j = 0; j < Dim; ++j)
           (*this)(j, i) = mat(j, i);
      }

      MandelVector mandel() const
      {
        SizeType topLeftRow = 0;
        SizeType topLeftColumn = 0;
        SizeType bottomRightRow = Dim-1;
        SizeType bottomRightColumn = Dim-1;
        SizeType topRightRow = 0;
        SizeType topRightColumn = Dim-1;

        MandelVector man;
        SizeType index = 0;
        while (index < (MANDEL_SIZE_-1))
        {
          SizeType i, j;
          // Add diagonal
          i = topLeftRow;
          j = topLeftColumn;
          while (i <= bottomRightRow && j <= bottomRightColumn)
            man(index++) = (*this)(i++, j++);

          // Add last column except value from diagonal
          i = bottomRightRow - 1;
          j = bottomRightColumn;
          while (i >= topRightRow)
            man(index++) = (*this)(i--, j);

          // Add first row except values from last 2 loops
          i = topRightRow;
          j = topRightColumn - 1;
          while (j > topLeftColumn)
            man(index++) = (*this)(i, j--);

          topLeftRow++;
          topLeftColumn += 2;
          bottomRightRow -= 2;
          bottomRightColumn--;
          topRightRow++;
          topRightColumn--;
        }

        static const Number sqrt2 = std::sqrt(2);
        for (SizeType i = Dim; i < MANDEL_SIZE_; ++i)
          man(i) *= sqrt2;
        return man;
      }

      TensorType anisotropicDeviatoric() const
      {
        const static Number third = 1.0/3.0;
        const TensorType trMat = TensorType(third*trace());
        return *this - trMat;
      }

      Number determinant() const
      {
        return asMatrix().determinant();
      }

      Number mode() const
      {
        const TensorType aniso_dev = anisotropicDeviatoric();
        const Number det = (aniso_dev/aniso_dev.frobeniusNorm()).determinant();
        return 3.0*std::sqrt(6.0) * det;
      }

    private:
      void eigvalConstructor()
      {
        ordering_ = OrderState::NONE;
        if (eigvals_.size() != Dim)
          THROW_INVALID_ARGUMENT("The number of input vectors must be " + std::to_string(Dim));
        eigvecs_.resize(Dim);
        for (SizeType i = 0; i < Dim; ++i)
        {
          eigvecs_[i] = VectorType();
          for (SizeType j = 0; j < Dim; ++j)
          {
            eigvecs_[i][j] = 0.0;
            (*this)(i,j) = 0.0;
          }
          eigvecs_[i][i] = 1.0;
          (*this)(i,i) = eigvals_[i];
        }
      }

      void assertEigenSize(SizeType vecDim, SizeType valDim)
      {
        if (vecDim != Dim)
          THROW_INVALID_ARGUMENT("The number of input eigvecs must be " + std::to_string(eigvecs_.size()));
        if (valDim != Dim)
          THROW_INVALID_ARGUMENT("The number of input eigvals must be " + std::to_string(eigvals_.size()));
      }

     enum class OrderState
     {
       NONE,
       DESCENDING,
       DESCENDING_RHS
     };
     mutable OrderState ordering_;
    };
  }
}
}

#endif
