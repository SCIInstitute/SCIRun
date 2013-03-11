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

#ifndef ENGINE_NETWORK_NETWORKEDITORCONTROLLER_H
#define ENGINE_NETWORK_NETWORKEDITORCONTROLLER_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Scheduler/SchedulerInterfaces.h>
#include <Dataflow/Engine/Controller/ControllerInterfaces.h>
#include <Dataflow/Engine/Scheduler/ExecutionStrategy.h>
#include <Dataflow/Engine/Controller/Share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {
  
  typedef boost::signals2::signal<void (const std::string&, Networks::ModuleHandle)> ModuleAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ModuleId&)> ModuleRemovedSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionDescription&)> ConnectionAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionDescription&)> InvalidConnectionSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionId&)> ConnectionRemovedSignalType;

  // TODO Refactoring: split this class into two classes, NetworkEditorService and Controller.
  //   Service object will hold the Domain objects (network, factories), while Controller will manage the signal forwarding and the service's thread 
  //   This will be done in issue #231

  class SCISHARE NetworkEditorController : public NetworkIOInterface<Networks::NetworkFileHandle>
  {
  public:
    explicit NetworkEditorController(Networks::ModuleFactoryHandle mf, Networks::ModuleStateFactoryHandle sf, ExecutionStrategyFactoryHandle executorFactory, Networks::ModulePositionEditor* mpg = 0);
    explicit NetworkEditorController(Networks::NetworkHandle network, ExecutionStrategyFactoryHandle executorFactory, Networks::ModulePositionEditor* mpg = 0);

    Networks::ModuleHandle addModule(const std::string& moduleName);
    void removeModule(const Networks::ModuleId& id);
    Networks::ModuleHandle duplicateModule(const Networks::ModuleId& id);
    void requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to);
    void removeConnection(const Networks::ConnectionId& id);

    boost::signals2::connection connectModuleAdded(const ModuleAddedSignalType::slot_type& subscriber); 
    boost::signals2::connection connectModuleRemoved(const ModuleRemovedSignalType::slot_type& subscriber);
    boost::signals2::connection connectConnectionAdded(const ConnectionAddedSignalType::slot_type& subscriber);
    boost::signals2::connection connectConnectionRemoved(const ConnectionRemovedSignalType::slot_type& subscriber);
    boost::signals2::connection connectInvalidConnection(const InvalidConnectionSignalType::slot_type& subscriber);
    
    boost::signals2::connection connectNetworkExecutionStarts(const ExecuteAllStartsSignalType::slot_type& subscriber);
    boost::signals2::connection connectNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber);

    void executeAll(const Networks::ExecutableLookup& lookup);

    virtual Networks::NetworkFileHandle saveNetwork() const;
    virtual void loadNetwork(const Networks::NetworkFileHandle& xml);
    virtual void clear();

    Networks::NetworkHandle getNetwork() const;
    Networks::NetworkGlobalSettings& getSettings();

    void setExecutorType(int type);

    //TODO: eek, getting bloated here. Figure out a better way to wire this one in.
    void setModulePositionEditor(Networks::ModulePositionEditor* editor) { modulePositionEditor_ = editor; }

  private:
    void printNetwork() const;
    Networks::NetworkHandle theNetwork_;
    Networks::ModuleFactoryHandle moduleFactory_;
    Networks::ModuleStateFactoryHandle stateFactory_;
    ExecutionStrategyHandle currentExecutor_;
    ExecutionStrategyFactoryHandle executorFactory_;
    Networks::ModulePositionEditor* modulePositionEditor_;

    ModuleAddedSignalType moduleAdded_;
    ModuleRemovedSignalType moduleRemoved_; //not used yet
    ConnectionAddedSignalType connectionAdded_;
    ConnectionRemovedSignalType connectionRemoved_;
    InvalidConnectionSignalType invalidConnection_;
  };

  typedef boost::shared_ptr<NetworkEditorController> NetworkEditorControllerHandle;

}
}
}

#endif