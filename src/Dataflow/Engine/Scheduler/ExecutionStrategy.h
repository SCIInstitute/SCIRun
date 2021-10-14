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

#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Engine/Scheduler/SchedulerInterfaces.h>
#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkQueue.h>
#include <boost/atomic.hpp>
#include <Core/Thread/ConditionVariable.h>
#include <Core/Thread/Interruptible.h>
#include <Dataflow/Engine/Scheduler/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  class SCISHARE ExecutionStrategy
  {
  public:
    virtual ~ExecutionStrategy() {}
    virtual std::future<int> execute(const ExecutionContext& context, Core::Thread::Mutex& executionLock) = 0;

    enum class Type
    {
      SERIAL,
      BASIC_PARALLEL,
      DYNAMIC_PARALLEL
      // next: pausable, then with loops
    };

  };

  typedef SharedPointer<ExecutionStrategy> ExecutionStrategyHandle;

  class SCISHARE ExecutionStrategyFactory
  {
  public:
    virtual ~ExecutionStrategyFactory() {}
    virtual ExecutionStrategyHandle create(ExecutionStrategy::Type type) const = 0;
    virtual ExecutionStrategyHandle createDefault() const = 0;
  };

  typedef SharedPointer<ExecutionStrategyFactory> ExecutionStrategyFactoryHandle;

  class SCISHARE ExecutionManager
  {
  public:
    virtual ~ExecutionManager() = default;

    virtual void initExecutor(ExecutionStrategyFactoryHandle factory) = 0;
    virtual void setExecutionStrategy(ExecutionStrategyHandle exec) = 0;
    virtual std::future<int> execute(ExecutionContextHandle context) = 0;

    virtual void stopExecution() = 0;
  };

  using ExecutionManagerHandle = SharedPointer<ExecutionManager>;

  class SCISHARE ExecutionManagerBase : public ExecutionManager
  {
  public:
    void initExecutor(ExecutionStrategyFactoryHandle factory) override;
    void setExecutionStrategy(ExecutionStrategyHandle exec) override;
  protected:
    ExecutionManagerBase();
    std::future<int> executeImpl(ExecutionContextHandle context);
    ExecutionStrategyHandle currentExecutor_;
    Core::Thread::Mutex executionMutex_;
  };

  class SCISHARE ExecutionQueueManager : public ExecutionManagerBase, public Core::Thread::Stoppable
  {
  public:
    ExecutionQueueManager();
    
    std::future<int> execute(ExecutionContextHandle context) override
    {
      enqueueContext(context);
      return {};
    }
    
    void stopExecution() override;
  private:
    void startExecution();
    void enqueueContext(ExecutionContextHandle context);
    
    typedef DynamicExecutor::WorkQueue<ExecutionContextHandle> ExecutionContextQueue;
    ExecutionContextQueue contexts_;
    ThreadPtr executionLaunchThread_;
    
    Core::Thread::ConditionVariable somethingToExecute_;
    boost::atomic<int> contextCount_; // need certain member function on spsc_queue, need to check boost version...

    void executeTopContext();
  };

  class SCISHARE SimpleExecutionManager : public ExecutionManagerBase
  {
  public:
    std::future<int> execute(ExecutionContextHandle context) override { return executeImpl(context); }
    void stopExecution() override {}
  };
}
}}

#endif
