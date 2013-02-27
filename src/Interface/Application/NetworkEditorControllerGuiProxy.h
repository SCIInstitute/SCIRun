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

#ifndef INTERFACE_APPLICATION_NETWORKEDITORCONTROLLERGUIPROXY_H
#define INTERFACE_APPLICATION_NETWORKEDITORCONTROLLERGUIPROXY_H

#include <QObject>
#include <boost/shared_ptr.hpp>
#include <Dataflow/Network/NetworkFwd.h>

namespace SCIRun {
  namespace Dataflow { namespace Engine { class NetworkEditorController; }}

namespace Gui {
  
  class NetworkEditorControllerGuiProxy : public QObject
  {
    Q_OBJECT
  public:
    explicit NetworkEditorControllerGuiProxy(boost::shared_ptr<SCIRun::Dataflow::Engine::NetworkEditorController> controller);
  public Q_SLOTS:
    void addModule(const std::string& moduleName);
    void removeModule(const std::string& id);
    void requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to);
    void removeConnection(const SCIRun::Dataflow::Networks::ConnectionId& id);
    SCIRun::Dataflow::Networks::NetworkFileHandle saveNetwork() const;
    void loadNetwork(const SCIRun::Dataflow::Networks::NetworkFileHandle& xml);
    void executeAll(const SCIRun::Dataflow::Networks::ExecutableLookup& lookup);
    size_t numModules() const;
    int errorCode() const;
  public:
    SCIRun::Dataflow::Networks::NetworkGlobalSettings& getSettings();
  Q_SIGNALS:
    void moduleAdded(const std::string& name, SCIRun::Dataflow::Networks::ModuleHandle module);
    void moduleRemoved(const std::string& id);
    void connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription& cd);
    void connectionRemoved(const SCIRun::Dataflow::Networks::ConnectionId& id);
    void executionStarted();
    void executionFinished(int returnCode);
  private:
    boost::shared_ptr<SCIRun::Dataflow::Engine::NetworkEditorController> controller_;
  };

}
}

#endif