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

#ifndef ALGORITHMS_MATH_SOLVELINEARSYSTEMWITHEIGEN_H
#define ALGORITHMS_MATH_SOLVELINEARSYSTEMWITHEIGEN_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
/// @todo: move
#include <Eigen/src/Core/util/Constants.h>
#include <Core/Algorithms/Math/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Math {
  
  /// @todo: this will be the base class of all the solvers. for now it will just contain the Eigen CG impl.
  class SCISHARE SolveLinearSystemAlgorithm : public AlgorithmBase
  {
  public:
    typedef boost::tuple<SCIRun::Core::Datatypes::MatrixHandle, SCIRun::Core::Datatypes::DenseColumnMatrixHandle> Inputs;
    typedef boost::tuple<double, int> Parameters;  
    typedef boost::tuple<SCIRun::Core::Datatypes::DenseColumnMatrixHandle, double, int> Outputs;

    Outputs run(const Inputs& input, const Parameters& params) const;

    AlgorithmOutput run(const AlgorithmInput& input) const;
  };

  typedef boost::error_info<struct tag_eigen_computation, Eigen::ComputationInfo> EigenComputationInfo;

}}}}

#endif