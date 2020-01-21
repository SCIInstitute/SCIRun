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


#include <Core/Utils/Exception.h>
#include <Dataflow/Engine/Scheduler/SerialExecutionStrategy.h>
#include <Dataflow/Engine/Scheduler/BasicParallelExecutionStrategy.h>
#include <Dataflow/Engine/Scheduler/DynamicParallelExecutionStrategy.h>
#include <Dataflow/Engine/Scheduler/DesktopExecutionStrategyFactory.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;

DesktopExecutionStrategyFactory::DesktopExecutionStrategyFactory(const boost::optional<std::string>& threadMode) :
  threadMode_(threadMode),
  serial_(new SerialExecutionStrategy),
  parallel_(new BasicParallelExecutionStrategy),
  dynamic_(new DynamicParallelExecutionStrategy)
{
}

ExecutionStrategyHandle DesktopExecutionStrategyFactory::create(ExecutionStrategy::Type type) const
{
  switch (type)
  {
  case ExecutionStrategy::SERIAL:
    return serial_;
  case ExecutionStrategy::BASIC_PARALLEL:
    return parallel_;
  case ExecutionStrategy::DYNAMIC_PARALLEL:
    return dynamic_;
  default:
    THROW_INVALID_ARGUMENT("Unknown execution strategy type.");
  }
}

ExecutionStrategyHandle DesktopExecutionStrategyFactory::createDefault() const
{
  const ExecutionStrategy::Type latestWorkingVersion = ExecutionStrategy::DYNAMIC_PARALLEL;
  if (threadMode_)
  {
    LOG_DEBUG("found thread mode: ", *threadMode_);
    if (*threadMode_ == "serial")
      return create(ExecutionStrategy::SERIAL);
    if (*threadMode_ == "basicParallel")
      return create(ExecutionStrategy::BASIC_PARALLEL);
    if (*threadMode_ == "dynamicParallel")
      return create(ExecutionStrategy::DYNAMIC_PARALLEL);
    else
      return create(latestWorkingVersion);
  }
  else
  {
    LOG_TRACE("no thread mode found, using dynamic parallel"); /// @todo: update this to best working version
    return create(latestWorkingVersion);
  }
}
