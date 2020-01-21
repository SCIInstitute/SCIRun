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


#include <Modules/Math/SolveComplexLinearSystem.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Math/SolveLinearSystemWithEigen.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

MODULE_INFO_DEF(SolveComplexLinearSystem, Math, SCIRun)

SolveComplexLinearSystem::SolveComplexLinearSystem() : Module(staticInfo_)
{
  INITIALIZE_PORT(LHS);
  INITIALIZE_PORT(RHS);
  INITIALIZE_PORT(Solution);
}

void SolveComplexLinearSystem::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Variables::TargetError, 1e-5);
  state->setValue(Variables::MaxIterations, 500);
  state->setValue(Variables::Method, std::string("cg"));
}

void SolveComplexLinearSystem::execute()
{
  auto lhs = getRequiredInput(LHS);
  auto rhs = getRequiredInput(RHS);

  if (needToExecute())
  {
    SolveLinearSystemAlgorithm algo;
    auto col = convertMatrix::toColumn(rhs);
    auto input = std::make_tuple(lhs, col);
    auto tolerance = get_state()->getValue(Variables::TargetError).toDouble();
    auto maxIterations = get_state()->getValue(Variables::MaxIterations).toInt();
    auto method = get_state()->getValue(Variables::Method).toString();
    auto params = std::make_tuple(tolerance, maxIterations, method);
    std::cout << "Running Eigen solver with " <<
      std::get<0>(params) << ", " <<
      std::get<1>(params) << ", " <<
      std::get<2>(params) << std::endl;

    auto x = algo.run(input, params);

    auto solution = std::get<0>(x);
    std::cout << "error: " << std::get<1>(x) << std::endl;
    std::cout << "iterations: " << std::get<2>(x) << std::endl;
    sendOutput(Solution, solution);
  }
}
