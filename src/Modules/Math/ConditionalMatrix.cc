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
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Math/ConditionalMatrixAlgo.h>
#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

/// @class ConditionalMatrix
/// This module is for comparing two matrices and sending outputs

MODULE_INFO_DEF(ConditionalMatrix, Math, SCIRun)

ConditionalMatrix::ConditionalMatrix() : Module(staticInfo_)
{
  INITIALIZE_PORT(MatrixA);
  INITIALIZE_PORT(MatrixB);
  INITIALIZE_PORT(PossibleOutput);
  INITIALIZE_PORT(OutputMatrix);
  INITIALIZE_PORT(Solution);
}

void ConditionalMatrix::setStateDefaults()
{
  setStateStringFromAlgoOption(Variables::Operator);
  setStateStringFromAlgoOption(Variables::Method);
  setStateStringFromAlgoOption(Variables::ObjectInfo);
  setStateStringFromAlgoOption(Variables::FormatString);
  setStateStringFromAlgoOption(Variables::FunctionString);
}

void
ConditionalMatrix::execute()
{
  auto matrixA = getRequiredInput(MatrixA);
  auto matrixB = getOptionalInput(MatrixB);
  auto possout = getOptionalInput(PossibleOutput);

  if (needToExecute())
  {
      
    setAlgoOptionFromState(Variables::Operator);
    setAlgoOptionFromState(Variables::Method);
    setAlgoOptionFromState(Variables::ObjectInfo);
    setAlgoOptionFromState(Variables::FormatString);
    setAlgoOptionFromState(Variables::FunctionString);
    

    auto output = algo().run(withInputData((Variables::FirstMatrix, matrixA)(Variables::SecondMatrix, optionalAlgoInput(matrixB))(Variables::InputMatrix, optionalAlgoInput(possout))));
    
//    Check for quit condition and then quit.  Arguably the most useful part of the module
    auto state = get_state();
    std::string then_statement = state->getValue(Variables::FormatString).toString();
    std::string else_statement = state->getValue(Variables::FunctionString).toString();
    
    
    
    if (then_statement == "quit" || else_statement == "quit")
    {
      auto out_mat=output.get<DenseMatrix>(Variables::Solution);
      
//      auto out  = castMatrix::toDense (out_mat);
//      double *data=out->data();
      std::cout<<"cond matrix :"<<out_mat<<std::endl;
      
//      if ((data[0]==1 && then_statement == "quit") || (data[0]==0 && else_statement == "quit"))
//      {
//        std::cout<<"ordered to quit. Not implemented yet"<<std::endl;
//      }
      
    }
      

    sendOutputFromAlgorithm(OutputMatrix, output);
    sendOutputFromAlgorithm(Solution, output);

  }
}
