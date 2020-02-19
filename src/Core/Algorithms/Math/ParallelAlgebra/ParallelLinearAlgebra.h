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


///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#ifndef CORE_ALGORITHMS_MATH_PARALLELALGEBRA_PARALLELLINEARALGEBRA_H
#define CORE_ALGORITHMS_MATH_PARALLELALGEBRA_PARALLELLINEARALGEBRA_H

#include <vector>
#include <list>
#include <boost/noncopyable.hpp>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Thread/Barrier.h>
#include <Core/Datatypes/Legacy/Base/Types.h>
#include <Core/Algorithms/Math/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Math {

  class ParallelLinearAlgebra;

  struct SCISHARE SolverInputs
  {
    Datatypes::SparseRowMatrixHandle A;
    Datatypes::DenseColumnMatrixHandle b;
    Datatypes::DenseColumnMatrixHandle x0;
    Datatypes::DenseColumnMatrixHandle x;

    void clear()
    {
      A.reset();
      b.reset();
      x0.reset();
      x.reset();
    }
  };

  class SCISHARE ParallelLinearAlgebraSharedData : boost::noncopyable
  {
  public:
    explicit ParallelLinearAlgebraSharedData(const SolverInputs& inputs, int numProcs);
    size_t getSize() const { return size_; }
    Datatypes::DenseColumnMatrixHandle getCurrentMatrix() const { return current_matrix_; }
    void setCurrentMatrix(Datatypes::DenseColumnMatrixHandle mat) { current_matrix_ = mat; }
    void addVector(Datatypes::DenseColumnMatrixHandle mat) { vectors_.push_back(mat); }
    void setFlag(size_t i, bool b) { success_[i] = b; }
    void setSuccess(size_t i) { success_[i] = true; }
    void setFail(size_t i) { success_[i] = false; }
    bool isSuccess(size_t i) const { return success_[i]; }

    void wait() { barrier_.wait(); }
    int numProcs() const { return numProcs_; }

    bool success() const
    {
      //return std::all_of(success_.begin(), success_.end(), [](bool b) {return b;});
      for (size_t j = 0; j < success_.size(); ++j)
        if (!success_[j])
        {
          return false;
        }
      return true;
    }

    SolverInputs& inputs() { return imatrices_; }

    double* reduceBuffer1() { return &reduce1_[0]; }
    double* reduceBuffer2() { return &reduce2_[0]; }

  private:
    size_t size_;
    Datatypes::DenseColumnMatrixHandle current_matrix_;
    std::list<Datatypes::DenseColumnMatrixHandle> vectors_;
    std::vector<bool> success_;
    SolverInputs imatrices_;
    SCIRun::Core::Thread::Barrier barrier_;
    int numProcs_;
    /// classes for communication
    std::vector<double> reduce1_;
    std::vector<double> reduce2_;
  };

// The algorithm that uses this should derive from this class
class SCISHARE ParallelLinearAlgebraBase : boost::noncopyable
{
public:
  ParallelLinearAlgebraBase();
  virtual ~ParallelLinearAlgebraBase();

  bool start_parallel(SolverInputs& matrices, int nproc = -1) const;

  virtual bool parallel(ParallelLinearAlgebra& PLA, SolverInputs& matrices) const = 0;

private:
  void run_parallel(ParallelLinearAlgebraSharedData& data, int proc) const;
  SolverInputs imatrices_;
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
      index_type* rows_;
      index_type* columns_;
      double* data_;

      size_t   m_;
      size_t   n_;
      size_t   nnz_;
  };

  // Constructor
  ParallelLinearAlgebra(ParallelLinearAlgebraSharedData& base, int proc);

  bool add_vector(Datatypes::DenseColumnMatrixHandle mat, ParallelVector& V);
  bool new_vector(ParallelVector& V);
  bool add_matrix(Datatypes::SparseRowMatrixHandle mat, ParallelMatrix& M);

  void mult(const ParallelVector& a, const ParallelVector& b, ParallelVector& r);
  void sub(const ParallelVector& a, const ParallelVector& b, ParallelVector& r);
  void copy(const ParallelVector& a, ParallelVector& r);

  // r = s*a + b;
  void scale_add(double s, const ParallelVector& a, const ParallelVector& b, ParallelVector& r);

  void add(const ParallelVector& a, const ParallelVector& b, ParallelVector& r);

  void scale(double s, ParallelVector& a, ParallelVector& r);
  void invert(ParallelVector& a, ParallelVector& r);
  void threshold_invert(ParallelVector& a, ParallelVector& r, double threshold);

  double min(const ParallelVector& a);

  double absmin(const ParallelVector& a);
  double absmax(const ParallelVector& a);

  void mult_trans(ParallelMatrix& a, ParallelVector& b, ParallelVector& r);

  void diag(ParallelMatrix& a, ParallelVector& r);
  void zeros(ParallelVector& r);

  void absthreshold_invert(const ParallelVector& a, ParallelVector& r, double threshold);

  double dot(const ParallelVector& a, const ParallelVector& b);
  double norm(const ParallelVector& a);
  double max(const ParallelVector& a);

  void mult(const ParallelMatrix& a, const ParallelVector& b, ParallelVector& r);

  void absdiag(const ParallelMatrix& a, ParallelVector& r);

  void ones(ParallelVector& r);

  int  proc() { return proc_; }
  int  nproc() { return nproc_; }

  bool first() { return proc_ == 0; }
  void wait();

private:
  double reduce_sum(double val);
  double reduce_min(double val);
  double reduce_max(double val);

  ParallelLinearAlgebraSharedData& data_;

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
