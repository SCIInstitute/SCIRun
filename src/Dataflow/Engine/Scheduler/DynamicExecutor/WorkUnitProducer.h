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


#ifndef ENGINE_SCHEDULER_DYNAMICEXECUTOR_WORKUNITPRODUCER_H
#define ENGINE_SCHEDULER_DYNAMICEXECUTOR_WORKUNITPRODUCER_H

#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkQueue.h>
#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkUnitProducerInterface.h>
#include <Dataflow/Engine/Scheduler/BoostGraphParallelScheduler.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Core/Thread/Mutex.h>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <spdlog/fmt/ostr.h>

#include <Dataflow/Engine/Scheduler/share.h>

namespace SCIRun {
  namespace Dataflow {
    namespace Engine {
      namespace DynamicExecutor {

        class SCISHARE ModuleProducer : public ProducerInterface, boost::noncopyable
        {
        public:
          ModuleProducer(const Networks::ModuleFilter& filter,
            const Networks::NetworkInterface* network, Core::Thread::Mutex* lock, ModuleWorkQueuePtr work, size_t numModules) :
            scheduler_(filter), network_(network), enqueueLock_(lock),
            work_(work), doneCount_(0), badGroup_(false),
            //shouldLog_(SCIRun::Core::Logging::Log::get().verbose()),
            numModules_(numModules)
          {
            //log_.setVerbose(shouldLog_);
          }

          virtual void enqueueReadyModules() const
          {
            Core::Thread::Guard g(enqueueLock_->get());
            if (!isDone())
            {
              auto order = scheduler_.schedule(*network_);
              //if (shouldLog_)
              //{
              //  log_->trace("Producer received this ordering: \n{}\nProducer processing min group {}", order, order.minGroup());
              //}
              if (order.minGroup() < 0)
              {
                badGroup_ = true;
              }
              auto groupIter = order.getGroup(order.minGroup());
              BOOST_FOREACH(const ParallelModuleExecutionOrder::ModulesByGroup::value_type& mod, groupIter)
              {
                auto module = network_->lookupModule(mod.second);

                if (module->executionState().currentState() == Networks::ModuleExecutionState::Waiting)
                {
                  //if (shouldLog_)
                  //  log_->trace("Producer pushing module {}", mod.second);

                  if (doneIds_.find(mod.second) != doneIds_.end())
                  {
                    //if (shouldLog_)
                    //  log_->info("Module producer: wants to enqueue module {} a second time. {}", mod.second, id_);
                  }
                  else
                  {
                    work_->push(module);
                    doneIds_.insert(mod.second);
                    doneCount_.fetch_add(1);

                    //if (shouldLog_)
                    //  log_->trace("Producer status: {} {} out of {}", id_, doneCount_, numModules_);
                  }
                }
              }
            }
          }

          void operator()() const
          {
            id_ = boost::this_thread::get_id();

            //log_->trace_if(shouldLog_, "Producer started {}", id_);

            enqueueReadyModules();

            while (!badGroup_ && !isDone())
            {
              boost::this_thread::sleep(boost::posix_time::milliseconds(100));
              //std::cout << "producer thread waiting " << id_ << std::endl;
            }

            if (badGroup_)
              std::cerr << "producer is done with bad group, something went wrong. probably a race condition..." << std::endl;

            //log_->trace_if(shouldLog_, "Producer is done. {}", id_);
          }

          bool isDone() const
          {
            return doneCount_ >= numModules_;
          }
        private:
          BoostGraphParallelScheduler scheduler_;
          const Networks::NetworkInterface* network_;
          Core::Thread::Mutex* enqueueLock_;
          ModuleWorkQueuePtr work_;
          mutable boost::atomic<int> doneCount_;
          mutable bool badGroup_;
          mutable std::set<Networks::ModuleId> doneIds_;
          //static Core::Logging::Logger2 log_;
          //bool shouldLog_;
          size_t numModules_;
          mutable boost::thread::id id_;
        };

        typedef boost::shared_ptr<ModuleProducer> ModuleProducerPtr;

      }}

  }}

#endif
