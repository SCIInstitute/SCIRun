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


#ifndef ALGORITHMS_MATH_EVALUATELINEARALGEBRAUNARY_H
#define ALGORITHMS_MATH_EVALUATELINEARALGEBRAUNARY_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Math/share.h>

/// \addtogroup Algorithms_Math
/// @{
///

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Math {

///
/// \class EvaluateLinearAlgebraUnaryAlgorithm
///
/// \brief Computes several unary operations on general matrices
///
/// \tparam _MatrixType the type of the matrix of which we are computing the
/// eigendecomposition; this is expected to be an instantiation of the Matrix
/// class template. Currently, only real matrices are supported.
///
/// The eigenvalues and eigenvectors of a matrix \f$ A \f$ are scalars
/// \f$ \lambda \f$ and vectors \f$ v \f$ such that \f$ Av = \lambda v \f$.  If
/// \f$ D \f$ is a diagonal matrix with the eigenvalues on the diagonal, and
/// \f$ V \f$ is a matrix with the eigenvectors as its columns, then \f$ A V =
/// V D \f$. The matrix \f$ V \f$ is almost always invertible, in which case we
/// have \f$ A = V D V^{-1} \f$. This is called the eigendecomposition.
///
/// Call the function compute() to compute the eigenvalues and eigenvectors of
/// a given matrix. Alternatively, you can use the
/// EigenSolver(const MatrixType&, bool) constructor which computes the
/// eigenvalues and eigenvectors at construction time. Once the eigenvalue and
/// eigenvectors are computed, they can be retrieved with the eigenvalues() and
/// eigenvectors() functions. The pseudoEigenvalueMatrix() and
/// pseudoEigenvectors() methods allow the construction of the
/// pseudo-eigendecomposition.
///
/// The documentation for EigenSolver(const MatrixType&, bool) contains an
/// example of the typical use of this class.
///
/// \note The implementation is adapted from
/// <a href="http://math.nist.gov/javanumerics/jama/">JAMA</a> (public domain).
/// Their code is based on EISPACK.
///
/// \sa MatrixBase::eigenvalues(), class ComplexEigenSolver, class SelfAdjointEigenSolver
///

  class SCISHARE EvaluateLinearAlgebraUnaryAlgorithm : public AlgorithmBase
  {
  public:
    enum Operator
    {
      NEGATE,
      TRANSPOSE,
      SCALAR_MULTIPLY,
			FUNCTION
    };

    typedef SCIRun::Core::Datatypes::MatrixHandle Inputs;
    struct Parameters { Operator op; double scalar; std::string func; };
    typedef SCIRun::Core::Datatypes::MatrixHandle Outputs;

    EvaluateLinearAlgebraUnaryAlgorithm();
    Outputs run(const Inputs& matrix, const Parameters& params) const;

    AlgorithmOutput run(const AlgorithmInput& input) const;
  };

}}}}


/*! @} End of Doxygen Groups*/

#endif
