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

#ifndef CORE_ALGORITHMS_MATH_PARALLELALGEBRA_PARALLELLINEARALGEBRA_H
#define CORE_ALGORITHMS_MATH_PARALLELALGEBRA_PARALLELLINEARALGEBRA_H 1

//! Datatypes that the algorithm uses
#include <Core/Datatypes/Matrix.h>
#include <Core/Thread/Thread.h>
#include <Core/Thread/Barrier.h>

#include <vector>
#include <list>
//! for Windows support
#include <Core/Datatypes/share.h>

namespace SCIRun {

class ParallelLinearAlgebraBase;
class ParallelLinearAlgebra;

// The algorithm that uses this should derive from this class
class ParallelLinearAlgebraBase {

  public:
    ParallelLinearAlgebraBase() :
      barrier_("Parallel Linear Algebra") {}
    virtual ~ParallelLinearAlgebraBase() {}
  
    bool start_parallel(std::vector<MatrixHandle>& matrices,int nproc=-1);
    void run_parallel(int proc, int nproc);
  
    virtual bool parallel(ParallelLinearAlgebra& PLA, std::vector<MatrixHandle>& matrices) = 0;


    //! classes for communication
    Barrier              barrier_;
    std::vector<double>  reduce1_;
    std::vector<double>  reduce2_;
    std::vector<bool>    success_;
    
    size_type    size_;
    int                  nproc_;
    
    MatrixHandle              current_matrix_;
    std::list<MatrixHandle>   vectors_;
    
    std::vector<MatrixHandle> imatrices_;
};


class ParallelLinearAlgebra {

  public:
    class ParallelVector {
      public:
        double* data_;
        size_type size_;
    };
    
    class ParallelMatrix {
      public:
        index_type* rows_;
        index_type* columns_;
        double* data_;
        
        size_type   m_;
        size_type   n_;
        size_type   nnz_;
    };
      
    // Constructor
    ParallelLinearAlgebra(ParallelLinearAlgebraBase* base, int proc, int nproc); 
    
    bool add_vector(MatrixHandle& mat, ParallelVector& V);
    bool new_vector(ParallelVector& V);
    bool add_matrix(MatrixHandle& mat, ParallelMatrix& M);

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
    
    int  proc() { return (proc_); }
    int  nproc() { return (nproc_); }
    
    bool first() { return (proc_ == 0); }
    void wait()  { base_->barrier_.wait(nproc_); }

  private:
    double reduce_sum(double val);
    double reduce_min(double val);
    double reduce_max(double val);
    
    ParallelLinearAlgebraBase* base_;
  
    int proc_;  // process number
    int nproc_; // number of processes

    size_type size_;
    size_type local_size_;
    size_type local_size16_;
    index_type start_;
    index_type end_;
    
    double* reduce_[2];
    int     reduce_buffer_;
};


}

#endif
