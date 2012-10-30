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
//TODO DAN: REFACTORING NEEDED: LEVEL HIGHEST
///////////////////////////

#include <boost/thread.hpp>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Math/ParallelAlgebra/ParallelLinearAlgebra.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

Barrier::Barrier(const std::string& name, size_t numThreads) : name_(name), barrier_(numThreads)
{
  std::cout << "Initiating barrier with thread count " << numThreads << std::endl;
}

void Barrier::wait()
{
  barrier_.wait();
}

ParallelLinearAlgebraBase::ParallelLinearAlgebraBase() 
{}

ParallelLinearAlgebraBase::~ParallelLinearAlgebraBase() 
{}

ParallelLinearAlgebra::ParallelLinearAlgebra(ParallelLinearAlgebraSharedData& data, int proc) 
  : data_(data),
  proc_(proc),
  nproc_(data.numProcs()),
  reduce1_(nproc_),
  reduce2_(nproc_)
{
  // Compute local size
  size_ = data.getSize();
  local_size_ = size_/nproc_;
  
  // Compute start and end index for this thread
  start_ = proc*local_size_;
  end_   = (proc+1)*local_size_;
  if (proc == nproc_-1) end_ = size_;
  if (proc == nproc_-1) local_size_ = end_ - start_;
  local_size16_ = (local_size_&(~0xf));

  std::cout << "PLA #" << proc_ << " out of " << nproc_ << " is handling range [" << start_ << "," << end_ << ")" << std::endl;
  
  // Set reduction buffers
  // To optimize performance we alternate buffers
  reduce_[0] = &(reduce1_[0]);
  reduce_[1] = &(reduce2_[0]);
  
  reduce_buffer_ = 0;
}

void ParallelLinearAlgebra::wait()
{
  std::cout << "PLA #" << proc_ << " out of " << nproc_ << " waiting..." << std::endl;
  data_.wait();
  std::cout << "PLA #" << proc_ << " out of " << nproc_ << " done waiting." << std::endl;
}

bool ParallelLinearAlgebra::add_vector(DenseColumnMatrixHandle mat, ParallelVector& V)
{
  // Basic checks
  if (!mat) { return (false); }
  if (mat->ncols() != 1)  { return (false); }
  if (mat->nrows() != size_) { return (false); }
  if (matrix_is::sparse(mat)) { return (false); }
  
  V.data_ = mat->data();
  V.size_ = size_;
  
  return true;
}

bool ParallelLinearAlgebra::new_vector(ParallelVector& V)
{
  wait();
  
  data_.setSuccess(proc_);
  if (proc_ == 0)
  {
    try
    {
      DenseColumnMatrixHandle mat(new DenseColumnMatrix(data_.getSize()));
      data_.setCurrentMatrix(mat);
      data_.addVector(mat);
    }
    catch (...)
    {
      data_.setFail(0);
    }
  }
  
  wait();

  if (!data_.isSuccess(0)) 
    return false;
  
  auto mat = data_.getCurrentMatrix();
  wait();

  return(add_vector(mat,V));
}

bool ParallelLinearAlgebra::add_matrix(SparseRowMatrixHandle mat, ParallelMatrix& M)
{
  if (!mat) return (false);
  if (mat->nrows() != size_) return (false);
  
  mat->makeCompressed(); //TODO: this should be an invariant of our SparseRowMatrix type.
  M.data_ = mat->valuePtr();
  M.rows_ = mat->outerIndexPtr();
  M.columns_ = mat->innerIndexPtr();
  
  M.m_ = mat->nrows();
  M.n_ = mat->ncols();
  M.nnz_ = mat->nonZeros();
  
  return (true);
}

//TODO: refactor duplication

void ParallelLinearAlgebra::mult(const ParallelVector& a, const ParallelVector& b, ParallelVector& r)
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

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
#endif

void ParallelLinearAlgebra::sub(const ParallelVector& a, const ParallelVector& b, ParallelVector& r)
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

void ParallelLinearAlgebra::copy(const ParallelVector& a, ParallelVector& r)
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

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
#endif

void ParallelLinearAlgebra::absthreshold_invert(const ParallelVector& a, ParallelVector& r,double threshold)
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

void ParallelLinearAlgebra::scale_add(double s, const ParallelVector& a, const ParallelVector& b, ParallelVector& r)
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

double ParallelLinearAlgebra::dot(const ParallelVector& a, const ParallelVector& b)
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
 
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
#endif

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
             
double ParallelLinearAlgebra::norm(const ParallelVector& a)
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

//TODO: refactor to use algorithm
double ParallelLinearAlgebra::max(const ParallelVector& a)
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

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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



double ParallelLinearAlgebra::absmin(const ParallelVector& a)
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

double ParallelLinearAlgebra::absmax(const ParallelVector& a)
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
#endif


void ParallelLinearAlgebra::mult(const ParallelMatrix& a, const ParallelVector& b, ParallelVector& r)
{
  wait();

  double* idata = b.data_;
  double* odata = r.data_;
  
  double* data = a.data_;
  int* rows = a.rows_;
  int* columns = a.columns_;
  
  for(size_t i=start_;i<end_;i++)
  {
    double sum = 0.0;
    int row_idx=rows[i];
    int next_idx=rows[i+1];
    for(size_t j=row_idx;j<next_idx;j++)
    {
	    sum+=data[j]*idata[columns[j]];
    }
    odata[i]=sum;
  }    
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
void ParallelLinearAlgebra::mult_trans(ParallelMatrix& a, ParallelVector& b, ParallelVector& r)
{
  wait();

  double* idata = b.data_;
  double* odata = r.data_;
  
  double* data = a.data_;
  int* rows = a.rows_;
  int* columns = a.columns_;  
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
  int* rows = a.rows_;
  int* columns = a.columns_;
  
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
#endif

void ParallelLinearAlgebra::absdiag(const ParallelMatrix& a, ParallelVector& r)
{
  double* odata = r.data_;
  
  double* data = a.data_;
  int* rows = a.rows_;
  int* columns = a.columns_;

  for(size_t i=start_;i<end_;i++)
  {
    double val = 0.0;
    size_t row_idx=rows[i];
    size_t next_idx=rows[i+1];

    for(size_t j=row_idx;j<next_idx;j++)
    {
      if (columns[j] == i) 
        val = data[j];
    }
    odata[i]=std::abs(val);
  }    
}

double ParallelLinearAlgebra::reduce_sum(double val)
{
  int buffer = reduce_buffer_;
  reduce_[buffer][proc_] = val;
  if (reduce_buffer_) 
    reduce_buffer_ = 0; 
  else 
    reduce_buffer_ = 1;
  wait();
  
  double ret = 0.0; for (int j=0; j<nproc_;j++) ret += reduce_[buffer][j];
  return (ret);
}

//TODO: std::max_element
double ParallelLinearAlgebra::reduce_max(double val)
{
  int buffer = reduce_buffer_;
  reduce_[buffer][proc_] = val;
  if (reduce_buffer_) 
    reduce_buffer_ = 0; 
  else 
    reduce_buffer_ = 1;
  wait();
  
  double ret = -(DBL_MAX); for (int j=0; j<nproc_;j++) if (reduce_[buffer][j] > ret) ret = reduce_[buffer][j];
  return (ret);
}

//TODO: std::min_element
double ParallelLinearAlgebra::reduce_min(double val)
{
  int buffer = reduce_buffer_;
  reduce_[buffer][proc_] = val;
  if (reduce_buffer_) 
    reduce_buffer_ = 0; 
  else 
    reduce_buffer_ = 1;
  wait();
  
  double ret = DBL_MAX; for (int j=0; j<nproc_;j++) if (reduce_[buffer][j] < ret) ret = reduce_[buffer][j];
  return (ret);
}

bool ParallelLinearAlgebraBase::start_parallel(SolverInputs& matrices, int nproc)
{
  size_t size = matrices.A->nrows();
  if (matrices.b->nrows() != size
    || matrices.x->nrows() != size
    || matrices.x0->nrows() != size)
    return false;

  //! Require a minimum of 50 variables per processor
  //! Below that parallelism is overhead
  if (nproc*50 > size) 
    nproc = size / 50;
  if (nproc < 1) 
    nproc = boost::thread::hardware_concurrency();

  ParallelLinearAlgebraSharedData sharedData(matrices, nproc);
  
#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
  Thread::parallel(this,&ParallelLinearAlgebraBase::run_parallel,nproc);
#endif

  return sharedData.success();
}

void ParallelLinearAlgebraBase::run_parallel(ParallelLinearAlgebraSharedData& data, int proc)
{
  ParallelLinearAlgebra PLA(data,proc);
  parallel(PLA, data.inputs());
}

ParallelLinearAlgebraSharedData::ParallelLinearAlgebraSharedData(const SolverInputs& inputs, int numProcs) : size_(inputs.A->nrows()), success_(size_), imatrices_(inputs), barrier_("Parallel Linear Algebra", numProcs), numProcs_(numProcs)
{
  if (inputs.b->nrows() != size_
    || inputs.x->nrows() != size_
    || inputs.x0->nrows() != size_)
    BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Dimension mismatch")); //TODO: use new DimensionMismatch exception type
}