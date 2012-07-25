/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <iostream>
#include <Engine/Scheduler/LinearSerialNetworkExecutor.h>
#include <Core/Dataflow/Network/ModuleInterface.h>
#include <Core/Dataflow/Network/NetworkInterface.h>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

using namespace SCIRun::Engine;
using namespace SCIRun::Domain::Networks;

struct LinearExecution
{
  LinearExecution(const ExecutableLookup& lookup, ModuleExecutionOrder order) : lookup_(lookup), order_(order)
  {
  }
  void operator()() const
  {
    BOOST_FOREACH(const std::string& id, order_)
    {
      ExecutableObject* obj = lookup_.lookupExecutable(id);
      if (obj)
      {
        obj->execute();
      }
    }
  }
  const ExecutableLookup& lookup_;
  ModuleExecutionOrder order_;
};

void LinearSerialNetworkExecutor::executeAll(const ExecutableLookup& lookup, ModuleExecutionOrder order)
{
  LinearExecution runner(lookup, order);
  //runner();
  boost::thread execution = boost::thread(runner);
}
