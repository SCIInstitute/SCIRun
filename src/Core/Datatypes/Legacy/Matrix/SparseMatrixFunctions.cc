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
///@file  SparseMatrixFunctions.cc 
/// 
///@author
///       Department of Computer Science
///       University of Utah
/// 

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseMatrixFunctions.h>

namespace SCIRun {

void
SparseMatrixFunctions::sparse_mult(const SparseRowMatrix& matrix, const DenseMatrix& x, DenseMatrix& b)
{
  // Compute A*x=b
  ASSERT(x.nrows() == matrix.ncols());
  ASSERT(b.nrows() == matrix.nrows());
  ASSERT(x.ncols() == b.ncols());
  index_type i, j, k;

  for (j = 0; j < b.ncols(); j++)
  {
    for (i = 0; i < b.nrows(); i++)
    {
      double sum = 0.0;
      for (k = matrix.get_row(i); k < matrix.get_row(i+1); k++)
      {
        sum += matrix.get_value(k) * x[matrix.get_col(k)][j];
      }
      b.put(i, j, sum);
    }
  }
}

void
SparseMatrixFunctions::sparse_mult_transXB(const SparseRowMatrix& matrix, const DenseMatrix& x,
                                     DenseMatrix& b)
{
  // Compute A*xT=bT
  ASSERT(x.ncols() == matrix.ncols());
  ASSERT(b.ncols() == matrix.nrows());
  ASSERT(x.nrows() == b.nrows());
  index_type i, j, k;

  for (j = 0; j < b.nrows(); j++)
  {
    for (i = 0; i < b.ncols(); i++)
    {
      double sum = 0.0;
      for (k = matrix.get_row(i); k < matrix.get_row(i+1); k++)
      {
       sum += matrix.get_value(k) * x.get(j, matrix.get_col(k));
      }
      b.put(j, i, sum);
    }
  }
}

struct SparseSparseElement {
  index_type     row; 
  index_type     col;
  double         val;
};

inline bool operator<(const SparseSparseElement& s1, const SparseSparseElement& s2)
{
  if (s1.row < s2.row) return(true);
  if ((s1.row == s2.row)&&(s1.col < s2.col)) return(true);
  return (false);
}

inline bool operator==(const SparseSparseElement& s1, const SparseSparseElement& s2)
{
  if ((s1.row == s2.row)&&(s1.col == s2.col)) return(true);
  return (false);
}

MatrixHandle
SparseMatrixFunctions::sparse_sparse_mult(const SparseRowMatrix& matrix, const SparseRowMatrix &b)
{
  // Compute A*B=C
  ASSERT(b.nrows() == matrix.ncols());
  
  size_type bncols = b.ncols();
  const index_type *brows = b.get_rows();
  const index_type *bcolumns = b.get_cols();
  const double* ba = b.get_vals();  

  if (brows==0 || bcolumns == 0 || ba == 0 || bncols == 0)
  {
    std::cerr << "Encountered an invalid sparse matrix (B)" << std::endl;
    return 0;
  }
  
  if (matrix.get_rows() == 0 || matrix.get_cols() == 0 || matrix.get_vals() == 0)
  {
    std::cerr << "Encountered an invalid sparse matrix (A)" << std::endl;
    return 0;
  }
  
  index_type k = 0;

  for (index_type r =0; r < matrix.nrows(); r++)
  {
    index_type ps = matrix.get_row(r);
    index_type pe = matrix.get_row(r+1);   
    for (index_type p = ps; p < pe; p++)
    {
      index_type s = matrix.get_col(p);
      index_type qs = brows[s];
      index_type qe = brows[s+1];
      k += qe-qs;
    }
  }

  std::vector<SparseSparseElement> elems(k);
  
  k = 0;
  for (index_type r =0; r < matrix.nrows(); r++)
  {
    index_type ps = matrix.get_row(r);
    index_type pe = matrix.get_row(r+1);   
    for (index_type p = ps; p < pe; p++)
    {
      index_type s = matrix.get_col(p);
      double v = matrix.get_value(p);
      index_type qs = brows[s];
      index_type qe = brows[s+1];
      for (index_type q=qs; q<qe; q++)
      {
        if (v*ba[q])
        {
          elems[k].row = r;
          elems[k].col = bcolumns[q];
          elems[k].val = v*ba[q];
          k++;
        }
      }
    }
  }
          
  std::sort(elems.begin(),elems.begin()+k);
  
  index_type s = 0;
  size_type nnz = 0;
  size_type nelems = static_cast<size_type>(k);
  while (s< nelems && elems[s].val == 0.0) s++;
  
  for (index_type r=1; r< nelems; r++)
  {
    if (elems[s] == elems[r])
    {
      elems[s].val += elems[r].val;
      elems[r].val = 0.0;
    }
    else
    {
      if (elems[r].val)
      {
        nnz++;
        s = r;
      }
    }
  }
  if (nelems && elems[s].val) nnz++;
  
  SparseRowMatrix::Data data(matrix.nrows() + 1, nnz);
  
  if (!data.allocated())
  {
    return MatrixHandle();
  }

  const SparseRowMatrix::Rows& rr = data.rows();
  const SparseRowMatrix::Columns& cc = data.columns();
  const SparseRowMatrix::Storage& vv = data.data();
  rr[0] = 0;
  index_type q = 0;
  size_type kk = 0;
  for( index_type p=0; p < matrix.nrows(); p++ )
  {
    while ((kk < nelems) && (elems[kk].row == p))
    {
      if (elems[kk].val)
      {
        cc[q] = elems[kk].col; 
        vv[q] = elems[kk].val; 
        q++;
      } 
      kk++; 
    }
    rr[p+1] = q;
  }   
    
  MatrixHandle output = new SparseRowMatrix(matrix.nrows(), bncols, data, nnz);
  return output;
}

SparseRowMatrix *
SparseMatrixFunctions::AddSparse(const SparseRowMatrix &a, const SparseRowMatrix &b)
{
  ASSERT(a.nrows() == b.nrows() && a.ncols() == b.ncols());

  std::vector<index_type> rows(a.nrows() + 1);
  std::vector<index_type> cols;
  std::vector<double> vals;

  index_type r, ca, cb;

  rows[0] = 0;
  for (r = 0; r < a.nrows(); r++)
  {
    rows[r+1] = rows[r];
    ca = a.get_row(r);
    cb = b.get_row(r);
    for (;;)
    {
      if (ca >= a.get_row(r+1) && cb >= b.get_row(r+1))
      {
        break;
      }
      else if (ca >= a.get_row(r+1))
      {
        cols.push_back(b.get_col(cb));
        vals.push_back(b.get_value(cb));
        rows[r+1]++;
        cb++;
      }
      else if (cb >= b.get_row(r+1))
      {
        cols.push_back(a.get_col(ca));
        vals.push_back(a.get_value(ca));
        rows[r+1]++;
        ca++;
      }
      else if (a.get_col(ca) < b.get_col(cb))
      {
        cols.push_back(a.get_col(ca));
        vals.push_back(a.get_value(ca));
        rows[r+1]++;
        ca++;
      }
      else if (a.get_col(ca) > b.get_col(cb))
      {
        cols.push_back(b.get_col(cb));
        vals.push_back(b.get_value(cb));
        rows[r+1]++;
        cb++;
      }
      else
      {
        cols.push_back(a.get_col(ca));
        vals.push_back(a.get_value(ca) + b.get_value(cb));
        rows[r+1]++;
        ca++;
        cb++;
      }
    }
  }

  SparseRowMatrix::Data data(rows, cols, vals);

  return new SparseRowMatrix(a.nrows(), a.ncols(), data, static_cast<size_type>(vals.size()));
}

SparseRowMatrix *
SparseMatrixFunctions::SubSparse(const SparseRowMatrix &a, const SparseRowMatrix &b)
{
  ASSERT(a.nrows() == b.nrows() && a.ncols() == b.ncols());

  std::vector<index_type> rows(a.nrows() + 1);
  std::vector<index_type> cols;
  std::vector<double> vals;

  index_type r, ca, cb;

  rows[0] = 0;
  for (r = 0; r < a.nrows(); r++)
  {
    rows[r+1] = rows[r];
    ca = a.get_row(r);
    cb = b.get_row(r);
    for( ;; )
    {
      if (ca >= a.get_row(r+1) && cb >= b.get_row(r+1))
      {
        break;
      }
      else if (ca >= a.get_row(r+1))
      {
        cols.push_back(b.get_col(cb));
        vals.push_back(-b.get_value(cb));
        rows[r+1]++;
        cb++;
      }
      else if (cb >= b.get_row(r+1))
      {
        cols.push_back(a.get_col(ca));
        vals.push_back(a.get_value(ca));
        rows[r+1]++;
        ca++;
      }
      else if (a.get_col(ca) < b.get_col(cb))
      {
        cols.push_back(a.get_col(ca));
        vals.push_back(a.get_value(ca));
        rows[r+1]++;
        ca++;
      }
      else if (a.get_col(ca) > b.get_col(cb))
      {
        cols.push_back(b.get_col(cb));
        vals.push_back(-b.get_value(cb));
        rows[r+1]++;
        cb++;
      }
      else
      {
        cols.push_back(a.get_col(ca));
        vals.push_back(a.get_value(ca) - b.get_value(cb));
        rows[r+1]++;
        ca++;
        cb++;
      }
    }
  }

  SparseRowMatrix::Data data(rows, cols, vals);
  //index_type *vcols = new index_type[cols.size()];
  //std::copy(cols.begin(), cols.end(), vcols);

  //double *vvals = new double[vals.size()];
  //std::copy(vals.begin(), vals.end(), vvals);

  return new SparseRowMatrix(a.nrows(), a.ncols(), data, static_cast<size_type>(vals.size()));
}



} // End namespace SCIRun

