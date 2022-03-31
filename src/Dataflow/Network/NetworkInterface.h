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


/// @todo Documentation Dataflow/Network/NetworkInterface.h


#ifndef DATAFLOW_NETWORK_NETWORK_INTERFACE_H
#define DATAFLOW_NETWORK_NETWORK_INTERFACE_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <vector>
#include <future>
#include <Dataflow/Network/share.h>

namespace SCIRun {
  using ThreadPtr = SharedPointer<std::thread>;
namespace Dataflow {
namespace Networks {

  /// @todo: hacky duplication...
  class SCISHARE ExecutableLookup
  {
  public:
    virtual ~ExecutableLookup() {}
    virtual ExecutableObject* lookupExecutable(const ModuleId& id) const = 0;
    virtual bool containsViewScene() const = 0;
    virtual int errorCode() const = 0;
  };

  typedef std::pair<ModuleHandle, PortId> ModulePortIdPair;

  struct SCISHARE ConnectionOutputPort : public ModulePortIdPair
  {
    ConnectionOutputPort(ModuleHandle m, const PortId& p) : ModulePortIdPair(m,p) {}

    /// @todo: only used in test code
    ConnectionOutputPort(ModuleHandle m, size_t index);
  };

  struct SCISHARE ConnectionInputPort : public ModulePortIdPair
  {
    ConnectionInputPort(ModuleHandle m,  const PortId& p) : ModulePortIdPair(m,p) {}

    /// @todo: only used in test code
    ConnectionInputPort(ModuleHandle m, size_t index);
  };


  class SCISHARE NetworkStateInterface : public ExecutableLookup
  {
  public:
    typedef std::vector<ConnectionDescription> ConnectionDescriptionList;

    virtual ~NetworkStateInterface() {}
    virtual ModuleHandle add_module(const ModuleLookupInfo& info) = 0;
    virtual bool remove_module(const ModuleId& id) = 0;
    virtual size_t nmodules() const = 0;
    virtual ModuleHandle module(size_t i) const = 0;
    virtual ModuleHandle lookupModule(const ModuleId& id) const = 0;

    virtual ConnectionId connect(const ConnectionOutputPort&, const ConnectionInputPort&) = 0;
    virtual bool disconnect(const ConnectionId&) = 0;
    virtual size_t nconnections() const = 0;
    virtual void disable_connection(const ConnectionId&) = 0;
    virtual ConnectionDescriptionList connections(bool includeVirtual) const = 0;
    virtual void incrementErrorCode(const ModuleId& moduleId) = 0;
    virtual NetworkGlobalSettings& settings() = 0;
    virtual void setModuleExecutionState(ModuleExecutionState::Value state, ModuleFilter filter) = 0;
    virtual std::vector<ModuleExecutionState::Value> moduleExecutionStates() const = 0;
    virtual void setExpandedModuleExecutionState(ModuleExecutionState::Value state, ModuleFilter filter) = 0;
    virtual void clear() = 0;

    virtual std::string toString() const = 0;
  };

  class SCISHARE NetworkSerializationInterface
  {
  public:
    virtual ~NetworkSerializationInterface() = default;
    virtual std::map<std::string, std::pair<ModuleLookupInfo, ModuleStateHandle>> modules() const = 0;
    virtual std::vector<ConnectionDescription> sortedConnections() const = 0;
  };

  class SCISHARE ConnectionMakerService
  {
  public:
    virtual ~ConnectionMakerService() {}
    virtual std::optional<ConnectionId> requestConnection(const PortDescriptionInterface* from, const PortDescriptionInterface* to) = 0;
  };

  struct SCISHARE NetworkAppendInfo
  {
    size_t newModuleStartIndex;
    std::map<std::string, std::string> moduleIdMapping;
  };

  class SCISHARE NetworkInterface : public ConnectionMakerService
  {
  public:
    virtual ~NetworkInterface() {}
    virtual NetworkStateHandle getNetwork() const = 0;
    virtual ModuleHandle addModule(const ModuleLookupInfo& info) = 0;
    virtual void enableSignals() = 0;
    virtual void disableSignals() = 0;
    virtual std::future<int> executeAll() = 0;
    virtual void loadXmlDataIntoNetwork(NetworkSerializationInterfaceHandle data) = 0;
    virtual NetworkAppendInfo appendXmlData(NetworkSerializationInterfaceHandle data) = 0;
    virtual NetworkHandle createSubnetwork() const = 0;
    virtual void setExecutableLookup(const ExecutableLookup* lookup) = 0;
  };

}}}


#endif
