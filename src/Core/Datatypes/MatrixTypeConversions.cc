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

#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>

using namespace SCIRun::Core::Datatypes;

DenseMatrixConstHandle matrix_cast::as_dense(const MatrixConstHandle& mh)
{
  return to<const DenseMatrix>(mh);
}

SparseRowMatrixConstHandle matrix_cast::as_sparse(const MatrixConstHandle& mh)
{
  return to<const SparseRowMatrix>(mh);
}

DenseColumnMatrixConstHandle matrix_cast::as_column(const MatrixConstHandle& mh)
{
  return to<const DenseColumnMatrix>(mh);
}

bool matrix_is::dense(const MatrixConstHandle& mh) 
{ 
  return matrix_cast::as_dense(mh) != 0; 
}

bool matrix_is::sparse(const MatrixConstHandle& mh) 
{ 
  return matrix_cast::as_sparse(mh) != 0; 
}

bool matrix_is::column(const MatrixConstHandle& mh) 
{ 
  return matrix_cast::as_column(mh) != 0; 
}

DenseColumnMatrixConstHandle matrix_convert::to_column(const MatrixConstHandle& mh)
{
  auto col = matrix_cast::as_column(mh);
  if (col)
    return col;

  auto dense = matrix_cast::as_dense(mh);
  if (dense)
    return DenseColumnMatrixConstHandle(new DenseColumnMatrix(dense->col(0)));

  return DenseColumnMatrixConstHandle();
}