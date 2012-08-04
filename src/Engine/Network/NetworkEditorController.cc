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

#include <iostream>

#include <Engine/Network/NetworkEditorController.h>

#include <Core/Dataflow/Network/Connection.h>
#include <Core/Dataflow/Network/Network.h>
#include <Core/Dataflow/Network/ModuleDescription.h>
#include <Core/Dataflow/Network/Module.h>
#include <Core/Serialization/Network/NetworkXMLSerializer.h>
#include <Engine/Scheduler/BoostGraphSerialScheduler.h>
#include <Engine/Scheduler/LinearSerialNetworkExecutor.h>

using namespace SCIRun;
using namespace SCIRun::Engine;
using namespace SCIRun::Domain::Networks;

NetworkEditorController::NetworkEditorController(ModuleFactoryHandle mf, ModuleStateFactoryHandle sf) : moduleFactory_(mf), stateFactory_(sf)
{
  //TODO should this class own or just keep a reference?
  theNetwork_.reset(new Network(mf, sf));
}

NetworkEditorController::NetworkEditorController(SCIRun::Domain::Networks::NetworkHandle network)
  : theNetwork_(network)
{
}

ModuleHandle NetworkEditorController::addModule(const std::string& moduleName)
{
  //TODO: should pass in entire info struct
  ModuleLookupInfo info;
  info.module_name_ = moduleName;
  ModuleHandle realModule = theNetwork_->add_module(info);
  /*emit*/ moduleAdded_(moduleName, realModule);
  printNetwork();
  return realModule;
}

void NetworkEditorController::removeModule(const std::string& id)
{
  //before or after?
  /*emit*/ moduleRemoved_(id);
  theNetwork_->remove_module(id);
  printNetwork();
}

void NetworkEditorController::printNetwork() const
{
  if (theNetwork_)
    std::cout << theNetwork_->toString() << std::endl;
}

void NetworkEditorController::addConnection(const SCIRun::Domain::Networks::ConnectionDescription& desc)
{
  ConnectionId id = theNetwork_->connect(theNetwork_->lookupModule(desc.moduleId1_), desc.port1_, theNetwork_->lookupModule(desc.moduleId2_), desc.port2_);
  if (!id.id_.empty())
    connectionAdded_(desc);
  printNetwork();
}

void NetworkEditorController::removeConnection(const ConnectionId& id)
{
  theNetwork_->disconnect(id);
  printNetwork();
}

boost::signals2::connection NetworkEditorController::connectModuleAdded(const ModuleAddedSignalType::slot_type& subscriber)
{
  return moduleAdded_.connect(subscriber);
}

boost::signals2::connection NetworkEditorController::connectModuleRemoved(const ModuleRemovedSignalType::slot_type& subscriber)
{
  return moduleRemoved_.connect(subscriber);
}

boost::signals2::connection NetworkEditorController::connectConnectionAdded(const ConnectionAddedSignalType::slot_type& subscriber)
{
  return connectionAdded_.connect(subscriber);
}

NetworkXMLHandle NetworkEditorController::saveNetwork() const
{
  NetworkToXML conv;
  return conv.to_xml_data(theNetwork_);
}

void NetworkEditorController::loadNetwork(const NetworkXML& xml)
{
  NetworkXMLConverter conv(moduleFactory_, stateFactory_);
  theNetwork_ = conv.from_xml_data(xml);
  for (size_t i = 0; i < theNetwork_->nmodules(); ++i)
  {
    ModuleHandle module = theNetwork_->module(i);
    moduleAdded_(module->get_module_name(), module);
  }
  BOOST_FOREACH(const ConnectionDescription& cd, theNetwork_->connections())
  {
    ConnectionId id = ConnectionId::create(cd);
    connectionAdded_(cd);
  }
}

void NetworkEditorController::executeAll(const SCIRun::Domain::Networks::ExecutableLookup& lookup)
{
  BoostGraphSerialScheduler scheduler;
  LinearSerialNetworkExecutor executor;
  try
  {
    executor.executeAll(lookup, scheduler.schedule(*theNetwork_));
  }
  catch (NetworkHasCyclesException&)
  {
    std::cout << "Cannot schedule execution: network has cycles. Please break all cycles and try again." << std::endl;
  }  
}

NetworkHandle NetworkEditorController::getNetwork() const 
{
  return theNetwork_;
}
