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

const ModuleLookupInfo InterfaceWithPython::staticInfo_("InterfaceWithPython", "Python", "SCIRun");
Mutex InterfaceWithPython::lock_("InterfaceWithPython");

InterfaceWithPython::InterfaceWithPython() : Module(staticInfo_) 
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(InputString);
  INITIALIZE_PORT(PythonMatrix);
  INITIALIZE_PORT(PythonField);
  INITIALIZE_PORT(PythonString);
}

void InterfaceWithPython::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::PythonCode, std::string("# Insert Python code here using the SCIRun API."));
  state->setValue(Parameters::PollingIntervalMilliseconds, 200);
  state->setValue(Parameters::NumberOfRetries, 50);
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
      PythonInterpreter::Instance().run_string(line);
  }

  //TODO: support multiple output objects
  PythonObjectForwarderImpl<InterfaceWithPython> impl(*this);
  impl.waitForOutputFromTransientState();
}
