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



///
///@file  MatrixOperations.cc
///@brief Matrix Operations
/// 
///@author
///       Michael Callahan
///       Department of Computer Science
///       University of Utah
///@date  August 2003
/// 

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseMatrixFunctions.h>
#include <Core/Datatypes/MatrixOperations.h>
#include <Core/Datatypes/ColumnMatrixFunctions.h>
#include <Core/Datatypes/MatrixTypeConverter.h>


namespace SCIRun {

using namespace SparseMatrixFunctions;

MatrixHandle
operator+(MatrixHandle A, MatrixHandle B)
{
  ASSERT(A.get_rep());
  ASSERT(B.get_rep());

  ASSERT(A->ncols() == B->ncols());
  ASSERT(A->nrows() == B->nrows());

  if (matrix_is::column(A) && matrix_is::column(B))
  {
    ColumnMatrix *ac = matrix_cast::as_column(A);
    ColumnMatrix *bc = matrix_cast::as_column(B);
    ColumnMatrix *cc = new ColumnMatrix(ac->nrows());
    Add(*cc, *ac, *bc);
    return cc;
  }
  else if (matrix_is::sparse(A) && matrix_is::sparse(B))
  {
    SparseRowMatrix *as = matrix_cast::as_sparse(A);
    SparseRowMatrix *bs = matrix_cast::as_sparse(B);
    SparseRowMatrix *cs = AddSparse(*as, *bs);
    return cs;
  }
  else
  {
    DenseMatrix *ad = A->dense();
    DenseMatrix *bd = B->dense();
    DenseMatrix *cd = new DenseMatrix(ad->nrows(), bd->ncols());
    Add(*cd, *ad, *bd);
    if (!(matrix_is::dense(A))) { delete ad; }
    if (!(matrix_is::dense(B))) { delete bd; }
    return cd;
  }
}

ColumnMatrix
operator+(const ColumnMatrix& A, const ColumnMatrix& B)
{
  ASSERT(A.ncols() == B.ncols());
  ASSERT(A.nrows() == B.nrows());

  ColumnMatrix C(A.nrows());
  Add(C, A, B);
  return C;
}

ColumnMatrix
operator-(const ColumnMatrix& A, const ColumnMatrix& B)
{
  return A + (-B);
}

MatrixHandle
operator-(MatrixHandle A, MatrixHandle B)
{
  ASSERT(A.get_rep());
  ASSERT(B.get_rep());

  ASSERT(A->ncols() == B->ncols());
  ASSERT(A->nrows() == B->nrows());

  if (matrix_is::column(A) && matrix_is::column(B))
  {
    ColumnMatrix *ac = matrix_cast::as_column(A);
    ColumnMatrix *bc = matrix_cast::as_column(B);
    ColumnMatrix *cc = new ColumnMatrix(ac->nrows());
    Sub(*cc, *ac, *bc);
    return cc;
  }
  else if (matrix_is::sparse(A) && matrix_is::sparse(B))
  {
    SparseRowMatrix *as = matrix_cast::as_sparse(A);
    SparseRowMatrix *bs = matrix_cast::as_sparse(B);
    SparseRowMatrix *cs = SubSparse(*as, *bs);
    return cs;
  }
  else
  {
    DenseMatrix *ad = A->dense();
    DenseMatrix *bd = B->dense();
    DenseMatrix *cd = new DenseMatrix(ad->nrows(), bd->ncols());
    Sub(*cd, *ad, *bd);
    if (!(matrix_is::dense(A))) { delete ad; }
    if (!(matrix_is::dense(B))) { delete bd; }
    return cd;
  }
}


MatrixHandle
operator*(MatrixHandle A, MatrixHandle B)
{
  ASSERT(A.get_rep());
  ASSERT(B.get_rep());

  ASSERTEQ(A->ncols(), B->nrows());

  if (matrix_is::column(B))
  {
    ColumnMatrix *cd = new ColumnMatrix(A->nrows());
    Mult(*cd, *A, *(B->column()));
    return cd;
  }
  else if (matrix_is::sparse(A) && matrix_is::sparse(B))
  {
    SparseRowMatrix *as = A->sparse();
    SparseRowMatrix *bs = B->sparse();
    return sparse_sparse_mult(*as, *bs);
  }
  else if (matrix_is::sparse(A))
  {
    SparseRowMatrix *ad = A->sparse();
    DenseMatrix *bd = B->dense();
    DenseMatrix *cd = new DenseMatrix(ad->nrows(), bd->ncols());
    sparse_mult(*ad, *bd, *cd);
    if (!(matrix_is::dense(B))) { delete bd; }
    return cd;
  }
  else if (matrix_is::sparse(B))
  {
    DenseMatrix *ad = A->dense();
    SparseRowMatrix *bst = B->sparse()->make_transpose();
    DenseMatrix *cd = new DenseMatrix(A->nrows(), B->ncols());
    sparse_mult_transXB(*bst, *ad, *cd);
    if (!(matrix_is::dense(A))) { delete ad; }
    delete bst;
    return cd;
  }
  else
  {
    DenseMatrix *ad = A->dense();
    DenseMatrix *bd = B->dense();
    DenseMatrix *cd = new DenseMatrix(ad->nrows(), bd->ncols());
    Mult(*cd, *ad, *bd);

    if (!(matrix_is::dense(A))) { delete ad; }
    if (!(matrix_is::dense(B))) { delete bd; }
    return cd;
  }
}

ColumnMatrix 
operator*(const DenseMatrix& A, const ColumnMatrix& B)
{
  ASSERTEQ(A.ncols(), B.nrows());

  ColumnMatrix cd(A.nrows());
  Mult(cd, A, B);
  return cd;
}

DenseMatrix 
operator*(const DenseMatrix& A, const DenseMatrix& B)
{
  ASSERTEQ(A.ncols(), B.nrows());

  DenseMatrix cd(A.nrows(), B.ncols());
  Mult(cd, A, B);
  return cd;
}

MatrixHandle
operator*(double a, MatrixHandle B)
{
  ASSERT(B.get_rep());

  MatrixHandle C = B->clone();
  C->scalar_multiply(a);
  return C;
}

MatrixHandle
operator-(MatrixHandle A)
{
  return -1 * A;
}

ColumnMatrix
operator-(const ColumnMatrix& A)
{
  return -1 * A;
}

MatrixHandle
operator*(MatrixHandle A, double b)
{
  return b*A;
}

ColumnMatrix
operator*(double a, const ColumnMatrix& B)
{
  ColumnMatrix C(B);
  C.scalar_multiply(a);
  return C;
}

ColumnMatrix
operator*(const ColumnMatrix& A, double b)
{
  return b*A;
}
  
SparseRowMatrix
operator*(double a, const SparseRowMatrix& B)
{
  SparseRowMatrix C(B);
  C.scalar_multiply(a);
  return C;
}
  
SparseRowMatrix
operator*(const SparseRowMatrix& A, double b)
{
  return b*A;
}

  
/// @todo: does not check if invert operation was successful or not
MatrixHandle
inv(MatrixHandle A)
{
  ASSERT(A.get_rep());
  MatrixHandle Ainv(A->clone());
  Ainv->invert();
  return Ainv;
}

} // End namespace SCIRun
