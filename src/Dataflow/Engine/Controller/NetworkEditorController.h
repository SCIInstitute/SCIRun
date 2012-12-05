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

#include <boost/signals2.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Controller/Share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {
  
  typedef boost::signals2::signal<void (const std::string&, Networks::ModuleHandle)> ModuleAddedSignalType;
  typedef boost::signals2::signal<void (const std::string&)> ModuleRemovedSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionDescription&)> ConnectionAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionId&)> ConnectionRemovedSignalType;


  class SCISHARE NetworkEditorController 
  {
  public:
    explicit NetworkEditorController(Networks::ModuleFactoryHandle mf, Networks::ModuleStateFactoryHandle sf);
    explicit NetworkEditorController(Networks::NetworkHandle network);
    Networks::ModuleHandle addModule(const std::string& moduleName);
    void removeModule(const std::string& id);
    void addConnection(const Networks::ConnectionDescription& desc);
    void removeConnection(const Networks::ConnectionId& id);

    boost::signals2::connection connectModuleAdded(const ModuleAddedSignalType::slot_type& subscriber); 
    boost::signals2::connection connectModuleRemoved(const ModuleRemovedSignalType::slot_type& subscriber);
    boost::signals2::connection connectConnectionAdded(const ConnectionAddedSignalType::slot_type& subscriber);
    boost::signals2::connection connectConnectionRemoved(const ConnectionRemovedSignalType::slot_type& subscriber);

    void executeAll(const Networks::ExecutableLookup& lookup, Networks::NetworkExecutionFinishedCallback func = 0);

    Networks::NetworkXMLHandle saveNetwork() const;
    void loadNetwork(const Networks::NetworkXML& xml);

    Networks::NetworkHandle getNetwork() const;

  private:
    void printNetwork() const;
    Networks::NetworkHandle theNetwork_;
    Networks::ModuleFactoryHandle moduleFactory_;
    Networks::ModuleStateFactoryHandle stateFactory_;
    ModuleAddedSignalType moduleAdded_;
    ModuleRemovedSignalType moduleRemoved_; //not used yet
    ConnectionAddedSignalType connectionAdded_;
    ConnectionRemovedSignalType connectionRemoved_;
  };

}
}
}

#endif