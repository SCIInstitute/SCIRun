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


#ifndef CORE_DATATYPES_MATRIX_COMPARISON_H
#define CORE_DATATYPES_MATRIX_COMPARISON_H 

#include <Core/Datatypes/Matrix.h>

namespace SCIRun {
namespace Domain {
namespace Datatypes {
  //TODO DAN

  template <typename T>
  bool operator==(const DenseMatrixGeneric<T>& lhs, const DenseMatrixGeneric<T>& rhs)
  {
    bool returnValue = 
      (lhs.nrows() == lhs.nrows()) &&
      (lhs.ncols() == rhs.ncols());

    if (returnValue)
    {
      for (size_t i = 0; returnValue && i < lhs.nrows(); ++i)
      {
        for (size_t j = 0; returnValue && j < lhs.ncols(); ++j)
        {
          returnValue &= lhs(i,j) == rhs(i,j);
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

}}}


#endif
