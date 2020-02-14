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


#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Math/SolveLinearSystemWithEigen.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Eigen/Sparse>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core;

namespace
{
  using EigenComputationInfo = boost::error_info<struct tag_eigen_computation, Eigen::ComputationInfo>;

  template <class ColumnMatrixType, template <typename> class SolverType>
  class SolveLinearSystemAlgorithmEigenCGImpl
  {
  public:
    SolveLinearSystemAlgorithmEigenCGImpl(SharedPointer<ColumnMatrixType> rhs, double tolerance, int maxIterations) :
        tolerance_(tolerance), maxIterations_(maxIterations), rhs_(rhs) {}

    using SolutionType = ColumnMatrixType;

    template <class MatrixType>
    typename ColumnMatrixType::EigenBase solveWithEigen(const MatrixType& lhs)
    {
      SolverType<typename MatrixType::EigenBase> solver;
      solver.compute(lhs);

      if (solver.info() != Eigen::Success)
        BOOST_THROW_EXCEPTION(AlgorithmInputException()
          << LinearAlgebraErrorMessage("Eigen solver initialization was unsuccessful")
          << EigenComputationInfo(solver.info()));

      solver.setTolerance(tolerance_);
      solver.setMaxIterations(maxIterations_);
      auto solution = solver.solve(*rhs_).eval();
      tolerance_ = solver.error();
      maxIterations_ = solver.iterations();
      return solution;
    }

    double tolerance_;
    int maxIterations_;
  private:
    SharedPointer<ColumnMatrixType> rhs_;
  };
}

SolveLinearSystemAlgorithm::Outputs SolveLinearSystemAlgorithm::run(const Inputs& input, const Parameters& params) const
{
  return runImpl<Inputs, Outputs>(input, params);
}

SolveLinearSystemAlgorithm::ComplexOutputs SolveLinearSystemAlgorithm::run(const ComplexInputs& input, const Parameters& params) const
{
  return runImpl<ComplexInputs, ComplexOutputs>(input, params);
}

template <typename T>
using CG = Eigen::ConjugateGradient<T>;
// Not available yet, need to upgrade Eigen
// template <typename T>
// using LSCG = Eigen::LeastSquaresConjugateGradient<T>;
template <typename T>
using BiCG = Eigen::BiCGSTAB<T>;

template <typename In, typename Out>
Out SolveLinearSystemAlgorithm::runImpl(const In& input, const Parameters& params) const
{
  auto A = std::get<0>(input);
  ENSURE_ALGORITHM_INPUT_NOT_NULL(A, "Null input matrix");

  auto b = std::get<1>(input);
  ENSURE_ALGORITHM_INPUT_NOT_NULL(b, "Null rhs vector");

  double tolerance = std::get<0>(params);
  ENSURE_POSITIVE_DOUBLE(tolerance, "Tolerance out of range!");

  int maxIterations = std::get<1>(params);
  ENSURE_POSITIVE_INT(maxIterations, "Max iterations out of range!");

  auto method = std::get<2>(params);

  using SolutionType = DenseColumnMatrixGeneric<typename std::tuple_element<0, In>::type::element_type::value_type>;
  using AlgoTypeCG = SolveLinearSystemAlgorithmEigenCGImpl<SolutionType, CG>;
  using AlgoTypeBiCG = SolveLinearSystemAlgorithmEigenCGImpl<SolutionType, BiCG>;

  if ("cg" == method)
    return solve<AlgoTypeCG, In, Out>(input, params);
  else if ("bicg" == method)
    return solve<AlgoTypeBiCG, In, Out>(input, params);
  else
  {
    BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("Need to upgrade Eigen for LSCG."));
  }
}

template <typename SolverType, typename In, typename Out>
Out SolveLinearSystemAlgorithm::solve(const In& input, const Parameters& params) const
{
  auto A = std::get<0>(input);
  auto b = std::get<1>(input);
  double tolerance = std::get<0>(params);
  int maxIterations = std::get<1>(params);

  SolverType impl(b, tolerance, maxIterations);

  typename SolverType::SolutionType x;
  if (matrixIs::dense(A))
  {
    auto dense = castMatrix::toDense(A);
    x = impl.solveWithEigen(*dense);
  }
  else if (matrixIs::sparse(A))
  {
    auto sparse = castMatrix::toSparse(A);
    x = impl.solveWithEigen(*sparse);
  }
  else
    BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("solveWithEigen can only handle dense and sparse matrices."));

  if (x.size() != 0)
  {
    auto solution(boost::make_shared<typename SolverType::SolutionType>(x));
    return Out(solution, impl.tolerance_, impl.maxIterations_);
  }
  else
    BOOST_THROW_EXCEPTION(AlgorithmProcessingException() << ErrorMessage("solveWithEigen produced an empty solution."));
}

AlgorithmOutput SolveLinearSystemAlgorithm::run(const AlgorithmInput& input) const
{
  throw 2;
}
