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

#ifndef ENGINE_SCHEDULER_EXECUTION_STRATEGY_H
#define ENGINE_SCHEDULER_EXECUTION_STRATEGY_H

#include <Dataflow/Engine/Scheduler/SchedulerInterfaces.h>
#include <Dataflow/Engine/Scheduler/Share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  class SCISHARE ExecutionStrategy
  {
  public:
    virtual ~ExecutionStrategy() {}
    virtual void executeAll(const Networks::NetworkInterface& network, const Networks::ExecutableLookup& lookup) = 0;

    enum Type
    {
      SERIAL,
      BASIC_PARALLEL
      // better parallel, etc
    };

    //TODO::::???? badness?
    static boost::signals2::connection connectNetworkExecutionStarts(const ExecuteAllStartsSignalType::slot_type& subscriber);
    static boost::signals2::connection connectNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber);
  protected:
    static ExecutionBounds executionBounds_;
  };

  typedef boost::shared_ptr<ExecutionStrategy> ExecutionStrategyHandle;

  class SCISHARE ExecutionStrategyFactory
  {
  public:
    virtual ~ExecutionStrategyFactory() {}
    virtual ExecutionStrategyHandle create(ExecutionStrategy::Type type) = 0;
  };

  typedef boost::shared_ptr<ExecutionStrategyFactory> ExecutionStrategyFactoryHandle;

}
}}

#endif