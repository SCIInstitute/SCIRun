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
#include <Core/Dataflow/Network/NetworkFwd.h>

namespace SCIRun {
namespace Engine {
  
  typedef boost::signals2::signal<void (const std::string&, const SCIRun::Domain::Networks::ModuleInfoProvider&)> ModuleAddedSignalType;
  typedef boost::signals2::signal<void (const std::string&)> ModuleRemovedSignalType;
  typedef boost::signals2::signal<void (const SCIRun::Domain::Networks::ConnectionId&)> ConnectionAddedSignalType;

  class NetworkEditorController 
  {
  public:
    NetworkEditorController();
    explicit NetworkEditorController(SCIRun::Domain::Networks::NetworkHandle network);
    void addModule(const std::string& moduleName);
    void removeModule(const std::string& id);
    void addConnection(const SCIRun::Domain::Networks::ConnectionDescription& desc);
    void removeConnection(const SCIRun::Domain::Networks::ConnectionId& id);
    boost::signals2::connection connectModuleAdded(const ModuleAddedSignalType::slot_type& subscriber); 
    boost::signals2::connection connectModuleRemoved(const ModuleRemovedSignalType::slot_type& subscriber);
    boost::signals2::connection connectConnectionAdded(const ConnectionAddedSignalType::slot_type& subscriber);
  private:
    void printNetwork() const;
    SCIRun::Domain::Networks::NetworkHandle theNetwork_;
    ModuleAddedSignalType moduleAdded_;
    ModuleRemovedSignalType moduleRemoved_; //not used yet
    ConnectionAddedSignalType connectionAdded_;
  };

}
}

#endif