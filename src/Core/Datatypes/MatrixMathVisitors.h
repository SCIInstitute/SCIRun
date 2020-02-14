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


#ifndef CORE_DATATYPES_MATRIX_MATH_VISITORS_H
#define CORE_DATATYPES_MATRIX_MATH_VISITORS_H

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {
  namespace Core {
    namespace Datatypes {

      namespace MatrixMath
      {
        class SCISHARE BinaryVisitor : public Matrix::Visitor
        {
        protected:
          explicit BinaryVisitor(MatrixHandle operand);
          static Matrix* cloneIfNotNull(MatrixHandle m);
          MatrixTypeCode typeCode_;
        };

        class SCISHARE AddMatrices : public BinaryVisitor
        {
        public:
          explicit AddMatrices(MatrixHandle addend);

          virtual void visit(DenseMatrixGeneric<double>& dense) override;
          virtual void visit(SparseRowMatrixGeneric<double>& sparse) override;
          virtual void visit(DenseColumnMatrixGeneric<double>& column) override;

          MatrixHandle sum_;
        };

        class SCISHARE MultiplyMatrices : public BinaryVisitor
        {
        public:
          explicit MultiplyMatrices(MatrixHandle factor);

          virtual void visit(DenseMatrixGeneric<double>& dense) override;
          virtual void visit(SparseRowMatrixGeneric<double>& sparse) override;
          virtual void visit(DenseColumnMatrixGeneric<double>& column) override;

          MatrixHandle getProduct() const { return product_; }
        private:
          MatrixHandle factor_, product_;
        };

        class SCISHARE NegateMatrix : public Matrix::Visitor
        {
        public:
          virtual void visit(DenseMatrixGeneric<double>& dense) override;
          virtual void visit(SparseRowMatrixGeneric<double>& sparse) override;
          virtual void visit(DenseColumnMatrixGeneric<double>& column) override;
        };

        class SCISHARE ScalarMultiplyMatrix : public Matrix::Visitor
        {
        public:
          explicit ScalarMultiplyMatrix(double scalar) : scalar_(scalar) {}
          virtual void visit(DenseMatrixGeneric<double>& dense) override;
          virtual void visit(SparseRowMatrixGeneric<double>& sparse) override;
          virtual void visit(DenseColumnMatrixGeneric<double>& column) override;
        private:
          double scalar_;
        };
      }

      template <typename T>
      struct NumberOfElements : MatrixBase<T>::Visitor
      {
        size_t value() const { return value_; }

        NumberOfElements() : value_(0) {}
        size_t value_;

        virtual void visit(DenseMatrixGeneric<T>& m) override
        {
          value_ = m.rows() * m.cols();
        }

        virtual void visit(DenseColumnMatrixGeneric<T>& m) override
        {
          value_ = m.nrows();
        }

        virtual void visit(SparseRowMatrixGeneric<T>& m) override
        {
          value_ = m.nonZeros();
        }
      };

      template <typename T>
      struct MinimumCoefficient : MatrixBase<T>::Visitor
      {
        T value() const { return value_; }

        MinimumCoefficient() : value_(0) {}
        T value_;

        virtual void visit(DenseMatrixGeneric<T>& m)
        {
          if (!m.empty())
            value_ = m.minCoeff();
        }

        virtual void visit(DenseColumnMatrixGeneric<T>& m)
        {
          if (!m.empty())
            value_ = m.minCoeff();
        }

        virtual void visit(SparseRowMatrixGeneric<T>& m)
        {
          if (!m.empty())
          {
            auto value = m.valuePtr();
            if (value)
              value_ = *std::min_element(m.valuePtr(), m.valuePtr() + m.nonZeros());
            else
              value_ = 0;
          }
        }
      };

      template <typename T>
      struct MaximumCoefficient : MatrixBase<T>::Visitor
      {
        T value() const { return value_; }

        MaximumCoefficient() : value_(0) {}
        T value_;

        virtual void visit(DenseMatrixGeneric<T>& m)
        {
          if (!m.empty())
            value_ = m.maxCoeff();
        }

        virtual void visit(DenseColumnMatrixGeneric<T>& m)
        {
          if (!m.empty())
            value_ = m.maxCoeff();
        }

        virtual void visit(SparseRowMatrixGeneric<T>& m)
        {
          if (!m.empty())
          {
            auto value = m.valuePtr();
            if (value)
              value_ = *std::max_element(m.valuePtr(), m.valuePtr() + m.nonZeros());
            else
              value_ = 0;
          }
        }
      };

    }
  }
}


#endif
