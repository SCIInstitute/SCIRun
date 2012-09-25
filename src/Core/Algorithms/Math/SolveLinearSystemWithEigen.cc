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
#include <Eigen/Sparse>

using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

SolveLinearSystemAlgorithm::Outputs SolveLinearSystemAlgorithm::run(const Inputs& input, const Parameters& params) const
{
  Eigen::ConjugateGradient<DenseMatrix::EigenBase> cg;
  cg.compute(*input.get<0>());

  if (cg.info() != Eigen::Success)
    return SolveLinearSystemAlgorithm::Outputs();

  cg.setTolerance(params.get<0>());
  cg.setMaxIterations(params.get<1>());
  auto x = cg.solve(*input.get<1>());
  //TODO: move ctor
  return SolveLinearSystemAlgorithm::Outputs(new DenseColumnMatrix(x));
}