/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#include <Modules/Python/InterfaceWithPython.h>
#include <Modules/Python/PythonObjectForwarder.h>
#include <Core/Python/PythonInterpreter.h>
#include <boost/algorithm/string.hpp>

using namespace SCIRun::Modules::Python;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Python;

ALGORITHM_PARAMETER_DEF(Python, PythonCode);
ALGORITHM_PARAMETER_DEF(Python, PythonString1Name);
ALGORITHM_PARAMETER_DEF(Python, PythonString2Name);
ALGORITHM_PARAMETER_DEF(Python, PythonString3Name);
ALGORITHM_PARAMETER_DEF(Python, PythonMatrix1Name);
ALGORITHM_PARAMETER_DEF(Python, PythonMatrix2Name);
ALGORITHM_PARAMETER_DEF(Python, PythonMatrix3Name);
ALGORITHM_PARAMETER_DEF(Python, PythonField1Name);
ALGORITHM_PARAMETER_DEF(Python, PythonField2Name);
ALGORITHM_PARAMETER_DEF(Python, PythonField3Name);

const ModuleLookupInfo InterfaceWithPython::staticInfo_("InterfaceWithPython", "Python", "SCIRun");
Mutex InterfaceWithPython::lock_("InterfaceWithPython");

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
}

void InterfaceWithPython::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::PythonCode, std::string("# Insert Python code here using the SCIRun API."));
  state->setValue(Parameters::PollingIntervalMilliseconds, 200);
  state->setValue(Parameters::NumberOfRetries, 50);

  state->setValue(Parameters::PythonField1Name, std::string("f1"));
  state->setValue(Parameters::PythonField2Name, std::string("f2"));
  state->setValue(Parameters::PythonField3Name, std::string("f3"));

  state->setValue(Parameters::PythonString1Name, std::string("s1"));
  state->setValue(Parameters::PythonString2Name, std::string("s2"));
  state->setValue(Parameters::PythonString3Name, std::string("s3"));

  state->setValue(Parameters::PythonMatrix1Name, std::string("m1"));
  state->setValue(Parameters::PythonMatrix2Name, std::string("m2"));
  state->setValue(Parameters::PythonMatrix3Name, std::string("m3"));
}

std::string InterfaceWithPython::convertInputOutputSyntax(const std::string& code) const
{
  return code;
}

void InterfaceWithPython::execute()
{
  auto state = get_state();
  {
    Guard g(lock_.get());
    
    auto code = state->getValue(Parameters::PythonCode).toString();
    std::vector<std::string> lines;
    boost::split(lines, code, boost::is_any_of("\n"));
    for (const auto& line : lines)
    {
      auto converted = convertInputOutputSyntax(line);
      PythonInterpreter::Instance().run_string(converted);
    }
  }

  //TODO: support multiple output objects
  PythonObjectForwarderImpl<InterfaceWithPython> impl(*this);

  if (oport_connected(PythonString1))
    impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonString1Name).toString(), PythonString1, PythonMatrix1, PythonField1);
  if (oport_connected(PythonString2))
    impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonString2Name).toString(), PythonString2, PythonMatrix1, PythonField1);
  if (oport_connected(PythonString3))
    impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonString3Name).toString(), PythonString3, PythonMatrix1, PythonField1);
  if (oport_connected(PythonMatrix1))
    impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonMatrix1Name).toString(), PythonString1, PythonMatrix1, PythonField1);
  if (oport_connected(PythonMatrix2))
    impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonMatrix2Name).toString(), PythonString1, PythonMatrix2, PythonField1);
  if (oport_connected(PythonMatrix3))
    impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonMatrix3Name).toString(), PythonString1, PythonMatrix3, PythonField1);
  if (oport_connected(PythonField1))
    impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonField1Name).toString(), PythonString1, PythonMatrix1, PythonField1);
  if (oport_connected(PythonField2))
    impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonField2Name).toString(), PythonString1, PythonMatrix1, PythonField2);
  if (oport_connected(PythonField3))
    impl.waitForOutputFromTransientState(state->getValue(Parameters::PythonField3Name).toString(), PythonString1, PythonMatrix1, PythonField3);
}

