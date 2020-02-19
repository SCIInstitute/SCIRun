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


/// @todo Documentation Core/Datatypes/MatrixComparison.h

#ifndef CORE_DATATYPES_MATRIX_COMPARISON_H
#define CORE_DATATYPES_MATRIX_COMPARISON_H

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixIO.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  template <typename T>
  bool compare(const T& lhs, const T& rhs)
  {
    return std::fabs(lhs - rhs) < 1e-15;
  }

  template <>
  inline bool compare(const std::complex<double>& lhs, const std::complex<double>& rhs)
  {
    return std::norm(lhs - rhs) < 1e-15;
  }

  template <typename T>
  bool operator==(const DenseMatrixGeneric<T>& lhs, const DenseMatrixGeneric<T>& rhs)
  {
    bool returnValue =
      (lhs.rows() == rhs.rows()) &&
      (lhs.cols() == rhs.cols());

    if (returnValue)
    {
      for (int i = 0; returnValue && i < lhs.rows(); ++i)
      {
        for (int j = 0; returnValue && j < lhs.cols(); ++j)
        {
          returnValue &= compare(lhs(i, j), rhs(i, j));
        }
      }
    }
    return returnValue;
  }

  template <typename T>
  bool operator!=(const DenseMatrixGeneric<T>& lhs, const DenseMatrixGeneric<T>& rhs)
  {
    return !(lhs == rhs);
  }

  template <typename T>
  bool operator==(const DenseColumnMatrixGeneric<T>& lhs, const DenseColumnMatrixGeneric<T>& rhs)
  {
    bool returnValue = (lhs.rows() == rhs.rows());

    if (returnValue)
    {
      for (int i = 0; returnValue && i < lhs.rows(); ++i)
      {
        returnValue &= std::fabs(lhs(i) - rhs(i)) < 1e-15;
      }
    }
    return returnValue;
  }

  template <typename T>
  bool operator!=(const DenseColumnMatrixGeneric<T>& lhs, const DenseColumnMatrixGeneric<T>& rhs)
  {
    return !(lhs == rhs);
  }

  template <typename T>
  bool operator==(const SparseRowMatrixGeneric<T>& lhs, const SparseRowMatrixGeneric<T>& rhs)
  {
    if (lhs.rows() != rhs.rows())
      return false;
    if (lhs.cols() != rhs.cols())
      return false;

    for (int k = 0; k < lhs.outerSize(); ++k)
    {
      typename SparseRowMatrixGeneric<T>::InnerIterator it1(lhs,k);
      typename SparseRowMatrixGeneric<T>::InnerIterator it2(rhs,k);
      for (; it1 && it2; ++it1, ++it2)
      {
        if (it1.index() != it2.index())
          return false;
        if (it1.value() != it2.value())
          return false;
      }
    }
    return true;
  }

  template <typename T>
  bool operator!=(const SparseRowMatrixGeneric<T>& lhs, const SparseRowMatrixGeneric<T>& rhs)
  {
    return !(lhs == rhs);
  }

}}}


#endif
