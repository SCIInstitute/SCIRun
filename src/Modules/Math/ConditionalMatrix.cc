/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Modules/Math/ConditionalMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Module.h>
#include <Core/Algorithms/Math/ConditionalMatrixAlgo.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

/// @class ConditionalMatrix
/// @brief This module sorts the matrix entries into ascending or descending order.

MODULE_INFO_DEF(ConditionalMatrix, Math, SCIRun)

ConditionalMatrix::ConditionalMatrix() : Module(staticInfo_)
{
  INITIALIZE_PORT(MatrixA);
  INITIALIZE_PORT(MatrixB);
  INITIALIZE_PORT(PossibleOutput);
  INITIALIZE_PORT(OutputMatrix);
  INITIALIZE_PORT(ConditionMatrix);
}

void ConditionalMatrix::setStateDefaults()
{
  setStateStringFromAlgoOption(Parameters::Operator);
  setStateStringFromAlgoOption(Parameters::Method);
  setStateStringFromAlgoOption(Parameters::Method2);
  setStateStringFromAlgoOption(Parameters::OutputFormat);
  setStateStringFromAlgoOption(Parameters::ObjectInfo);
}

void
ConditionalMatrix::execute()
{
  auto matrixA = getRequiredInput(MatrixA);
  auto matrixB = getOptionalInput(MatrixB);
  auto possout = getOptionalInput(PossibleOutput);

  if (needToExecute())
  {
    update_state(Executing);
      
    setAlgoOptionFromState(Variables::Operator);
    setAlgoOptionFromState(Variables::Method);
    setAlgoOptionFromState(Variables::Method2);
    setAlgoOptionFromState(Variables::OutputFormat);
    setAlgoOptionFromState(Variables::ObjectInfo);

    auto Output = algo().run(withInputData((MatrixA, matrixA)(MatrixB, optionalAlgoInput(matrixB))(PossibleOutput, optionalAlgoInput(possout))));

    sendOutputFromAlgorithm(OutputMatrix, output);
    sendOutputFromAlgorithm(ConditionMatrix, output);

  }
}
