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
#include <stdexcept>
#include <Modules/Math/EvaluateLinearAlgebraBinary.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinary.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/DenseMatrix.h> //TODO: try to remove this--now it's needed to convert pointers, but actually this module shouldn't need the full def of DenseMatrix.

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

EvaluateLinearAlgebraBinaryModule::EvaluateLinearAlgebraBinaryModule() :
Module(ModuleLookupInfo("EvaluateLinearAlgebraBinary", "Math", "SCIRun"))
{
}

void EvaluateLinearAlgebraBinaryModule::execute()
{
  auto lhs = getRequiredInput(InputLHS);
  auto rhs = getRequiredInput(InputRHS);

  ModuleStateHandle state = get_state();
  EvaluateLinearAlgebraBinaryAlgorithm::Parameters oper = (EvaluateLinearAlgebraBinaryAlgorithm::Parameters)
    state->getValue
    (EvaluateLinearAlgebraBinaryAlgorithm::OperatorName).getInt();

  //psuedo-code
  //this line will
  // 1. get the current value in the appropriate type
  // 2. 
  //EvaluateLinearAlgebraBinaryAlgorithm::Parameters oper = state->getOperatorName();


  EvaluateLinearAlgebraBinaryAlgorithm algo; //TODO inject
  DenseMatrixHandle output = algo.run(EvaluateLinearAlgebraBinaryAlgorithm::Inputs(lhs, rhs), oper); 

  send_output_handle(0, output);
}