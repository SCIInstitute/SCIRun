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


#include <Modules/Math/BooleanCompare.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Math/BooleanCompareAlgo.h>
#include <Core/Logging/Log.h>
#include <spdlog/fmt/ostr.h>

#ifdef BUILD_WITH_PYTHON
#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>
#endif

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

/// @class BooleanCompare
/// This module is for comparing two matrices and sending outputs

MODULE_INFO_DEF(BooleanCompare, Flow Control, SCIRun)

BooleanCompare::BooleanCompare() : Module(staticInfo_)
{
  INITIALIZE_PORT(MatrixA);
  INITIALIZE_PORT(MatrixB);
  INITIALIZE_PORT(PossibleOutput);
  INITIALIZE_PORT(OutputMatrix);
  INITIALIZE_PORT(BooleanResult);
}

void BooleanCompare::setStateDefaults()
{
  setStateStringFromAlgoOption(Parameters::Comparison_Option);
  setStateStringFromAlgoOption(Parameters::Value_Option_1);
  setStateStringFromAlgoOption(Parameters::Value_Option_2);
  setStateStringFromAlgoOption(Parameters::Then_Option);
  setStateStringFromAlgoOption(Parameters::Else_Option);
}

void
BooleanCompare::execute()
{
  auto matrixA = getRequiredInput(MatrixA);
  auto matrixB = getOptionalInput(MatrixB);
  auto possout = getOptionalInput(PossibleOutput);

  if (needToExecute())
  {

    setAlgoOptionFromState(Parameters::Comparison_Option);
    setAlgoOptionFromState(Parameters::Value_Option_1);
    setAlgoOptionFromState(Parameters::Value_Option_2);
    setAlgoOptionFromState(Parameters::Then_Option);
    setAlgoOptionFromState(Parameters::Else_Option);


    auto output = algo().run(withInputData((Variables::FirstMatrix, matrixA)(Variables::SecondMatrix, optionalAlgoInput(matrixB))(Variables::InputMatrix, optionalAlgoInput(possout))));

//    Check for quit condition and then quit.  Arguably the most useful part of the module
    auto state = get_state();
    std::string then_statement = state->getValue(Parameters::Then_Option).toString();
    std::string else_statement = state->getValue(Parameters::Else_Option).toString();

    auto out_mat=output.get<DenseMatrix>(Variables::Solution);
    double *data=out_mat->data();

    if ((data[0]==1 && then_statement == "quit") || (data[0]==0 && else_statement == "quit"))
    {


#ifdef BUILD_WITH_PYTHON
//      quit nicely
      LOG_DEBUG("cond matrix : {}", *out_mat);
      remark("Ordered to quit. Trying to quit.");
      std::ostringstream ostr;
      std::string q_message = NetworkEditorPythonAPI::quit(false);
      ostr << "Quit message = "<<q_message;
      remark(ostr.str());
#else
      remark(" cannot quit without building with python");
#endif


    }


    sendOutputFromAlgorithm(OutputMatrix, output);
    sendOutput(BooleanResult, out_mat);

  }
}
