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
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <boost/thread.hpp>

using namespace SCIRun::Modules::Python;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Python;

//ALGORITHM_PARAMETER_DEF(Python, PollingIntervalMilliseconds);
//ALGORITHM_PARAMETER_DEF(Python, NumberOfRetries);
ALGORITHM_PARAMETER_DEF(Python, PythonCode);

const ModuleLookupInfo InterfaceWithPython::staticInfo_("InterfaceWithPython", "Python", "SCIRun");

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
  state->setValue(Parameters::PythonCode, std::string("1 + 1"));
}

void InterfaceWithPython::execute()
{
/*
  auto state = get_state();
  int tries = 0;
  const int maxTries = state->getValue(Parameters::NumberOfRetries).toInt();
  const int waitTime = state->getValue(Parameters::PollingIntervalMilliseconds).toInt();
  auto valueOption = state->getTransientValue(Parameters::PythonObject);
  
  while (tries < maxTries && !valueOption)
  {
    std::ostringstream ostr;
    ostr << "PythonObjectForwarder looking up value attempt #" << (tries+1) << "/" << maxTries;
    remark(ostr.str());

    valueOption = state->getTransientValue(Parameters::PythonObject);

    tries++;
    boost::this_thread::sleep(boost::posix_time::milliseconds(waitTime));
  }

  if (valueOption)
  {
    auto var = transient_value_cast<Variable>(valueOption);
    if (var.name().name() == "string")
    {
      auto valueStr = var.toString();
      if (!valueStr.empty())
        sendOutput(PythonString, boost::make_shared<String>(valueStr));
      else
        sendOutput(PythonString, boost::make_shared<String>("Empty string or non-string received"));
    }
    else if (var.name().name() == "dense matrix")
    {
      auto dense = boost::dynamic_pointer_cast<DenseMatrix>(var.getDatatype());
      if (dense)
        sendOutput(PythonMatrix, dense);
    }
    else if (var.name().name() == "sparse matrix")
    {
      auto sparse = boost::dynamic_pointer_cast<SparseRowMatrix>(var.getDatatype());
      if (sparse)
        sendOutput(PythonMatrix, sparse);
    }
  }
  */
}
