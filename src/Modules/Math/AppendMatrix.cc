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


#include <Modules/Math/AppendMatrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun;

AppendMatrix::AppendMatrix() : Module(ModuleLookupInfo("AppendMatrix", "Math", "SCIRun"))
{
  INITIALIZE_PORT(FirstMatrix);
  INITIALIZE_PORT(SecondMatrix);
  INITIALIZE_PORT(InputMatrices);
  INITIALIZE_PORT(ResultMatrix);
}

void AppendMatrix::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Variables::RowsOrColumns, 0);
}

void AppendMatrix::execute()
{
  auto matrixLHS = getRequiredInput(FirstMatrix);
  auto matrixRHS = getRequiredInput(SecondMatrix);
  auto param = get_state()->getValue(Variables::RowsOrColumns).toInt();
  auto input_matrices = getOptionalDynamicInputs(InputMatrices);
  algo().set(Variables::RowsOrColumns, param);

  if (needToExecute())
  {
   AlgorithmOutput output;
   output = algo().run(withInputData((FirstMatrix, matrixLHS)(SecondMatrix, matrixRHS)(InputMatrices, input_matrices)));
   sendOutputFromAlgorithm(ResultMatrix, output);
  }
}
