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

#ifndef ENGINE_SCHEDULER_DYNAMICEXECUTOR_WORKUNITPRODUCER_H
#define ENGINE_SCHEDULER_DYNAMICEXECUTOR_WORKUNITPRODUCER_H

#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkQueue.h>
#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkUnitProducerInterface.h>
#include <Dataflow/Engine/Scheduler/BoostGraphParallelScheduler.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Core/Thread/Mutex.h>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <Dataflow/Engine/Scheduler/share.h>

namespace SCIRun {
  namespace Dataflow {
    namespace Engine {
      namespace DynamicExecutor {

        class SCISHARE ModuleProducer : public ProducerInterface, boost::noncopyable
        {
        public:
          ModuleProducer(const Networks::ExecutableLookup* lookup, const ExecutionBounds& bounds, 
            const Networks::NetworkInterface* network, Core::Thread::Mutex* lock, ModuleWorkQueuePtr work) :
          lookup_(lookup), bounds_(bounds), network_(network), lock_(lock),
            work_(work), doneCount_(0)
          {
          }

          virtual void enqueueReadyModules() const
          {
            if (!isDone())
            {
              ParallelModuleExecutionOrder order;
              {
                Core::Thread::Guard g(lock_->get());
                order = scheduler_.schedule(*network_);
              }
              log_ << Core::Logging::DEBUG_LOG << "Producer processing min group " << order.minGroup();
              auto groupIter = order.getGroup(order.minGroup());
              BOOST_FOREACH(const ParallelModuleExecutionOrder::ModulesByGroup::value_type& mod, groupIter)
              {
                auto module = network_->lookupModule(mod.second);

                if (module->executionState() != Networks::ModuleInterface::Executing)
                {
                  log_ << Core::Logging::DEBUG_LOG << "Producer pushing module " << mod.second << std::endl;
                  work_->push(module);
                  doneCount_.fetch_add(1);
                  log_ << Core::Logging::DEBUG_LOG << "Producer status: " << doneCount_ << " out of " << network_->nmodules() << std::endl;
                }
              }
            }
          }

          void operator()() const
          {
            ScopedExecutionBoundsSignaller signaller(bounds_, [=]() { return lookup_->errorCode(); });

            log_ << Core::Logging::DEBUG_LOG << "Producer started" << std::endl;

            enqueueReadyModules();

            while (!isDone())
              boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

            log_ << Core::Logging::DEBUG_LOG << "Producer is done." << std::endl;
          }

          bool isDone() const 
          {
            return doneCount_ >= network_->nmodules();
          }
        private:

          struct ModuleWaiting
          {
            bool operator()(Networks::ModuleHandle mh) const
            {
              return mh->executionState() != Networks::ModuleInterface::Completed;
            }
          };

          static ModuleWaiting filter() { return ModuleWaiting(); }
          static BoostGraphParallelScheduler scheduler_;
          const Networks::ExecutableLookup* lookup_;
          const ExecutionBounds& bounds_;
          const Networks::NetworkInterface* network_;
          Core::Thread::Mutex* lock_;
          ModuleWorkQueuePtr work_;
          mutable boost::atomic<int> doneCount_;
          static Core::Logging::Log& log_;
        };

        typedef boost::shared_ptr<ModuleProducer> ModuleProducerPtr;

      }}

  }}

#endif