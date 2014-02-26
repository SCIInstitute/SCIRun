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
  struct ScopedModuleExecutionBounds
  {
    static void logExecStart(const ModuleId& id)
    {
      Log::get() << INFO << "!!! starting " << id.id_;
    }

    static void logExecEnd(const ModuleId& id)
    {
      Log::get() << INFO << " ~~~ ending " << id.id_;
    }
  };

  template <class Unit>
  class WorkQueue
  {
  public:
    typedef boost::lockfree::spsc_queue<Unit> Impl;
  };

  typedef WorkQueue<ModuleHandle>::Impl ModuleWorkQueue;
  typedef boost::shared_ptr<ModuleWorkQueue> ModuleWorkQueuePtr;

  typedef std::list<ModuleHandle> WaitingList;
  typedef std::list<ModuleHandle> DoneList;

  class FinishingProcess
  {
  public:
    virtual ~FinishingProcess() {}
    virtual bool isDone() const = 0;
  };

  class ModuleProducer
  {

  public:
    ModuleProducer(const NetworkInterface* network, const Scheduler<ParallelModuleExecutionOrder>* scheduler, Mutex& lock,
      ModuleWorkQueue& workQueue, WaitingList& list) : 
      network_(network), scheduler_(scheduler), lock_(lock), work_(workQueue), waiting_(list)
    {
      //waiting_.sort();
      //log_ << INFO << "WorkUnitProducer starting. Sorted work list:";
      //std::for_each(list.begin(), list.end(), [](UnitPtr u) { log_ << INFO << *u << "\n"; });
    }

      #if 0
    void run()
    {
      log_ << INFO << "Producer started." << std::endl;
      while (!waiting_.empty())
      {
        while (readyToProcess_)
        {
          Guard g(lock_.get());
          if (scheduler_ && network_)
          {
            auto order = scheduler_->schedule(*network_);
            Log::get() << INFO << "NETWORK ORDER~~~completed module = " << id << " ~~~next up:", order
            BOOST_FOREACH(const ParallelModuleExecutionOrder::value_type& v, order.getGroup(order.minGroup()))
            {
              stream << "\n\t" << v.first << " " << v.second;
            }
          }
          else
            Log::get() << INFO << "NETWORK ORDER~~~\n <<<null>>>\n";
        }


        for (auto i = waiting_.begin(); i != waiting_.end(); )
        {
          if ((*i)->ready)
          {
            log_ << INFO << "\tProducer: Transferring ready unit " << (*i)->id << std::endl;

            work_.push(*i);

            log_ << INFO << "\tProducer: Done transferring ready unit " << (*i)->id << std::endl;

            i = waiting_.erase(i);
          }
          else
            ++i;
        }
        if (workDone() && !waiting_.empty() && (*waiting_.begin())->priority > currentPriority_)
        {
          currentPriority_ = (*waiting_.begin())->priority;
          log_ << INFO << "\tProducer: Setting as ready units with priority = " << currentPriority_ << std::endl;
          for (auto i = waiting_.begin(); i != waiting_.end(); ++i)
          {
            if ((*i)->priority == currentPriority_)
              (*i)->ready = true;
          }
        }
      }
      log_ << INFO << "Producer done." << std::endl;
    }
    #endif
    bool isDone() const
    {
      return waiting_.empty();
    }
    bool workDone() const
    {
      return work_.empty();
    }
  private:
    const NetworkInterface* network_;
    const Scheduler<ParallelModuleExecutionOrder>* scheduler_;
    Mutex& lock_;
    ModuleWorkQueue& work_;
    WaitingList& waiting_;
    static Log& log_;
  };

  class SchedulePrinter
  {
  public:
    SchedulePrinter(const NetworkInterface* network, const Scheduler<ParallelModuleExecutionOrder>* scheduler, Mutex& lock) : network_(network), scheduler_(scheduler), lock_(lock)
    {}

    void printNetworkOrder(const ModuleId& id)
    {
      Guard g(lock_.get());
      if (scheduler_ && network_)
      {
        auto order = scheduler_->schedule(*network_);
        printMinGroup(Log::get() << INFO << "NETWORK ORDER~~~completed module = " << id << " ~~~next up:", order);
      }
      else
        Log::get() << INFO << "NETWORK ORDER~~~\n <<<null>>>\n";
    }

  private:
    void printMinGroup(Log::Stream& stream, const ParallelModuleExecutionOrder& order)
    {
      BOOST_FOREACH(const ParallelModuleExecutionOrder::value_type& v, order.getGroup(order.minGroup()))
      {
        stream << "\n\t" << v.first << " " << v.second;
      }
    }


    const NetworkInterface* network_;
    const Scheduler<ParallelModuleExecutionOrder>* scheduler_;
    Mutex& lock_;
  };

  struct ModuleExecutor
  {
    ModuleExecutor(ModuleHandle mod, const ExecutableLookup* lookup, SchedulePrinter* printer) : module_(mod), lookup_(lookup), printer_(printer)
    {
    }
    void run()
    {
      Log::get() << INFO << "Module Executor: " << module_->get_id() << std::endl;
      //auto exec = lookup_->lookupExecutable(module_->get_id());
      //boost::signals2::scoped_connection s(exec->connectExecuteEnds(boost::bind(&SchedulePrinter::printNetworkOrder, *printer_, _1)));
      //exec->execute();
    }

    ModuleHandle module_;
    const ExecutableLookup* lookup_;
    SchedulePrinter* printer_;
  };

  class ModuleConsumer
  {
  public:
    explicit ModuleConsumer(ModuleWorkQueue* workQueue, const ExecutableLookup* lookup, SchedulePrinter* printer) :
    work_(workQueue), producer_(0), lookup_(lookup), printer_(printer), callCount_(0)
    {
      log_ << INFO << "Consumer created." << std::endl;
    }
    void operator()() const
    {
      if (!producer_)
        return;
      callCount_.fetch_add(1);
      log_ << INFO << "Consumer started. #" << callCount_;
      while (!producer_->isDone())
      {
        //log_ << INFO << "\tConsumer thinks producer is not done.";
        while (moreWork())
        {
          log_ << INFO << "\tConsumer thinks work queue is not empty.";

          log_ << INFO << "\tConsumer accessing front of work queue.";
          ModuleHandle unit;
          work_->pop(unit);
          log_ << INFO << "\tConsumer popping front of work queue.";

          if (unit)
          {

            log_ << INFO << "~~~Processing " << unit->get_id();

            //unit->run();
            ModuleExecutor executor(unit, lookup_, printer_);
            boost::thread t(boost::bind(&ModuleExecutor::run, executor));
          }
          else
            log_ << INFO << "\tConsumer received null module";

          //done_.push_back(unit);

          //log_ << INFO << "\tConsumer: adding done unit, done size = " << done_.size();
        }
      }
      log_ << INFO << "Consumer done." << std::endl;
    }

    bool moreWork() const
    {
      return !work_->empty();
    }

    void setProducer(boost::shared_ptr<const FinishingProcess> producer) { producer_ = producer; }

  private:
    ModuleWorkQueue* work_;
    boost::shared_ptr<const FinishingProcess> producer_;
    const ExecutableLookup* lookup_;
    SchedulePrinter* printer_;
    //DoneList& done_;
    static Log& log_;
    mutable boost::atomic<int> callCount_;
  };

  Log& ModuleProducer::log_ = Log::get();
  Log& ModuleConsumer::log_ = Log::get();
  
  typedef boost::shared_ptr<ModuleConsumer> ModuleConsumerPtr;

  struct ModuleWaiting
  {
    bool operator()(ModuleHandle mh) const
    {
      return mh->executionState() != ModuleInterface::Completed;
    }
  };

  struct DynamicParallelExecution : public FinishingProcess
  {
    DynamicParallelExecution(const ExecutableLookup* lookup, const ParallelModuleExecutionOrder& order, const ExecutionBounds& bounds, 
      const NetworkInterface* network, Mutex& lock, ModuleWorkQueuePtr work, ModuleConsumerPtr consumer) :
        lookup_(lookup), order_(order), bounds_(bounds), network_(network),
        printer_(network_, &scheduler_, lock), work_(work), consumer_(consumer),
        isDone_(false)
    {
    }
    
    void operator()() const
    {
      ScopedExecutionBoundsSignaller signaller(bounds_, [&]() { return lookup_->errorCode(); });
      for (int group = order_.minGroup(); group <= order_.maxGroup(); ++group)
      {
        auto groupIter = order_.getGroup(group);

        std::vector<boost::function<void()>> tasks;

        std::transform(groupIter.first, groupIter.second, std::back_inserter(tasks), 
          [&](const ParallelModuleExecutionOrder::ModulesByGroup::value_type& mod) -> boost::function<void()>
        {
          return [=]() 
          { 
            Log::get() << INFO << "Producer looking up " << mod.second;
            work_->push(network_->lookupModule(mod.second));
            auto exec = lookup_->lookupExecutable(mod.second);
            boost::signals2::scoped_connection s(exec->connectExecuteEnds(boost::bind(&SchedulePrinter::printNetworkOrder, printer_, _1)));
            exec->execute(); 
          };
        });

        //std::cout << "Running group " << group << " of size " << tasks.size();
        Parallel::RunTasks([&](int i) { tasks[i](); }, static_cast<int>(tasks.size()));
      }
      isDone_ = true;
    }

    bool isDone() const 
    {
      return isDone_;
    }

    static ModuleWaiting filter() { return ModuleWaiting(); }
    static BoostGraphParallelScheduler scheduler_;
    const ExecutableLookup* lookup_;
    ParallelModuleExecutionOrder order_;
    const ExecutionBounds& bounds_;
    const NetworkInterface* network_;
    SchedulePrinter printer_;
    ModuleWorkQueuePtr work_;
    ModuleConsumerPtr consumer_;
    mutable bool isDone_;
  };
  
  BoostGraphParallelScheduler DynamicParallelExecution::scheduler_(filter());
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
    consumer_.reset(new ModuleConsumer(work_.get(), lookup, 0));
    runner_.reset(new DynamicParallelExecution(lookup, order, bounds, network, lock, work_, consumer_));
    consumer_->setProducer(runner_);
  }
  void run()
  {
    boost::thread consume(boost::ref(*consumer_));
    boost::thread execution(boost::ref(*runner_));
  }
private:
  boost::shared_ptr<::detail::DynamicParallelExecution> runner_;
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
