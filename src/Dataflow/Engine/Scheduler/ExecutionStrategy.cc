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

ExecutionBounds ExecutionContext::executionBounds_;

boost::signals2::connection ExecutionContext::connectNetworkExecutionStarts(const ExecuteAllStartsSignalType::slot_type& subscriber)
{
  return executionBounds_.executeStarts_.connect(subscriber);
}
boost::signals2::connection ExecutionContext::connectNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber)
{
  return executionBounds_.executeFinishes_.connect(subscriber);
}

ModuleFilter ExecutionContext::addAdditionalFilter(ModuleFilter filter) const
{
  if (!filter)
    return additionalFilter;
  if (!additionalFilter)
    return filter;

  return boost::bind(filter, _1) && boost::bind(additionalFilter, _1);
}

ExecutionQueueManager::ExecutionQueueManager() :
  contexts_(10),
  executionMutex_("executionQueue"),
  somethingToExecute_("executionQueue"),
  contextCount_(0)
{
}

void ExecutionQueueManager::setExecutionStrategy(ExecutionStrategyHandle exec)
{
  Guard g(executionMutex_.get());
  currentExecutor_ = exec;
}

void ExecutionQueueManager::initExecutor(ExecutionStrategyFactoryHandle factory)
{
  if (!currentExecutor_ && factory)
    currentExecutor_ = factory->createDefault();
}

void ExecutionQueueManager::start()
{
  executionLaunchThread_.reset(new boost::thread([this]() { executeTopContext(); }));
}

boost::shared_ptr<boost::thread> ExecutionQueueManager::enqueueContext(ExecutionContextHandle context)
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
      start();
    somethingToExecute_.conditionBroadcast();
  }
  return executionLaunchThread_;
}

void ExecutionQueueManager::executeTopContext()
{
  while (true)
  {
    UniqueLock lock(executionMutex_.get());
    while (0 == contextCount_)
    {
      somethingToExecute_.wait(lock);
    }
    if (contexts_.consume_one([&](ExecutionContextHandle ctx) { executeImpl(ctx); }))
    {
      contextCount_.fetch_sub(1);
    }
  }
}

void ExecutionQueueManager::executeImpl(ExecutionContextHandle ctx)
{
  if (currentExecutor_ && ctx)
  {
    ctx->preexecute();
    currentExecutor_->execute(*ctx, executionMutex_);
  }
}

void ExecutionQueueManager::stop()
{
  if (executionLaunchThread_)
  {
    executionLaunchThread_->interrupt();
    executionLaunchThread_.reset();
  }
}
