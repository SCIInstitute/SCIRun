/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

  struct SCISHARE ModuleCounter
  {
    ModuleCounter() : count(new boost::atomic<int>(0)) {}
    ModuleCounter(const ModuleCounter& rhs) : count(rhs.count)
    {
      //std::cout << "ModuleCounter copied" << std::endl;
    }
    void increment() const
    {
      count->fetch_add(1);
    }
    mutable boost::shared_ptr<boost::atomic<int>> count;
  };

  typedef boost::signals2::signal<void (const std::string&, Networks::ModuleHandle, ModuleCounter)> ModuleAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ModuleId&)> ModuleRemovedSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionDescription&)> ConnectionAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionDescription&)> InvalidConnectionSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionId&)> ConnectionRemovedSignalType;
  typedef boost::signals2::signal<void (const Networks::ModuleId&, const Networks::PortId&)> PortAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ModuleId&, const Networks::PortId&)> PortRemovedSignalType;
  typedef boost::signals2::signal<void (int)> NetworkDoneLoadingSignalType;

  class DynamicPortManager;

  struct SCISHARE DisableDynamicPortSwitch
  {
    explicit DisableDynamicPortSwitch(boost::shared_ptr<DynamicPortManager> dpm);
    ~DisableDynamicPortSwitch();
  private:
    bool first_;
    boost::shared_ptr<DynamicPortManager> dpm_;
  };

  /// @todo Refactoring: split this class into two classes, NetworkEditorService and Controller.
  //   Service object will hold the Domain objects (network, factories), while Controller will manage the signal forwarding and the service's thread
  //   This will be done in issue #231

  class SCISHARE NetworkEditorController : public NetworkIOInterface<Networks::NetworkFileHandle>, public Networks::NetworkEditorControllerInterface
  {
  public:
    NetworkEditorController(Networks::ModuleFactoryHandle mf,
      Networks::ModuleStateFactoryHandle sf,
      ExecutionStrategyFactoryHandle executorFactory,
      Core::Algorithms::AlgorithmFactoryHandle algoFactory,
      Networks::ReexecuteStrategyFactoryHandle reexFactory,
      Core::Commands::GlobalCommandFactoryHandle cmdFactory,
      Networks::NetworkEditorSerializationManager* nesm = 0);
    NetworkEditorController(Networks::NetworkHandle network, ExecutionStrategyFactoryHandle executorFactory, Networks::NetworkEditorSerializationManager* nesm = 0);

//////////////////////////////////////////////////////////////////////////
//////////////////////Start: To be Pythonized/////////////////////////////
    virtual Networks::ModuleHandle addModule(const Networks::ModuleLookupInfo& info) override;
    Networks::ModuleHandle addModule(const std::string& name);
    void removeModule(const Networks::ModuleId& id);
    void interruptModule(const Networks::ModuleId& id);

    Networks::ModuleHandle duplicateModule(const Networks::ModuleHandle& module);
    void connectNewModule(const Networks::ModuleHandle& moduleToConnectTo, const Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);

    boost::optional<Networks::ConnectionId> requestConnection(const Networks::PortDescriptionInterface* from, const Networks::PortDescriptionInterface* to) override;
    void removeConnection(const Networks::ConnectionId& id);

    void executeAll(const Networks::ExecutableLookup* lookup);
    void executeModule(const Networks::ModuleHandle& module, const Networks::ExecutableLookup* lookup);

    virtual Networks::NetworkFileHandle saveNetwork() const override;
    virtual void loadNetwork(const Networks::NetworkFileHandle& xml) override;

    Networks::NetworkFileHandle serializeNetworkFragment(Networks::ModuleFilter modFilter, Networks::ConnectionFilter connFilter) const;
    void appendToNetwork(const Networks::NetworkFileHandle& xml);
//////////////////////End: To be Pythonized///////////////////////////////
//////////////////////////////////////////////////////////////////////////

    virtual void clear() override;

    boost::signals2::connection connectModuleAdded(const ModuleAddedSignalType::slot_type& subscriber);
    boost::signals2::connection connectModuleRemoved(const ModuleRemovedSignalType::slot_type& subscriber);
    boost::signals2::connection connectConnectionAdded(const ConnectionAddedSignalType::slot_type& subscriber);
    boost::signals2::connection connectConnectionRemoved(const ConnectionRemovedSignalType::slot_type& subscriber);
    boost::signals2::connection connectInvalidConnection(const InvalidConnectionSignalType::slot_type& subscriber);
    boost::signals2::connection connectPortAdded(const PortAddedSignalType::slot_type& subscriber);
    boost::signals2::connection connectPortRemoved(const PortRemovedSignalType::slot_type& subscriber);

    boost::signals2::connection connectNetworkExecutionStarts(const ExecuteAllStartsSignalType::slot_type& subscriber);
    boost::signals2::connection connectNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber);

    boost::signals2::connection connectNetworkDoneLoading(const NetworkDoneLoadingSignalType::slot_type& subscriber);

    virtual void enableSignals() override;
    virtual void disableSignals() override;

    virtual Networks::NetworkHandle getNetwork() const override;
    virtual void setNetwork(Networks::NetworkHandle nh) override;
    Networks::NetworkGlobalSettings& getSettings();

    boost::shared_ptr<DisableDynamicPortSwitch> createDynamicPortSwitch();

    void setExecutorType(int type);

    /// @todo: eek, getting bloated here. Figure out a better way to wire this one in.
    void setSerializationManager(Networks::NetworkEditorSerializationManager* nesm) { serializationManager_ = nesm; }

    const Networks::ModuleDescriptionMap& getAllAvailableModuleDescriptions() const;

    const Networks::ReplacementImpl::ModuleLookupInfoSet& possibleReplacements(Networks::ModuleHandle module);

    void updateModulePositions(const SCIRun::Dataflow::Networks::ModulePositions& modulePositions);

    void cleanUpNetwork();

    const Networks::ModuleFactory& moduleFactory() const { return *moduleFactory_; }  //TOOD: lazy

  private:
    void printNetwork() const;
    Networks::ModuleHandle addModuleImpl(const Networks::ModuleLookupInfo& info);

    void executeGeneric(const Networks::ExecutableLookup* lookup, Networks::ModuleFilter filter);
    void initExecutor();
    ExecutionContextHandle createExecutionContext(const Networks::ExecutableLookup* lookup, Networks::ModuleFilter filter);

    Networks::NetworkHandle theNetwork_;
    Networks::ModuleFactoryHandle moduleFactory_;
    Networks::ModuleStateFactoryHandle stateFactory_;
    Core::Algorithms::AlgorithmFactoryHandle algoFactory_;
    Networks::ReexecuteStrategyFactoryHandle reexFactory_;
    ExecutionStrategyHandle currentExecutor_;
    ExecutionStrategyFactoryHandle executorFactory_;
    Core::Commands::GlobalCommandFactoryHandle cmdFactory_;
    Networks::NetworkEditorSerializationManager* serializationManager_;

    ExecutionQueueManager executionManager_;

    ModuleAddedSignalType moduleAdded_;
    ModuleRemovedSignalType moduleRemoved_; //not used yet
    ConnectionAddedSignalType connectionAdded_;
    ConnectionRemovedSignalType connectionRemoved_;
    InvalidConnectionSignalType invalidConnection_;
    NetworkDoneLoadingSignalType networkDoneLoading_;

    boost::shared_ptr<DynamicPortManager> dynamicPortManager_;
    bool signalSwitch_;
    boost::shared_ptr<Networks::ReplacementImpl::ModuleReplacementFilter> replacementFilter_;
  };

  typedef boost::shared_ptr<NetworkEditorController> NetworkEditorControllerHandle;

}
}
}

#endif
