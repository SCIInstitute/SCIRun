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

#include <sstream>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>
#include <Core/Datatypes/MatrixMathVisitors.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;
using namespace SCIRun::Core::Datatypes::MatrixMath;

#define MATRIX_VERSION 3

void MatrixIOBase::io(Piostream& stream)
{
  int version = stream.begin_class("Matrix", MATRIX_VERSION);
  if (version < 2) 
  {
    int tmpsym;
    stream.io(tmpsym);
  }
  if (version > 2) 
  {
    PropertyManager().io(stream);
  }
  stream.end_class();
}

PersistentTypeID MatrixIOBase::type_id("MatrixIOBase", "Datatype", 0);


BinaryVisitor::BinaryVisitor(MatrixHandle operand) : typeCode_(matrix_is::typeCode(operand)) {}

Matrix* BinaryVisitor::cloneIfNotNull(MatrixHandle m)
{
  ENSURE_NOT_NULL(m, "Addend");
  return m->clone();
}

AddMatrices::AddMatrices(MatrixHandle addend) : BinaryVisitor(addend), sum_(cloneIfNotNull(addend))
{
}

void AddMatrices::visit(DenseMatrixGeneric<double>& dense)
{
  switch (typeCode_)
  {
  case DENSE:
    *matrix_cast::as_dense(sum_) += dense;
    break;
  case COLUMN:
    *matrix_cast::as_column(sum_) += dense;
    break;
  case SPARSE_ROW:
    *matrix_cast::as_sparse(sum_) = *matrix_cast::as_sparse(sum_) + *matrix_convert::denseToSparse(dense);
    break;
  }
}
void AddMatrices::visit(SparseRowMatrixGeneric<double>& sparse)
{
  switch (typeCode_)
  {
  case DENSE:
    sum_.reset(new SparseRowMatrix(*matrix_convert::to_sparse(sum_) + sparse));
    typeCode_ = SPARSE_ROW;
    break;
  case COLUMN:
    sum_.reset(new SparseRowMatrix(*matrix_convert::to_sparse(sum_) + sparse));
    typeCode_ = SPARSE_ROW;
    break;
  case SPARSE_ROW:
    *matrix_cast::as_sparse(sum_) = *matrix_cast::as_sparse(sum_) + sparse;
    break;
  }
}
void AddMatrices::visit(DenseColumnMatrixGeneric<double>& column)
{
  switch (typeCode_)
  {
  case DENSE:
    *matrix_cast::as_dense(sum_) += column;
    break;
  case COLUMN:
    *matrix_cast::as_column(sum_) += column;
    break;
  case SPARSE_ROW:
    *matrix_cast::as_sparse(sum_) = *matrix_cast::as_sparse(sum_) + *matrix_convert::denseToSparse(column);
    break;
  }
}

MultiplyMatrices::MultiplyMatrices(MatrixHandle factor) : BinaryVisitor(factor), product_(cloneIfNotNull(factor))
{
}

void MultiplyMatrices::visit(DenseMatrixGeneric<double>& dense)
{
  switch (typeCode_)
  {
  case DENSE:
    *matrix_cast::as_dense(product_) *= dense;
    break;
  case COLUMN:
    *matrix_cast::as_column(product_) *= dense;
    break;
  case SPARSE_ROW:
    *matrix_cast::as_sparse(product_) = *matrix_cast::as_sparse(product_) * *matrix_convert::denseToSparse(dense);
    break;
  }
}
void MultiplyMatrices::visit(SparseRowMatrixGeneric<double>& sparse)
{
  switch (typeCode_)
  {
  case DENSE:
    product_.reset(new SparseRowMatrix(*matrix_convert::to_sparse(product_) * sparse));
    typeCode_ = SPARSE_ROW;
    break;
  case COLUMN:
    product_.reset(new SparseRowMatrix(*matrix_convert::to_sparse(product_) * sparse));
    typeCode_ = SPARSE_ROW;
    break;
  case SPARSE_ROW:
    *matrix_cast::as_sparse(product_) = *matrix_cast::as_sparse(product_) * sparse;
    break;
  }
}
void MultiplyMatrices::visit(DenseColumnMatrixGeneric<double>& column)
{
  switch (typeCode_)
  {
  case DENSE:
    *matrix_cast::as_dense(product_) *= column;
    break;
  case COLUMN:
    *matrix_cast::as_column(product_) *= column;
    break;
  case SPARSE_ROW:
    *matrix_cast::as_sparse(product_) = *matrix_cast::as_sparse(product_) * *matrix_convert::denseToSparse(column);
    break;
  }
}

void NegateMatrix::visit(DenseMatrixGeneric<double>& dense)
{
  dense *= -1;
}
void NegateMatrix::visit(SparseRowMatrixGeneric<double>& sparse)
{
  sparse *= -1;
}
void NegateMatrix::visit(DenseColumnMatrixGeneric<double>& column)
{
  column *= -1;
}

void ScalarMultiplyMatrix::visit(DenseMatrixGeneric<double>& dense)
{
  dense *= scalar_;
}
void ScalarMultiplyMatrix::visit(SparseRowMatrixGeneric<double>& sparse)
{
  sparse *= scalar_;
}
void ScalarMultiplyMatrix::visit(DenseColumnMatrixGeneric<double>& column)
{
  column *= scalar_;
}