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

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#include <boost/thread.hpp>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Math/ParallelAlgebra/ParallelLinearAlgebra.h>

using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

ParallelLinearAlgebraBase::ParallelLinearAlgebraBase() 
{}

ParallelLinearAlgebra::ParallelLinearAlgebra(ParallelLinearAlgebraBase* base, int proc, int nproc) : barrier_("Parallel Linear Algebra", nproc)
{
  //base_ = base;
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
  reduce_[0] = &(base->reduce1_[0]);
  reduce_[1] = &(base->reduce2_[0]);
  
  reduce_buffer_ = 0;
}

bool ParallelLinearAlgebra::add_vector(DenseColumnMatrixHandle mat, ParallelVector& V)
{
  // Basic checks
  if (!mat) { return (false); }
  if (mat->ncols() != 1)  { return (false); }
  if (mat->nrows() != size_) { return (false); }
  if (matrix_is::sparse(mat)) { return (false); }
  
  V.data_ = mat->get_data_pointer();
  V.size_ = size_;
  
  return (true);
}

bool ParallelLinearAlgebra::new_vector(ParallelVector& V)
{
  barrier_.wait();
  
  base_->success_[proc_] = true;
  if (proc_ == 0)
  {
    try
    {
      DenseColumnMatrixHandle mat(new DenseColumnMatrix(base_->size_);
      base_->current_matrix_ = mat;
      base_->vectors_.push_back(mat);
    }
    catch (...)
    {
      base_->success_[0] = false;    
    }
  }
  
  barrier_.wait();

  if (base_->success_[0] == false) return (false);
  
  MatrixHandle mat = base_->current_matrix_;
  barrier_.wait();

  return(add_vector(mat,V));
}

bool ParallelLinearAlgebra::add_matrix(SparseRowMatrixHandle mat, ParallelMatrix& M)
{
  if (!mat) return (false);
  if (mat->nrows() != size_) return (false);
  
  M.data_ = mat->get_vals();
  M.rows_ = mat->get_rows();
  M.columns_ = mat->get_cols();
  
  M.m_ = mat->nrows();
  M.n_ = mat->ncols();
  M.nnz_ = mat->get_nnz();
  
  return (true);
}

void ParallelLinearAlgebra::mult(ParallelVector& a, ParallelVector& b, ParallelVector& r)
{ 
  double* a_ptr = a.data_+start_; 
  double* b_ptr = b.data_+start_; 
  double* r_ptr = r.data_+start_;

  size_t j = 0;
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

void ParallelLinearAlgebra::add(ParallelVector& a, ParallelVector& b, ParallelVector& r)
{ 
  double* a_ptr = a.data_+start_; 
  double* b_ptr = b.data_+start_; 
  double* r_ptr = r.data_+start_;

  size_t j = 0;
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

void ParallelLinearAlgebra::sub(ParallelVector& a, ParallelVector& b, ParallelVector& r)
{ 
  double* a_ptr = a.data_+start_; 
  double* b_ptr = b.data_+start_; 
  double* r_ptr = r.data_+start_;
  
  size_t j = 0;
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

void ParallelLinearAlgebra::copy(ParallelVector& a, ParallelVector& r)
{
  double* a_ptr = a.data_+start_; 
  double* r_ptr = r.data_+start_;

  size_t j=0;
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

void ParallelLinearAlgebra::scale(double s, ParallelVector& a, ParallelVector& r)
{
  double* a_ptr = a.data_+start_; 
  double* r_ptr = r.data_+start_;

  size_t j=0;
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

void ParallelLinearAlgebra::invert(ParallelVector& a, ParallelVector& r)
{
  double* a_ptr = a.data_+start_; 
  double* r_ptr = r.data_+start_;

  size_t j=0;
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

void ParallelLinearAlgebra::threshold_invert(ParallelVector& a, ParallelVector& r,double threshold)
{
  double* a_ptr = a.data_+start_; 
  double* r_ptr = r.data_+start_;

  for (size_t j=0; j<local_size_; j++) 
  {
    if (*a_ptr > threshold) *r_ptr = 1.0/(*a_ptr); 
    else *r_ptr = 1.0;
    r_ptr++; a_ptr++;
  }
}

void ParallelLinearAlgebra::absthreshold_invert(ParallelVector& a, ParallelVector& r,double threshold)
{
  double* a_ptr = a.data_+start_; 
  double* r_ptr = r.data_+start_;

  for (size_t j=0; j<local_size_; j++) 
  {
    if (std::abs(*a_ptr) > threshold) *r_ptr = 1.0/(*a_ptr); 
    else *r_ptr = 1.0;
    r_ptr++; a_ptr++;
  }
}

void ParallelLinearAlgebra::scale_add(double s, ParallelVector& a, ParallelVector& b, ParallelVector& r)
{ 
  double* a_ptr = a.data_+start_; 
  double* b_ptr = b.data_+start_; 
  double* r_ptr = r.data_+start_;
  
  size_t j=0;
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

double ParallelLinearAlgebra::dot(ParallelVector& a, ParallelVector& b)
{
  double* a_ptr = a.data_+start_; 
  double* b_ptr = b.data_+start_;

  double val = 0.0;
  size_t j=0;
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
 
void ParallelLinearAlgebra::zeros(ParallelVector& a)
{
  double* a_ptr = a.data_+start_;

  size_t j=0;

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

void ParallelLinearAlgebra::ones(ParallelVector& a)
{
  double* a_ptr = a.data_+start_;

  size_t j=0;

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
             
double ParallelLinearAlgebra::norm(ParallelVector& a)
{
  double* a_ptr = a.data_+start_;

  double val = 0.0;
  size_t j=0;

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

double ParallelLinearAlgebra::min(ParallelVector& a)
{
  double m = DBL_MAX;
  double* a_ptr = a.data_+start_;

  for (size_t j=0; j<local_size_; j++) 
  { 
    if (m > (*a_ptr)) m = (*a_ptr);
    a_ptr++;
  }
  
  return(reduce_min(m));
}

double ParallelLinearAlgebra::max(ParallelVector& a)
{
  double m = -(DBL_MAX);
  double* a_ptr = a.data_+start_;

  for (size_t j=0; j<local_size_; j++) 
  { 
    if (m < (*a_ptr)) m = (*a_ptr);
    a_ptr++;
  }
  
  return(reduce_max(m));
}

double ParallelLinearAlgebra::absmin(ParallelVector& a)
{
  double m = DBL_MAX;
  double* a_ptr = a.data_+start_;

  for (size_t j=0; j<local_size_; j++) 
  { 
    double val = std::abs(*a_ptr);
    if (m > val) m = val;
    a_ptr++;
  }
  
  return(reduce_min(m));
}

double ParallelLinearAlgebra::absmax(ParallelVector& a)
{
  double m = -(DBL_MAX);
  double* a_ptr = a.data_+start_;

  for (size_t j=0; j<local_size_; j++) 
  { 
    double val = std::abs(*a_ptr);
    if (m < val) m = val;
    a_ptr++;
  }
  
  return(reduce_max(m));
}

void ParallelLinearAlgebra::mult(ParallelMatrix& a, ParallelVector& b, ParallelVector& r)
{
  barrier_.wait();

  double* idata = b.data_;
  double* odata = r.data_;
  
  double* data = a.data_;
  size_t* rows = a.rows_;
  size_t* columns = a.columns_;
  
  for(size_t i=start_;i<end_;i++)
  {
    double sum = 0.0;
    size_t row_idx=rows[i];
    size_t next_idx=rows[i+1];
    for(size_t j=row_idx;j<next_idx;j++)
    {
	    sum+=data[j]*idata[columns[j]];
    }
    odata[i]=sum;
  }    
}

void ParallelLinearAlgebra::mult_trans(ParallelMatrix& a, ParallelVector& b, ParallelVector& r)
{
  barrier_.wait();

  double* idata = b.data_;
  double* odata = r.data_;
  
  double* data = a.data_;
  size_t* rows = a.rows_;
  size_t* columns = a.columns_;  
  size_t m = a.m_;
  
  for (size_t i=start_; i<end_; i++) odata[i] = 0.0;
  for (size_t j=0; j<m; j++)
  {
    if (idata[j] == 0.0) continue;
    double xj = idata[j];
    size_t row_idx = rows[j];
    size_t next_idx = rows[j+1];
    size_t i=row_idx;
    for (; i<next_idx && columns[i] < start_; i++);
    for (; i<next_idx && columns[i] < end_; i++)
      odata[columns[i]] += data[i]*xj;
  }
}

void ParallelLinearAlgebra::diag(ParallelMatrix& a, ParallelVector& r)
{
  double* odata = r.data_;
  
  double* data = a.data_;
  size_t* rows = a.rows_;
  size_t* columns = a.columns_;
  
  for(size_t i=start_;i<end_;i++)
  {
    double val = 0.0;
    size_t row_idx=rows[i];
    size_t next_idx=rows[i+1];

    for(size_t j=row_idx;j<next_idx;j++)
    {
      if (columns[j] == i) val = data[j];
    }
    odata[i]=val;
  }    
}

void ParallelLinearAlgebra::absdiag(ParallelMatrix& a, ParallelVector& r)
{
  double* odata = r.data_;
  
  double* data = a.data_;
  size_t* rows = a.rows_;
  size_t* columns = a.columns_;

  for(size_t i=start_;i<end_;i++)
  {
    double val = 0.0;
    size_t row_idx=rows[i];
    size_t next_idx=rows[i+1];

    for(size_t j=row_idx;j<next_idx;j++)
    {
      if (columns[j] == i) val = data[j];
    }
    odata[i]=std::abs(val);
  }    
}

double ParallelLinearAlgebra::reduce_sum(double val)
{
  int buffer = reduce_buffer_;
  reduce_[buffer][proc_] = val;
  if (reduce_buffer_) reduce_buffer_ = 0; else reduce_buffer_ = 1;
  barrier_.wait();
  
  double ret = 0.0; for (int j=0; j<nproc_;j++) ret += reduce_[buffer][j];
  return (ret);
}

double ParallelLinearAlgebra::reduce_max(double val)
{
  int buffer = reduce_buffer_;
  reduce_[buffer][proc_] = val;
  if (reduce_buffer_) reduce_buffer_ = 0; else reduce_buffer_ = 1;
  barrier_.wait();
  
  double ret = -(DBL_MAX); for (int j=0; j<nproc_;j++) if (reduce_[buffer][j] > ret) ret = reduce_[buffer][j];
  return (ret);
}

double ParallelLinearAlgebra::reduce_min(double val)
{
  int buffer = reduce_buffer_;
  reduce_[buffer][proc_] = val;
  if (reduce_buffer_) reduce_buffer_ = 0; else reduce_buffer_ = 1;
  barrier_.wait();
  
  double ret = DBL_MAX; for (int j=0; j<nproc_;j++) if (reduce_[buffer][j] < ret) ret = reduce_[buffer][j];
  return (ret);
}

bool ParallelLinearAlgebraBase::start_parallel(std::vector<SparseRowMatrixHandle>& matrices, int nproc)
{
  size_t size = -1;
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
  if (nproc < 1) nproc = boost::thread::hardware_concurrency();

  imatrices_.resize(matrices.size());
  for (size_t j=0; j<matrices.size(); j++) imatrices_[j] = matrices[j];

  reduce1_.resize(nproc);
  reduce2_.resize(nproc);
  success_.resize(nproc);
  
  Thread::parallel(this,&ParallelLinearAlgebraBase::run_parallel,nproc,nproc);

// clear temp memory
  vectors_.clear();
  current_matrix_.reset();
  imatrices_.clear();

  //return std::all_of(success_.begin(), success_.end(), [](bool b) {return b;});
  for (size_t j = 0; j < success_.size(); ++j)
    if (!success_[j]) return (false);
  
  return (true);  
}

void ParallelLinearAlgebraBase::run_parallel(int proc,  int nproc)
{
  ParallelLinearAlgebra PLA(this,proc,nproc);
  success_[proc] = parallel(PLA,imatrices_);
}
