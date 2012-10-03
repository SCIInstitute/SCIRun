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

#include <iostream>
#include <Modules/Math/SolveLinearSystem.h>
#include <Core/Algorithms/Math/SolveLinearSystemWithEigen.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;

SolveLinearSystemModule::SolveLinearSystemModule() : Module(ModuleLookupInfo("SolveLinearSystem", "Math", "SCIRun")) {}

//TODO: move
class matrix_convert
{
public:
  static DenseColumnMatrixConstHandle to_column(const MatrixConstHandle& mh)
  {
    auto col = matrix_cast::as_column(mh);
      if (col)
        return col;
    
    auto dense = matrix_cast::as_dense(mh);
    if (dense)
      return DenseColumnMatrixConstHandle(new DenseColumnMatrix(dense->col(0)));
    
    return DenseColumnMatrixConstHandle();
  }
};

void SolveLinearSystemModule::execute()
{
  auto A = getRequiredInput<Matrix>(0);
  auto rhs = getRequiredInput<Matrix>(1);

  if (rhs->ncols() != 1)
    BOOST_THROW_EXCEPTION(AlgorithmInputException() << ErrorMessage("Right hand side matrix must contain only one column."));

  auto rhsCol = matrix_cast::as_column(rhs);
  if (!rhsCol)
    rhsCol = matrix_convert::to_column(rhs);

  auto tolerance = get_state()->getValue(SolveLinearSystemAlgorithm::Tolerance).getDouble();
  std::cout << "-------SLS: tolerance = " << tolerance << std::endl;
  auto maxIterations = get_state()->getValue(SolveLinearSystemAlgorithm::MaxIterations).getInt();
  std::cout << "-------SLS: maxIter = " << maxIterations << std::endl;

  SolveLinearSystemAlgorithm algo;
  auto x = algo.run(
    SolveLinearSystemAlgorithm::Inputs(A, rhsCol), 
    SolveLinearSystemAlgorithm::Parameters(tolerance, maxIterations));

  send_output_handle(0, x);
}