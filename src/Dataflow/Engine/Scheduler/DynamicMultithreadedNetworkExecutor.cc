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
#include <Dataflow/Engine/Scheduler/DynamicMultithreadedNetworkExecutor.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Core/Thread/Parallel.h>
#include <Core/Logging/Log.h>
#include <boost/thread.hpp>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Logging;

namespace 
{
  struct DynamicParallelExecution
  {
    DynamicParallelExecution(const ExecutableLookup* lookup, const ParallelModuleExecutionOrder& order, const ExecutionBounds& bounds) : lookup_(lookup), order_(order), bounds_(bounds)
    {}
    
    void operator()() const
    {
      std::cout << "HELLO NEW EXECUTOR" << std::endl;
      Log::get() << INFO << "HELLO NEW EXECUTOR" << std::endl;
      ScopedExecutionBoundsSignaller signaller(bounds_, [&]() { return lookup_->errorCode(); });
      for (int group = order_.minGroup(); group <= order_.maxGroup(); ++group)
      {
        auto groupIter = order_.getGroup(group);

        std::vector<boost::function<void()>> tasks;

        std::transform(groupIter.first, groupIter.second, std::back_inserter(tasks), 
          [&](const ParallelModuleExecutionOrder::ModulesByGroup::value_type& mod) -> boost::function<void()>
        {
          return [=]() 
          { 
            std::cout << "foo" << std::endl;
            Log::get() << INFO << "!!! starting " << mod.second.id_ << std::endl;
            lookup_->lookupExecutable(mod.second)->execute(); 
            Log::get() << INFO << "~~~ ending " << mod.second.id_ << std::endl;
          };
        });

        //std::cout << "Running group " << group << " of size " << tasks.size() << std::endl;
        Parallel::RunTasks([&](int i) { tasks[i](); }, tasks.size());
      }
    }

    const ExecutableLookup* lookup_;
    ParallelModuleExecutionOrder order_;
    const ExecutionBounds& bounds_;
  };

}

void DynamicMultithreadedNetworkExecutor::executeAll(const ExecutableLookup& lookup, ParallelModuleExecutionOrder order, const ExecutionBounds& bounds)
{
  std::cout << "HELLO NEW EXECUTOR" << std::endl;
  Log::get() << INFO << "HELLO NEW EXECUTOR" << std::endl;
  DynamicParallelExecution runner(&lookup, order, bounds);
  boost::thread execution(runner);
}
