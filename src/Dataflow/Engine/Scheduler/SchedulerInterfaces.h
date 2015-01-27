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

#ifndef ENGINE_SCHEDULER_SCHEDULER_INTERFACES_H
#define ENGINE_SCHEDULER_SCHEDULER_INTERFACES_H

#include <iostream>
#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Logging/Log.h>
#include <Core/Utils/Exception.h>
#include <boost/signals2.hpp>
#include <Dataflow/Engine/Scheduler/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  struct NetworkHasCyclesException : virtual Core::InvalidArgumentException {};

  template <class OrderType>
  class Scheduler
  {
  public:
    virtual ~Scheduler() {}
    virtual OrderType schedule(const Networks::NetworkInterface& network) const = 0;
  };

  typedef boost::signals2::signal<void()> ExecuteAllStartsSignalType;
  typedef boost::signals2::signal<void(int)> ExecuteAllFinishesSignalType;

  struct SCISHARE ExecutionBounds : boost::noncopyable
  {
    ExecuteAllStartsSignalType executeStarts_;
    ExecuteAllFinishesSignalType executeFinishes_;
  };

  class SCISHARE ScopedExecutionBoundsSignaller
  {
  public:
    ScopedExecutionBoundsSignaller(const ExecutionBounds* bounds, boost::function<int()> errorCodeRetriever);
    ~ScopedExecutionBoundsSignaller();
  private:
    const ExecutionBounds* bounds_;
    boost::function<int()> errorCodeRetriever_;
  };

  struct SCISHARE ExecutionContext : boost::noncopyable
  {
    explicit ExecutionContext(const Networks::NetworkInterface& net);
    ExecutionContext(const Networks::NetworkInterface& net,
                     const Networks::ExecutableLookup& lkp) : network(net), lookup(lkp) {}

    ExecutionContext(const Networks::NetworkInterface& net,
                     const Networks::ExecutableLookup& lkp, Networks::ModuleFilter filter) : network(net), lookup(lkp), additionalFilter(filter) {}

    const Networks::NetworkInterface& network;
    const Networks::ExecutableLookup& lookup;
    Networks::ModuleFilter additionalFilter;

    Networks::ModuleFilter addAdditionalFilter(Networks::ModuleFilter filter) const;
    const ExecutionBounds& bounds() const;

    //todo: seems like a better place for this
    static boost::signals2::connection connectNetworkExecutionStarts(const ExecuteAllStartsSignalType::slot_type& subscriber);
    static boost::signals2::connection connectNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber);
    static ExecutionBounds executionBounds_;
  };

  template <class OrderType>
  class NetworkExecutor
  {
  public:
    virtual ~NetworkExecutor() {}
    //NOTE: OrderType passed by value so it can be copied across threads--it's more temporary than the network and the bounds objects
    virtual void execute(const ExecutionContext& context, OrderType order) = 0;
  };

  class ModuleExecutionOrder;
  typedef boost::shared_ptr<NetworkExecutor<ModuleExecutionOrder>> SerialNetworkExecutorHandle;

  template <class OrderType>
  void executeWithCycleCheck(Scheduler<OrderType>& scheduler, NetworkExecutor<OrderType>& executor, const ExecutionContext& context)
  {
    OrderType order;
    try
    {
      order = scheduler.schedule(context.network);
    }
    catch (NetworkHasCyclesException&)
    {
      /// @todo: use real logger here--or just let this exception bubble up--needs testing.
      SCIRun::Core::Logging::Log::get() << SCIRun::Core::Logging::ERROR_LOG << "Cannot schedule execution: network has cycles. Please break all cycles and try again." << std::endl;
      return;
    }
    executor.execute(context, order);
  }

  struct SCISHARE ExecuteAllModules
  {
    bool operator()(SCIRun::Dataflow::Networks::ModuleHandle) const { return true; }
    static const ExecuteAllModules& Instance();
  };

  struct SCISHARE ModuleWaitingFilter
  {
    bool operator()(SCIRun::Dataflow::Networks::ModuleHandle mh) const;
    static const ModuleWaitingFilter& Instance();
  };

  struct SCISHARE ExecuteSingleModule
  {
    explicit ExecuteSingleModule(SCIRun::Dataflow::Networks::ModuleHandle mod, const SCIRun::Dataflow::Networks::NetworkInterface& network);
    bool operator()(SCIRun::Dataflow::Networks::ModuleHandle) const;
  private:
    SCIRun::Dataflow::Networks::ModuleHandle module_;
    std::map<std::string, int> components_;
  };

  class SCISHARE WaitsForStartupInitialization
  {
  public:
    void waitForStartupInit(const Networks::ExecutableLookup& lookup) const;
  private:
    static bool waitedAlready_;
  };

}}}

#endif