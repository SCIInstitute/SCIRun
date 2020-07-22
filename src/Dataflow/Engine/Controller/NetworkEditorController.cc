/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

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


/// @todo Documentation Dataflow/Engine/Controller/NetworkEditorController.cc

#include <iostream>
#include <boost/thread.hpp>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>

#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Serialization/Network/NetworkXMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Engine/Controller/DynamicPortManager.h>
#include <Core/Logging/Log.h>
#include <Dataflow/Engine/Scheduler/BoostGraphParallelScheduler.h>
#include <Dataflow/Engine/Scheduler/GraphNetworkAnalyzer.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/foreach.hpp>

#ifdef BUILD_WITH_PYTHON
#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>
#include <Dataflow/Engine/Controller/PythonImpl.h>
#endif

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::ReplacementImpl;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::Thread;

NetworkEditorController::NetworkEditorController(ModuleFactoryHandle mf, ModuleStateFactoryHandle sf, ExecutionStrategyFactoryHandle executorFactory,
  AlgorithmFactoryHandle af, ReexecuteStrategyFactoryHandle reex, GlobalCommandFactoryHandle cmdFactory,
  NetworkEventCommandFactoryHandle eventCmdFactory, NetworkEditorSerializationManager* nesm) :
  theNetwork_(new Network(mf, sf, af, reex)),
  moduleFactory_(mf),
  stateFactory_(sf),
  algoFactory_(af),
  reexFactory_(reex),
  executorFactory_(executorFactory),
  cmdFactory_(cmdFactory),
  eventCmdFactory_(eventCmdFactory ? eventCmdFactory : boost::make_shared<NullCommandFactory>()),
  serializationManager_(nesm),
  signalSwitch_(true),
  loadingContext_(false)
{
  dynamicPortManager_.reset(new DynamicPortManager(connectionAdded_, connectionRemoved_, this));

  /// @todo should this class own the network or just keep a reference?

#ifdef BUILD_WITH_PYTHON
  NetworkEditorPythonAPI::setImpl(boost::make_shared<PythonImpl>(*this, cmdFactory_));
#endif

  eventCmdFactory_->create(NetworkEventCommands::ApplicationStart)->execute();
}

NetworkEditorController::NetworkEditorController(NetworkHandle network, ExecutionStrategyFactoryHandle executorFactory, NetworkEditorSerializationManager* nesm)
  : theNetwork_(network), executorFactory_(executorFactory),
  eventCmdFactory_(new NullCommandFactory),
  serializationManager_(nesm),
  signalSwitch_(true)
{
}

NetworkEditorController::~NetworkEditorController()
{
#ifdef BUILD_WITH_PYTHON
  NetworkEditorPythonAPI::clearImpl();
#endif
  executionManager_.stop();
}

namespace
{
  class SnippetHandler
  {
  public:
    explicit SnippetHandler(NetworkEditorController& nec) : nec_(nec) {}
    bool isSnippetName(const std::string& label) const
    {
      if (label.empty())
        return false;
      return label.front() == '[' && label.back() == ']';
    }
    ModuleHandle createSnippet(const std::string& label)
    {
      auto modsNeeded = parseModules(label);

      ModulePositions positions;
      int i = 0;
      const double MODULE_VERTICAL_SPACING = 110;
      const double MODULE_HORIZONTAL_SPACING = 50;
      const double MODULE_SPACING_OFFSET = 10;
      const std::pair<double,double> initialPosition = {-500, -500};
      static double snippetSpacer = MODULE_SPACING_OFFSET;
      static int numSnips = 0;
      for (auto m : modsNeeded)
      {
        bool uiVisible = false;
        if (m.back() == '*')
        {
          m = m.substr(0, m.length() - 1);
          uiVisible = true;
        }
        auto mod = nec_.addModule(m);
        if (mod->hasUI())
          mod->setUiVisible(uiVisible);
        mods_.push_back(mod);
        positions.modulePositions[mod->id().id_] =
          { initialPosition.first + numSnips * MODULE_HORIZONTAL_SPACING + snippetSpacer,
            initialPosition.second + MODULE_VERTICAL_SPACING * i++ + snippetSpacer };
      }
      numSnips = (numSnips + 1) % 3;
      if (0 == numSnips)
      {
        snippetSpacer += MODULE_SPACING_OFFSET;
      }

      auto connsNeeded = parseConnections(label);
      for (const auto& c : connsNeeded)
      {
        if (c.first && c.second)
          nec_.requestConnection(c.first, c.second);
      }

      nec_.updateModulePositions(positions, true);

      return mods_.back();
    }
  private:
    std::vector<std::string> parseModules(const std::string& label) const
    {
      if (!isSnippetName(label))
        return {};

      std::vector<std::string> mods;
      std::string strippedLabel(label.begin() + 1, label.end() - 1);
      boost::split(mods, strippedLabel, boost::is_any_of("->()"), boost::token_compress_on);
      return mods;
    }

    typedef std::pair<PortDescriptionInterface*,PortDescriptionInterface*> PortPair;
    typedef std::vector<PortPair> PortPairVector;
    PortPairVector parseConnections(const std::string& label) const
    {
      if (!isSnippetName(label))
        return {};

      parseModules(label);

      if (mods_.size() < 2)
        return {};

      PortPairVector portPairs;
      for (auto i = mods_.begin(); i != mods_.end(); ++i)
      {
        if (i + 1 != mods_.end())
        {
          portPairs.push_back(findFirstMatchingPortPair(*i, *(i+1)));
        }
      }
      return portPairs;
    }

    PortPair findFirstMatchingPortPair(ModuleHandle from, ModuleHandle to) const
    {
      for (const auto& output : from->outputPorts())
      {
        for (const auto& input : to->inputPorts())
        {
          if (output->get_typename() == input->get_typename())
            return PortPair(input.get(), output.get());
        }
      }
      return PortPair();
    }

    NetworkEditorController& nec_;
    std::vector<ModuleHandle> mods_;
  };
}

ModuleHandle NetworkEditorController::addModule(const std::string& name)
{
  //XTODO: 1. snippet checker move here
  //XTODO: 2. parse snippet string for connections
  //XTODO: 3. call connection code
  //XTODO: 4. move modules around nicely. this one might be difficult, use a separate signal when snippet is done loading. pass a string of module ids

  SnippetHandler snippet(*this);
  if (snippet.isSnippetName(name))
  {
    return snippet.createSnippet(name);
  }

  return addModule(ModuleLookupInfo(name, "Category TODO", "SCIRun"));
}

ModuleHandle NetworkEditorController::addModule(const ModuleLookupInfo& info)
{
  auto realModule = addModuleImpl(info);
  if (signalSwitch_)
  {
    static ModuleCounter dummy;
    /*emit*/ moduleAdded_(info.module_name_, realModule, dummy);
  }
  printNetwork();

  if (!loadingContext_)
    eventCmdFactory_->create(NetworkEventCommands::PostModuleAdd)->execute();

  return realModule;
}

ModuleHandle NetworkEditorController::addModuleImpl(const ModuleLookupInfo& info)
{
  auto realModule = theNetwork_->add_module(info);
  if (realModule) /// @todo: mock network throws here due to null, need to have it return a mock module.
  {
    realModule->addPortConnection(connectPortAdded(boost::bind(&ModuleInterface::portAddedSlot, realModule.get(), _1, _2)));
    realModule->addPortConnection(connectPortRemoved(boost::bind(&ModuleInterface::portRemovedSlot, realModule.get(), _1, _2)));
  }
  return realModule;
}

void NetworkEditorController::removeModule(const ModuleId& id)
{
  theNetwork_->remove_module(id);
  //before or after?
  // deciding on after: ProvenanceWindow/Manager wants the state *after* removal.
  /*emit*/ moduleRemoved_(id);

  printNetwork();
}

void NetworkEditorController::interruptModule(const ModuleId& id)
{
  theNetwork_->interruptModuleRequest(id);
  ///*emit*/ networkInterrupted_();
}

namespace
{
  InputPortHandle getFirstAvailableDynamicPortWithName(ModuleHandle mod, const std::string& name)
  {
    auto ports = mod->findInputPortsWithName(name);
    auto firstEmptyDynamicPortWithName = std::find_if(ports.begin(), ports.end(),
      [](InputPortHandle iport) { return iport->nconnections() == 0; });
    return firstEmptyDynamicPortWithName != ports.end() ? *firstEmptyDynamicPortWithName : nullptr;
  }
}

ModuleHandle NetworkEditorController::duplicateModule(const ModuleHandle& module)
{
  ENSURE_NOT_NULL(module, "Cannot duplicate null module");
  auto id(module->id());
  auto newModule = addModuleImpl(module->info());
  newModule->setState(module->get_state()->clone());
  static ModuleCounter dummy;
  moduleAdded_(id.name_, newModule, dummy);

  /// @todo: probably a pretty poor way to deal with what I think is a race condition with signaling the GUI to place the module widget.
  boost::this_thread::sleep(boost::posix_time::milliseconds(1));

  for (const auto& originalInputPort : module->inputPorts())
  {
    if (originalInputPort->nconnections() == 1)
    {
      auto conn = originalInputPort->connection(0);
      auto source = conn->oport_;
      if (!originalInputPort->isDynamic())
        requestConnection(source.get(), newModule->getInputPort(originalInputPort->id()).get());
      else
      {
        auto toConnect = getFirstAvailableDynamicPortWithName(newModule, originalInputPort->get_portname());
        if (toConnect)
          requestConnection(source.get(), toConnect.get());
      }
    }
  }

  if (serializationManager_)
  {
    serializationManager_->copyNote(module, newModule);
  }

  return newModule;
}

ModuleHandle NetworkEditorController::connectNewModule(const PortDescriptionInterface* portToConnect, const std::string& newModuleName)
{
  auto newMod = addModule(newModuleName);

  /// @todo: see above
  boost::this_thread::sleep(boost::posix_time::milliseconds(1));

  /// @todo duplication
  if (portToConnect->isInput())
  {
    for (const auto& p : newMod->outputPorts())
    {
      if (p->get_typename() == portToConnect->get_typename())
      {
        requestConnection(p.get(), portToConnect);
        return newMod;
      }
    }
  }
  else
  {
    for (const auto& p : newMod->inputPorts())
    {
      if (p->get_typename() == portToConnect->get_typename())
      {
        requestConnection(p.get(), portToConnect);
        return newMod;
      }
    }
  }
  return newMod;
}

ModuleHandle NetworkEditorController::insertNewModule(const PortDescriptionInterface* portToConnect, const InsertInfo& info)
{
  auto newMod = connectNewModule(portToConnect, info.newModuleName);

  auto endModule = theNetwork_->lookupModule(ModuleId(info.endModuleId));

  auto newModOutputPorts = newMod->outputPorts();
  auto firstMatchingOutputPort = std::find_if(newModOutputPorts.begin(), newModOutputPorts.end(),
    [&](OutputPortHandle oport) { return oport->get_typename() == portToConnect->get_typename(); }
    );

  if (firstMatchingOutputPort != newModOutputPorts.end())
  {
    auto newOutputPortToConnectFrom = *firstMatchingOutputPort;

    auto endModuleInputPortOptions = endModule->findInputPortsWithName(info.inputPortName);
    if (!endModuleInputPortOptions.empty())
    {
      auto firstPort = endModuleInputPortOptions[0];

      if (!firstPort->isDynamic())  // easy case
      {
        auto connId = firstPort->connection(0)->id_;
        removeConnection(connId);
        requestConnection(newOutputPortToConnectFrom.get(), firstPort.get());
      }
      else //dynamic: match portId exactly, remove, then retrieve list again to find first empty dynamic port of same name.
      {
        auto exactMatch = std::find_if(endModuleInputPortOptions.begin(), endModuleInputPortOptions.end(),
          [&](InputPortHandle iport) { return iport->id().toString() == info.inputPortId; });
        if (exactMatch != endModuleInputPortOptions.end())
        {
          auto connId = (*exactMatch)->connection(0)->id_;
          removeConnection(connId);
          auto firstEmptyDynamicPortWithName = getFirstAvailableDynamicPortWithName(endModule, info.inputPortName);
          if (firstEmptyDynamicPortWithName)
          {
            requestConnection(newOutputPortToConnectFrom.get(), firstEmptyDynamicPortWithName.get());
          }
        }
      }
    }
  }
  return newMod;
}

void NetworkEditorController::printNetwork() const
{
  /// @todo: and make this switchable
  if (false)
  {
    if (theNetwork_)
      LOG_DEBUG(theNetwork_->toString());
  }
}

boost::optional<ConnectionId> NetworkEditorController::requestConnection(const PortDescriptionInterface* from, const PortDescriptionInterface* to)
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
    auto id = theNetwork_->connect(ConnectionOutputPort(theNetwork_->lookupModule(desc.out_.moduleId_), desc.out_.portId_),
      ConnectionInputPort(theNetwork_->lookupModule(desc.in_.moduleId_), desc.in_.portId_));
    if (!id.id_.empty())
      connectionAdded_(desc);

    printNetwork();
    return id;
  }

  logWarning("Invalid Connection request: input port is full, or ports are different datatype or same i/o type, or on the same module.");
  invalidConnection_(desc);
  return boost::none;
}

void NetworkEditorController::removeConnection(const ConnectionId& id)
{
  if (theNetwork_->disconnect(id))
  {
    connectionRemoved_(id);
  }
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
  return ExecutionContext::connectNetworkExecutionStarts(subscriber);
}

boost::signals2::connection NetworkEditorController::connectNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber)
{
  return ExecutionContext::connectNetworkExecutionFinished(subscriber);
}

boost::signals2::connection NetworkEditorController::connectPortAdded(const PortAddedSignalType::slot_type& subscriber)
{
  return dynamicPortManager_->connectPortAdded(subscriber);
}

boost::signals2::connection NetworkEditorController::connectPortRemoved(const PortRemovedSignalType::slot_type& subscriber)
{
  return dynamicPortManager_->connectPortRemoved(subscriber);
}

boost::signals2::connection NetworkEditorController::connectNetworkDoneLoading(const NetworkDoneLoadingSignalType::slot_type& subscriber)
{
  return networkDoneLoading_.connect(subscriber);
}

NetworkFileHandle NetworkEditorController::serializeNetworkFragment(ModuleFilter modFilter, ConnectionFilter connFilter) const
{
  NetworkToXML conv(serializationManager_);
  return conv.to_xml_data(theNetwork_, modFilter, connFilter);
}

NetworkFileHandle NetworkEditorController::saveNetwork() const
{
  NetworkToXML conv(serializationManager_);
  return conv.to_xml_data(theNetwork_);
}

NetworkEditorController::LoadingContext::LoadingContext(bool& load) : load_(load)
{
  load_ = true;
}

NetworkEditorController::LoadingContext::~LoadingContext()
{
  load_ = false;
}

////////
// TODO: refactor the next two functions into one
///////

void NetworkEditorController::loadNetwork(const NetworkFileHandle& xml)
{
  LoadingContext ctx(loadingContext_);
  if (xml)
  {
    try
    {
      NetworkXMLConverter conv(moduleFactory_, stateFactory_, algoFactory_, reexFactory_, this);
      theNetwork_ = conv.from_xml_data(xml->network);
      ModuleCounter modulesDone;
      for (size_t i = 0; i < theNetwork_->nmodules(); ++i)
      {
        auto module = theNetwork_->module(i);
        moduleAdded_(module->name(), module, modulesDone);
        networkDoneLoading_(static_cast<int>(i));
      }

      {
        auto disable(createDynamicPortSwitch());
        //this is handled by NetworkXMLConverter now--but now the logic is convoluted.
        //They need to be signaled again after the modules are signaled to alert the GUI. Hence the disabling of DPM
        for (const auto& cd : theNetwork_->connections(true))
        {
          auto id = ConnectionId::create(cd);
          connectionAdded_(cd);
        }
      }
      if (serializationManager_)
      {
        serializationManager_->updateModuleNotes(xml->moduleNotes);
        serializationManager_->updateConnectionNotes(xml->connectionNotes);
        serializationManager_->updateDisabledComponents(xml->disabledComponents);
        serializationManager_->updateSubnetworks(xml->subnetworks);
        serializationManager_->updateModulePositions(xml->modulePositions, false);
        serializationManager_->updateModuleTags(xml->moduleTags);
      }
      else
      {
#ifndef BUILD_HEADLESS
        logInfo("module position editor unavailable, module positions at default");
#endif
      }
      networkDoneLoading_(static_cast<int>(theNetwork_->nmodules()) + 1);
    }
    catch (ExceptionBase& e)
    {
      logError("File load failed: exception while processing xml network data: {}", e.what());
      theNetwork_->clear();
      throw;
    }
    eventCmdFactory_->create(NetworkEventCommands::OnNetworkLoad)->execute();
  }
}

namespace
{
  const int xMoveIncrement = 10;
  int xMoveIndex = 1;
  int yMoveIndex = 0;
  const int moveMod = 4;
  const int yMoveIncrement = 300;
  void shiftAppendedModules(ModulePositions::Data& positions)
  {
    for (auto& pos : positions)
    {
      pos.second.first += xMoveIncrement * xMoveIndex;
      pos.second.second += yMoveIncrement * yMoveIndex;
    }
    xMoveIndex = (xMoveIndex + 1) % moveMod;
    if (0 == xMoveIndex)
      yMoveIndex++;
  }
}

void NetworkEditorController::appendToNetwork(const NetworkFileHandle& xml)
{
  if (xml)
  {
    try
    {
      NetworkXMLConverter conv(moduleFactory_, stateFactory_, algoFactory_, reexFactory_, this);

      auto originalConnections = theNetwork_->connections(true);

      auto info = conv.appendXmlData(xml->network);
      auto startIndex = info.newModuleStartIndex;
      ModuleCounter modulesDone;
      for (size_t i = startIndex; i < theNetwork_->nmodules(); ++i)
      {
        auto module = theNetwork_->module(i);
        moduleAdded_(module->name(), module, modulesDone);
      }

      {
        auto disable(createDynamicPortSwitch());
        //this is handled by NetworkXMLConverter now--but now the logic is convoluted.
        //They need to be signaled again after the modules are signaled to alert the GUI. Hence the disabling of DPM
        for (const auto& cd : theNetwork_->connections(true))
        {
          if (std::find(originalConnections.begin(), originalConnections.end(), cd) == originalConnections.end())
          {
            auto id = ConnectionId::create(cd);
            connectionAdded_(cd);
          }
        }
      }

      if (serializationManager_)
      {
        xml->modulePositions.modulePositions = remapIdBasedContainer(xml->modulePositions.modulePositions, info.moduleIdMapping);
        shiftAppendedModules(xml->modulePositions.modulePositions);
        serializationManager_->updateModulePositions(xml->modulePositions, false); // need to shift everything.
        xml->moduleNotes.notes = remapIdBasedContainer(xml->moduleNotes.notes, info.moduleIdMapping);
        serializationManager_->updateModuleNotes(xml->moduleNotes);
        xml->connectionNotes.notes = remapIdBasedContainer(xml->connectionNotes.notes, info.moduleIdMapping);
        serializationManager_->updateConnectionNotes(xml->connectionNotes);
        xml->moduleTags.tags = remapIdBasedContainer(xml->moduleTags.tags, info.moduleIdMapping);
        serializationManager_->updateModuleTags(xml->moduleTags);
        //TODO: need disabled here?
      }
      else
        logInfo("module position editor unavailable, module positions at default");
    }
    catch (ExceptionBase& e)
    {
      logError("File load failed: exception while processing xml network data: {}", e.what());
      theNetwork_->clear();
      throw;
    }
  }
}

void NetworkEditorController::clear()
{
  LOG_DEBUG("NetworkEditorController::clear()");
}

// TODO:
// - [X] refactor duplication
// - [X] set up execution context queue
// - [X] separate threads for looping through queue: another producer/consumer pair

boost::shared_ptr<boost::thread> NetworkEditorController::executeAll(const ExecutableLookup* lookup)
{
  return executeGeneric(lookup, ExecuteAllModules::Instance());
}

void NetworkEditorController::executeModule(const ModuleHandle& module, const ExecutableLookup* lookup, bool executeUpstream)
{
  try
  {
    ExecuteSingleModule filter(module, *theNetwork_, executeUpstream);
    executeGeneric(lookup, filter);
  }
  catch (NetworkHasCyclesException&)
  {
    logError("Cannot schedule execution: network has cycles. Please break all cycles and try again.");
    ExecutionContext::executionBounds_.executeFinishes_(-1);
    return;
  }
}

void NetworkEditorController::initExecutor()
{
  executionManager_.initExecutor(executorFactory_);
}

ExecutionContextHandle NetworkEditorController::createExecutionContext(const ExecutableLookup* lookup, ModuleFilter filter)
{
  return boost::make_shared<ExecutionContext>(*theNetwork_, lookup ? *lookup : *theNetwork_, filter);
}

boost::shared_ptr<boost::thread> NetworkEditorController::executeGeneric(const ExecutableLookup* lookup, ModuleFilter filter)
{
  initExecutor();
  auto context = createExecutionContext(lookup, filter);
  return executionManager_.enqueueContext(context);
}

void NetworkEditorController::stopExecutionContextLoopWhenExecutionFinishes()
{
  connectNetworkExecutionFinished([this](int)
  {
    std::cout << "Execution manager thread stopped." << std::endl;
    executionManager_.stop();
  });
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
  executionManager_.setExecutionStrategy(executorFactory_->create(static_cast<ExecutionStrategy::Type>(type)));
}

const ModuleDescriptionMap& NetworkEditorController::getAllAvailableModuleDescriptions() const
{
  return moduleFactory_->getAllAvailableModuleDescriptions();
}

boost::shared_ptr<DisableDynamicPortSwitch> NetworkEditorController::createDynamicPortSwitch()
{
  return boost::make_shared<DisableDynamicPortSwitch>(dynamicPortManager_);
}

DisableDynamicPortSwitch::DisableDynamicPortSwitch(boost::shared_ptr<DynamicPortManager> dpm) : first_(true), dpm_(dpm)
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

void NetworkEditorController::disableSignals()
{
  signalSwitch_ = false;
}

void NetworkEditorController::enableSignals()
{
  signalSwitch_ = true;
}

const ModuleLookupInfoSet& NetworkEditorController::possibleReplacements(ModuleHandle module)
{
  if (!replacementFilter_)
  {
    auto descMap = moduleFactory_->getDirectModuleDescriptionLookupMap();
    ModuleReplacementFilterBuilder builder(descMap);
    replacementFilter_ = builder.build();
  }
  return replacementFilter_->findReplacements(makeConnectedPortInfo(module));
}

void NetworkEditorController::updateModulePositions(const ModulePositions& modulePositions, bool selectAll)
{
  if (serializationManager_)
  {
    serializationManager_->updateModulePositions(modulePositions, selectAll);
  }
}

void NetworkEditorController::cleanUpNetwork()
{
  auto all = boost::lambda::constant(true);
  NetworkGraphAnalyzer analyze(*theNetwork_, all, true);
  auto connected = analyze.connectedComponents();

  std::map<int, std::map<int, std::string>> modulesByComponentAndGroup;

  BoostGraphParallelScheduler scheduleAll(all);
  auto order = scheduleAll.schedule(*theNetwork_);
  for (int group = order.minGroup(); group <= order.maxGroup(); ++group)
  {
    auto groupIter = order.getGroup(group);
    BOOST_FOREACH(auto g, groupIter)
    {
      modulesByComponentAndGroup[connected[g.second]][g.first] = g.second;
    }
  }

  ModulePositions cleanedUp;
  //std::cout << "COMPONENT--GROUP--MODULE MAP" << std::endl;
  for (const auto& c : modulesByComponentAndGroup)
  {
    for (const auto& g : c.second)
    {
      //std::cout << "component " << c.first << " group " << g.first << " module " << g.second << std::endl;
      cleanedUp.modulePositions[g.second] = { c.first * 400.0, g.first * 80.0 };
    }
  }

  updateModulePositions(cleanedUp, false);
}

std::vector<ModuleExecutionState::Value> NetworkEditorController::moduleExecutionStates() const
{
  return theNetwork_ ? theNetwork_->moduleExecutionStates() : std::vector<ModuleExecutionState::Value>();
}
