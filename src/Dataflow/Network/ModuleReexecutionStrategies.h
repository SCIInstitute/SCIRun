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


#ifndef DATAFLOW_NETWORK_MODULEREEXECUTIONSTRATEGIES_H
#define DATAFLOW_NETWORK_MODULEREEXECUTIONSTRATEGIES_H

#include <boost/lexical_cast.hpp>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/PortManager.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE AlwaysReexecuteStrategy : public ModuleReexecutionStrategy
  {
  public:
    bool needToExecute() const override { return true; }
  };

  class SCISHARE InputsChangedChecker
  {
  public:
    virtual ~InputsChangedChecker() {}

    virtual bool inputsChanged() const = 0;
  };

  typedef boost::shared_ptr<InputsChangedChecker> InputsChangedCheckerHandle;

  class SCISHARE StateChangedChecker
  {
  public:
    virtual ~StateChangedChecker() {}

    virtual bool newStatePresent() const = 0;
  };

  typedef boost::shared_ptr<StateChangedChecker> StateChangedCheckerHandle;

  class SCISHARE OutputPortsCachedChecker
  {
  public:
    virtual ~OutputPortsCachedChecker() {}

    virtual bool outputPortsCached() const = 0;
  };

  typedef boost::shared_ptr<OutputPortsCachedChecker> OutputPortsCachedCheckerHandle;

  class SCISHARE DynamicReexecutionStrategy : public ModuleReexecutionStrategy
  {
  public:
    DynamicReexecutionStrategy(
      InputsChangedCheckerHandle inputsChanged,
      StateChangedCheckerHandle stateChanged,
      OutputPortsCachedCheckerHandle outputsCached);
    virtual bool needToExecute() const override;
  private:
    InputsChangedCheckerHandle inputsChanged_;
    StateChangedCheckerHandle stateChanged_;
    OutputPortsCachedCheckerHandle outputsCached_;
  };

  class SCISHARE InputsChangedCheckerImpl : public InputsChangedChecker
  {
  public:
    explicit InputsChangedCheckerImpl(const Module& module);
    virtual bool inputsChanged() const override;
  private:
    const Module& module_;
  };

  class SCISHARE StateChangedCheckerImpl : public StateChangedChecker
  {
  public:
    explicit StateChangedCheckerImpl(const Module& module);
    virtual bool newStatePresent() const override;
  private:
    const Module& module_;
  };

  class SCISHARE OutputPortsCachedCheckerImpl : public OutputPortsCachedChecker
  {
  public:
    explicit OutputPortsCachedCheckerImpl(const Module& module);
    virtual bool outputPortsCached() const override;
  private:
    const Module& module_;
  };

  class SCISHARE DynamicReexecutionStrategyFactory : public ReexecuteStrategyFactory
  {
  public:
    explicit DynamicReexecutionStrategyFactory(const boost::optional<std::string>& reexMode);
    ModuleReexecutionStrategyHandle create(const Module& module) const override;
  private:
    boost::optional<std::string> reexecuteMode_;
  };

}}}

#endif
