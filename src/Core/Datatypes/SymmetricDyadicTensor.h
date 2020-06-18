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

#ifndef CORE_DATATYPES_SYMMETRIC_DYADIC_TENSOR_H
#define CORE_DATATYPES_SYMMETRIC_DYADIC_TENSOR_H

#include <Core/Datatypes/DyadicTensor.h>
#include "Core/Datatypes/DenseMatrix.h"
#include "Core/Datatypes/MatrixFwd.h"
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
  namespace Datatypes {
    template <typename T>
    class SymmetricDyadicTensor : public DyadicTensor<T>
    {
     public:
      SymmetricDyadicTensor(size_t dim) : DyadicTensor<T>(dim, dim), dim_(dim)
      {
        addSymmetryGroups();
      }
      SymmetricDyadicTensor(size_t dim, T val) : DyadicTensor<T>(dim, dim, val), dim_(dim)
      {
        addSymmetryGroups();
      }
      SymmetricDyadicTensor(const std::vector<DenseColumnMatrixGeneric<T>>& eigvecs)
          : DyadicTensor<T>(eigvecs.size(), eigvecs.size()), dim_(eigvecs.size())
      {
        addSymmetryGroups();
      }

     private:
      void addSymmetryGroups()
      {
        Eigen::DynamicSGroup d;
        for (int i = 0; i < dim_; ++i)
          for (int j = 0; j < i; ++j)
          {
            std::cout << "i " << i << " j " << j << "\n";
            if (i != j) std::cout << "adding sym" << "\n", d.addSymmetry(j, i), std::cout << "added\n";;
          }
      }

     protected:
      size_t dim_ = 1;
      Eigen::DynamicSGroup dsym_;
    };
  }
}
}

#endif
