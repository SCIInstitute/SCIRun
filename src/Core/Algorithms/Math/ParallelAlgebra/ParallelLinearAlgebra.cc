/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Core/Math/MiscMath.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Core/Algorithms/Math/ParallelAlgebra/ParallelLinearAlgebra.h>

#include <float.h>

namespace SCIRun {

ParallelLinearAlgebra::
ParallelLinearAlgebra(ParallelLinearAlgebraBase* base, int proc, int nproc)
{
  base_ = base;
  proc_ = proc;
  nproc_ = nproc;
  
  // Compute local size
  size_ = base->size_;
  local_size_ = size_/nproc;
  
  // Compute start and end index for this thread
  start_ = proc*local_size_;
  end_   = (proc+1)*local_size_;
  if (proc == nproc_-1) end_ = size_;
  if (proc == nproc_-1) local_size_ = end_ - start_;
  local_size16_ = (local_size_&(~0xf));
  
  // Set reduction buffers
  // To optimize performance we alternate buffers
  reduce_[0] = &(base_->reduce1_[0]);
  reduce_[1] = &(base_->reduce2_[0]);
  
  reduce_buffer_ = 0;
}

bool
ParallelLinearAlgebra::add_vector(MatrixHandle& mat, ParallelVector& V)
{
  // Basic checks
  if (mat.get_rep() == 0) { return (false); }
  if (mat->ncols() != 1)  { return (false); }
  if (mat->nrows() != size_) { return (false); }
  if (matrix_is::sparse(mat)) { return (false); }
  
  V.data_ = mat->get_data_pointer();
  V.size_ = size_;
  
  return (true);
}

bool
ParallelLinearAlgebra::new_vector(ParallelVector& V)
{
  base_->barrier_.wait(nproc_);
  
  base_->success_[proc_] = true;
  if (proc_ == 0)
  {
    try
    {
      MatrixHandle mat = new ColumnMatrix(base_->size_);
      base_->current_matrix_ = mat.get_rep();
      base_->vectors_.push_back(mat);
    }
    catch (...)
    {
      base_->success_[0] = false;    
    }
  }
  
  base_->barrier_.wait(nproc_);

  if (base_->success_[0] == false) return (false);
  
  MatrixHandle mat = base_->current_matrix_;
  base_->barrier_.wait(nproc_);

  return(add_vector(mat,V));
}

bool 
ParallelLinearAlgebra::add_matrix(MatrixHandle& mat, ParallelMatrix& M)
{
  if (mat.get_rep() == 0) return (false);
  if (mat->nrows() != size_) return (false);
  if (!(matrix_is::sparse(mat))) return (false);
  
  SparseRowMatrix* spr = mat->sparse();
  M.data_ = spr->get_vals();
  M.rows_ = spr->get_rows();
  M.columns_ = spr->get_cols();
  
  M.m_ = mat->nrows();
  M.n_ = mat->ncols();
  M.nnz_ = spr->get_nnz();
  
  return (true);
}


void 
ParallelLinearAlgebra::mult(ParallelVector& a, ParallelVector& b, ParallelVector& r)
{ 
  double* a_ptr = a.data_+start_; 
  double* b_ptr = b.data_+start_; 
  double* r_ptr = r.data_+start_;

  size_type j = 0;
  for (; j<local_size16_; j+=16) 
  {
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;

    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
  }
  
  for (; j<local_size_; j++) 
  {
    *r_ptr = (*a_ptr)*(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
  }
}

void 
ParallelLinearAlgebra::add(ParallelVector& a, ParallelVector& b, ParallelVector& r)
{ 
  double* a_ptr = a.data_+start_; 
  double* b_ptr = b.data_+start_; 
  double* r_ptr = r.data_+start_;

  size_type j = 0;
  for (; j<local_size16_; j+=16) 
  {
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;

    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
  }
  
  for (; j<local_size_; j++) 
  {
    *r_ptr = (*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
  }
}

void 
ParallelLinearAlgebra::sub(ParallelVector& a, ParallelVector& b, ParallelVector& r)
{ 
  double* a_ptr = a.data_+start_; 
  double* b_ptr = b.data_+start_; 
  double* r_ptr = r.data_+start_;
  
  size_type j = 0;
  for (; j<local_size16_; j+=16) 
  {
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;

    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
  }
  
  for (; j<local_size_; j++) 
  {
    *r_ptr = (*a_ptr)-(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
  }
}

void 
ParallelLinearAlgebra::copy(ParallelVector& a, ParallelVector& r)
{
  double* a_ptr = a.data_+start_; 
  double* r_ptr = r.data_+start_;

  size_type j=0;
  for (; j<local_size16_; j+=16) 
  {
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;

    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
  }
  
  for (; j<local_size_; j++) 
  {
    *r_ptr = *a_ptr; r_ptr++; a_ptr++;
  }
}

void 
ParallelLinearAlgebra::scale(double s, ParallelVector& a, ParallelVector& r)
{
  double* a_ptr = a.data_+start_; 
  double* r_ptr = r.data_+start_;

  size_type j=0;
  for (; j<local_size16_; j+=16) 
  {
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;

    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
  }

  for (; j<local_size_; j++) 
  {
    *r_ptr = s*(*a_ptr); r_ptr++; a_ptr++;
  }
}

void 
ParallelLinearAlgebra::invert(ParallelVector& a, ParallelVector& r)
{
  double* a_ptr = a.data_+start_; 
  double* r_ptr = r.data_+start_;

  size_type j=0;
  for (; j<local_size16_; j+=16) 
  {
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;

    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
  }

  for (; j<local_size_; j++) 
  {
    *r_ptr = 1.0/(*a_ptr); r_ptr++; a_ptr++;
  }
}

void 
ParallelLinearAlgebra::threshold_invert(ParallelVector& a, ParallelVector& r,double threshold)
{
  double* a_ptr = a.data_+start_; 
  double* r_ptr = r.data_+start_;

  for (size_type j=0; j<local_size_; j++) 
  {
    if (*a_ptr > threshold) *r_ptr = 1.0/(*a_ptr); 
    else *r_ptr = 1.0;
    r_ptr++; a_ptr++;
  }
}


void 
ParallelLinearAlgebra::absthreshold_invert(ParallelVector& a, ParallelVector& r,double threshold)
{
  double* a_ptr = a.data_+start_; 
  double* r_ptr = r.data_+start_;

  for (size_type j=0; j<local_size_; j++) 
  {
    if (Abs(*a_ptr) > threshold) *r_ptr = 1.0/(*a_ptr); 
    else *r_ptr = 1.0;
    r_ptr++; a_ptr++;
  }
}


void 
ParallelLinearAlgebra::scale_add(double s, ParallelVector& a, ParallelVector& b, ParallelVector& r)
{ 
  double* a_ptr = a.data_+start_; 
  double* b_ptr = b.data_+start_; 
  double* r_ptr = r.data_+start_;
  
  size_type j=0;
  for (; j<local_size16_; j+=16) 
  {
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;

    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
  }

  for (; j<local_size_; j++) 
  {
    *r_ptr = s*(*a_ptr)+(*b_ptr); r_ptr++; a_ptr++; b_ptr++;
  }
}


double 
ParallelLinearAlgebra::dot(ParallelVector& a, ParallelVector& b)
{
  double* a_ptr = a.data_+start_; 
  double* b_ptr = b.data_+start_;

  double val = 0.0;
  size_type j=0;
  for (; j<local_size16_; j+=16) 
  {
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;

    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
  }

  for (; j<local_size_; j++) 
  {
    val += (*a_ptr)*(*b_ptr); b_ptr++; a_ptr++;
  }
  
  return(reduce_sum(val));
}
 
void
ParallelLinearAlgebra::zeros(ParallelVector& a)
{
  double* a_ptr = a.data_+start_;

  size_type j=0;

  for (; j<local_size16_; j+=16) 
  {
    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;
    
    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;

    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;
    
    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;
    (*a_ptr) = 0.0; a_ptr++;
  }
  
  for (; j<local_size_; j++) 
  {  
    (*a_ptr) = 0.0; a_ptr++;
  }
}    
       

void
ParallelLinearAlgebra::ones(ParallelVector& a)
{
  double* a_ptr = a.data_+start_;

  size_type j=0;

  for (; j<local_size16_; j+=16) 
  {
    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;
    
    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;

    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;
    
    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;
    (*a_ptr) = 1.0; a_ptr++;
  }
  
  for (; j<local_size_; j++) 
  {  
    (*a_ptr) = 1.0; a_ptr++;
  }
}    
             
double 
ParallelLinearAlgebra::norm(ParallelVector& a)
{
  double* a_ptr = a.data_+start_;

  double val = 0.0;
  size_type j=0;

  for (; j<local_size16_; j+=16) 
  {
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;

    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;
    val += (*a_ptr)*(*a_ptr); a_ptr++;
  }
  
  for (; j<local_size_; j++) 
  {  
    val += (*a_ptr)*(*a_ptr); a_ptr++;
  }
  
  return(sqrt(reduce_sum(val)));
}

double 
ParallelLinearAlgebra::min(ParallelVector& a)
{
  double m = DBL_MAX;
  double* a_ptr = a.data_+start_;

  for (size_type j=0; j<local_size_; j++) 
  { 
    if (m > (*a_ptr)) m = (*a_ptr);
    a_ptr++;
  }
  
  return(reduce_min(m));
}

double 
ParallelLinearAlgebra::max(ParallelVector& a)
{
  double m = -(DBL_MAX);
  double* a_ptr = a.data_+start_;

  for (size_type j=0; j<local_size_; j++) 
  { 
    if (m < (*a_ptr)) m = (*a_ptr);
    a_ptr++;
  }
  
  return(reduce_max(m));
}


double 
ParallelLinearAlgebra::absmin(ParallelVector& a)
{
  double m = DBL_MAX;
  double* a_ptr = a.data_+start_;

  for (size_type j=0; j<local_size_; j++) 
  { 
    double val = Abs(*a_ptr);
    if (m > val) m = val;
    a_ptr++;
  }
  
  return(reduce_min(m));
}

double 
ParallelLinearAlgebra::absmax(ParallelVector& a)
{
  double m = -(DBL_MAX);
  double* a_ptr = a.data_+start_;

  for (size_type j=0; j<local_size_; j++) 
  { 
    double val = Abs(*a_ptr);
    if (m < val) m = val;
    a_ptr++;
  }
  
  return(reduce_max(m));
}

void
ParallelLinearAlgebra::mult(ParallelMatrix& a, ParallelVector& b, ParallelVector& r)
{
  base_->barrier_.wait(nproc_);

  double* idata = b.data_;
  double* odata = r.data_;
  
  double* data = a.data_;
  index_type* rows = a.rows_;
  index_type* columns = a.columns_;
  
  for(index_type i=start_;i<end_;i++)
  {
    double sum = 0.0;
    index_type row_idx=rows[i];
    index_type next_idx=rows[i+1];
    for(index_type j=row_idx;j<next_idx;j++)
    {
	    sum+=data[j]*idata[columns[j]];
    }
    odata[i]=sum;
  }    
}

void
ParallelLinearAlgebra::mult_trans(ParallelMatrix& a, ParallelVector& b, ParallelVector& r)
{
  base_->barrier_.wait(nproc_);

  double* idata = b.data_;
  double* odata = r.data_;
  
  double* data = a.data_;
  index_type* rows = a.rows_;
  index_type* columns = a.columns_;  
  size_type m = a.m_;
  
  for (index_type i=start_; i<end_; i++) odata[i] = 0.0;
  for (index_type j=0; j<m; j++)
  {
    if (idata[j] == 0.0) continue;
    double xj = idata[j];
    index_type row_idx = rows[j];
    index_type next_idx = rows[j+1];
    index_type i=row_idx;
    for (; i<next_idx && columns[i] < start_; i++);
    for (; i<next_idx && columns[i] < end_; i++)
      odata[columns[i]] += data[i]*xj;
  }
}

void
ParallelLinearAlgebra::diag(ParallelMatrix& a, ParallelVector& r)
{
  double* odata = r.data_;
  
  double* data = a.data_;
  index_type* rows = a.rows_;
  index_type* columns = a.columns_;
  
  for(index_type i=start_;i<end_;i++)
  {
    double val = 0.0;
    index_type row_idx=rows[i];
    index_type next_idx=rows[i+1];

    for(index_type j=row_idx;j<next_idx;j++)
    {
      if (columns[j] == i) val = data[j];
    }
    odata[i]=val;
  }    
}

void
ParallelLinearAlgebra::absdiag(ParallelMatrix& a, ParallelVector& r)
{
  double* odata = r.data_;
  
  double* data = a.data_;
  index_type* rows = a.rows_;
  index_type* columns = a.columns_;

  for(index_type i=start_;i<end_;i++)
  {
    double val = 0.0;
    index_type row_idx=rows[i];
    index_type next_idx=rows[i+1];

    for(index_type j=row_idx;j<next_idx;j++)
    {
      if (columns[j] == i) val = data[j];
    }
    odata[i]=Abs(val);
  }    
}


double
ParallelLinearAlgebra::reduce_sum(double val)
{
  int buffer = reduce_buffer_;
  reduce_[buffer][proc_] = val;
  if (reduce_buffer_) reduce_buffer_ = 0; else reduce_buffer_ = 1;
  base_->barrier_.wait(nproc_);
  
  double ret = 0.0; for (int j=0; j<nproc_;j++) ret += reduce_[buffer][j];
  return (ret);
}


double
ParallelLinearAlgebra::reduce_max(double val)
{
  int buffer = reduce_buffer_;
  reduce_[buffer][proc_] = val;
  if (reduce_buffer_) reduce_buffer_ = 0; else reduce_buffer_ = 1;
  base_->barrier_.wait(nproc_);
  
  double ret = -(DBL_MAX); for (int j=0; j<nproc_;j++) if (reduce_[buffer][j] > ret) ret = reduce_[buffer][j];
  return (ret);
}

double
ParallelLinearAlgebra::reduce_min(double val)
{
  int buffer = reduce_buffer_;
  reduce_[buffer][proc_] = val;
  if (reduce_buffer_) reduce_buffer_ = 0; else reduce_buffer_ = 1;
  base_->barrier_.wait(nproc_);
  
  double ret = DBL_MAX; for (int j=0; j<nproc_;j++) if (reduce_[buffer][j] < ret) ret = reduce_[buffer][j];
  return (ret);
}


bool 
ParallelLinearAlgebraBase::start_parallel(std::vector<MatrixHandle>& matrices, int nproc)
{
  size_type size = -1;
  if (matrices.size() == 0) return (false);
  
  for (size_t j=0; j<matrices.size(); j++)
  {
    if (size == -1) size = matrices[j]->nrows();
    if (matrices[j]->nrows() != size) return (false);
  }

  // Store base size in base class
  size_ = size;
  
  //! Require a minimum of 50 variables per processor
  //! Below that parallelism is overhead
  if (nproc*50 > size_) nproc = size_/50;
  if (nproc < 1) nproc = Thread::numProcessors();

  imatrices_.resize(matrices.size());
  for (size_t j=0; j<matrices.size(); j++) imatrices_[j] = matrices[j];

  reduce1_.resize(nproc);
  reduce2_.resize(nproc);
  success_.resize(nproc);
  
  Thread::parallel(this,&ParallelLinearAlgebraBase::run_parallel,nproc,nproc);

// clear temp memory
  vectors_.clear();
  current_matrix_ = 0;
  imatrices_.clear();

  for (size_t j=0;j<success_.size(); j++)
    if (success_[j] == false) return (false);
  
  return (true);  
}

void
ParallelLinearAlgebraBase::run_parallel(int proc,  int nproc)
{
  ParallelLinearAlgebra PLA(this,proc,nproc);
  success_[proc] = parallel(PLA,imatrices_);
}


} //end namespaced
