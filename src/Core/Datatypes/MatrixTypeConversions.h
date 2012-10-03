/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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


#ifndef CORE_DATATYPES_MATRIX_TYPE_CONVERSIONS_H
#define CORE_DATATYPES_MATRIX_TYPE_CONVERSIONS_H 

#include <Core/Datatypes/Matrix.h>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <Core/Datatypes/Share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  // No conversion is done.
  // NULL is returned if the matrix is not of the appropriate type.
  class SCISHARE matrix_cast
  {
  public:
    template <class ToType>
    static boost::shared_ptr<ToType> to(const MatrixConstHandle& matrix, typename boost::enable_if<boost::is_base_of<MatrixBase<typename ToType::value_type>, ToType> >::type* = 0)
    {
      return boost::dynamic_pointer_cast<ToType>(matrix);
    }

    static DenseMatrixConstHandle as_dense(const MatrixConstHandle& mh);
    static SparseRowMatrixConstHandle as_sparse(const MatrixConstHandle& mh);
    static DenseColumnMatrixConstHandle as_column(const MatrixConstHandle& mh);

  private:
    matrix_cast();
  };

  class SCISHARE matrix_is
  {
  public:
    // Test to see if the matrix is this subtype.
    static bool dense(const MatrixConstHandle& mh);
    static bool sparse(const MatrixConstHandle& mh);
    static bool column(const MatrixConstHandle& mh);
  private:
    matrix_is();
  };
 

}}}


#endif
