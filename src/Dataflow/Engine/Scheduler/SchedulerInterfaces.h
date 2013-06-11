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
#include <boost/signals2.hpp>
#include <Core/Utils/Exception.h>
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
    virtual OrderType schedule(const Networks::NetworkInterface& network) = 0;
  };

  typedef boost::signals2::signal<void()> ExecuteAllStartsSignalType;
  typedef boost::signals2::signal<void(int)> ExecuteAllFinishesSignalType;

  struct SCISHARE ExecutionBounds : boost::noncopyable
  {
    ExecuteAllStartsSignalType executeStarts_;
    ExecuteAllFinishesSignalType executeFinishes_;
  };

  template <class OrderType>
  class NetworkExecutor
  {
  public:
    virtual ~NetworkExecutor() {}
    //NOTE: OrderType passed by value so it can be copied across threads--it's more temporary than the network and the bounds objects
    virtual void executeAll(const Networks::ExecutableLookup& lookup, OrderType order, const ExecutionBounds& bounds) = 0;
  };

  class ModuleExecutionOrder;
  typedef boost::shared_ptr<NetworkExecutor<ModuleExecutionOrder>> SerialNetworkExecutorHandle;

  template <class OrderType>
  void executeWithCycleCheck(Scheduler<OrderType>& scheduler, NetworkExecutor<OrderType>& executor,
    const Networks::NetworkInterface& network, const Networks::ExecutableLookup& lookup, const ExecutionBounds& bounds)
  {
    OrderType order;
    try
    {
      order = scheduler.schedule(network);
    }
    catch (NetworkHasCyclesException&)
    {
      //TODO: use real logger here--or just let this exception bubble up--needs testing. 
      std::cout << "Cannot schedule execution: network has cycles. Please break all cycles and try again." << std::endl;
      return;
    }
    executor.executeAll(lookup, order, bounds);
  }


}}}

#endif