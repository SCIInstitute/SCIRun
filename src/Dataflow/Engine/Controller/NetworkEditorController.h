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


/// @todo Documentation Dataflow/Engine/Controller/NetworkEditorController.h

#ifndef ENGINE_NETWORK_NETWORKEDITORCONTROLLER_H
#define ENGINE_NETWORK_NETWORKEDITORCONTROLLER_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/NetworkInterface.h> /// @todo: split out connectionmakerservice
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Dataflow/Engine/Scheduler/SchedulerInterfaces.h>
#include <Dataflow/Engine/Controller/ControllerInterfaces.h>
#include <Dataflow/Engine/Scheduler/ExecutionStrategy.h>
#include <Dataflow/Network/ModuleFactory.h> // todo split out replacement impl types
#include <Core/Command/CommandFactory.h>
#include <Dataflow/Engine/Controller/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  /// @todo Refactoring: split this class into two classes, NetworkEditorService and Controller.
  //   Service object will hold the Domain objects (network, factories), while Controller will manage the signal forwarding and the service's thread
  //   This will be done in issue #231

  class SCISHARE NetworkCollaborators
  {
  public:
    Networks::NetworkStateHandle theNetwork_;
    Networks::ModuleFactoryHandle moduleFactory_;
    Networks::ModuleStateFactoryHandle stateFactory_;
    Core::Algorithms::AlgorithmFactoryHandle algoFactory_;
    Networks::ReexecuteStrategyFactoryHandle reexFactory_;
    ExecutionStrategyHandle currentExecutor_;
    ExecutionStrategyFactoryHandle executorFactory_;
    Core::Commands::GlobalCommandFactoryHandle cmdFactory_;
    Core::Commands::NetworkEventCommandFactoryHandle eventCmdFactory_;
    Networks::NetworkEditorSerializationManager* serializationManager_;
    const Networks::ExecutableLookup* lookup_ {nullptr};

    ExecutionManagerHandle executionManager_;
    SharedPointer<DynamicPortManager> dynamicPortManager_;
    SharedPointer<Networks::ReplacementImpl::ModuleReplacementFilter> replacementFilter_;
  };

  class SCISHARE NetworkSignalManager
  {
  public:
    ModuleAddedSignalType moduleAdded_;
    ModuleRemovedSignalType moduleRemoved_; //not used yet
    ConnectionAddedSignalType connectionAdded_;
    ConnectionRemovedSignalType connectionRemoved_;
    InvalidConnectionSignalType invalidConnection_;
    NetworkDoneLoadingSignalType networkDoneLoading_;


    bool signalSwitch_, loadingContext_;


    struct LoadingContext
    {
      explicit LoadingContext(bool& load);
      ~LoadingContext();
    private:
      bool& load_;
    };
  };

  class SCISHARE NetworkEditorController :
    public NetworkIOInterface<Networks::NetworkFileHandle>,
    public Networks::NetworkInterface
  {
  public:
    NetworkEditorController(Networks::ModuleFactoryHandle mf,
      Networks::ModuleStateFactoryHandle sf,
      ExecutionStrategyFactoryHandle executorFactory,
      Core::Algorithms::AlgorithmFactoryHandle algoFactory,
      Networks::ReexecuteStrategyFactoryHandle reexFactory,
      Core::Commands::GlobalCommandFactoryHandle cmdFactory,
      Core::Commands::NetworkEventCommandFactoryHandle eventCmdFactory,
      Networks::NetworkEditorSerializationManager* nesm = nullptr);
    NetworkEditorController(Networks::NetworkStateHandle network, ExecutionStrategyFactoryHandle executorFactory, Networks::NetworkEditorSerializationManager* nesm = nullptr);
    ~NetworkEditorController();

//////////////////////////////////////////////////////////////////////////
//////////////////////Start: To be Pythonized/////////////////////////////
    Networks::ModuleHandle addModule(const Networks::ModuleLookupInfo& info) override;
    Networks::ModuleHandle addModule(const std::string& name);
    void removeModule(const Networks::ModuleId& id);

    Networks::ModuleHandle duplicateModule(const Networks::ModuleHandle& module);
    Networks::ModuleHandle connectNewModule(const Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);
    struct InsertInfo { std::string newModuleName, endModuleId, inputPortName, inputPortId; };
    Networks::ModuleHandle insertNewModule(const Networks::PortDescriptionInterface* portToConnect, const InsertInfo& info);

    std::optional<Networks::ConnectionId> requestConnection(const Networks::PortDescriptionInterface* from, const Networks::PortDescriptionInterface* to) override;
    void removeConnection(const Networks::ConnectionId& id);

    std::future<int> executeAll() override;
    void executeModule(const Networks::ModuleHandle& module, bool executeUpstream);

    Networks::NetworkFileHandle saveNetwork() const override;
    void loadNetwork(const Networks::NetworkFileHandle& xml) override;

    void loadXmlDataIntoNetwork(Networks::NetworkSerializationInterfaceHandle data) override;
    Networks::NetworkAppendInfo appendXmlData(Networks::NetworkSerializationInterfaceHandle data) override;

    Networks::NetworkFileHandle serializeNetworkFragment(Networks::ModuleFilter modFilter, Networks::ConnectionFilter connFilter) const;
    void appendToNetwork(const Networks::NetworkFileHandle& xml);
//////////////////////End: To be Pythonized///////////////////////////////
//////////////////////////////////////////////////////////////////////////

    void clear() override;
    Networks::NetworkHandle createSubnetwork() const override;

    boost::signals2::connection connectModuleAdded(const ModuleAddedSignalType::slot_type& subscriber);
    boost::signals2::connection connectModuleRemoved(const ModuleRemovedSignalType::slot_type& subscriber);
    boost::signals2::connection connectConnectionAdded(const ConnectionAddedSignalType::slot_type& subscriber);
    boost::signals2::connection connectConnectionRemoved(const ConnectionRemovedSignalType::slot_type& subscriber);
    boost::signals2::connection connectInvalidConnection(const InvalidConnectionSignalType::slot_type& subscriber);
    boost::signals2::connection connectPortAdded(const PortAddedSignalType::slot_type& subscriber);
    boost::signals2::connection connectPortRemoved(const PortRemovedSignalType::slot_type& subscriber);

    boost::signals2::connection connectStaticNetworkExecutionStarts(const ExecuteAllStartsSignalType::slot_type& subscriber);
    boost::signals2::connection connectStaticNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber);

    boost::signals2::connection connectNetworkDoneLoading(const NetworkDoneLoadingSignalType::slot_type& subscriber);

    // headless hack
    void stopExecutionContextLoopWhenExecutionFinishes();

    void enableSignals() override;
    void disableSignals() override;

    Networks::NetworkStateHandle getNetwork() const override;
    Networks::NetworkGlobalSettings& getSettings();

    SharedPointer<DisableDynamicPortSwitch> createDynamicPortSwitch();

    void setExecutorType(int type);

    void setExecutableLookup(const Networks::ExecutableLookup* lookup) override;

    /// @todo: eek, getting bloated here. Figure out a better way to wire this one in.
    void setSerializationManager(Networks::NetworkEditorSerializationManager* nesm)
    {
      collabs_.serializationManager_ = nesm;
    }

    const Networks::ModuleDescriptionMap& getAllAvailableModuleDescriptions() const;

    const Networks::ReplacementImpl::ModuleLookupInfoSet& possibleReplacements(Networks::ModuleHandle module);

    void updateModulePositions(const Networks::ModulePositions& modulePositions, bool selectAll);

    void cleanUpNetwork();

    const Networks::ModuleFactory& moduleFactory() const { return *collabs_.moduleFactory_; }  //TOOD: lazy

    std::vector<Networks::ModuleExecutionState::Value> moduleExecutionStates() const;

  private:
    void printNetwork() const;
    Networks::ModuleHandle addModuleImpl(const Networks::ModuleLookupInfo& info);
    NetworkEditorController(const NetworkEditorController& other);

    std::future<int> executeGeneric(Networks::ModuleFilter filter);
    void initExecutor();
    ExecutionContextHandle createExecutionContext(Networks::ModuleFilter filter) const;

    NetworkCollaborators collabs_;
    NetworkSignalManager signals_;
  };

  typedef SharedPointer<NetworkEditorController> NetworkEditorControllerHandle;

}
}
}

#endif
