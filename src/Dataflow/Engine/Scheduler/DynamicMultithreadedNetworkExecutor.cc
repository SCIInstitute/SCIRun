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


#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkQueue.h>
#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkUnitConsumer.h>
#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkUnitExecutor.h>
#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkUnitProducer.h>

#include <Dataflow/Engine/Scheduler/DynamicMultithreadedNetworkExecutor.h>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Logging;

namespace SCIRun {
  namespace Dataflow {
    namespace Engine {
      namespace DynamicExecutor
      {
        //Logger2 ModuleConsumer::log_ = spdlog::stdout_color_mt("consumer");
        //Logger2 ModuleProducer::log_ = spdlog::stdout_color_mt("producer");
        //Logger2 ModuleExecutor::log_ = spdlog::stdout_color_mt("executor");
      }

      /// @todo: templatize along with producer/consumer
      class DynamicMultithreadedNetworkExecutorImpl : public WaitsForStartupInitialization//, boost::noncopyable
      {
      public:
        DynamicMultithreadedNetworkExecutorImpl(const ExecutionContext& context, const NetworkInterface* network,
          Mutex* lock, size_t numModules, Mutex* executionLock, DynamicExecutor::ExecutionThreadGroupPtr threadGroup) :
          executeThreads_(threadGroup),
          lookup_(&context.lookup_),
          bounds_(&context.bounds()),
          work_(new DynamicExecutor::ModuleWorkQueue(numModules)),
          producer_(new DynamicExecutor::ModuleProducer(context.addAdditionalFilter(ModuleWaitingFilter::Instance()),
            network, lock, work_, numModules)),
            consumer_(new DynamicExecutor::ModuleConsumer(work_, lookup_, producer_, executeThreads_)),
          network_(network),
          executionLock_(executionLock)
        {
        }
        ~DynamicMultithreadedNetworkExecutorImpl()
        {
        }
        void operator()() const
        {
          Guard g(executionLock_->get());

          ScopedExecutionBoundsSignaller signaller(bounds_, [=]() { return lookup_->errorCode(); });

          waitForStartupInit(*network_);

          std::thread consume(std::ref(*consumer_));
          std::thread produce(std::ref(*producer_));
          consume.join();
          produce.join();
          executeThreads_->joinAll();
        }

      private:
        mutable DynamicExecutor::ExecutionThreadGroupPtr executeThreads_;
        const Networks::ExecutableLookup* lookup_;
        const ExecutionBounds* bounds_;
        DynamicExecutor::ModuleWorkQueuePtr work_;
        DynamicExecutor::ModuleProducerPtr producer_;
        DynamicExecutor::ModuleConsumerPtr consumer_;
        const NetworkInterface* network_;
        Mutex* executionLock_;
      };
}}}

DynamicMultithreadedNetworkExecutor::DynamicMultithreadedNetworkExecutor(const NetworkInterface& network) :
  network_(network),
  threadGroup_(new DynamicExecutor::ExecutionThreadGroup)
{
}

void DynamicMultithreadedNetworkExecutor::execute(const ExecutionContext& context, ParallelModuleExecutionOrder order, Mutex& executionLock)
{
  static Mutex lock("live-scheduler");

  //if (Log::get().verbose())
    LOG_TRACE("DMTNE::executeAll order received: {}", order);

  threadGroup_->clear();
  DynamicMultithreadedNetworkExecutorImpl runner(context, &network_, &lock, order.size(), &executionLock, threadGroup_);
  Core::Thread::Util::launchAsyncThread(runner);
}

bool ModuleWaitingFilter::operator()(ModuleHandle mh) const
{
  auto state = mh->executionState().currentState();
  return state != Networks::ModuleExecutionState::Completed;// || state != Networks::ModuleExecutionState::Errored;
}

const ModuleWaitingFilter& ModuleWaitingFilter::Instance()
{
  static ModuleWaitingFilter instance_;
  return instance_;
}
