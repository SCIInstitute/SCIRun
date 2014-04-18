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

        BoostGraphParallelScheduler ModuleProducer::scheduler_(filter());
      }

      /// @todo: templatize along with producer/consumer
      class DynamicMultithreadedNetworkExecutorImpl
      {
      public:
        DynamicMultithreadedNetworkExecutorImpl(const ExecutableLookup* lookup, const ExecutionBounds& bounds, const NetworkInterface* network, Mutex* lock) : 
          work_(new DynamicExecutor::ModuleWorkQueue(network->nmodules())),
          producer_(new DynamicExecutor::ModuleProducer(lookup, bounds, network, lock, work_)),
          consumer_(new DynamicExecutor::ModuleConsumer(work_, lookup, producer_))
        {
        }
        void operator()() const
        {
          boost::thread consume(boost::ref(*consumer_));
          boost::thread produce(boost::ref(*producer_));
          consume.join();
          produce.join();
        }
      private:
        DynamicExecutor::ModuleWorkQueuePtr work_;
        DynamicExecutor::ModuleProducerPtr producer_;
        DynamicExecutor::ModuleConsumerPtr consumer_;
      };
}}}

DynamicMultithreadedNetworkExecutor::DynamicMultithreadedNetworkExecutor(const NetworkInterface& network) : network_(network) {}

void DynamicMultithreadedNetworkExecutor::executeAll(const ExecutableLookup& lookup, ParallelModuleExecutionOrder order, const ExecutionBounds& bounds)
{
  static Mutex lock("live-scheduler");
  
  DynamicMultithreadedNetworkExecutorImpl runner(&lookup, bounds, &network_, &lock);
  boost::thread execution(runner);
}
