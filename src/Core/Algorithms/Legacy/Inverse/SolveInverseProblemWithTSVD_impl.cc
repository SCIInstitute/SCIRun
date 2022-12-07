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

         Author: 							Jaume Coll-Font, Yesim
   Serinagaoglu & Alireza Ghodrati Last Modification:		September 6 2017
*/

#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Logging/LoggerInterface.h>

// Tikhonov inverse libraries
#include <Core/Algorithms/Legacy/Inverse/SolveInverseProblemWithTSVD_impl.h>
#include <Core/Algorithms/Legacy/Inverse/TikhonovAlgoAbstractBase.h>

// EIGEN LIBRARY
#include <Eigen/Eigen>
#include <Eigen/SVD>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
// using namespace SCIRun::Modules::Inverse;
// using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Inverse;

///////////////////////////////////////////////////////////////////
/////// prealocate Matrices for inverse compuation
///     This function precalcualtes the SVD of the forward matrix and prepares singular vectors and
///     values for posterior computations
///////////////////////////////////////////////////////////////////
void SolveInverseProblemWithTSVD_impl::preAlocateInverseMatrices(const DenseMatrix&,
    const DenseMatrix& measuredData_, const DenseMatrix&, const DenseMatrix&,
    const DenseMatrix& matrixU_, const DenseMatrix& singularValues_, const DenseMatrix& matrixV_)
{
  // alocate U and V matrices
  svd_MatrixU = matrixU_;
  svd_MatrixV = matrixV_;

  // alocate singular values
  if (singularValues_.ncols() == 1) { svd_SingularValues = singularValues_; }
  else
  {
    svd_SingularValues = singularValues_.diagonal();
  }

  // Compute the projection of data y on the left singular vectors
  Uy = svd_MatrixU.transpose() * (measuredData_);

  // determine rank
  rank = svd_SingularValues.nrows();
}

void SolveInverseProblemWithTSVD_impl::preAlocateInverseMatrices(const DenseMatrix& forwardMatrix_,
    const DenseMatrix& measuredData_, const DenseMatrix&, const DenseMatrix&)
{
  // Compute the SVD of the forward matrix
  Eigen::JacobiSVD<DenseMatrix::EigenBase> SVDdecomposition(
      forwardMatrix_, Eigen::ComputeFullU | Eigen::ComputeFullV);

  // alocate the left and right singular vectors and the singular values
  svd_MatrixU = SVDdecomposition.matrixU();
  svd_MatrixV = SVDdecomposition.matrixV();
  svd_SingularValues = SVDdecomposition.singularValues();

  // determine rank
  rank = SVDdecomposition.nonzeroSingularValues();

  // Compute the projection of data y on the left singular vectors
  Uy = svd_MatrixU.transpose() * (measuredData_);
}

//////////////////////////////////////////////////////////////////////
// THIS FUNCTION returns regularized solution by tikhonov method
//////////////////////////////////////////////////////////////////////
DenseMatrix SolveInverseProblemWithTSVD_impl::computeInverseSolution(
    double lambda, bool inverseCalculation) const
{
  // prealocate matrices
  const int N = svd_MatrixV.cols();
  const int M = svd_MatrixU.rows();
  const int numTimeSamples = Uy.ncols();
  DenseMatrix solution(DenseMatrix::Zero(N, numTimeSamples));
  DenseMatrix tempInverse(DenseMatrix::Zero(N, M));

  const int truncationPoint =
      std::min({static_cast<int>(lambda), rank, static_cast<int>(9999999999999)});

  // Compute inverse SolveInverseProblemWithTSVD
  for (int rr = 0; rr < truncationPoint; rr++)
  {
    // evaluate filter factor
    double singVal = svd_SingularValues[rr];
    auto filterFactor_i = 1 / (singVal);

    // update solution
    solution += filterFactor_i * svd_MatrixV.col(rr) * Uy.row(rr);

    // update inverse operator
    if (inverseCalculation)
      tempInverse += filterFactor_i * (svd_MatrixV.col(rr) * svd_MatrixU.col(rr).transpose());
  }

  // output solutions
  //   if (inverseCalculation)
  //       inverseMatrix_.reset( new DenseMatrix(tempInverse) );

  return solution;
}

//////////////////////////////////////////////////////////////////////
// THIS FUNCTION returns a string of lambdas from which the L-curve is computed
//////////////////////////////////////////////////////////////////////
std::vector<double> SolveInverseProblemWithTSVD_impl::computeLambdaArray(
    double lambdaMin, double, int nLambda) const
{
  std::vector<double> lambdaArray(nLambda, 0.0);
  const double lam_step = 1;

  lambdaArray[0] = lambdaMin;
  for (int j = 1; j < nLambda; j++) { lambdaArray[j] = lambdaArray[j - 1] + lam_step; }
  return lambdaArray;
}
