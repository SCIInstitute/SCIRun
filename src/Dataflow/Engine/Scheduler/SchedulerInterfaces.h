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

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Utils/Exception.h>
#include <boost/signals2.hpp>
#include <Dataflow/Engine/Scheduler/Share.h>

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

  //TODO: types for ParallelScheduler, etc

  typedef boost::signals2::signal<void()> ExecuteAllStartsSignalType;
  typedef boost::signals2::signal<void(int)> ExecuteAllFinishesSignalType;

  template <class OrderType>
  class NetworkExecutor
  {
  public:
    virtual ~NetworkExecutor() {}
    virtual void executeAll(const Networks::ExecutableLookup& lookup, const OrderType& order) = 0;
    
    boost::signals2::connection connectNetworkExecutionStarts(const ExecuteAllStartsSignalType::slot_type& subscriber)
    {
      return executeStarts_.connect(subscriber);
    }
    boost::signals2::connection connectNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber)
    {
      return executeFinishes_.connect(subscriber);
    }

  protected:
    ExecuteAllStartsSignalType executeStarts_;
    ExecuteAllFinishesSignalType executeFinishes_;
  };

  class ModuleExecutionOrder;
  typedef boost::shared_ptr<NetworkExecutor<ModuleExecutionOrder>> SerialNetworkExecutorHandle;
}
}}

#endif