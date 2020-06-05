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


#include <Core/Datatypes/SymmetricTensor.h>

using namespace SCIRun;
using namespace Core::Geometry;
using namespace Core::Datatypes;

template <typename T>
SymmetricTensor<T>::SymmetricTensor(const Vector& e1, const Vector& e2, const Vector& e3)
  : Eigen::Tensor<T, 2, Eigen::RowMajor>(3, 3)
{
  eigvals_ = {e1.length(), e2.length(), e3.length()};
  eigvecs_ = {e1 / eigvals_[0], e2 / eigvals_[1], e3 / eigvals_[2]};
}

template <typename T>
SymmetricTensor<T>::SymmetricTensor(const Vector& e1, const Vector& e2, const Vector& e3,
                                 double v1, double v2, double v3)
  : Eigen::Tensor<T, 2, Eigen::RowMajor>(3, 3)
{
  eigvals_ = {v1, v2, v3};
  eigvecs_ = {e1, e2, e3};
}

