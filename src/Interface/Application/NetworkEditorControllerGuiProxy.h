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

#ifndef INTERFACE_APPLICATION_NETWORKEDITORCONTROLLERGUIPROXY_H
#define INTERFACE_APPLICATION_NETWORKEDITORCONTROLLERGUIPROXY_H

#include <QObject>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/ConnectionId.h>
#include <boost/optional/optional.hpp>

namespace SCIRun {
  namespace Dataflow { namespace Engine { class NetworkEditorController; struct DisableDynamicPortSwitch; struct ModuleCounter; }}

namespace Gui {

  class NetworkEditorControllerGuiProxy : public QObject
  {
    Q_OBJECT
  public:
    explicit NetworkEditorControllerGuiProxy(boost::shared_ptr<SCIRun::Dataflow::Engine::NetworkEditorController> controller);
  public Q_SLOTS:
    void addModule(const std::string& moduleName);
    void removeModule(const SCIRun::Dataflow::Networks::ModuleId& id);
    void interrupt(const SCIRun::Dataflow::Networks::ModuleId& id);
    boost::optional<SCIRun::Dataflow::Networks::ConnectionId> requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to);
    void removeConnection(const SCIRun::Dataflow::Networks::ConnectionId& id);
    void duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle& module);
    void connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo, const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);
    SCIRun::Dataflow::Networks::NetworkFileHandle saveNetwork() const;
    SCIRun::Dataflow::Networks::NetworkFileHandle serializeNetworkFragment(SCIRun::Dataflow::Networks::ModuleFilter modFilter, SCIRun::Dataflow::Networks::ConnectionFilter connFilter) const;
    void loadNetwork(const SCIRun::Dataflow::Networks::NetworkFileHandle& xml);
    void appendToNetwork(const SCIRun::Dataflow::Networks::NetworkFileHandle& xml);
    void executeAll(const SCIRun::Dataflow::Networks::ExecutableLookup& lookup);
    void executeModule(const SCIRun::Dataflow::Networks::ModuleHandle& module, const SCIRun::Dataflow::Networks::ExecutableLookup& lookup);
    size_t numModules() const;
    int errorCode() const;
    void setExecutorType(int type);
    void cleanUpNetwork();
  public:
    const SCIRun::Dataflow::Networks::ModuleDescriptionMap& getAllAvailableModuleDescriptions() const;
    SCIRun::Dataflow::Networks::NetworkGlobalSettings& getSettings();
    boost::shared_ptr<SCIRun::Dataflow::Engine::DisableDynamicPortSwitch> createDynamicPortSwitch();
  Q_SIGNALS:
    void moduleAdded(const std::string& name, SCIRun::Dataflow::Networks::ModuleHandle module, const SCIRun::Dataflow::Engine::ModuleCounter& count);
    void moduleRemoved(const SCIRun::Dataflow::Networks::ModuleId& id);
    void connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription& cd);
    void connectionRemoved(const SCIRun::Dataflow::Networks::ConnectionId& id);
    void portAdded(const SCIRun::Dataflow::Networks::ModuleId& mid, const SCIRun::Dataflow::Networks::PortId& pid);
    void portRemoved(const SCIRun::Dataflow::Networks::ModuleId& mid, const SCIRun::Dataflow::Networks::PortId& pid);
    void executionStarted();
    void executionFinished(int returnCode);
    void networkDoneLoading(int nMod);
    void snippetNeedsMoving(const std::string& name);
  private:
    boost::shared_ptr<SCIRun::Dataflow::Engine::NetworkEditorController> controller_;
  };

}
}

#endif
