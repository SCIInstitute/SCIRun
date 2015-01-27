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
        Log& ModuleConsumer::log_ = Log::get("consumer");
        Log& ModuleProducer::log_ = Log::get("producer");
      }

      /// @todo: templatize along with producer/consumer
      class DynamicMultithreadedNetworkExecutorImpl : public WaitsForStartupInitialization
      {
      public:
        DynamicMultithreadedNetworkExecutorImpl(const ExecutionContext& context, const NetworkInterface* network, Mutex* lock, size_t numModules) :
          executeThreads_(new boost::thread_group),
          lookup_(&context.lookup),
          bounds_(&context.bounds()),
          work_(new DynamicExecutor::ModuleWorkQueue(numModules)),
          producer_(new DynamicExecutor::ModuleProducer(context.addAdditionalFilter(ModuleWaitingFilter::Instance()),
            network, lock, work_, numModules)),
            consumer_(new DynamicExecutor::ModuleConsumer(work_, lookup_, producer_, *executeThreads_)),
          network_(network)
        {
        }
        void operator()() const
        {
          ScopedExecutionBoundsSignaller signaller(bounds_, [=]() { return lookup_->errorCode(); });

          waitForStartupInit(*network_);

          boost::thread consume(boost::ref(*consumer_));
          boost::thread produce(boost::ref(*producer_));
          consume.join();
          produce.join();
          executeThreads_->join_all();
        }
      private:
        mutable boost::shared_ptr<boost::thread_group> executeThreads_;
        const Networks::ExecutableLookup* lookup_;
        const ExecutionBounds* bounds_;
        DynamicExecutor::ModuleWorkQueuePtr work_;
        DynamicExecutor::ModuleProducerPtr producer_;
        DynamicExecutor::ModuleConsumerPtr consumer_;
        const NetworkInterface* network_;
      };
}}}

DynamicMultithreadedNetworkExecutor::DynamicMultithreadedNetworkExecutor(const NetworkInterface& network) : network_(network) {}

void DynamicMultithreadedNetworkExecutor::execute(const ExecutionContext& context, ParallelModuleExecutionOrder order)
{
  static Mutex lock("live-scheduler");

  if (Log::get().verbose())
    LOG_DEBUG("DMTNE::executeAll order received: " << order << std::endl);

  DynamicMultithreadedNetworkExecutorImpl runner(context, &network_, &lock, order.size());
  boost::thread execution(runner);
}

bool ModuleWaitingFilter::operator()(SCIRun::Dataflow::Networks::ModuleHandle mh) const
{
  return mh->executionState() != Networks::ModuleInterface::Completed;
}

const ModuleWaitingFilter& ModuleWaitingFilter::Instance()
{
  static ModuleWaitingFilter instance_;
  return instance_;
}
