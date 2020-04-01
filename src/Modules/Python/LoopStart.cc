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

MODULE_INFO_DEF(LoopStart, Python, SCIRun)

//Mutex InterfaceWithPython::lock_("InterfaceWithPython");
//bool InterfaceWithPython::matlabInitialized_{ false };

LoopStart::LoopStart() : Module(staticInfo_)
{
  INITIALIZE_PORT(LoopStartCodeObject);
  INITIALIZE_PORT(LoopIncrementCodeObject);
//
// #ifdef BUILD_WITH_PYTHON
//   translator_.reset(new InterfaceWithPythonCodeTranslatorImpl([this]() { return id().id_; }, get_state()));
// #endif
}

void LoopStart::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Parameters::LoopStartCode, std::string("# Insert your Python code here. The SCIRun API package is automatically imported."));
  state->setValue(Parameters::IterationCount, 0);
}

void LoopStart::postStateChangeInternalSignalHookup()
{
  setProgrammableInputPortEnabled(true);
  get_state()->setValue(Parameters::IterationCount, 0);
}

void LoopStart::execute()
{
#ifdef BUILD_WITH_PYTHON

  // auto matrices = getOptionalDynamicInputs(InputMatrix);
  // auto fields = getOptionalDynamicInputs(InputField);
  auto incrCode = getRequiredInput(LoopIncrementCodeObject);
  //if (needToExecute())
  {
    auto state = get_state();
    auto startCode = state->getValue(Parameters::LoopStartCode).toString();
    //remark(startCode);



    const auto counter = state->getValue(Parameters::IterationCount).toInt();
    auto code = 0 == counter ? startCode : incrCode->program();

    sendOutput(LoopStartCodeObject, boost::make_shared<PythonExecutingMetadataObject>(code));
    state->setValue(Parameters::IterationCount, counter + 1);
  }
#else
  error("This module does nothing, turn on BUILD_WITH_PYTHON to enable.");
#endif
}
