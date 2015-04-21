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


/*
 *  sci_lapack.cc
 * 
 *  Written by:
 *   Author: Andrew Shafer
 *   Department of Computer Science
 *   University of Utah
 *   Date: Oct 21, 2003
 *
 */

#include <sci_defs/lapack_defs.h>

#include <sstream>
#include <math.h>
#include <stdlib.h>

#include <Core/Math/sci_lapack.h>
#include <Core/Util/Assert.h>

#include <Core/Exceptions/LapackError.h>
#include <Core/Exceptions/AssertionFailed.h>

// Functions to switch between Fortran and C style matrices

namespace SCIRun {

double *ctof(double **c, int rows, int cols)
{
  double *f;
  int i, j;

  f = new double[rows*cols];

  for (i=0; i<rows; i++)
  { 
    for(j=0; j<cols; j++)
    {
      f[i+(j*rows)] = c[i][j];
    }
  }
  return(f);
}


void ftoc(double *f, double **c, int rows, int cols)
{
  int i, j;
  for (i=0; i<rows; i++)
  {
    for (j=0; j<cols; j++)
    {
      c[i][j] = f[i+j*cols];
    }
  }
}

void sort_eigens(double *Er, double *Ei, int N, double **Evecs=0)
{
  double temp, *E2;
  int i, j, k;
  
  E2 = new double[N];
  for (i=0; i<N; i++) E2[i] = Er[i]*Er[i]+Ei[i]*Ei[i];
  
  for (j=0; j<N; j++) for (i=0; i<N-1; i++)
    if (fabs(E2[i])<fabs(E2[i+1])) {
      temp = E2[i]; E2[i] = E2[i+1]; E2[i+1] = temp;
      temp = Er[i]; Er[i] = Er[i+1]; Er[i+1] = temp;
      temp = Ei[i]; Ei[i] = Ei[i+1]; Ei[i+1] = temp;

      if (Evecs) {
        for (k=0; k<N; k++) {
          temp = Evecs[k][i];
          Evecs[k][i] = Evecs[k][i+1];
          Evecs[k][i+1] = temp;
        }
      }
    }

  delete E2;
}

#if defined(HAVE_LAPACK)

// LAPACK interfaces appear to be fairly consistent
// in their use of double (sometimes typedef'd to variations of doublereal),
// float (sometimes typedef'd to variations of real) and char

#if defined(APPLE)
#  include <vecLib.h>

  typedef __CLPK_integer    integer_type;

#elif defined (HAVE_SCIRUN_NETLIB)
#  include <f2c.h>
#  include <clapack.h>

  // applies to typedef'd f2c types translated from fortran
  // in Netlib 3.2.1, which is bundled with SCIRun
  typedef integer    integer_type;
  
#else
  /// @todo: Are there other lapack libraries that we are likely to use?
  // New netlib (v3.3 or greater) uses int as integer_type

  typedef int    integer_type;

extern "C" {
  int dgetrf_(integer_type *m, integer_type *n, double *a, integer_type *lda, integer_type *ipiv, integer_type *info);
  int dgetri_(integer_type *m, double *a, integer_type *lda, integer_type *ipiv, 
	      double *work, integer_type *lwork, integer_type *info);
  int dgesvd_(char *jobu, char *jobvt, integer_type *m, integer_type *n, double *a, integer_type *lda, 
	      double *S, double *u, integer_type *ldu, double *vt, integer_type *ldvt, 
	      double *work, integer_type *lwork, integer_type *info);
  int dgeev_(char *jobvl, char *jobvr, integer_type *n, double *a, integer_type *lda,
	     double *Er, double *Ei, double *vl, integer_type *ldvl, double *vr, 
	     integer_type *ldvr, double *work, integer_type *lwork, integer_type *info);     
  void dgesv_(int *,int *, double *, int *, int *, double *, int *, int *);  
}

#endif // APPLE or HAVE_SCIRUN_NETLIB

void lapackinvert(double *A, int n)
{
  // A is the matrix
  // n is the order of A (A is n*n)
  // P an int array to store the permutations

  // lda: the leading dimension of the matrix a.
  integer_type lda=n, lwork, info, N = n;

  integer_type *P = new integer_type[n];  //int array that stores permutations.
  
  /* DGETRF */
  /*  INFO    (output) INTEGER */
  /*          = 0:  successful exit */
  /*          < 0:  if INFO = -i, the i-th argument had an illegal value */
  /*          > 0:  if INFO = i, U(i,i) is exactly zero. The factorization */
  /*                has been completed, but the factor U is exactly */
  /*                singular, and division by zero will occur if it is used */
  /*                to solve a system of equations. */
  
  dgetrf_(&N, &N, A, &lda, P, &info);  

  if (info < 0)
  {
    delete [] P;
    std::stringstream oss;
    oss << "dgetrf failed: argument " << abs(info) << " has an illegal value";
    SCI_THROW(LapackError(oss.str(), __FILE__, __LINE__));
  }
  
  if (info > 0)
  {
    delete [] P;
    std::stringstream oss;
    oss << "dgetrf failed: U(" << info << "," << info
        << ") is exactly zero. The factorization has been completed, but the factor U is exactly singular, and division by zero will occur if it is used to solve a system of equations";
    SCI_THROW(LapackError(oss.str(), __FILE__, __LINE__));
  }

  /* DGETRI */
  /*  INFO    (output) INTEGER */
  /*          = 0:  successful exit */
  /*          < 0:  if INFO = -i, the i-th argument had an illegal value */
  /*          > 0:  if INFO = i, U(i,i) is exactly zero; the matrix is */
  /*                singular and its inverse could not be computed. */
  
  lwork = n*64;
  double* work = new double[lwork];
  
  lda = n;
  lwork = n;

  dgetri_(&N, A, &lda, P, work, &lwork, &info);
  
  delete [] work;
  delete [] P;

  if (info < 0)
  {
    std::stringstream oss;
    oss << "dgetri failed: argument " << abs(info) << " has an illegal value";
    SCI_THROW(LapackError(oss.str(), __FILE__, __LINE__));
  }
  
  if (info > 0)
  {
    std::stringstream oss;
    oss << "dgetrf failed: U(" << info << "," << info
        << ") is exactly zero. The matrix is singular and its inverse could not be computed";
    SCI_THROW(LapackError(oss.str(), __FILE__, __LINE__));
  }
}

void lapacksolvelinearsystem(double **a, int m, int n, double *rhs, int o, int p)
{
  
  // DGESV computes the solution to a real system of linear equations
  //   A * X = B,
  // where A is an N-by-N matrix and X and B are N-by-NRHS matrices.
  //
  // The LU decomposition with partial pivoting and row interchanges is
  // used to factor A as
  //   A = P * L * U,
  // where P is a permutation matrix, L is unit lower triangular, and U is
  // upper triangular.  The factored form of A is then used to solve the
  // system of equations A * X = B.
  //
  // DGESV Arguments:
  // N = Integer (number of linear equations) >= 0
  // NRHS = The number of right hand sides >= 0
  // A = DOUBLE PRECISION array, dimension (LDA,N)
  // LDA (input) INTEGER - The leading dimension of the array A.  LDA >= max(1,N).
  // IPIV (output) INTEGER array, dimension (N) - The pivot indices that define the permutation matrix P; row i of the matrix was interchanged with row IPIV(i).
  // B (input/output) DOUBLE PRECISION array, dimension (LDB,NRHS) On entry, the N-by-NRHS matrix of right hand side matrix B. On exit, if INFO = 0, the N-by-NRHS solution matrix X
  // LDB (input) Integer: The leading dimension of the array B. LDB >= max(1,N)
  // INFO (output) INTEGER, = 0:  successful exit, < 0:  if INFO = -i, the i-th argument had an illegal value, > 0:  if INFO = i, U(i,i)
  // is exactly zero. The factorization has been completed, but the factor U is exactly singular, so the solution could not be computed.

  double *A = 0;

  integer_type N=m, NRHS=p, LDA=m, LDB=o, INFO=0;
  integer_type* IPIV = new integer_type[m];
  A = ctof(a, m, n);
  
  dgesv_(&N, &NRHS,  A, &LDA, IPIV, rhs, &LDB, &INFO);
  
  delete [] A;
  delete [] IPIV;
  
  if (INFO < 0)
  {
    std::stringstream oss;
    oss << "dgesv failed: argument " << abs(INFO) << " has an illegal value";
    SCI_THROW(LapackError(oss.str(), __FILE__, __LINE__));
  }
  
  if (INFO > 0)
  {
    std::stringstream oss;
    oss << "dgesv failed: U(" << INFO << "," << INFO
        << ") is exactly zero. The factorization has been completed, but the factor U is exactly singular, so the solution could not be computed";
    SCI_THROW(LapackError(oss.str(), __FILE__, __LINE__));
  }
}

void lapacksvd(double **A, int m, int n, double *S, double **U, double **VT)
{
  char jobu, jobvt;
  integer_type lda, ldu, ldvt, lwork, info, M = m, N = n;
  double *a = 0, *u = 0, *vt = 0, *work = 0;

  int minmn, maxmn;

  jobu = 'A'; /* Specifies options for computing U.
		 A: all M columns of U are returned in array U;
		 S: the first min(m,n) columns of U (the left
		    singular vectors) are returned in the array U;
		 O: the first min(m,n) columns of U (the left
		    singular vectors) are overwritten on the array A;
		 N: no columns of U (no left singular vectors) are
		    computed. */

  if (U == 0) jobu = 'N';

  jobvt = 'A'; /* Specifies options for computing VT.
		  A: all N rows of V**T are returned in the array
		     VT;
		  S: the first min(m,n) rows of V**T (the right
		     singular vectors) are returned in the array VT;
		  O: the first min(m,n) rows of V**T (the right
		     singular vectors) are overwritten on the array A;
		  N: no rows of V**T (no right singular vectors) are
		     computed. */

  if (VT == 0) jobvt = 'N';

  lda = m; // The leading dimension of the matrix a.
  a = ctof(A, m, n); /* Convert the matrix A from double pointer
			  C form to single pointer Fortran form. */


  /* If A is not a square matrix, we have to make some decisions
     based on which dimension is shorter. */

  if (m>=n)
  {
    minmn = n; maxmn = m;
  }
  else
  {
    minmn = m; maxmn = n;
  }

  ldu = m; // Left singular vector matrix
  if (U) u = new double[ldu*m];

  ldvt = n; // Right singular vector matrix
  if (VT) vt = new double[ldvt*n];

  lwork = 5*maxmn; // Set up the work array, larger than needed.
  work = new double[lwork];

  dgesvd_(&jobu, &jobvt, &M, &N, a, &lda, S, u,
	  &ldu, vt, &ldvt, work, &lwork, &info);

  if (U) ftoc(u, U, ldu, m);
  if (VT) ftoc(vt, VT, ldvt, n);
  
  delete [] a;
  if (U) delete [] u;
  if (VT) delete [] vt;
  delete [] work;
  
  /* DGESVD */
  /*  INFO    (output) INTEGER */
  /*          = 0:  successful exit. */
  /*          < 0:  if INFO = -i, the i-th argument had an illegal value. */
  /*          > 0:  if DBDSQR did not converge, INFO specifies how many */
  /*                superdiagonals of an intermediate bidiagonal form B */
  /*                did not converge to zero. See the description of WORK */
  /*                above for details. */

  if (info < 0)
  {
    std::stringstream oss;
    oss << "dgesvd failed: argument " << abs(info) << " has an illegal value";
    SCI_THROW(LapackError(oss.str(), __FILE__, __LINE__));
  }
  
  if (info > 0)
  {
    std::stringstream oss;
    oss << "dgesv failed: DBDSQR did not converge, " << info
        << " specifies how many superdiagonals of an intermediate bidiagonal form B did not converge to zero. See the description of WORK in dgesvd's documentation for details";
    SCI_THROW(LapackError(oss.str(), __FILE__, __LINE__));
  }
}


void lapackeigen(double **H, int n, double *Er, double *Ei, double **Evecs)
{
  char jobvl, jobvr;
  integer_type lda = 0, ldvl = 0, ldvr = 0, lwork = 0, info = 0, N = n;
  double *a = 0, *vl = 0, *vr = 0, *work = 0;
  
  jobvl = 'N'; /* V/N to calculate/not calculate the left eigenvectors
		  of the matrix H.*/

  if (Evecs) 
    jobvr = 'V'; // As above, but for the right eigenvectors.
  else
    jobvr = 'N';

  lda = n; // The leading dimension of the matrix a.
  a = ctof(H, n, lda); /* Convert the matrix H from double pointer
				C form to single pointer Fortran form. */

  /* Whether we want them or not, we need to define the matrices
     for the eigenvectors, and give their leading dimensions.
     We also create a vector for work space. */

  ldvl = n;
  ldvr = n;
  if (Evecs)
    vr = new double[n*n];
  lwork = 4*n;

  work = new double[lwork];
  
  dgeev_(&jobvl, &jobvr, &N, a, &lda, Er, Ei, vl,
	 &ldvl, vr, &ldvr, work, &lwork, &info);

  if (Evecs)
  {
    ftoc(vr, Evecs, n, ldvr);
    sort_eigens(Er, Ei, n, Evecs); /* Sort the results by eigenvalue in
					 decreasing magnitude. */
  }
  else
  {
    sort_eigens(Er, Ei, n);
  }

  delete [] a;
  if (Evecs) delete [] vr;
  delete [] work;

  /* DGEEV */
  /*  INFO    (output) INTEGER */
  /*          = 0:  successful exit */
  /*          < 0:  if INFO = -i, the i-th argument had an illegal value. */
  /*          > 0:  if INFO = i, the QR algorithm failed to compute all the */
  /*                eigenvalues, and no eigenvectors have been computed; */
  /*                elements i+1:N of WR and WI contain eigenvalues which */
  /*                have converged. */
  
  
  if (info < 0)
  {
    std::stringstream oss;
    oss << "dgeev failed: argument " << abs(info) << " has an illegal value";
    SCI_THROW(LapackError(oss.str(), __FILE__, __LINE__));
  }
  
  if (info > 0)
  {
    std::stringstream oss;
    oss << "dgeev failed: the QR algorithm failed to compute all the eigenvalues, and no eigenvectors have been computed; elements "
       << info << "+1:N of WR and WI contain eigenvalues which have converged";
    SCI_THROW(LapackError(oss.str(), __FILE__, __LINE__));
  }
}

#else

// stubs for LAPACK wrappers
void lapackinvert(double *, int)
{
  ASSERTFAIL("Build was not configured with LAPACK");
}

void lapacksvd(double **, int, int, double *, double **, double **)
{
  ASSERTFAIL("Build was not configured with LAPACK");
}

void lapackeigen(double **, int, double *, double *, double **)
{
  ASSERTFAIL("Build was not configured with LAPACK");
}

#endif

} // End namespace SCIRun
