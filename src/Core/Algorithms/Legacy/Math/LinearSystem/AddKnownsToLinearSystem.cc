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

#include <Core/Algorithms/Math/LinearSystem/AddKnownsToLinearSystem.h>

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

// For NaN testing
#include <teem/air.h>

namespace SCIRunAlgo {

using namespace SCIRun;


bool 
AddKnownsToLinearSystemAlgo::run(MatrixHandle a_in, 
                                 MatrixHandle b_in,
                                 MatrixHandle x_known, 
                                 SparseRowMatrixHandle& a_out, 
                                 MatrixHandle& b_out,
                                 bool convert_matrix_types)
{
  algo_start("AddKnownsToLinearSystem");
  
  if (a_in.get_rep() == 0)
  {
    error("No input matrix A");
    algo_end();
    return (false);
  }

  if (! matrix_is::sparse(a_in))
  {
    if (convert_matrix_types)
    {
      warning("Matrix A is not sparse - converting.");
      MatrixHandle tmp = a_in->sparse();
      a_in = tmp;
    }
    else
    {
      error("Matrix A is not sparse");
      algo_end();
      return (false);
    }
  }

  if (a_in->ncols() != a_in->nrows())
  {
    error("Matrix A is not square");
    return (false);  
  }

  /// @todo: if matrix A is sparse (checked and/or converted),
  // then is this call to sparse needed?
  // If the matrix is already sparse, then the pointer to the
  // sparse matrix object is returned...
  a_out = a_in->sparse();
  a_out.detach();  

  size_type m = a_in->nrows();
  
  // Check and process RHS vector
  
  if (b_in.get_rep() == 0)
  {
    b_out = new DenseMatrix(m,1);
    if (b_out.get_rep() == 0)
    {
      error("Could not allocate new b matrix");
      algo_end(); return (false);       
    }
    
    b_out->zero();
  }
  else
  {
    if (b_in->nrows() * b_in->ncols() != m)
    {
      error("The dimensions of matrix b do not match the dimensions of matrix A");
      algo_end(); return (false); 
    }
  
    b_out = b_in->dense();
    b_out.detach();
    
    if (b_out.get_rep() == 0)
    {
      error("Could not allocate new b matrix");
      algo_end(); return (false);      
    }
  }
  
  if (x_known.get_rep() == 0)
  {
    error("No x vector was given");
    algo_end(); return (false);    
  }
  
  if (x_known->nrows() * x_known->ncols() != m)
  {
    error("The dimensions of matrix x do not match the dimensions of matrix A");
    algo_end(); return (false);           
  }

  MatrixHandle Temp;
  
  Temp = x_known->dense();
  x_known = Temp;
    
  if (x_known.get_rep() == 0)
  {
    error("Could not allocate x matrix");
    algo_end(); return (false);      
  }
  
  double* x_ptr = x_known->get_data_pointer();
  double* b_ptr = b_out->get_data_pointer();
  
  index_type *idx_nzero; 
  double *val_nzero = 0;
  size_type  idx_nzerosize;
  index_type idx_nzerostride;
  
  index_type* rows = a_out->get_rows();
  index_type* columns = a_out->get_cols();
  double* a = a_out->get_vals();
  
  index_type cnt = 0;
  
  index_type knowns = 0;
  index_type unknowns = 0;

  for (index_type p=0; p<m;p++)
  {
    if (airExists(x_ptr[p]))
    {
      knowns++;
      a_out->getRowNonzerosNoCopy(p, idx_nzerosize, idx_nzerostride, idx_nzero, val_nzero);
      
      for (index_type i=rows[p]; i<rows[p+1]; i++)
      {
        index_type j = columns[i];
        b_ptr[j] += - x_ptr[p] * a[i]; 
      }    
    }
    else
    {
      unknowns++;
    }
    cnt++;
    if (cnt == 1000)
    {
      cnt = 0;
      update_progress(p, 2*m);
    }
  }
  
  cnt = 0;
  for (index_type p=0; p<m;p++)
  {
    if (airExists(x_ptr[p]))
    {
      for (index_type i=rows[p]; i<rows[p+1]; i++)
      {
        index_type j = columns[i];
        a_out->put(p, j, 0.0);
        a_out->put(j, p, 0.0); 
      }
      
      /// updating dirichlet node and corresponding entry in b_ptr
      a_out->put(p, p, 1.0);
      b_ptr[p] = x_ptr[p];
    }
    cnt++;
    if (cnt == 1000)
    {
      cnt = 0;
      update_progress(p+m, 2*m);
    }
  }

  algo_end();
  return (true);
}

} // end namespace
