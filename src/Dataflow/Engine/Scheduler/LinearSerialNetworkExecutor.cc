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


#include <iostream>
#include <Dataflow/Engine/Scheduler/LinearSerialNetworkExecutor.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <boost/thread.hpp>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;

namespace
{
  struct LinearExecution : public WaitsForStartupInitialization
  {
    LinearExecution(const ExecutableLookup& lookup, const ModuleExecutionOrder& order, const ExecutionBounds& bounds, Mutex* executionLock)
      : lookup_(lookup), order_(order), bounds_(bounds), executionLock_(executionLock)
    {
    }
    void operator()() const
    {
      waitForStartupInit(lookup_);
      Guard g(executionLock_->get());
      bounds_.executeStarts_();
      for (const ModuleId& id : order_)
      {
        ExecutableObject* obj = lookup_.lookupExecutable(id);
        if (obj)
        {
          obj->executeWithSignals();
        }
      }
      bounds_.executeFinishes_(lookup_.errorCode());
    }
    const ExecutableLookup& lookup_;
    ModuleExecutionOrder order_;
    const ExecutionBounds& bounds_;
    Mutex* executionLock_;
  };
}

void LinearSerialNetworkExecutor::execute(const ExecutionContext& context, ModuleExecutionOrder order, Mutex& executionLock)
{
  LinearExecution runner(context.lookup, order, context.bounds(), &executionLock);
  boost::thread execution(runner);
}
