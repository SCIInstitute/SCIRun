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
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>

#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/ModuleFactory.h>
#include <Dataflow/Serialization/Network/NetworkXMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Engine/Controller/DynamicPortManager.h>

#ifdef BUILD_WITH_PYTHON
#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>
#include <Dataflow/Engine/Controller/PythonImpl.h>
#endif


#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

NetworkEditorController::NetworkEditorController(ModuleFactoryHandle mf, ModuleStateFactoryHandle sf, ExecutionStrategyFactoryHandle executorFactory, AlgorithmFactoryHandle af, ModulePositionEditor* mpg) : 
  theNetwork_(new Network(mf, sf, af)),
  moduleFactory_(mf), 
  stateFactory_(sf), 
  algoFactory_(af),
  executorFactory_(executorFactory),
  modulePositionEditor_(mpg)
{
  dynamicPortManager_.reset(new DynamicPortManager(connectionAdded_, connectionRemoved_, this));

  //TODO should this class own the network or just keep a reference?

#ifdef BUILD_WITH_PYTHON
  NetworkEditorPythonAPI::setImpl(boost::make_shared<PythonImpl>(*this));
#endif

  configureLoggingLibrary();
}

NetworkEditorController::NetworkEditorController(SCIRun::Dataflow::Networks::NetworkHandle network, ExecutionStrategyFactoryHandle executorFactory, ModulePositionEditor* mpg)
  : theNetwork_(network), executorFactory_(executorFactory), modulePositionEditor_(mpg)
{
}

ModuleHandle NetworkEditorController::addModule(const std::string& moduleName)
{
  auto realModule = addModuleImpl(moduleName);
  /*emit*/ moduleAdded_(moduleName, realModule);
  printNetwork();
  return realModule;
}

ModuleHandle NetworkEditorController::addModuleImpl(const std::string& moduleName)
{
  //TODO: should pass in entire info struct
  ModuleLookupInfo info;
  info.module_name_ = moduleName;
  ModuleHandle realModule = theNetwork_->add_module(info);
  return realModule;
}

void NetworkEditorController::removeModule(const ModuleId& id)
{
  //auto disableDynamicPortManager(createDynamicPortSwitch());
  theNetwork_->remove_module(id);
  //before or after?
  // deciding on after: ProvenanceWindow/Manager wants the state *after* removal.
  /*emit*/ moduleRemoved_(id);
  
  printNetwork();
}

ModuleHandle NetworkEditorController::duplicateModule(const ModuleHandle& module)
{
  //auto disableDynamicPortManager(createDynamicPortSwitch());
  ENSURE_NOT_NULL(module, "Cannot duplicate null module");
  ModuleId id(module->get_id());
  auto newModule = addModuleImpl(id.name_);
  newModule->set_state(module->get_state()->clone());
  moduleAdded_(id.name_, newModule);

  //TODO: probably a pretty poor way to deal with what I think is a race condition with signaling the GUI to place the module widget.
  boost::this_thread::sleep(boost::posix_time::milliseconds(1));
  
  BOOST_FOREACH(InputPortHandle input, module->inputPorts())
  {
    if (input->nconnections() == 1)
    {
      auto conn = input->connection(0);
      auto source = conn->oport_;
      //TODO: this will work if we define PortId.id# to be 0..n, unique for each module. But what about gaps?
      requestConnection(source.get(), newModule->getInputPort(input->id()).get());
    }
  }
  
  return newModule;
}

void NetworkEditorController::connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo, const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName)
{
  auto newMod = addModule(newModuleName);

  //TODO: see above
  boost::this_thread::sleep(boost::posix_time::milliseconds(1));

  //TODO duplication
  if (portToConnect->isInput())
  {
    BOOST_FOREACH(OutputPortHandle p, newMod->outputPorts())
    {
      if (p->get_typename() == portToConnect->get_typename())
      {
        requestConnection(p.get(), portToConnect);
        return;
      }
    }
  }
  else
  {
    BOOST_FOREACH(InputPortHandle p, newMod->inputPorts())
    {
      if (p->get_typename() == portToConnect->get_typename())
      {
        requestConnection(p.get(), portToConnect);
        return;
      }
    }
  }
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

  ConnectionDescription desc(
    OutgoingConnectionDescription(out->getUnderlyingModuleId(), out->id()), 
    IncomingConnectionDescription(in->getUnderlyingModuleId(), in->id()));

  PortConnectionDeterminer q;
  if (q.canBeConnected(*from, *to))
  {
    ConnectionId id = theNetwork_->connect(ConnectionOutputPort(theNetwork_->lookupModule(desc.out_.moduleId_), desc.out_.portId_),
      ConnectionInputPort(theNetwork_->lookupModule(desc.in_.moduleId_), desc.in_.portId_));
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
  return ExecutionStrategy::connectNetworkExecutionStarts(subscriber);
}

boost::signals2::connection NetworkEditorController::connectNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber)
{
  return ExecutionStrategy::connectNetworkExecutionFinished(subscriber);
}

boost::signals2::connection NetworkEditorController::connectPortAdded(const PortAddedSignalType::slot_type& subscriber)
{
  return dynamicPortManager_->connectPortAdded(subscriber);
}

boost::signals2::connection NetworkEditorController::connectPortRemoved(const PortRemovedSignalType::slot_type& subscriber)
{
  return dynamicPortManager_->connectPortRemoved(subscriber);
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
    NetworkXMLConverter conv(moduleFactory_, stateFactory_, algoFactory_, this);
    theNetwork_ = conv.from_xml_data(xml->network);
    for (size_t i = 0; i < theNetwork_->nmodules(); ++i)
    {
      ModuleHandle module = theNetwork_->module(i);
      moduleAdded_(module->get_module_name(), module);
    }
    {
      auto flipper(createDynamicPortSwitch());
      //this is handled by NetworkXMLConverter now--but now the logic is convoluted. 
      //They need to be signaled again after the modules are signaled to alert the GUI. Hence the disabling of DPM
      BOOST_FOREACH(const ConnectionDescription& cd, theNetwork_->connections())
      {
        ConnectionId id = ConnectionId::create(cd);
        connectionAdded_(cd);
      }
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

void NetworkEditorController::executeAll(const ExecutableLookup* lookup)
{
  if (!currentExecutor_)
    currentExecutor_ = executorFactory_->create(ExecutionStrategy::BASIC_PARALLEL); //TODO: read some setting for default executor type

  currentExecutor_->executeAll(*theNetwork_, lookup ? *lookup : *theNetwork_);
}

NetworkHandle NetworkEditorController::getNetwork() const 
{
  return theNetwork_;
}

void NetworkEditorController::setNetwork(NetworkHandle nh)
{
  ENSURE_NOT_NULL(nh, "Null network.");
  theNetwork_ = nh;
}

NetworkGlobalSettings& NetworkEditorController::getSettings() 
{
  return theNetwork_->settings();
}

void NetworkEditorController::setExecutorType(int type)
{
  currentExecutor_ = executorFactory_->create((ExecutionStrategy::Type)type);
}

const ModuleDescriptionMap& NetworkEditorController::getAllAvailableModuleDescriptions() const
{
  return moduleFactory_->getAllAvailableModuleDescriptions();
}

void NetworkEditorController::configureLoggingLibrary()
{
  std::string pattern("%d{%Y-%m-%d %H:%M:%S.%l} [%p] %m%n");

  log4cpp::Appender *appender1 = new log4cpp::OstreamAppender("console", &std::cout);
  auto layout1 = new log4cpp::PatternLayout();
  std::string backupPattern1 = layout1->getConversionPattern();
  try
  {
    layout1->setConversionPattern(pattern);
  }
  catch (log4cpp::ConfigureFailure& exception)
  {
    // TODO: log?
    std::cerr << "Caught ConfigureFailure exception: " << exception.what() << std::endl
      << "Restoring original pattern: (" << backupPattern1 << ")" << std::endl;
    layout1->setConversionPattern(backupPattern1);
  }
  appender1->setLayout(layout1);

  log4cpp::Appender *appender2 = new log4cpp::FileAppender("default", "scirun5.log");
  auto layout2 = new log4cpp::PatternLayout();
  std::string backupPattern2 = layout1->getConversionPattern();
  try
  {
    layout2->setConversionPattern(pattern);
  }
  catch (log4cpp::ConfigureFailure& exception)
  {
    // TODO: log?
    std::cerr << "Caught ConfigureFailure exception: " << exception.what() << std::endl
      << "Restoring original pattern: (" << backupPattern2 << ")" << std::endl;
    layout2->setConversionPattern(backupPattern2);
  }
  appender2->setLayout(layout2);

  log4cpp::Category& root = log4cpp::Category::getRoot();
  //root.setPriority(log4cpp::Priority::DEBUG);
  root.addAppender(appender1);
  root.addAppender(appender2);
}

boost::shared_ptr<DisableDynamicPortSwitch> NetworkEditorController::createDynamicPortSwitch()
{
  return boost::make_shared<DisableDynamicPortSwitch>(dynamicPortManager_);
}

DisableDynamicPortSwitch::DisableDynamicPortSwitch(boost::shared_ptr<DynamicPortManager> dpm) : dpm_(dpm), first_(true)
{
  if (dpm_)
  {
    first_ = !dpm_->isDisabled();
    if (first_)
      dpm_->disable();
  }
}

DisableDynamicPortSwitch::~DisableDynamicPortSwitch()
{
  if (dpm_ && first_)
    dpm_->enable();
}