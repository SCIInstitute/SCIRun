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


#ifndef ENGINE_SCHEDULER_EXECUTION_STRATEGY_H
#define ENGINE_SCHEDULER_EXECUTION_STRATEGY_H

#include <Dataflow/Engine/Scheduler/SchedulerInterfaces.h>
#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkQueue.h>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <Core/Thread/ConditionVariable.h>
#include <Dataflow/Engine/Scheduler/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  class SCISHARE ExecutionStrategy
  {
  public:
    virtual ~ExecutionStrategy() {}
    virtual void execute(const ExecutionContext& context, Core::Thread::Mutex& executionLock) = 0;

    enum Type
    {
      SERIAL,
      BASIC_PARALLEL,
      DYNAMIC_PARALLEL
      // next: pausable, then with loops
    };

  };

  typedef boost::shared_ptr<ExecutionStrategy> ExecutionStrategyHandle;

  class SCISHARE ExecutionStrategyFactory
  {
  public:
    virtual ~ExecutionStrategyFactory() {}
    virtual ExecutionStrategyHandle create(ExecutionStrategy::Type type) const = 0;
    virtual ExecutionStrategyHandle createDefault() const = 0;
  };

  typedef boost::shared_ptr<ExecutionStrategyFactory> ExecutionStrategyFactoryHandle;


  class SCISHARE ExecutionQueueManager
  {
  public:
    ExecutionQueueManager();
    void initExecutor(ExecutionStrategyFactoryHandle factory);
    void setExecutionStrategy(ExecutionStrategyHandle exec);
    boost::shared_ptr<boost::thread> enqueueContext(ExecutionContextHandle context);
    void start();
    void stop();
  private:
    void executeImpl(ExecutionContextHandle context);
    typedef DynamicExecutor::WorkQueue<ExecutionContextHandle>::Impl ExecutionContextQueue;
    ExecutionContextQueue contexts_;

    ExecutionStrategyHandle currentExecutor_;

    boost::shared_ptr<boost::thread> executionLaunchThread_;
    Core::Thread::Mutex executionMutex_;
    Core::Thread::ConditionVariable somethingToExecute_;
    boost::atomic<int> contextCount_; // need certain member function on spsc_queue, need to check boost version...

    void executeTopContext();
  };
}
}}

#endif
