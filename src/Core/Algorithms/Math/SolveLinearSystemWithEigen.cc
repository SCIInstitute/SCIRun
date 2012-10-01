/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Math/SolveLinearSystemWithEigen.h>
#include <Core/Datatypes/DenseMatrix.h>
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
    DenseColumnMatrix solveWithEigen(const MatrixType& lhs)
    {
      Eigen::ConjugateGradient<typename MatrixType::EigenBase> cg;
      cg.compute(lhs);

      if (cg.info() != Eigen::Success)
        BOOST_THROW_EXCEPTION(AlgorithmInputException() 
          << LinearAlgebraErrorMessage("Conjugate gradient initialization was unsuccessful")
          << EigenComputationInfo(cg.info()));

      cg.setTolerance(tolerance_);
      cg.setMaxIterations(maxIterations_);
      return cg.solve(rhs_);
    }
  private:
    const DenseColumnMatrix& rhs_;
    double tolerance_;
    int maxIterations_;
  };
}

SolveLinearSystemAlgorithm::Outputs SolveLinearSystemAlgorithm::run(const Inputs& input, const Parameters& params) const
{
  //TODO: make convenience macros for these common error conditions (also to increase readability)
  MatrixConstHandle A = input.get<0>();
  if (!A)
    BOOST_THROW_EXCEPTION(AlgorithmInputException() << NullObjectInfo("Null input matrix"));

  auto b = input.get<1>();
  if (!b)
    BOOST_THROW_EXCEPTION(AlgorithmInputException() << NullObjectInfo("Null rhs vector"));
  
  double tolerance = params.get<0>();
  if (tolerance < 0)
    BOOST_THROW_EXCEPTION(AlgorithmInputException() << DoubleOutOfRangeInfo(
      DoubleOutOfRangeInfo::value_type(
        std::string("Tolerance out of range!"), 
        tolerance, 
        boost::numeric::interval<double>(0, std::numeric_limits<double>::infinity()))));

  int maxIterations = params.get<1>();
  if (maxIterations < 0)
    BOOST_THROW_EXCEPTION(AlgorithmInputException() << IntOutOfRangeInfo(
      IntOutOfRangeInfo::value_type(
        std::string("Max iterations out of range!"), 
        maxIterations, 
         boost::numeric::interval<int>(0, std::numeric_limits<int>::infinity()))));

  SolveLinearSystemAlgorithmEigenCGImpl impl(*b, tolerance, maxIterations);
  DenseColumnMatrix x;
  if (matrix_is::dense(A))
    x = impl.solveWithEigen(*matrix_cast::as_dense(A));
  else if (matrix_is::sparse(A))
    x = impl.solveWithEigen(*matrix_cast::as_sparse(A));
  //TODO: move ctor
  if (x.size())
    return SolveLinearSystemAlgorithm::Outputs(new DenseColumnMatrix(x));
  else
    return SolveLinearSystemAlgorithm::Outputs();
}