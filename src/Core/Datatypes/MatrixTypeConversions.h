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
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  /// No conversion is done.
  /// NULL is returned if the matrix is not of the appropriate type.
  class SCISHARE matrix_cast
  {
  public:
    template <class ToType>
    static boost::shared_ptr<ToType> to(const MatrixHandle& matrix, typename boost::enable_if<boost::is_base_of<MatrixBase<typename ToType::value_type>, ToType> >::type* = 0)
    {
      return boost::dynamic_pointer_cast<ToType>(matrix);
    }

    static DenseMatrixHandle as_dense(const MatrixHandle& mh);
    static SparseRowMatrixHandle as_sparse(const MatrixHandle& mh);
    static DenseColumnMatrixHandle as_column(const MatrixHandle& mh);

  private:
    matrix_cast();
  };

  class SCISHARE matrix_is
  {
  public:
    // Test to see if the matrix is this subtype.
    static bool dense(const MatrixHandle& mh);
    static bool sparse(const MatrixHandle& mh);
    static bool column(const MatrixHandle& mh);
    static std::string whatType(const MatrixHandle& mh);
    static MatrixTypeCode typeCode(const MatrixHandle& mh);
  private:
    matrix_is();
  };
 
  /// @todo: move
  class SCISHARE matrix_convert
  {
  public:
    static DenseColumnMatrixHandle to_column(const MatrixHandle& mh);
    static DenseMatrixHandle to_dense(const MatrixHandle& mh);
    static SparseRowMatrixHandle to_sparse(const MatrixHandle& mh);
    static SparseRowMatrixHandle denseToSparse(const DenseMatrix& mh);
  private:
    matrix_convert();
    static const double zero_threshold;  /// defines a threshold below that its a zero matrix element (sparsematrix)
  };

}}}


#endif
