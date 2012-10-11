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

#ifndef CORE_ALGORITHMS_MATH_PARALLELALGEBRA_PARALLELLINEARALGEBRA_H
#define CORE_ALGORITHMS_MATH_PARALLELALGEBRA_PARALLELLINEARALGEBRA_H

#include <vector>
#include <list>
#include <boost/noncopyable.hpp>
#include <boost/thread/barrier.hpp>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Algorithms/Math/Share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Math {

  class ParallelLinearAlgebra;

// The algorithm that uses this should derive from this class
class SCISHARE ParallelLinearAlgebraBase 
{
public:
  ParallelLinearAlgebraBase(); //:      barrier_("Parallel Linear Algebra") {}
  virtual ~ParallelLinearAlgebraBase();// {}
  
  bool start_parallel(std::vector<Datatypes::MatrixHandle>& matrices, int nproc = -1);
  void run_parallel(int proc, int nproc);
  
  virtual bool parallel(ParallelLinearAlgebra& PLA, std::vector<Datatypes::MatrixHandle>& matrices) = 0;

  //! classes for communication
  boost::barrier barrier_;
  std::vector<double> reduce1_;
  std::vector<double> reduce2_;
  std::vector<bool> success_;
    
  size_t size_;
  int nproc_;
    
  Datatypes::MatrixHandle current_matrix_;
  std::list<Datatypes::MatrixHandle> vectors_;
  std::vector<Datatypes::MatrixHandle> imatrices_;
};


class SCISHARE ParallelLinearAlgebra : boost::noncopyable
{
public:
  class ParallelVector {
    public:
      double* data_;
      size_t size_;
  };
    
  class ParallelMatrix {
    public:
      size_t* rows_;
      size_t* columns_;
      double* data_;
        
      size_t   m_;
      size_t   n_;
      size_t   nnz_;
  };
      
  // Constructor
  ParallelLinearAlgebra(ParallelLinearAlgebraBase* base, int proc, int nproc); 
    
  bool add_vector(Datatypes::MatrixHandle& mat, ParallelVector& V);
  bool new_vector(ParallelVector& V);
  bool add_matrix(Datatypes::MatrixHandle& mat, ParallelMatrix& M);

  void mult(ParallelVector& a, ParallelVector& b, ParallelVector& r);
  void add(ParallelVector& a, ParallelVector& b, ParallelVector& r);
  void sub(ParallelVector& a, ParallelVector& b, ParallelVector& r);
  void copy(ParallelVector& a, ParallelVector& r);

  void scale_add(double s, ParallelVector& a, ParallelVector& b, ParallelVector& r);
  void scale(double s, ParallelVector& a, ParallelVector& r);
  void invert(ParallelVector& a, ParallelVector& r);
  void threshold_invert(ParallelVector& a, ParallelVector& r, double threshold);
  void absthreshold_invert(ParallelVector& a, ParallelVector& r, double threshold);
    
  double dot(ParallelVector& a, ParallelVector& b);
  double norm(ParallelVector& a);
  double min(ParallelVector& a);
  double max(ParallelVector& a);
  double absmin(ParallelVector& a);
  double absmax(ParallelVector& a);
    
  void mult(ParallelMatrix& a, ParallelVector& b, ParallelVector& r);
  void mult_trans(ParallelMatrix& a, ParallelVector& b, ParallelVector& r);

  void diag(ParallelMatrix& a, ParallelVector& r);
  void absdiag(ParallelMatrix& a, ParallelVector& r);
    
  void zeros(ParallelVector& r);
  void ones(ParallelVector& r);
    
  int  proc() { return proc_; }
  int  nproc() { return nproc_; }
    
  bool first() { return proc_ == 0; }
  void wait();//  { base_->barrier_.wait(nproc_); }

private:
  double reduce_sum(double val);
  double reduce_min(double val);
  double reduce_max(double val);
    
  ParallelLinearAlgebraBase* base_;
  
  int proc_;  // process number
  int nproc_; // number of processes

  size_t size_;
  size_t local_size_;
  size_t local_size16_;
  size_t start_;
  size_t end_;
    
  double* reduce_[2];
  int     reduce_buffer_;
};


}}}}

#endif
