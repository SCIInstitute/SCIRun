/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef ENGINE_SCHEDULER_DYNAMICEXECUTOR_WORKUNITCONSUMER_H
#define ENGINE_SCHEDULER_DYNAMICEXECUTOR_WORKUNITCONSUMER_H

#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkQueue.h>
#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkUnitProducerInterface.h>
#include <Dataflow/Engine/Scheduler/DynamicExecutor/WorkUnitExecutor.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Core/Logging/Log.h>
#include <boost/thread/thread.hpp>

#include <Dataflow/Engine/Scheduler/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {
namespace DynamicExecutor {

  class SCISHARE ExecutionThreadGroup : boost::noncopyable
  {
  public:
    ExecutionThreadGroup() : executeThreads_(new boost::thread_group) {}
    void startExecution(const ModuleExecutor& executor)
    {
      executeThreads_->create_thread(boost::bind(&ModuleExecutor::run, executor));
    }
    void joinAll()
    {
      executeThreads_->join_all();
    }
  private:
    mutable boost::shared_ptr<boost::thread_group> executeThreads_;
    //std::map<std::string, boost::thread*> threadsByModuleId_;
  };

  class SCISHARE ModuleConsumer : boost::noncopyable
  {
  public:
    explicit ModuleConsumer(ModuleWorkQueuePtr workQueue, const Networks::ExecutableLookup* lookup, ProducerInterfacePtr producer,
      ExecutionThreadGroup& executeThreadGroup) :
    work_(workQueue), producer_(producer), lookup_(lookup),
    executeThreadGroup_(executeThreadGroup),
    shouldLog_(SCIRun::Core::Logging::Log::get().verbose())
    {
      log_.setVerbose(shouldLog_);
      if (shouldLog_)
        log_ << Core::Logging::DEBUG_LOG << "Consumer created." << std::endl;
    }
    void operator()() const
    {
      if (!producer_)
      {
        if (shouldLog_)
          log_ << Core::Logging::DEBUG_LOG << "Consumer quitting due to no producer pointer." << std::endl;
        return;
      }

      log_ << Core::Logging::DEBUG_LOG << "Consumer started." << std::endl;

      while (!producer_->isDone() || moreWork())
      {
        if (moreWork())
        {
          if (shouldLog_)
            log_ << Core::Logging::DEBUG_LOG << "\tConsumer thinks work queue is not empty.";

          if (shouldLog_)
            log_ << Core::Logging::DEBUG_LOG << "\tConsumer accessing front of work queue.";
          Networks::ModuleHandle unit;
          work_->pop(unit);
          if (shouldLog_)
            log_ << Core::Logging::DEBUG_LOG << "\tConsumer popping front of work queue.";

          if (unit)
          {
            if (shouldLog_)
              log_ << Core::Logging::DEBUG_LOG << "~~~Processing " << unit->get_id();

            ModuleExecutor executor(unit, lookup_, producer_);
            executeThreadGroup_.startExecution(executor);
          }
          else
          {
            if (shouldLog_)
              log_ << Core::Logging::DEBUG_LOG << "\tConsumer received null module";
          }
        }
      }
      log_ << Core::Logging::DEBUG_LOG << "Consumer done." << std::endl;
    }

    bool moreWork() const
    {
      return !work_->empty();
    }

  private:
    ModuleWorkQueuePtr work_;
    ProducerInterfacePtr producer_;
    const Networks::ExecutableLookup* lookup_;
    ExecutionThreadGroup& executeThreadGroup_;

    static Core::Logging::Log& log_;
    bool shouldLog_;
  };

  typedef boost::shared_ptr<ModuleConsumer> ModuleConsumerPtr;

}}
}}

#endif
