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

#include <Dataflow/Engine/Controller/NetworkEditorController.h>

#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Serialization/Network/NetworkXMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Engine/Scheduler/BoostGraphSerialScheduler.h>
#include <Dataflow/Engine/Scheduler/LinearSerialNetworkExecutor.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

NetworkEditorController::NetworkEditorController(ModuleFactoryHandle mf, ModuleStateFactoryHandle sf, SerialNetworkExecutorHandle exe, ModulePositionEditor* mpg) : 
  moduleFactory_(mf), 
  stateFactory_(sf), 
  executor_(exe),
  modulePositionEditor_(mpg)
{
  //TODO should this class own or just keep a reference?
  theNetwork_.reset(new Network(mf, sf));
}

NetworkEditorController::NetworkEditorController(SCIRun::Dataflow::Networks::NetworkHandle network, SerialNetworkExecutorHandle exe, ModulePositionEditor* mpg)
  : theNetwork_(network), executor_(exe), modulePositionEditor_(mpg)
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
  theNetwork_->remove_module(id);
  //before or after?
  // deciding on after: HistoryWindow/Manager wants the state *after* removal.
  /*emit*/ moduleRemoved_(id);
  
  printNetwork();
}

void NetworkEditorController::printNetwork() const
{
  //TODO: and make this switchable
  if (false)
  {
    //TODO: use real logger here
    if (theNetwork_)
      std::cout << theNetwork_->toString() << std::endl;
  }
}

void NetworkEditorController::requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to)
{
  ENSURE_NOT_NULL(from, "from port");
  ENSURE_NOT_NULL(to, "to port");

  auto out = from->isInput() ? to : from;     
  auto in = from->isInput() ? from : to;     

  SCIRun::Dataflow::Networks::ConnectionDescription desc(
    SCIRun::Dataflow::Networks::OutgoingConnectionDescription(out->getUnderlyingModuleId(), out->getIndex()), 
    SCIRun::Dataflow::Networks::IncomingConnectionDescription(in->getUnderlyingModuleId(), in->getIndex()));

  PortConnectionDeterminer q;
  if (q.canBeConnected(*from, *to))
  {
    ConnectionId id = theNetwork_->connect(ConnectionOutputPort(theNetwork_->lookupModule(desc.out_.moduleId_), desc.out_.port_),
      ConnectionInputPort(theNetwork_->lookupModule(desc.in_.moduleId_), desc.in_.port_));
    if (!id.id_.empty())
      connectionAdded_(desc);
    
    printNetwork();
  }
  else
  {
    //TODO: use real logger
    std::cout << "Invalid Connection request: input port is full, or ports are different datatype or same i/o type, or on the same module." << std::endl;
    invalidConnection_(desc);
  }
}

void NetworkEditorController::removeConnection(const ConnectionId& id)
{
  if (theNetwork_->disconnect(id))
    connectionRemoved_(id);
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

boost::signals2::connection NetworkEditorController::connectConnectionRemoved(const ConnectionRemovedSignalType::slot_type& subscriber)
{
  return connectionRemoved_.connect(subscriber);
}

boost::signals2::connection NetworkEditorController::connectInvalidConnection(const InvalidConnectionSignalType::slot_type& subscriber)
{
  return invalidConnection_.connect(subscriber);
}

boost::signals2::connection NetworkEditorController::connectNetworkExecutionStarts(const ExecuteAllStartsSignalType::slot_type& subscriber)
{
  return executor_->connectNetworkExecutionStarts(subscriber);
}

boost::signals2::connection NetworkEditorController::connectNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber)
{
  return executor_->connectNetworkExecutionFinished(subscriber);
}

NetworkFileHandle NetworkEditorController::saveNetwork() const
{
  NetworkToXML conv(modulePositionEditor_);
  return conv.to_xml_data(theNetwork_);
}

void NetworkEditorController::loadNetwork(const NetworkFileHandle& xml)
{
  if (xml)
  {
    NetworkXMLConverter conv(moduleFactory_, stateFactory_);
    theNetwork_ = conv.from_xml_data(xml->network);
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
    if (modulePositionEditor_)
      modulePositionEditor_->moveModules(xml->modulePositions);
    else
      std::cout << "module position editor is null" << std::endl;
  }
}

void NetworkEditorController::clear()
{
  //std::cout << "NetworkEditorController::clear()" << std::endl;
}

void NetworkEditorController::executeAll(const ExecutableLookup& lookup)
{
  ModuleExecutionOrder order;
  try
  {
    BoostGraphSerialScheduler scheduler;
    order = scheduler.schedule(*theNetwork_);
  }
  catch (NetworkHasCyclesException&)
  {
    //TODO: use real logger here--or just let this exception bubble up--needs testing. 
    std::cout << "Cannot schedule execution: network has cycles. Please break all cycles and try again." << std::endl;
    return;
  }  
  executor_->executeAll(lookup, order);
}

NetworkHandle NetworkEditorController::getNetwork() const 
{
  return theNetwork_;
}

NetworkGlobalSettings& NetworkEditorController::getSettings() 
{
  return theNetwork_->settings();
}