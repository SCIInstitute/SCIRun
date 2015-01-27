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
/// @todo Documentation Modules/Math/EvaluateLinearAlgebraUnary.cc

#include <iostream>
#include <stdexcept>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/DenseMatrix.h> /// @todo: try to remove this--now it's needed to convert pointers, but actually this module shouldn't need the full def of DenseMatrix.

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

EvaluateLinearAlgebraUnaryModule::EvaluateLinearAlgebraUnaryModule() :
  Module(ModuleLookupInfo("EvaluateLinearAlgebraUnary", "Math", "SCIRun"))
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(Result);
}

void EvaluateLinearAlgebraUnaryModule::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Variables::Operator, 0);
  state->setValue(Variables::ScalarValue, 0);
	state->setValue(Variables::FunctionString, std::string("x+10")); 
}

void EvaluateLinearAlgebraUnaryModule::execute()
{
  auto denseInput = getRequiredInput(InputMatrix);

  /// @todo: standardize on camel case!
  auto state = get_state();
  auto oper = state->getValue(Variables::Operator).toInt();
  double scalar = state->getValue(Variables::ScalarValue).toDouble();
	auto func = state->getValue(Variables::FunctionString).toString(); 

  algo().set(Variables::Operator, oper);
  algo().set(Variables::ScalarValue, scalar);
	algo().set(Variables::FunctionString, func); 
  auto output = algo().run_generic(withInputData((InputMatrix, denseInput)));
  sendOutputFromAlgorithm(Result, output);
}

