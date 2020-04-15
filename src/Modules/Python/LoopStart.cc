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


#include <Modules/Python/LoopStart.h>
#include <Modules/Python/PythonObjectForwarder.h>
#include <Modules/Python/ModuleStateModifierTester.h>
#include <Modules/Python/InterfaceWithPython.h>
#ifdef BUILD_WITH_PYTHON
#include <Modules/Python/PythonInterfaceParser.h>
#include <Core/Python/PythonInterpreter.h>
#include <Core/Logging/Log.h>
#include <Core/Datatypes/MetadataObject.h>
#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>
#endif

using namespace SCIRun;
using namespace SCIRun::Modules::Python;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Python;

ALGORITHM_PARAMETER_DEF(Python, LoopStartCode);
ALGORITHM_PARAMETER_DEF(Python, IterationCount);
ALGORITHM_PARAMETER_DEF(Python, LoopIncrementCode);
ALGORITHM_PARAMETER_DEF(Python, LoopOutputCode);

MODULE_INFO_DEF(LoopStart, Python, SCIRun)

LoopStart::LoopStart() : Module(staticInfo_)
{
  INITIALIZE_PORT(LoopStartCodeObject);
  INITIALIZE_PORT(LoopEndCodeObject);
  INITIALIZE_PORT(PythonMatrix1);
  INITIALIZE_PORT(PythonField1);
  INITIALIZE_PORT(PythonString1);
  INITIALIZE_PORT(PythonMatrix2);
  INITIALIZE_PORT(PythonField2);
  INITIALIZE_PORT(PythonString2);

#ifdef BUILD_WITH_PYTHON
  translator_.reset(new InterfaceWithPythonCodeTranslatorImpl([this]() { return id().id_; }, get_state(),
    { Parameters::PythonOutputMatrix1Name, Parameters::PythonOutputMatrix2Name,
    Parameters::PythonOutputField1Name, Parameters::PythonOutputField2Name,
    Parameters::PythonOutputString1Name, Parameters::PythonOutputString2Name}));
#endif
}

void LoopStart::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Parameters::LoopStartCode, std::string("# Insert your loop start Python code here. The SCIRun API package is automatically imported."));
  state->setValue(Parameters::LoopIncrementCode, std::string("# Insert your loop increment Python code here. The SCIRun API package is automatically imported."));
  state->setValue(Parameters::LoopOutputCode, std::string("# Insert your loop output variable Python code here. Use the InterfaceWithPython default names for now."));
  state->setValue(Parameters::IterationCount, 0);
  state->setValue(Parameters::NumberOfRetries, 5);
  state->setValue(Parameters::PollingIntervalMilliseconds, 10);

  state->setValue(Parameters::PythonOutputField1Name, std::string("fieldOutput1"));
  state->setValue(Parameters::PythonOutputField2Name, std::string("fieldOutput2"));

  state->setValue(Parameters::PythonOutputString1Name, std::string("stringOutput1"));
  state->setValue(Parameters::PythonOutputString2Name, std::string("stringOutput2"));

  state->setValue(Parameters::PythonOutputMatrix1Name, std::string("matrixOutput1"));
  state->setValue(Parameters::PythonOutputMatrix2Name, std::string("matrixOutput2"));
}

void LoopStart::postStateChangeInternalSignalHookup()
{
  setProgrammableInputPortEnabled(true);
  get_state()->setValue(Parameters::IterationCount, 0);
}

void LoopStart::execute()
{
#ifdef BUILD_WITH_PYTHON
  auto loopInfo = getOptionalInput(LoopEndCodeObject);
  //if (needToExecute())
  {
    auto state = get_state();
    auto startCode = state->getValue(Parameters::LoopStartCode).toString();
    auto incrCode = state->getValue(Parameters::LoopIncrementCode).toString();

    const auto counter = state->getValue(Parameters::IterationCount).toInt();
    auto code = 0 == counter ? startCode : incrCode;

    sendOutput(LoopStartCodeObject, boost::make_shared<PythonExecutingMetadataObject>(code));
    state->setValue(Parameters::IterationCount, counter + 1);

    if (counter > 0)
    {
      auto outputCode = state->getValue(Parameters::LoopOutputCode).toString();
      //logCritical("LoopStart outputCode: {}", outputCode);
      auto convertedCode = translator_->translate(outputCode);
      //logCritical("LoopStart converted outputCode: {}", convertedCode.code);
      NetworkEditorPythonAPI::PythonModuleContextApiDisabler disabler;
      PythonInterpreter::Instance().run_script(convertedCode.code);

      PythonObjectForwarderImpl<LoopStart> impl(*this);

      DummyPortName nil;
      if (oport_connected(PythonString1))
       impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputString1Name).toString(), PythonString1, nil, nil);
      if (oport_connected(PythonString2))
       impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputString2Name).toString(), PythonString2, nil, nil);
      if (oport_connected(PythonMatrix1))
       impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputMatrix1Name).toString(), nil, PythonMatrix1, nil);
      if (oport_connected(PythonMatrix2))
       impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputMatrix2Name).toString(), nil, PythonMatrix2, nil);
      if (oport_connected(PythonField1))
       impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputField1Name).toString(), nil, nil, PythonField1);
      if (oport_connected(PythonField2))
       impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputField2Name).toString(), nil, nil, PythonField2);
    }
  }
#else
  error("This module does nothing, turn on BUILD_WITH_PYTHON to enable.");
#endif
}
