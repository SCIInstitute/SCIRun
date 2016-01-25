/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Modules/Python/PythonObjectForwarder.h>
#include <Core/Datatypes/String.h>
#include <boost/thread.hpp>

using namespace SCIRun::Modules::Python;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

const ModuleLookupInfo PythonObjectForwarder::staticInfo_("PythonObjectForwarder", "Python", "SCIRun");

PythonObjectForwarder::PythonObjectForwarder() : Module(staticInfo_, false) 
{
  INITIALIZE_PORT(NewString);
}

void PythonObjectForwarder::setStateDefaults()
{
}

void PythonObjectForwarder::execute()
{
  const std::string key = "PythonString";
  auto state = get_state();
  int tries = 0;
  const int MAX_TRIES = 50;
  auto valueOption = state->getTransientValue(key);
  while (tries < MAX_TRIES && !valueOption)
  {
    std::cout << "PythonObjectForwarder looking up value attempt # " << tries << std::endl;

    valueOption = state->getTransientValue(key);

    tries++;
    boost::this_thread::sleep(boost::posix_time::milliseconds(200));
  }
  if (valueOption)
  {
    auto valueStr = makeVariable("name", transient_value_cast<AlgorithmParameter::Value>(valueOption)).toString();
    if (!valueStr.empty())
      sendOutput(NewString, boost::make_shared<String>(valueStr));
    else
      sendOutput(NewString, boost::make_shared<String>("Empty string or non-string received"));
  }
  else
    sendOutput(NewString, boost::make_shared<String>("No value received"));
}
