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


#include <Modules/Python/ModuleStateModifierTester.h>
#include <Modules/Python/PythonObjectForwarder.h>
#ifdef BUILD_WITH_PYTHON
#include <Modules/Python/PythonInterfaceParser.h>
#include <Core/Python/PythonInterpreter.h>
#include <Core/Logging/Log.h>
#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>
#include <regex>
#endif

using namespace SCIRun;
using namespace SCIRun::Modules::Python;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Python;

ALGORITHM_PARAMETER_DEF(Python, StateModifyingCode);

MODULE_INFO_DEF(ModuleStateModifierTester, Python, SCIRun)

ModuleStateModifierTester::ModuleStateModifierTester() : Module(staticInfo_)
{
  INITIALIZE_PORT(MetadataCode);
}

void ModuleStateModifierTester::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Parameters::StateModifyingCode, std::string("# Insert your Python code here. The SCIRun API package is automatically imported."));
}

void ModuleStateModifierTester::postStateChangeInternalSignalHookup()
{
  setProgrammableInputPortEnabled(true);
}

Mutex PythonExecutingMetadataObject::lock_("PythonExecutingMetadataObject");

void PythonExecutingMetadataObject::process(const std::string& modId)
{
  MetadataObject::process(modId);
#ifdef BUILD_WITH_PYTHON
  auto progWithId = std::regex_replace(programData_, std::regex("\\%moduleId\\%"), "\"" + modId + "\"");
  //logCritical("Post-processed code: {}", progWithId);
  {
    NamedGuard g(lock_.get(), "PythonExecutingMetadataObject");
    PythonInterpreter::Instance().run_script(progWithId);
  }
  //logCritical("Done python execution.");
#endif
}

void ModuleStateModifierTester::execute()
{
#ifdef BUILD_WITH_PYTHON
  if (needToExecute())
  {
    auto code = get_state()->getValue(Parameters::StateModifyingCode).toString();
    remark(code);
    sendOutput(MetadataCode, boost::make_shared<PythonExecutingMetadataObject>(code));
  }
#else
  error("This module does nothing, turn on BUILD_WITH_PYTHON to enable.");
#endif
}
