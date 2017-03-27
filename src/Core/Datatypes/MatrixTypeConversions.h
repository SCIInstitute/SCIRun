/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
/// @todo Documentation Core/Datatypes/MatrixTypeConversion.h

#ifndef CORE_DATATYPES_MATRIX_TYPE_CONVERSIONS_H
#define CORE_DATATYPES_MATRIX_TYPE_CONVERSIONS_H

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  /// No conversion is done.
  /// NULL is returned if the matrix is not of the appropriate type.
  class SCISHARE castMatrix
  {
  public:
    template <class ToType>
    static SharedPointer<ToType> to(const SharedPointer<MatrixBase<typename ToType::value_type>>& matrix, typename boost::enable_if<boost::is_base_of<MatrixBase<typename ToType::value_type>, ToType> >::type* = nullptr)
    {
      return boost::dynamic_pointer_cast<ToType>(matrix);
    }

    template <typename T>
    static SharedPointer<DenseMatrixGeneric<T>> toDense(const SharedPointer<MatrixBase<T>>& mh)
    {
      return to<DenseMatrixGeneric<T>>(mh);
    }

    template <typename T>
    static SharedPointer<SparseRowMatrixGeneric<T>> toSparse(const SharedPointer<MatrixBase<T>>& mh)
    {
      return to<SparseRowMatrixGeneric<T>>(mh);
    }
    
    static DenseColumnMatrixHandle toColumn(const MatrixHandle& mh);

    castMatrix() = delete;
  };

  class SCISHARE matrixIs
  {
  public:
    // Test to see if the matrix is this subtype.
    template <typename T>
    static bool dense(const SharedPointer<MatrixBase<T>>& mh)
    {
      return castMatrix::toDense(mh) != nullptr;
    }
    
    template <typename T>
    static bool sparse(const SharedPointer<MatrixBase<T>>& mh)
    {
      return castMatrix::toSparse(mh) != nullptr;
    }

    static bool column(const MatrixHandle& mh);
    static std::string whatType(const MatrixHandle& mh);
    static std::string whatType(const ComplexMatrixHandle& mh);
    static MatrixTypeCode typeCode(const MatrixHandle& mh);

    matrixIs() = delete;
  };

  /// @todo: move
  class SCISHARE convertMatrix
  {
  public:
    template <typename T, template <typename> class MatrixType>
    static boost::shared_ptr<DenseColumnMatrixGeneric<typename MatrixType<T>::value_type>> toColumn(const MatrixType<T>& mh)
    {
      auto col = castMatrix::toColumn(mh);
      if (col)
        return col;

      auto dense = castMatrix::toDense(mh);
      if (dense)
        return boost::make_shared<DenseColumnMatrixGeneric<T>>(dense->col(0));

      auto sparse = castMatrix::toSparse(mh);
      if (sparse)
      {
        auto dense_col(DenseColumnMatrixGeneric<T>::Zero(sparse->nrows()));
        for (auto i = 0; i < sparse->nrows(); i++)
          dense_col(i, 0) = sparse->coeff(i, 0);

        return boost::make_shared<DenseColumnMatrixGeneric<T>>(dense_col);
      }

      return nullptr;
    }
    static DenseMatrixHandle toDense(const MatrixHandle& mh);
    static SparseRowMatrixHandle toSparse(const MatrixHandle& mh);
    static SparseRowMatrixHandle fromDenseToSparse(const DenseMatrix& mh);

    convertMatrix() = delete;
  private:
    static const double zero_threshold;  /// defines a threshold below that its a zero matrix element (sparsematrix)
  };

}}}


#endif
