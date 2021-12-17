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


#include <Dataflow/Engine/Scheduler/ExecutionStrategy.h>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;

ExecutionBounds ExecutionContext::globalExecutionBounds_;

ExecutionBounds& ExecutionContext::globalExecutionBounds() { return globalExecutionBounds_; }

boost::signals2::connection ExecutionContext::connectGlobalNetworkExecutionStarts(const ExecuteAllStartsSignalType::slot_type& subscriber)
{
  return globalExecutionBounds_.executeStarts_.connect(subscriber);
}

boost::signals2::connection ExecutionContext::connectGlobalNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber)
{
  return globalExecutionBounds_.executeFinishes_.connect(subscriber);
}

ModuleFilter ExecutionContext::addAdditionalFilter(ModuleFilter filter) const
{
  if (!filter)
    return additionalFilter_;
  if (!additionalFilter_)
    return filter;

  auto additional = additionalFilter_;
  return [filter, additional](ModuleHandle mh) { return filter(mh) && additional(mh); };
}

ExecutionManagerBase::ExecutionManagerBase() : executionMutex_("executionManager")
{
  
}

ExecutionQueueManager::ExecutionQueueManager() : 
  contexts_(10),
  somethingToExecute_("executionQueue"),
  contextCount_(0)
{
}

void ExecutionManagerBase::setExecutionStrategy(ExecutionStrategyHandle exec)
{
  Guard g(executionMutex_.get());
  currentExecutor_ = exec;
}

void ExecutionManagerBase::initExecutor(ExecutionStrategyFactoryHandle factory)
{
  if (!currentExecutor_ && factory)
    currentExecutor_ = factory->createDefault();
}

void ExecutionQueueManager::startExecution()
{
  //logCritical("startExecution");
  resetStoppability();
  auto task = [this] { executeTopContext(); }; 

  executionLaunchThread_.reset(new std::thread(task));
}

void ExecutionQueueManager::enqueueContext(ExecutionContextHandle context)
{
  bool contextReady;
  {
    Guard g(executionMutex_.get());
    contextReady = contexts_.push(context);
    if (contextReady)
      contextCount_.fetch_add(1);
  }
  if (contextReady)
  {
    if (!executionLaunchThread_)
      startExecution();
    somethingToExecute_.conditionBroadcast();
  }
}

void ExecutionQueueManager::executeTopContext()
{
  while (true)
  {
    UniqueLock lock(executionMutex_.get());
    while (0 == contextCount_)
    {
      //logCritical("in loop while 0");
      somethingToExecute_.wait(lock);
      if (stopRequested())
      {
        return;
      }
    }
    if (contexts_.consume_one([&](ExecutionContextHandle ctx) { executeImpl(ctx); }))
    {
      contextCount_.fetch_sub(1);
    }
  }
}

std::future<int> ExecutionManagerBase::executeImpl(ExecutionContextHandle context)
{
  if (currentExecutor_ && context)
  {
    context->preexecute();
    return currentExecutor_->execute(*context, executionMutex_);
  }
  return {};
}

void ExecutionQueueManager::stopExecution()
{
  if (executionLaunchThread_)
  {
    sendStopRequest();
    somethingToExecute_.conditionBroadcast();
    if (executionLaunchThread_->joinable())
      executionLaunchThread_->join();
    executionLaunchThread_.reset();
  }
}
