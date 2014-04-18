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
/// @todo Documentation Dataflow/Engine/Controller/NetworkEditorController.h

#ifndef ENGINE_NETWORK_NETWORKEDITORCONTROLLER_H
#define ENGINE_NETWORK_NETWORKEDITORCONTROLLER_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/NetworkInterface.h> /// @todo: split out connectionmakerservice
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Dataflow/Engine/Scheduler/SchedulerInterfaces.h>
#include <Dataflow/Engine/Controller/ControllerInterfaces.h>
#include <Dataflow/Engine/Scheduler/ExecutionStrategy.h>
#include <Dataflow/Engine/Controller/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {
  
  typedef boost::signals2::signal<void (const std::string&, Networks::ModuleHandle)> ModuleAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ModuleId&)> ModuleRemovedSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionDescription&)> ConnectionAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionDescription&)> InvalidConnectionSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionId&)> ConnectionRemovedSignalType;
  typedef boost::signals2::signal<void (const Networks::ModuleId&, const Networks::PortId&)> PortAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ModuleId&, const Networks::PortId&)> PortRemovedSignalType;

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
    explicit NetworkEditorController(Networks::ModuleFactoryHandle mf, 
      Networks::ModuleStateFactoryHandle sf, 
      ExecutionStrategyFactoryHandle executorFactory, 
      Core::Algorithms::AlgorithmFactoryHandle algoFactory,
      Networks::ModulePositionEditor* mpg = 0);
    explicit NetworkEditorController(Networks::NetworkHandle network, ExecutionStrategyFactoryHandle executorFactory, Networks::ModulePositionEditor* mpg = 0);

//////////////////////////////////////////////////////////////////////////
//////////////////////Start: To be Pythonized/////////////////////////////
    virtual Networks::ModuleHandle addModule(const Networks::ModuleLookupInfo& info) override;
    Networks::ModuleHandle addModule(const std::string& name);
    void removeModule(const Networks::ModuleId& id);
    
    Networks::ModuleHandle duplicateModule(const Networks::ModuleHandle& module);
    void connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo, const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);

    void requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to);
    void removeConnection(const Networks::ConnectionId& id);

    void executeAll(const Networks::ExecutableLookup* lookup);

    virtual Networks::NetworkFileHandle saveNetwork() const;
    virtual void loadNetwork(const Networks::NetworkFileHandle& xml);
//////////////////////End: To be Pythonized///////////////////////////////
//////////////////////////////////////////////////////////////////////////

    virtual void clear();

    boost::signals2::connection connectModuleAdded(const ModuleAddedSignalType::slot_type& subscriber); 
    boost::signals2::connection connectModuleRemoved(const ModuleRemovedSignalType::slot_type& subscriber);
    boost::signals2::connection connectConnectionAdded(const ConnectionAddedSignalType::slot_type& subscriber);
    boost::signals2::connection connectConnectionRemoved(const ConnectionRemovedSignalType::slot_type& subscriber);
    boost::signals2::connection connectInvalidConnection(const InvalidConnectionSignalType::slot_type& subscriber);
    boost::signals2::connection connectPortAdded(const PortAddedSignalType::slot_type& subscriber);
    boost::signals2::connection connectPortRemoved(const PortRemovedSignalType::slot_type& subscriber);

    boost::signals2::connection connectNetworkExecutionStarts(const ExecuteAllStartsSignalType::slot_type& subscriber);
    boost::signals2::connection connectNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber);

    virtual void enableSignals() override;
    virtual void disableSignals() override;

    virtual Networks::NetworkHandle getNetwork() const override;
    virtual void setNetwork(Networks::NetworkHandle nh) override; 
    Networks::NetworkGlobalSettings& getSettings();

    boost::shared_ptr<DisableDynamicPortSwitch> createDynamicPortSwitch();

    void setExecutorType(int type);

    /// @todo: eek, getting bloated here. Figure out a better way to wire this one in.
    void setModulePositionEditor(Networks::ModulePositionEditor* editor) { modulePositionEditor_ = editor; }

    const Networks::ModuleDescriptionMap& getAllAvailableModuleDescriptions() const;

  private:
    void printNetwork() const;
    Networks::ModuleHandle addModuleImpl(const std::string& moduleName);
    Networks::NetworkHandle theNetwork_;
    Networks::ModuleFactoryHandle moduleFactory_;
    Networks::ModuleStateFactoryHandle stateFactory_;
    Core::Algorithms::AlgorithmFactoryHandle algoFactory_;
    ExecutionStrategyHandle currentExecutor_;
    ExecutionStrategyFactoryHandle executorFactory_;
    Networks::ModulePositionEditor* modulePositionEditor_;

    ModuleAddedSignalType moduleAdded_;
    ModuleRemovedSignalType moduleRemoved_; //not used yet
    ConnectionAddedSignalType connectionAdded_;
    ConnectionRemovedSignalType connectionRemoved_;
    InvalidConnectionSignalType invalidConnection_;

    boost::shared_ptr<DynamicPortManager> dynamicPortManager_;
    bool signalSwitch_;
  };

  typedef boost::shared_ptr<NetworkEditorController> NetworkEditorControllerHandle;

}
}
}

#endif
