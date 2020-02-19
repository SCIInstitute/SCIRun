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


#ifndef DATAFLOW_NETWORK_MODULEEXECUTIONINTERFACES_H
#define DATAFLOW_NETWORK_MODULEEXECUTIONINTERFACES_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/ExecutableObject.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE ModuleReexecutionStrategy
  {
  public:
    virtual ~ModuleReexecutionStrategy() {}
    virtual bool needToExecute() const = 0;
  };

  using ModuleReexecutionStrategyHandle = SharedPointer<ModuleReexecutionStrategy>;

  class SCISHARE ModuleExecutionState
  {
  public:
    enum Value
    {
      NotExecuted,
      Waiting,
      Executing,
      Completed,
      Errored
    };
    virtual Value currentState() const = 0;

    using ExecutionStateChangedSignalType = boost::signals2::signal<void(int)>;

    virtual boost::signals2::connection connectExecutionStateChanged(const ExecutionStateChangedSignalType::slot_type& subscriber) = 0;
    virtual bool transitionTo(Value state) = 0;

    virtual Value expandedState() const = 0;
    virtual void setExpandedState(Value state) = 0;

    virtual std::string currentColor() const = 0;
    virtual ~ModuleExecutionState() {}
  };

  using ModuleExecutionStateHandle = SharedPointer<ModuleExecutionState>;

  class SCISHARE ReexecuteStrategyFactory
  {
  public:
    virtual ~ReexecuteStrategyFactory() {}
    virtual ModuleReexecutionStrategyHandle create(const class Module& module) const = 0;
  };

}}}

#endif
