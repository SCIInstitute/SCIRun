/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Math/SolveLinearSystemWithEigen.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Eigen/Sparse>

using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core;

namespace
{
  class SolveLinearSystemAlgorithmEigenCGImpl
  {
  public:
    SolveLinearSystemAlgorithmEigenCGImpl(const DenseColumnMatrix& rhs, double tolerance, int maxIterations) : 
        rhs_(rhs), tolerance_(tolerance), maxIterations_(maxIterations) {}

    template <class MatrixType>
    DenseColumnMatrix::EigenBase solveWithEigen(const MatrixType& lhs)
    {
      Eigen::ConjugateGradient<typename MatrixType::EigenBase> cg;
      cg.compute(lhs);

      if (cg.info() != Eigen::Success)
        BOOST_THROW_EXCEPTION(AlgorithmInputException() 
          << LinearAlgebraErrorMessage("Conjugate gradient initialization was unsuccessful")
          << EigenComputationInfo(cg.info()));

      cg.setTolerance(tolerance_);
      cg.setMaxIterations(maxIterations_);
      auto solution = cg.solve(rhs_).eval();
      tolerance_ = cg.error();
      maxIterations_ = cg.iterations();
      return solution;
    }

    double tolerance_;
    int maxIterations_;
  private:
    const DenseColumnMatrix& rhs_;
  };
}

SolveLinearSystemAlgorithm::Outputs SolveLinearSystemAlgorithm::run(const Inputs& input, const Parameters& params) const
{
  auto A = input.get<0>();
  ENSURE_ALGORITHM_INPUT_NOT_NULL(A, "Null input matrix");

  auto b = input.get<1>();
  ENSURE_ALGORITHM_INPUT_NOT_NULL(b, "Null rhs vector");
  
  double tolerance = params.get<0>();
  ENSURE_POSITIVE_DOUBLE(tolerance, "Tolerance out of range!");

  int maxIterations = params.get<1>();
  ENSURE_POSITIVE_INT(maxIterations, "Max iterations out of range!");

  SolveLinearSystemAlgorithmEigenCGImpl impl(*b, tolerance, maxIterations);
  DenseColumnMatrix x;
  if (matrixIs::dense(A))
  {
    x = impl.solveWithEigen(*castMatrix::toDense(A));
  }
  else if (matrixIs::sparse(A))
  {
    x = impl.solveWithEigen(*castMatrix::toSparse(A));
  }
  else
    BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("solveWithEigen can only handle dense and sparse matrices."));
  
  if (x.size() != 0)
  {
    /// @todo: move ctor
    DenseColumnMatrixHandle solution(boost::make_shared<DenseColumnMatrix>(x));
    return SolveLinearSystemAlgorithm::Outputs(solution, impl.tolerance_, impl.maxIterations_);
  }
  else
    BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("solveWithEigen produced an empty solution."));
}

AlgorithmOutput SolveLinearSystemAlgorithm::run(const AlgorithmInput& input) const
{
  throw 2;
}