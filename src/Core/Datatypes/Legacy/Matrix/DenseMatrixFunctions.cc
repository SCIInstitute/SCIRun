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
/// @todo Documentation Core/Datatypes/Legacy/Matrix/DenseMatrixFunctions.cc

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Util/FancyAssert.h>

#include <stdexcept>

#if defined(HAVE_CBLAS)
#if defined(__APPLE__)
#include <vecLib/cblas.h>
#else
extern "C"{
#include <cblas.h>
}
#endif
#endif

namespace SCIRun {

SCISHARE void
Mult(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
#ifdef DEBUG
  if (m1.ncols() != m2.nrows())
    throw std::invalid_argument("dimension mismatch");
  if (out.nrows() != m1.nrows())
    throw std::invalid_argument("dimension mismatch");
  if (out.ncols() != m2.ncols())
    throw std::invalid_argument("dimension mismatch");
#endif
  ASSERTEQ(m1.ncols(), m2.nrows());
  ASSERTEQ(out.nrows(), m1.nrows());
  ASSERTEQ(out.ncols(), m2.ncols());
#if defined(HAVE_CBLAS)
  double ALPHA = 1.0;
  double BETA = 0.0;
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m1.nrows(),
              m2.ncols(), m1.ncols(), ALPHA, m1.dataptr_, m1.ncols(),
              m2.dataptr_, m2.ncols(), BETA,
              out.dataptr_, out.ncols());
#else
  size_type nr = out.nrows();
  size_type nc = out.ncols();
  size_type ndot=m1.ncols();
  for (index_type i=0;i<nr;i++)
  {
    const double* row = m1[i];
    for (index_type j=0;j<nc;j++)
    {
      double d = 0.0;
      for (index_type k=0;k<ndot;k++)
      {
        d += row[k] * m2[k][j];
      }
      out[i][j] = d;
    }
  }
#endif
}


void
Add(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  ASSERTEQ(m1.ncols(), m2.ncols());
  ASSERTEQ(out.ncols(), m2.ncols());
  ASSERTEQ(m1.nrows(), m2.nrows());
  ASSERTEQ(out.nrows(), m2.nrows());

  size_type nr=out.nrows();
  size_type nc=out.ncols();

  size_type m8 = (nr*nc)/8;
  size_type m = (nr*nc)-m8*8;
  
  double *p0 = out.get_data_pointer();
  double *p1 = m1.get_data_pointer();
  double *p2 = m2.get_data_pointer();
  
  index_type i;
  for (i=0;i<m8;i++)
  {
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
  }

  for (i=0;i<m;i++)
  {
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;  
  }
}


void
Sub(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  ASSERTEQ(m1.ncols(), m2.ncols());
  ASSERTEQ(out.ncols(), m2.ncols());
  ASSERTEQ(m1.nrows(), m2.nrows());
  ASSERTEQ(out.nrows(), m2.nrows());

  size_type nr=out.nrows();
  size_type nc=out.ncols();

  size_type m8 = (nr*nc)/8;
  size_type m = (nr*nc)-m8*8;
  
  double *p0 = out.get_data_pointer();
  double *p1 = m1.get_data_pointer();
  double *p2 = m2.get_data_pointer();
  
  index_type i;
  for (i=0;i<m8;i++)
  {
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
  }

  for (i=0;i<m;i++)
  {
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;  
  }

}


void
Add(DenseMatrix& out, double f1, const DenseMatrix& m1,
    double f2, const DenseMatrix& m2)
{
  ASSERTEQ(m1.ncols(), m2.ncols());
  ASSERTEQ(out.ncols(), m2.ncols());
  ASSERTEQ(m1.nrows(), m2.nrows());
  ASSERTEQ(out.nrows(), m2.nrows());

  size_type nr=out.nrows();
  size_type nc=out.ncols();

  size_type m8 = (nr*nc)/8;
  size_type m = (nr*nc)-m8*8;
  
  double *p0 = out.get_data_pointer();
  double *p1 = m1.get_data_pointer();
  double *p2 = m2.get_data_pointer();
  
  index_type i;
  for (i=0;i<m8;i++)
  {
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
  }

  for (i=0;i<m;i++)
  {
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;  
  }
}


/// @todo: This code does not look OK
void
Add(double f1, DenseMatrix& out, const DenseMatrix& m1)
{
  ASSERTEQ(out.ncols(), m1.ncols());
  ASSERTEQ(out.nrows(), m1.nrows());
  
  size_type nr=out.nrows();
  size_type nc=out.ncols();

  size_type m8 = (nr*nc)/8;
  size_type m = (nr*nc)-m8*8;
  
  double *p0 = out.get_data_pointer();
  double *p1 = m1.get_data_pointer();
  
  index_type i;
  for (i=0;i<m8;i++)
  {
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
  }

  for (i=0;i<m;i++)
  {
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
  }
}
  

void
Mult_trans_X(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  ASSERTEQ(m1.nrows(), m2.nrows());
  ASSERTEQ(out.nrows(), m1.ncols());
  ASSERTEQ(out.ncols(), m2.ncols());
  size_type nr=out.nrows();
  size_type nc=out.ncols();
  size_type ndot=m1.nrows();
  for (index_type i=0;i<nr;i++)
  {
    for (index_type j=0;j<nc;j++)
    {
      double d = 0.0;
      for (index_type k=0;k<ndot;k++)
      {
        d += m1[k][i] * m2[k][j];
      }
      out[i][j] = d;
    }
  }
}


void
Mult_X_trans(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  ASSERTEQ(m1.ncols(), m2.ncols());
  ASSERTEQ(out.nrows(), m1.nrows());
  ASSERTEQ(out.ncols(), m2.nrows());

  size_type nr=out.nrows();
  size_type nc=out.ncols();
  size_type ndot=m1.ncols();

  for (index_type i=0;i<nr;i++)
  {
    const double* row = m1[i];
    for (index_type j=0;j<nc;j++)
    {
      double d = 0.0;
      for (index_type k=0; k<ndot; k++)
      {
        d += row[k] * m2[j][k];
      }
      out[i][j]=d;
    }
  }
}

// Added by Saeed Babaeizadeh, Jan. 2006
void 
Concat_rows(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  index_type r, c;
  ASSERTEQ(m1.ncols(), m2.ncols());
  for (r = 0; r <= m1.nrows()-1; r++) 
  {
    for (c = 0; c <= m1.ncols()-1; c++) 
    {
      out[r][c] = m1[r][c];
    }
  }
  for (r = m1.nrows(); r <= m1.nrows()+m2.nrows()-1; r++) 
  {
    for (c = 0; c <= m2.ncols()-1; c++) 
    {
      out[r][c] = m2[r-m1.nrows()][c];
    }
  }
  return;
}

// Added by Saeed Babaeizadeh, Jan. 2006
void 
Concat_cols(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  index_type r, c;
  ASSERTEQ(m1.nrows(), m2.nrows());
  for (r = 0; r <= m1.nrows()-1; r++) 
  {
    for (c = 0; c <= m1.ncols()-1; c++) 
    {
      out[r][c] = m1[r][c];
    }
  }
  for (r = 0; r <= m2.nrows()-1; r++) 
  {
    for (c = m1.ncols(); c <= m1.ncols()+m2.ncols()-1; c++) 
    {
      out[r][c] = m2[r][c-m1.ncols()];
    }
  }
  return;
}

} // End namespace SCIRun
