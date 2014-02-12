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
#include <Dataflow/Engine/Scheduler/BoostGraphParallelScheduler.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Core/Thread/Parallel.h>
#include <Core/Logging/Log.h>
#include <Core/Thread/Mutex.h>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Logging;

namespace detail
{
  struct ScopedModuleExecutionBounds
  {
    static void logExecStart(const ModuleId& id)
    {
      Log::get() << INFO << "!!! starting " << id.id_;
    }

    static void logExecEnd(const ModuleId& id)
    {
      Log::get() << INFO << " ~~~ ending " << id.id_;
    }
  };

  class ModuleWorkQueue
  {
  
  };
  
  class SchedulePrinter
  {
  public:
    SchedulePrinter(const NetworkInterface* network, const Scheduler<ParallelModuleExecutionOrder>* scheduler, Mutex& lock) : network_(network), scheduler_(scheduler), lock_(lock)
    {}

    void printNetworkOrder(const ModuleId& id)
    {
      Guard g(lock_.get());
      if (scheduler_ && network_)
      {
        auto order = scheduler_->schedule(*network_);
        Log::get() << INFO << "NETWORK ORDER~~~completed module = " << id << " ~~~next up:\n\t";
        printMinGroup(order);
      }
      else
        Log::get() << INFO << "NETWORK ORDER~~~\n <<<null>>>\n";
    }

  private:
    void printMinGroup(const ParallelModuleExecutionOrder& order)
    {
      BOOST_FOREACH(const ParallelModuleExecutionOrder::value_type& v, order.getGroup(order.minGroup()))
      {
        Log::get() << INFO << v.first << " " << v.second << std::endl;
      }
    }
  
  
    const NetworkInterface* network_;
    const Scheduler<ParallelModuleExecutionOrder>* scheduler_;
    Mutex& lock_;
  };

  struct ModuleWaiting
  {
    bool operator()(ModuleHandle mh) const
    {
      return mh->executionState() != ModuleInterface::Completed;
    }
  };

  struct DynamicParallelExecution
  {
    DynamicParallelExecution(const ExecutableLookup* lookup, const ParallelModuleExecutionOrder& order, const ExecutionBounds& bounds, 
      const NetworkInterface* network, Mutex& lock) :
        lookup_(lookup), order_(order), bounds_(bounds), network_(network),
        printer_(network_, &scheduler_, lock)
    {}
    
    void operator()() const
    {
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
            auto exec = lookup_->lookupExecutable(mod.second);
            boost::signals2::scoped_connection s(exec->connectExecuteEnds(boost::bind(&SchedulePrinter::printNetworkOrder, printer_, _1)));
            exec->execute(); 
          };
        });

        //std::cout << "Running group " << group << " of size " << tasks.size() << std::endl;
        Parallel::RunTasks([&](int i) { tasks[i](); }, static_cast<int>(tasks.size()));
      }
    }

    static ModuleWaiting filter() { return ModuleWaiting(); }
    static BoostGraphParallelScheduler scheduler_;
    const ExecutableLookup* lookup_;
    ParallelModuleExecutionOrder order_;
    const ExecutionBounds& bounds_;
    const NetworkInterface* network_;
    SchedulePrinter printer_;
  };
  
  BoostGraphParallelScheduler DynamicParallelExecution::scheduler_(filter());
}

DynamicMultithreadedNetworkExecutor::DynamicMultithreadedNetworkExecutor(const NetworkInterface& network) : network_(network) {}

void DynamicMultithreadedNetworkExecutor::executeAll(const ExecutableLookup& lookup, ParallelModuleExecutionOrder order, const ExecutionBounds& bounds)
{
  static Mutex lock("live-scheduler");
  detail::DynamicParallelExecution runner(&lookup, order, bounds, &network_, lock);
  boost::thread execution(runner);
}
