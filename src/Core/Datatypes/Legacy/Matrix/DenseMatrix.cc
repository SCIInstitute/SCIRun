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
/// @todo Documentation Core/Datatypes/Legacy/Matrix/DenseMatrix.cc

#include <sci_defs/lapack_defs.h>
#include <sci_defs/blas_defs.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <cstdio>

#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <vector>

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>

#include <Core/Exceptions/DimensionMismatch.h>

#include <boost/shared_array.hpp>

#include <Core/Math/MiscMath.h>

namespace SCIRun {

// NOTE: returns 1 if successful, or 0 if unsuccessful (i.e. ignore the solution vector)
int
LinearAlgebra::solve(const DenseMatrix& matrix, ColumnMatrix& sol)
{
  ColumnMatrix b(sol);
  return solve(matrix, b, sol);
}


// NOTE: returns 1 if successful, or 0 if unsuccessful (i.e. ignore the solution vector)
int
LinearAlgebra::solve(const DenseMatrix& matrix, const ColumnMatrix& rhs, ColumnMatrix& lhs)
{
  ASSERT(matrix.nrows() == matrix.ncols());
  ASSERT(rhs.nrows() == matrix.ncols());
  lhs=rhs;

  double **A;
  double **cpy = 0;
  double *lhsp = lhs.get_data_pointer();
	
  cpy = new double*[matrix.nrows()]; 

  for (index_type j=0; j < matrix.nrows(); j++)
    cpy[j] = matrix.get_raw_2D_pointer()[j];

  A = cpy;

  // Gauss-Jordan with partial pivoting
  index_type i;
  for (i=0; i < matrix.nrows(); i++)
  {
    double max=Abs(A[i][i]);
    index_type row=i;
    index_type j;
    for (j=i+1; j < matrix.nrows(); j++)
    {
      if(Abs(A[j][i]) > max)
      {
        max=Abs(A[j][i]);
        row=j;
      }
    }
    //  ASSERT(Abs(max) > 1.e-12);
    if (Abs(max) < 1.e-12)
    {
      lhs=rhs;
      delete cpy;
      return 0;
    }
    if(row != i)
    {
      // Switch rows (actually their pointers)
      std::swap(A[i], A[row]);
      std::swap(lhsp[i], lhsp[row]);
    }
    double denom=1./A[i][i];
    double* r1=A[i];
    double s1=lhsp[i];
    for (j=i+1; j<matrix.nrows(); j++)
    {
      double factor=A[j][i]*denom;
      double* r2=A[j];
      for (int k=i; k<matrix.nrows(); k++)
        r2[k]-=factor*r1[k];
      lhsp[j]-=factor*s1;
    }
  }

  // Back-substitution
  for (i=1; i<matrix.nrows(); i++)
  {
    //  cout << "Solve: " << i << " of " << nr << endl;
    //  ASSERT(Abs(A[i][i]) > 1.e-12);
    if (Abs(A[i][i]) < 1.e-12)
    {
      lhs=rhs;
      delete cpy;
      return 0;
    }
    double denom=1./A[i][i];
    double* r1=A[i];
    double s1=lhsp[i];
    for (index_type j=0;j<i;j++)
    {
      double factor=A[j][i]*denom;
      double* r2=A[j];
      for (index_type k=i; k<matrix.nrows(); k++)
        r2[k] -= factor*r1[k];
      lhsp[j] -= factor*s1;
    }
  }

  // Normalize
  for (i=0; i<matrix.nrows(); i++)
  {
    //  cout << "Solve: " << i << " of " << nr << endl;
    //  ASSERT(Abs(A[i][i]) > 1.e-12);
    if (Abs(A[i][i]) < 1.e-12)
    {
      lhs=rhs;
      delete cpy;
      return 0;
    }
    double factor=1./A[i][i];
    for (index_type j=0; j<matrix.nrows(); j++)
      A[i][j] *= factor;
    lhsp[i] *= factor;
  }
  delete cpy;
  return 1;
}




#if defined(HAVE_LAPACK)

void
LinearAlgebra::solve_lapack(const DenseMatrix& matrix, const ColumnMatrix& rhs, ColumnMatrix& lhs)
{
  if (matrix.nrows() != matrix.ncols())
  {
    SCI_THROW(DimensionMismatch(matrix.nrows(), matrix.ncols(), __FILE__, __LINE__));
  }

  if (rhs.nrows() != matrix.ncols())
  {
    SCI_THROW(DimensionMismatch(rhs.nrows(), matrix.ncols(), __FILE__, __LINE__));
  }
  
  ColumnMatrix rhsCopy(rhs); //need to make a copy because 
  
  try
  {
    lapacksolvelinearsystem(matrix.get_raw_2D_pointer(), matrix.nrows(), matrix.ncols(), rhsCopy.get_data_pointer(), rhs.nrows(), rhs.ncols());
  }
  catch (const SCIRun::LapackError& exception)
  {
    std::ostringstream oss;
    oss << "Caught LapackError exception: " << exception.message();
    // in the absence of a logging service
    std::cerr << oss.str() << std::endl;
    throw;
  }
  
  lhs=rhsCopy;
  
}
  
void
LinearAlgebra::svd(const DenseMatrix& matrix, DenseMatrix& U, SparseRowMatrixHandle& S, DenseMatrix& VT)
{
  ASSERTEQ(U.ncols(), U.nrows());
  ASSERTEQ(VT.ncols(), VT.nrows());
  ASSERTEQ(U.nrows(), matrix.nrows());
  ASSERTEQ(VT.ncols(), matrix.ncols());

  /*
   * LAPACK function dgesvd argument S is a DOUBLE PRECISION array
   * with dimension (min(M,N)).
   */
  const size_type SIGMA_LEN = std::min(matrix.nrows(), matrix.ncols());
  boost::shared_array<double> sigma(new double[SIGMA_LEN]);

  try
  {
    lapacksvd(matrix.get_raw_2D_pointer(), matrix.nrows(), matrix.ncols(), sigma.get(), U.get_raw_2D_pointer(), VT.get_raw_2D_pointer());
  }
  catch (const SCIRun::LapackError& exception)
  {
    std::ostringstream oss;
    oss << "Caught LapackError exception: " << exception.message();
    // in the absence of a logging service
    std::cerr << oss.str() << std::endl;
    throw;
  }

  size_type nnz = 0;
  for (size_type i = 0; i < SIGMA_LEN; ++i)
  {
    if ( fabs(sigma[i]) >= std::numeric_limits<double>::epsilon() )
    {
      ++nnz;
    }
  }

  SparseRowMatrix::Data sparseData(matrix.nrows() + 1, nnz);
  const SparseRowMatrix::Rows& rows = sparseData.rows();
  const SparseRowMatrix::Columns& columns = sparseData.columns();
  const SparseRowMatrix::Storage& a = sparseData.data();
  if (!sparseData.allocated())
  {
    std::cerr << "Could not allocate memory for sparse matrix buffers "
    << __FILE__ << ": " << __LINE__ << std::endl;

    return;
  }

  // singular values on diagonal
  index_type count = 0;
  index_type i = 0;
  const index_type NROWS = matrix.nrows();
  for (index_type r = 0; r < NROWS; r++)
  {
    rows[r] = count;
    if ( r < matrix.ncols() && i < SIGMA_LEN && fabs(sigma[i]) >= std::numeric_limits<double>::epsilon() )
    {
      columns[count] = r;
      a[count] = sigma[i];
      ++count;
      ++i;
    }
  }
  rows[matrix.nrows()] = count;
  
  S = new SparseRowMatrix(matrix.nrows(), matrix.ncols(), sparseData, nnz);
}

void
LinearAlgebra::svd(const DenseMatrix& matrix, DenseMatrix& U, DenseMatrix& S, DenseMatrix& VT)
{
  ASSERTEQ(U.ncols(), U.nrows());
  ASSERTEQ(VT.ncols(), VT.nrows());
  ASSERTEQ(U.nrows(), matrix.nrows());
  ASSERTEQ(VT.ncols(), matrix.ncols());
  ASSERTEQ(S.nrows(), matrix.nrows());
  ASSERTEQ(S.ncols(), matrix.ncols());

  /*
   * LAPACK function dgesvd argument S is a DOUBLE PRECISION array
   * with dimension (min(M,N)).
   */
  const size_type SIGMA_LEN = std::min(matrix.nrows(), matrix.ncols());
  boost::shared_array<double> sigma(new double[SIGMA_LEN]);

  try
  {
    lapacksvd(matrix.get_raw_2D_pointer(), matrix.nrows(), matrix.ncols(), sigma.get(), U.get_raw_2D_pointer(), VT.get_raw_2D_pointer());
  }
  catch (const SCIRun::LapackError& exception)
  {
    std::ostringstream oss;
    oss << "Caught LapackError exception: " << exception.message();
    // in the absence of a logging service
    std::cerr << oss.str() << std::endl;
    throw;
  }

  // Put singular values on diagonal.
  for (size_type i = 0; i < SIGMA_LEN; ++i)
    S.put(i, i, sigma[i]);
}

void
LinearAlgebra::svd(const DenseMatrix& matrix, DenseMatrix& U, ColumnMatrix& S, DenseMatrix& VT)
{
  // Check whether matrices are square
  if (U.ncols() != U.nrows())
  {
    SCI_THROW(DimensionMismatch(U.ncols(), U.nrows(), __FILE__, __LINE__));
  }
  
  if (VT.ncols() != VT.nrows())
  {
    SCI_THROW(DimensionMismatch(VT.ncols(), VT.nrows(), __FILE__, __LINE__));
  }
  
  if (U.nrows())
  {
    if (U.nrows() != matrix.nrows())
    {
      SCI_THROW(DimensionMismatch(U.nrows(), matrix.nrows(), __FILE__, __LINE__));
    }    
  }
  if (VT.ncols())
  {
    if (VT.ncols() != matrix.ncols())
    {
      SCI_THROW(DimensionMismatch(VT.ncols(), matrix.ncols(), __FILE__, __LINE__));
    }
  }
  
  if (matrix.nrows() < matrix.ncols())
  {
    if (S.nrows() != matrix.nrows())
    {
      SCI_THROW(DimensionMismatch(S.nrows(), matrix.nrows(), __FILE__, __LINE__));
    }
  }
  else
  {
    if (S.nrows() != matrix.ncols())
    {
      SCI_THROW(DimensionMismatch(S.nrows(), matrix.ncols(), __FILE__, __LINE__));
    }
  }

  try
  {
    if (U.nrows() == 0 && VT.nrows() == 0)
      lapacksvd(matrix.get_raw_2D_pointer(), matrix.nrows(), matrix.ncols(), S.get_data_pointer(), 0, 0);
    else if (U.nrows() != 0 && VT.nrows() == 0)
      lapacksvd(matrix.get_raw_2D_pointer(), matrix.nrows(), matrix.ncols(), S.get_data_pointer(), U.get_raw_2D_pointer(), 0);
    else if (U.nrows() == 0 && VT.nrows() != 0)
      lapacksvd(matrix.get_raw_2D_pointer(), matrix.nrows(), matrix.ncols(), S.get_data_pointer(), 0, VT.get_raw_2D_pointer());
    else  
      lapacksvd(matrix.get_raw_2D_pointer(), matrix.nrows(), matrix.ncols(), S.get_data_pointer(), U.get_raw_2D_pointer(), VT.get_raw_2D_pointer());
  }
  catch (const SCIRun::LapackError& exception)
  {
    std::ostringstream oss;
    oss << "Caught LapackError exception: " << exception.message();
    // in the absence of a logging service
    std::cerr << oss.str() << std::endl;
    throw;
  }
}

void
LinearAlgebra::eigenvalues(const DenseMatrix& matrix, ColumnMatrix& R, ColumnMatrix& I)
{
  ASSERTEQ(matrix.ncols(), matrix.nrows());
  ASSERTEQ(R.nrows(), I.nrows());
  ASSERTEQ(matrix.ncols(), R.nrows());

  boost::shared_array<double> Er(new double[matrix.nrows()]);
  boost::shared_array<double> Ei(new double[matrix.nrows()]);

  try
  {
    lapackeigen(matrix.get_raw_2D_pointer(), matrix.nrows(), Er.get(), Ei.get());
  }
  catch (const SCIRun::LapackError& exception)
  {
    std::ostringstream oss;
    oss << "Caught LapackError exception: " << exception.message();
    // in the absence of a logging service
    std::cerr << oss.str() << std::endl;
    throw;
  }

  for (index_type i = 0; i < matrix.nrows(); i++)
  {
    R[i] = Er[i];
    I[i] = Ei[i];
  }
}

void
LinearAlgebra::eigenvectors(const DenseMatrix& matrix, ColumnMatrix& R, ColumnMatrix& I, DenseMatrix& Vecs)
{
  ASSERTEQ(matrix.ncols(), matrix.nrows());
  ASSERTEQ(R.nrows(), I.nrows());
  ASSERTEQ(matrix.ncols(), R.nrows());

  boost::shared_array<double> Er(new double[matrix.nrows()]);
  boost::shared_array<double> Ei(new double[matrix.nrows()]);

  try
  {
    lapackeigen(matrix.get_raw_2D_pointer(), matrix.nrows(), Er.get(), Ei.get(), Vecs.get_raw_2D_pointer());
  }
  catch (const SCIRun::LapackError& exception)
  {
    std::ostringstream oss;
    oss << "Caught LapackError exception: " << exception.message();
    // in the absence of a logging service
    std::cerr << oss.str() << std::endl;
    throw;
  }


  for (index_type i = 0; i<matrix.nrows(); i++)
  {
    R[i] = Er[i];
    I[i] = Ei[i];
  }
}


#else

void
LinearAlgebra::solve_lapack(const DenseMatrix& matrix, const ColumnMatrix& rhs, ColumnMatrix& lhs)
{
  ASSERTFAIL("Build was not configured with LAPACK. LinearAlgebra::solve_lapack is not available.");
}

void
LinearAlgebra::svd(const DenseMatrix& matrix, DenseMatrix& U, SparseRowMatrixHandle& S, DenseMatrix& VT)
{
  ASSERTFAIL("Build was not configured with LAPACK LinearAlgebra::svd is not available.");
}

void
LinearAlgebra::svd(const DenseMatrix& matrix, DenseMatrix& U, ColumnMatrix& S, DenseMatrix& VT)
{
  ASSERTFAIL("Build was not configured with LAPACK LinearAlgebra::svd is not available.");
}
	
void
LinearAlgebra::svd(const DenseMatrix& matrix, DenseMatrix& U, DenseMatrix& S, DenseMatrix& VT)
{
  ASSERTFAIL("Build was not configured with LAPACK LinearAlgebra::svd is not available.");
}
	
void
LinearAlgebra::eigenvalues(const DenseMatrix& matrix, ColumnMatrix& R, ColumnMatrix& I)
{
  ASSERTFAIL("Build was not configured with LAPACK LinearAlgebra::svd is not available.");
}

void
LinearAlgebra::eigenvectors(const DenseMatrix& matrix, ColumnMatrix& R, ColumnMatrix& I, DenseMatrix& Vecs)
{
  ASSERTFAIL("Build was not configured with LAPACK LinearAlgebra::svd is not available.");
}
	
#endif

} // End namespace SCIRun
