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


///
///@class Mat
///@brief Simple matrix calculations
///
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///
///@date  October 1994
///

#include <Core/Math/Mat.h>
#include <stdio.h>

#define Abs(x) ((x)<0?-(x):(x))
#define Max(x,y) ((x)<(y)?(y):(x))
#define SWITCH_ROWS(m, r1, r2) \
    for(k=0;k<3;k++){ \
        double tmp=m[r1][k]; \
        m[r1][k]=m[r2][k]; \
        m[r2][k]=tmp; \
    }

#define SUB_ROWS(m, r1, r2, mul) \
    for(k=0;k<3;k++){ \
        m[r1][k] -= m[r2][k]*mul; \
    }

void min_norm_least_sq_3(double *A[3], double *b, double *x, double *bprime, int size) {
  int i,j;
  double AtA[3][3];
  double Ab[3];
  for (i=0; i<3; i++) {
    for (j=0; j<3; j++)
      AtA[i][j]=0;
    Ab[i]=0;
  }
  for (i=0; i<size; i++) {
    AtA[0][0] += A[0][i]*A[0][i];
    AtA[0][1] += A[0][i]*A[1][i];
    AtA[0][2] += A[0][i]*A[2][i];
    AtA[1][1] += A[1][i]*A[1][i];
    AtA[1][2] += A[1][i]*A[2][i];
    AtA[2][2] += A[2][i]*A[2][i];
  }
  AtA[1][0] = AtA[0][1];
  AtA[2][0] = AtA[0][2];
  AtA[2][1] = AtA[1][2];
  for (i=0; i<size; i++)
    for (j=0; j<3; j++)
      Ab[j] += A[j][i]*b[i];

  matsolve3by3(AtA, Ab);

  for (i=0; i<3; i++) x[i]=Ab[i];

  for (i=0; i<size; i++) {
    bprime[i]=0;
    for (j=0; j<3; j++)
      bprime[i] += x[j]*A[j][i];
  }
}

void matsolve3by3(double mat[3][3], double rhs[3])
{
    double B[3];
    double t7 = mat[2][1]*mat[0][2];
    double t20 = mat[2][1]*mat[0][0];
    double t23 = mat[0][1]*mat[2][0];
    double t26 = 1/(mat[2][2]*mat[0][0]*mat[1][1]-mat[2][2]*mat[1][0]*mat[0][1]-mat[2][0]*mat[0][2]*mat[1][1]-t20*mat[1][2]+t7*mat[1][0]+t23*mat[1][2]);
    double t34 = rhs[0]*mat[2][0];
    B[0] = (mat[0][1]*mat[1][2]*rhs[2]-mat[0][1]*rhs[1]*mat[2][2]-mat[0][2]*rhs[2]*mat[1][1]+t7*rhs[1]+rhs[0]*mat[2][2]*mat[1][1]-rhs[0]*mat[2][1]*mat[1][2])*t26;
    B[1] = -(mat[0][0]*mat[1][2]*rhs[2]-mat[0][0]*rhs[1]*mat[2][2]+rhs[1]*mat[2][0]*mat[0][2]-t34*mat[1][2]-mat[0][2]*mat[1][0]*rhs[2]+mat[1][0]*rhs[0]*mat[2][2])*t26;
    B[2] = (rhs[2]*mat[0][0]*mat[1][1]-rhs[2]*mat[1][0]*mat[0][1]-t34*mat[1][1]-t20*rhs[1]+mat[2][1]*mat[1][0]*rhs[0]+t23*rhs[1])*t26;
    rhs[0]=B[0];
    rhs[1]=B[1];
    rhs[2]=B[2];
}

void matsolve3by3_cond(double mat[3][3], double rhs[3], double* rcond)
{
    double imat[3][3];
    double norm=0;
    int i, j, k;
    double denom;
    double inorm;
    double lhs[3];
    double factor;

    for(i=0;i<3;i++){
	double sum=0;
	for(j=0;j<3;j++)
	    sum+=Abs(mat[i][j]);
	norm=Max(norm, sum);
    }

    for(i=0;i<3;i++){
        for(j=0;j<3;j++){
            imat[i][j]=0.0;
        }
        imat[i][i]=1.0;
    }

    /* Gauss-Jordan with partial pivoting */
    for(i=0;i<3;i++){
        double max=Abs(mat[i][i]);
        int row=i;
        for(j=i+i;j<3;j++){
            if(Abs(mat[j][i]) > max){
                max=Abs(mat[j][i]);
                row=j;
            }
        }
	if(max==0){
	    rcond=0;
	    return;
	}
        if(row!=i){
	    SWITCH_ROWS(mat, i, row);
	    SWITCH_ROWS(imat, i, row);
        }
        denom=1./mat[i][i];
        for(j=i+1;j<3;j++){
            double factor=mat[j][i]*denom;
            SUB_ROWS(mat, j, i, factor);
            SUB_ROWS(imat, j, i, factor);
        }
    }

    /* Jordan */
    for(i=1;i<3;i++){
	if(mat[i][i]==0){
	    rcond=0;
	    return;
	}
        denom=1./mat[i][i];
        for(j=0;j<i;j++){
            double factor=mat[j][i]*denom;
            SUB_ROWS(mat, j, i, factor);
            SUB_ROWS(imat, j, i, factor);
        }
    }

    /* Normalize */
    for(i=0;i<3;i++){
	if(mat[i][i]==0){
	    rcond=0;
	    return;
	}
        factor=1./mat[i][i];
        for(j=0;j<3;j++){
            imat[i][j] *= factor;
	}
    }

    inorm=0;
    for(i=0;i<3;i++){
	double sum=0;
	for(j=0;j<3;j++)
	    sum+=Abs(imat[i][j]);
	inorm=Max(inorm, sum);
    }
    *rcond=1./(norm*inorm);

    /* Compute the solution... */
    for(i=0;i<3;i++){
	lhs[i]=0;
	for(j=0;j<3;j++){
	    lhs[i]+=imat[i][j]*rhs[j];
	}
    }
    for(i=0;i<3;i++){
	rhs[i]=lhs[i];
    }
}
