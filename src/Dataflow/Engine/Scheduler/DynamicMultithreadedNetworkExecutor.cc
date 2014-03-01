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
#include <boost/lockfree/spsc_queue.hpp>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Logging;

namespace detail
{
  template <class Unit>
  class WorkQueue
  {
  public:
    typedef boost::lockfree::spsc_queue<Unit> Impl;
  };

  typedef WorkQueue<ModuleHandle>::Impl ModuleWorkQueue;
  typedef boost::shared_ptr<ModuleWorkQueue> ModuleWorkQueuePtr;

  typedef std::list<ModuleHandle> WaitingList;

  class ProducerInterface
  {
  public:
    virtual ~ProducerInterface() {}
    virtual bool isDone() const = 0;
    virtual void enqueueReadyModules() const = 0;
  };

  struct ModuleExecutor
  {
    ModuleExecutor(ModuleHandle mod, const ExecutableLookup* lookup, const ProducerInterface* producer) : module_(mod), lookup_(lookup), producer_(producer)
    {
    }
    void run()
    {
      Log::get("executor") << DEBUG_LOG << "Module Executor: " << module_->get_id() << std::endl;
      auto exec = lookup_->lookupExecutable(module_->get_id());
      boost::signals2::scoped_connection s(exec->connectExecuteEnds(boost::bind(&ProducerInterface::enqueueReadyModules, boost::ref(*producer_))));
      exec->execute();
    }

    ModuleHandle module_;
    const ExecutableLookup* lookup_;
    const ProducerInterface* producer_;
  };

  class ModuleConsumer
  {
  public:
    explicit ModuleConsumer(ModuleWorkQueue* workQueue, const ExecutableLookup* lookup) :
    work_(workQueue), producer_(0), lookup_(lookup), shouldLog_(false)
    {
      log_.setVerbose(shouldLog_);
      if (shouldLog_)
        log_ << DEBUG_LOG << "Consumer created." << std::endl;
    }
    void operator()() const
    {
      if (!producer_)
      {
        if (shouldLog_)
          log_ << DEBUG_LOG << "Consumer quitting due to no producer pointer." << std::endl;
        return;
      }
      
      log_ << DEBUG_LOG << "Consumer started." << std::endl;

      while (!producer_->isDone() || moreWork())
      {
        {
          if (shouldLog_)
            log_ << DEBUG_LOG << "\tConsumer thinks work queue is not empty.";

          if (shouldLog_)
            log_ << DEBUG_LOG << "\tConsumer accessing front of work queue.";
          ModuleHandle unit;
          work_->pop(unit);
          if (shouldLog_)
            log_ << DEBUG_LOG << "\tConsumer popping front of work queue.";

          if (unit)
          {
            if (shouldLog_)
              log_ << DEBUG_LOG << "~~~Processing " << unit->get_id();

            ModuleExecutor executor(unit, lookup_, producer_.get());
            //TODO: thread pool
            boost::thread t(boost::bind(&ModuleExecutor::run, executor));
          }
          else
          {
            if (shouldLog_)
              log_ << DEBUG_LOG << "\tConsumer received null module";
          }
        }
      }
      log_ << DEBUG_LOG << "Consumer done." << std::endl;
    }

    bool moreWork() const
    {
      return !work_->empty();
    }

    void setProducer(boost::shared_ptr<const ProducerInterface> producer) 
    { 
      if (shouldLog_)
        log_ << DEBUG_LOG << "Consumer has producer set." << std::endl;
      producer_ = producer; 
    }

  private:
    ModuleWorkQueue* work_;
    boost::shared_ptr<const ProducerInterface> producer_;
    const ExecutableLookup* lookup_;
    static Log& log_;
    bool shouldLog_;
  };

  Log& ModuleConsumer::log_ = Log::get("consumer");
  
  typedef boost::shared_ptr<ModuleConsumer> ModuleConsumerPtr;

  struct ModuleWaiting
  {
    bool operator()(ModuleHandle mh) const
    {
      return mh->executionState() != ModuleInterface::Completed;
    }
  };

  class ModuleProducer : public ProducerInterface
  {
  public:
    ModuleProducer(const ExecutableLookup* lookup, const ExecutionBounds& bounds, 
      const NetworkInterface* network, Mutex& lock, ModuleWorkQueuePtr work, ModuleConsumerPtr consumer) :
    lookup_(lookup), bounds_(bounds), network_(network), lock_(lock),
      work_(work), consumer_(consumer), doneCount_(0)
    {
    }

    virtual void enqueueReadyModules() const
    {
      if (!isDone())
      {
        ParallelModuleExecutionOrder order;
        {
          Guard g(lock_.get());
          order = scheduler_.schedule(*network_);
        }
        log_ << DEBUG_LOG << "Producer processing min group " << order.minGroup();
        auto groupIter = order.getGroup(order.minGroup());
        BOOST_FOREACH(const ParallelModuleExecutionOrder::ModulesByGroup::value_type& mod, groupIter)
        {
          auto module = network_->lookupModule(mod.second);

          if (module->executionState() != ModuleInterface::Executing)
          {
            log_ << DEBUG_LOG << "Producer pushing module " << mod.second << std::endl;
            work_->push(module);
            doneCount_.fetch_add(1);
            log_ << DEBUG_LOG << "Producer status: " << doneCount_ << " out of " << network_->nmodules() << std::endl;
          }
        }
      }
    }

    void operator()() const
    {
      ScopedExecutionBoundsSignaller signaller(bounds_, [&]() { return lookup_->errorCode(); });

      log_ << DEBUG_LOG << "Producer started" << std::endl;

      enqueueReadyModules();

      while (!isDone())
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
      
      log_ << DEBUG_LOG << "Producer is done." << std::endl;
    }

    bool isDone() const 
    {
      return doneCount_ >= network_->nmodules();
    }
  private:
    static ModuleWaiting filter() { return ModuleWaiting(); }
    static BoostGraphParallelScheduler scheduler_;
    const ExecutableLookup* lookup_;
    const ExecutionBounds& bounds_;
    const NetworkInterface* network_;
    Mutex& lock_;
    ModuleWorkQueuePtr work_;
    ModuleConsumerPtr consumer_;
    mutable boost::atomic<int> doneCount_;
    static Log& log_;
  };

  Log& ModuleProducer::log_ = Log::get("producer");

  BoostGraphParallelScheduler ModuleProducer::scheduler_(filter());
}

namespace SCIRun {
  namespace Dataflow {
    namespace Engine {
      class DynamicMultithreadedNetworkExecutorImpl
      {
      public:
        void init(const ExecutableLookup* lookup, const ParallelModuleExecutionOrder& order, const ExecutionBounds& bounds, const NetworkInterface* network, Mutex& lock)
        {
          using namespace ::detail;
          work_.reset(new ModuleWorkQueue(network->nmodules()));
          consumer_.reset(new ModuleConsumer(work_.get(), lookup));
          runner_.reset(new ModuleProducer(lookup, bounds, network, lock, work_, consumer_));
          consumer_->setProducer(runner_);
        }
        void run()
        {
          boost::thread consume(boost::ref(*consumer_));
          boost::thread execution(boost::ref(*runner_));
        }
      private:
        boost::shared_ptr<::detail::ModuleProducer> runner_;
        ::detail::ModuleWorkQueuePtr work_;
        ::detail::ModuleConsumerPtr consumer_;
      };
}}}

DynamicMultithreadedNetworkExecutor::DynamicMultithreadedNetworkExecutor(const NetworkInterface& network) : network_(network), impl_(new DynamicMultithreadedNetworkExecutorImpl) {}

void DynamicMultithreadedNetworkExecutor::executeAll(const ExecutableLookup& lookup, ParallelModuleExecutionOrder order, const ExecutionBounds& bounds)
{
  static Mutex lock("live-scheduler");
  
  impl_->init(&lookup, order, bounds, &network_, lock);
  impl_->run();
}
