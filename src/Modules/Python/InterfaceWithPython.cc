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


#include <Modules/Python/InterfaceWithPython.h>
#include <Modules/Python/PythonObjectForwarder.h>
#ifdef BUILD_WITH_PYTHON
#include <Modules/Python/PythonInterfaceParser.h>
#include <Core/Python/PythonInterpreter.h>
#include <Core/Logging/Log.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <boost/algorithm/string.hpp>
#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>
#endif

using namespace SCIRun::Modules::Python;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Python;

ALGORITHM_PARAMETER_DEF(Python, PythonCode);
ALGORITHM_PARAMETER_DEF(Python, PythonTopLevelCode);
ALGORITHM_PARAMETER_DEF(Python, PythonInputStringNames);
ALGORITHM_PARAMETER_DEF(Python, PythonInputMatrixNames);
ALGORITHM_PARAMETER_DEF(Python, PythonInputFieldNames);
ALGORITHM_PARAMETER_DEF(Python, PythonOutputString1Name);
ALGORITHM_PARAMETER_DEF(Python, PythonOutputString2Name);
ALGORITHM_PARAMETER_DEF(Python, PythonOutputString3Name);
ALGORITHM_PARAMETER_DEF(Python, PythonOutputMatrix1Name);
ALGORITHM_PARAMETER_DEF(Python, PythonOutputMatrix2Name);
ALGORITHM_PARAMETER_DEF(Python, PythonOutputMatrix3Name);
ALGORITHM_PARAMETER_DEF(Python, PythonOutputField1Name);
ALGORITHM_PARAMETER_DEF(Python, PythonOutputField2Name);
ALGORITHM_PARAMETER_DEF(Python, PythonOutputField3Name);

MODULE_INFO_DEF(InterfaceWithPython, Python, SCIRun)

Mutex InterfaceWithPython::lock_("InterfaceWithPython");
bool InterfaceWithPython::matlabInitialized_{ false };

InterfaceWithPython::InterfaceWithPython() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(InputString);
  INITIALIZE_PORT(PythonMatrix1);
  INITIALIZE_PORT(PythonField1);
  INITIALIZE_PORT(PythonString1);
  INITIALIZE_PORT(PythonMatrix2);
  INITIALIZE_PORT(PythonField2);
  INITIALIZE_PORT(PythonString2);
  INITIALIZE_PORT(PythonMatrix3);
  INITIALIZE_PORT(PythonField3);
  INITIALIZE_PORT(PythonString3);

#ifdef BUILD_WITH_PYTHON
  translator_.reset(new InterfaceWithPythonCodeTranslatorImpl([this]() { return id().id_; }, get_state(), outputNameParameters()));
#endif
}

void InterfaceWithPython::setStateDefaults()
{
  auto state = get_state();

  state->setValue(Parameters::PythonCode, std::string("# Insert your Python code here. The SCIRun API package is automatically imported."));
  state->setValue(Parameters::PythonTopLevelCode,
    std::string("# Main namespace/top level context code goes here; for example, import statements and global variables.\n"
    "# This code will be executed before the 'Code' tab, and no input/output variables are available."));
  state->setValue(Parameters::PollingIntervalMilliseconds, 200);
  state->setValue(Parameters::NumberOfRetries, 50);

  state->setValue(Parameters::PythonOutputField1Name, std::string("fieldOutput1"));
  state->setValue(Parameters::PythonOutputField2Name, std::string("fieldOutput2"));
  state->setValue(Parameters::PythonOutputField3Name, std::string("fieldOutput3"));

  state->setValue(Parameters::PythonOutputString1Name, std::string("stringOutput1"));
  state->setValue(Parameters::PythonOutputString2Name, std::string("stringOutput2"));
  state->setValue(Parameters::PythonOutputString3Name, std::string("stringOutput3"));

  state->setValue(Parameters::PythonOutputMatrix1Name, std::string("matrixOutput1"));
  state->setValue(Parameters::PythonOutputMatrix2Name, std::string("matrixOutput2"));
  state->setValue(Parameters::PythonOutputMatrix3Name, std::string("matrixOutput3"));
}

std::vector<AlgorithmParameterName> InterfaceWithPython::outputNameParameters()
{
  return { Parameters::PythonOutputMatrix1Name, Parameters::PythonOutputMatrix2Name, Parameters::PythonOutputMatrix3Name,
    Parameters::PythonOutputField1Name, Parameters::PythonOutputField2Name, Parameters::PythonOutputField3Name,
    Parameters::PythonOutputString1Name, Parameters::PythonOutputString2Name, Parameters::PythonOutputString3Name};
}

std::vector<std::string> InterfaceWithPython::connectedPortIds() const
{
  std::vector<std::string> ids;
  for (const auto& port : inputPorts())
  {
    if (port->nconnections() > 0)
    {
      ids.push_back(port->id().toString());
    }
  }
  return ids;
}

void InterfaceWithPython::execute()
{
#ifdef BUILD_WITH_PYTHON
  auto matrices = getOptionalDynamicInputs(InputMatrix);
  auto fields = getOptionalDynamicInputs(InputField);
  auto strings = getOptionalDynamicInputs(InputString);
  if (needToExecute() || alwaysExecuteEnabled())
  {
    auto state = get_state();
    {
      Guard g(lock_.get());

      runTopLevelCode();

      translator_->updatePorts(connectedPortIds());
      auto code = state->getValue(Parameters::PythonCode).toString();
      auto convertedCode = translator_->translate(code);
      NetworkEditorPythonAPI::PythonModuleContextApiDisabler disabler;
      if (convertedCode.isMatlab && !matlabInitialized_)
      {
        PythonInterpreter::Instance().run_string("import matlab.engine");
        PythonInterpreter::Instance().run_string("__eng = matlab.engine.start_matlab()");
        PythonInterpreter::Instance().run_string("from MatlabConversion import *");
        matlabInitialized_ = true;
      }
      PythonInterpreter::Instance().run_script(convertedCode.code);
    }

    PythonObjectForwarderImpl<InterfaceWithPython> impl(*this);

    DummyPortName nil;
    if (oport_connected(PythonString1))
      impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputString1Name).toString(), PythonString1, nil, nil);
    if (oport_connected(PythonString2))
      impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputString2Name).toString(), PythonString2, nil, nil);
    if (oport_connected(PythonString3))
      impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputString3Name).toString(), PythonString3, nil, nil);
    if (oport_connected(PythonMatrix1))
      impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputMatrix1Name).toString(), nil, PythonMatrix1, nil);
    if (oport_connected(PythonMatrix2))
      impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputMatrix2Name).toString(), nil, PythonMatrix2, nil);
    if (oport_connected(PythonMatrix3))
      impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputMatrix3Name).toString(), nil, PythonMatrix3, nil);
    if (oport_connected(PythonField1))
      impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputField1Name).toString(), nil, nil, PythonField1);
    if (oport_connected(PythonField2))
      impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputField2Name).toString(), nil, nil, PythonField2);
    if (oport_connected(PythonField3))
      impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonOutputField3Name).toString(), nil, nil, PythonField3);
  }
#else
  error("This module does nothing, turn on BUILD_WITH_PYTHON to enable.");
#endif
}

void InterfaceWithPython::runTopLevelCode() const
{
#ifdef BUILD_WITH_PYTHON
  auto topLevelCode = cstate()->getValue(Parameters::PythonTopLevelCode).toString();
  std::vector<std::string> lines;
  boost::split(lines, topLevelCode, boost::is_any_of("\n"));
  for (const auto& line : lines)
  {
    PythonInterpreter::Instance().run_string(line);
  }
#endif
}
