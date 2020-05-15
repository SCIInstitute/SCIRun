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


/// @todo Documentation Dataflow/Network/Network.h


#ifndef DATAFLOW_NETWORK_NETWORK_H
#define DATAFLOW_NETWORK_NETWORK_H

#include <boost/noncopyable.hpp>
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Dataflow/Network/NetworkSettings.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE Network : public NetworkInterface, boost::noncopyable
  {
  public:
    using Connections = std::map<ConnectionId, ConnectionHandle, OrderedByConnectionId>;
    using Modules = std::vector<ModuleHandle>;

    Network(ModuleFactoryHandle moduleFactory, ModuleStateFactoryHandle stateFactory, Core::Algorithms::AlgorithmFactoryHandle algoFactory, ReexecuteStrategyFactoryHandle reexFactory);
    ~Network();

    ModuleHandle add_module(const ModuleLookupInfo& info) override;
    bool remove_module(const ModuleId& id) override;
    size_t nmodules() const override;
    ModuleHandle module(size_t i) const override;
    ExecutableObject* lookupExecutable(const ModuleId& id) const override;
    ModuleHandle lookupModule(const ModuleId& id) const override;
    ConnectionId connect(const ConnectionOutputPort&, const ConnectionInputPort&) override;
    bool disconnect(const ConnectionId&) override;
    size_t nconnections() const override;
    void disable_connection(const ConnectionId&) override;
    ConnectionDescriptionList connections(bool includeVirtual) const override;
    int errorCode() const override;
    void incrementErrorCode(const ModuleId& moduleId) override;
    bool containsViewScene() const override;
    NetworkGlobalSettings& settings() override;
    std::string toString() const override;
    void setModuleExecutionState(ModuleExecutionState::Value state, ModuleFilter filter) override;
    std::vector<ModuleExecutionState::Value> moduleExecutionStates() const override;
    void setExpandedModuleExecutionState(ModuleExecutionState::Value state, ModuleFilter filter) override;
    boost::signals2::connection connectModuleInterrupted(ModuleInterruptedSignal::slot_function_type subscriber) const override;
    void interruptModuleRequest(const ModuleId& id) override;
    void clear() override;
  private:
    ModuleFactoryHandle moduleFactory_;
    ModuleStateFactoryHandle stateFactory_;
    Connections connections_;
    Modules modules_;
    int errorCode_;
    NetworkGlobalSettings settings_;
    mutable ModuleInterruptedSignal interruptModule_;
  };

}}}


#endif
