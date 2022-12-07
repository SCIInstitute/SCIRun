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

#include <boost/foreach.hpp>
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
  NetworkEventCommandFactoryHandle eventCmdFactory, NetworkEditorSerializationManager* nesm)
{
  collabs_.theNetwork_.reset(new Network(mf, sf, af, reex));
  collabs_.moduleFactory_ = mf;
  collabs_.stateFactory_ = sf;
  collabs_.algoFactory_ = af;
  collabs_.reexFactory_ = reex;
  collabs_.executorFactory_ = executorFactory;
  #ifndef BUILD_HEADLESS
  collabs_.executionManager_.reset(new ExecutionQueueManager);
  #else
  collabs_.executionManager_.reset(new SimpleExecutionManager);
  #endif
  collabs_.cmdFactory_ = cmdFactory;
  collabs_.eventCmdFactory_ = eventCmdFactory ? eventCmdFactory : makeShared<NullCommandFactory>();
  collabs_.serializationManager_ = nesm;
  signals_.signalSwitch_ = true;
  signals_.loadingContext_ = false;

  collabs_.dynamicPortManager_.reset(new DynamicPortManager(signals_.connectionAdded_, signals_.connectionRemoved_, this));

  /// @todo should this class own the network or just keep a reference?

#ifdef BUILD_WITH_PYTHON
  NetworkEditorPythonAPI::setImpl(makeShared<PythonImpl>(*this, collabs_.cmdFactory_));
#endif

  collabs_.eventCmdFactory_->create(NetworkEventCommands::ApplicationStart)->execute();
}

NetworkEditorController::NetworkEditorController(const NetworkEditorController& other)
{
  collabs_.theNetwork_.reset(new Network(other.collabs_.moduleFactory_,
    other.collabs_.stateFactory_, other.collabs_.algoFactory_, other.collabs_.reexFactory_));
  collabs_.moduleFactory_ = other.collabs_.moduleFactory_;
  collabs_.stateFactory_ = other.collabs_.stateFactory_;
  collabs_.algoFactory_ = other.collabs_.algoFactory_;
  collabs_.reexFactory_ = other.collabs_.reexFactory_;
  collabs_.executorFactory_ = other.collabs_.executorFactory_;
  collabs_.executionManager_.reset(new SimpleExecutionManager);

  //collabs_.cmdFactory_ = other.collabs_.cmdFactory_;
  //collabs_.eventCmdFactory_ = eventCmdFactory ? eventCmdFactory : makeShared<NullCommandFactory>();
  collabs_.serializationManager_ = other.collabs_.serializationManager_;
  signals_.signalSwitch_ = true;
  signals_.loadingContext_ = false;

  collabs_.dynamicPortManager_.reset(new DynamicPortManager(signals_.connectionAdded_,
    signals_.connectionRemoved_, this));

  /// @todo should this class own the network or just keep a reference?

#ifdef BUILD_WITH_PYTHON
  //NetworkEditorPythonAPI::setImpl(makeShared<PythonImpl>(*this, collabs_.cmdFactory_));
#endif

  //collabs_.eventCmdFactory_->create(NetworkEventCommands::ApplicationStart)->execute();
}

NetworkEditorController::NetworkEditorController(NetworkStateHandle network, ExecutionStrategyFactoryHandle executorFactory, NetworkEditorSerializationManager* nesm)
{
  collabs_.theNetwork_ = network;
  collabs_.executorFactory_ = executorFactory;
  collabs_.eventCmdFactory_.reset(new NullCommandFactory);
  collabs_.serializationManager_ = nesm;
  signals_.signalSwitch_ = true;
}

NetworkEditorController::~NetworkEditorController()
{
#ifdef BUILD_WITH_PYTHON
  NetworkEditorPythonAPI::clearImpl();
#endif
  collabs_.executionManager_->stopExecution();
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
  if (signals_.signalSwitch_)
  {
    static ModuleCounter dummy;
    /*emit*/ signals_.moduleAdded_(info.module_name_, realModule, dummy);
  }
  printNetwork();

  if (!signals_.loadingContext_ && collabs_.eventCmdFactory_)
    collabs_.eventCmdFactory_->create(NetworkEventCommands::PostModuleAdd)->execute();

  return realModule;
}

ModuleHandle NetworkEditorController::addModuleImpl(const ModuleLookupInfo& info)
{
  auto realModule = collabs_.theNetwork_->add_module(info);
  if (realModule) /// @todo: mock network throws here due to null, need to have it return a mock module.
  {
    realModule->addPortConnection(connectPortAdded([realModule](const ModuleId& mid, const PortId& pid) { realModule->portAddedSlot(mid, pid); }));
    realModule->addPortConnection(connectPortRemoved([realModule](const ModuleId& mid, const PortId& pid) { realModule->portRemovedSlot(mid, pid); }));
  }
  realModule->setNetwork(this);
  return realModule;
}

void NetworkEditorController::removeModule(const ModuleId& id)
{
  collabs_.theNetwork_->remove_module(id);
  //before or after?
  // deciding on after: ProvenanceWindow/Manager wants the state *after* removal.
  /*emit*/ signals_.moduleRemoved_(id);

  printNetwork();
}

namespace
{
  InputPortHandle getFirstAvailableDynamicPortWithName(ModuleHandle mod, const std::string& name)
  {
    auto ports = mod->findInputPortsWithName(name);
    const auto firstEmptyDynamicPortWithName = std::find_if(ports.begin(), ports.end(),
      [](InputPortHandle iport) { return iport->nconnections() == 0; });
    return firstEmptyDynamicPortWithName != ports.end() ? *firstEmptyDynamicPortWithName : nullptr;
  }
}

ModuleHandle NetworkEditorController::duplicateModule(const ModuleHandle& module)
{
  ENSURE_NOT_NULL(module, "Cannot duplicate null module");
  const auto id(module->id());
  auto newModule = addModuleImpl(module->info());
  newModule->setState(module->get_state()->clone());
  static ModuleCounter dummy;
  signals_.moduleAdded_(id.name_, newModule, dummy);

  /// @todo: probably a pretty poor way to deal with what I think is a race condition with signaling the GUI to place the module widget.
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  for (const auto& originalInputPort : module->inputPorts())
  {
    if (originalInputPort->nconnections() == 1)
    {
      auto conn = originalInputPort->connection(0);
      auto source = conn->oport_;
      if (!originalInputPort->isDynamic())
        requestConnection(source.get(), newModule->getInputPort(originalInputPort->externalId()).get());
      else
      {
        auto toConnect = getFirstAvailableDynamicPortWithName(newModule, originalInputPort->get_portname());
        if (toConnect)
          requestConnection(source.get(), toConnect.get());
      }
    }
  }

  if (collabs_.serializationManager_)
  {
    collabs_.serializationManager_->copyNote(module, newModule);
  }

  return newModule;
}

ModuleHandle NetworkEditorController::connectNewModule(const PortDescriptionInterface* portToConnect, const std::string& newModuleName)
{
  auto newMod = addModule(newModuleName);

  /// @todo: see above
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

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

  const auto endModule = collabs_.theNetwork_->lookupModule(ModuleId(info.endModuleId));

  auto newModOutputPorts = newMod->outputPorts();
  const auto firstMatchingOutputPort = std::find_if(newModOutputPorts.begin(), newModOutputPorts.end(),
    [&](OutputPortHandle oport) { return oport->get_typename() == portToConnect->get_typename(); }
    );

  if (firstMatchingOutputPort != newModOutputPorts.end())
  {
    const auto newOutputPortToConnectFrom = *firstMatchingOutputPort;

    auto endModuleInputPortOptions = endModule->findInputPortsWithName(info.inputPortName);
    if (!endModuleInputPortOptions.empty())
    {
      auto firstPort = endModuleInputPortOptions[0];

      if (!firstPort->isDynamic())  // easy case
      {
        const auto connId = firstPort->connection(0)->id_;
        removeConnection(connId);
        requestConnection(newOutputPortToConnectFrom.get(), firstPort.get());
      }
      else //dynamic: match portId exactly, remove, then retrieve list again to find first empty dynamic port of same name.
      {
        const auto exactMatch = std::find_if(endModuleInputPortOptions.begin(), endModuleInputPortOptions.end(),
          [&](InputPortHandle iport) { return iport->externalId().toString() == info.inputPortId; });
        if (exactMatch != endModuleInputPortOptions.end())
        {
          const auto connId = (*exactMatch)->connection(0)->id_;
          removeConnection(connId);
          const auto firstEmptyDynamicPortWithName = getFirstAvailableDynamicPortWithName(endModule, info.inputPortName);
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
    if (collabs_.theNetwork_)
      LOG_DEBUG(collabs_.theNetwork_->toString());
  }
}

std::optional<ConnectionId> NetworkEditorController::requestConnection(const PortDescriptionInterface* from, const PortDescriptionInterface* to)
{
  ENSURE_NOT_NULL(from, "from port");
  ENSURE_NOT_NULL(to, "to port");

  const auto out = from->isInput() ? to : from;
  const auto in = from->isInput() ? from : to;

  const ConnectionDescription desc(
    OutgoingConnectionDescription(out->getUnderlyingModuleId(), out->externalId()),
    IncomingConnectionDescription(in->getUnderlyingModuleId(), in->externalId()));

  PortConnectionDeterminer q;
  if (q.canBeConnected(*from, *to))
  {
    auto id = collabs_.theNetwork_->connect(ConnectionOutputPort(collabs_.theNetwork_->lookupModule(desc.out_.moduleId_), desc.out_.portId_),
      ConnectionInputPort(collabs_.theNetwork_->lookupModule(desc.in_.moduleId_), desc.in_.portId_));
    if (!id.id_.empty())
      signals_.connectionAdded_(desc);

    printNetwork();
    return id;
  }

  logWarning("Invalid Connection request: input port is full, or ports are different datatype or same i/o type, or on the same module.");
  signals_.invalidConnection_(desc);
  return {};
}

void NetworkEditorController::removeConnection(const ConnectionId& id)
{
  if (collabs_.theNetwork_->disconnect(id))
  {
    signals_.connectionRemoved_(id);
  }
  printNetwork();
}

boost::signals2::connection NetworkEditorController::connectModuleAdded(const ModuleAddedSignalType::slot_type& subscriber)
{
  return signals_.moduleAdded_.connect(subscriber);
}

boost::signals2::connection NetworkEditorController::connectModuleRemoved(const ModuleRemovedSignalType::slot_type& subscriber)
{
  return signals_.moduleRemoved_.connect(subscriber);
}

boost::signals2::connection NetworkEditorController::connectConnectionAdded(const ConnectionAddedSignalType::slot_type& subscriber)
{
  return signals_.connectionAdded_.connect(subscriber);
}

boost::signals2::connection NetworkEditorController::connectConnectionRemoved(const ConnectionRemovedSignalType::slot_type& subscriber)
{
  return signals_.connectionRemoved_.connect(subscriber);
}

boost::signals2::connection NetworkEditorController::connectInvalidConnection(const InvalidConnectionSignalType::slot_type& subscriber)
{
  return signals_.invalidConnection_.connect(subscriber);
}

boost::signals2::connection NetworkEditorController::connectStaticNetworkExecutionStarts(const ExecuteAllStartsSignalType::slot_type& subscriber)
{
  return ExecutionContext::connectGlobalNetworkExecutionStarts(subscriber);
}

boost::signals2::connection NetworkEditorController::connectStaticNetworkExecutionFinished(const ExecuteAllFinishesSignalType::slot_type& subscriber)
{
  return ExecutionContext::connectGlobalNetworkExecutionFinished(subscriber);
}

boost::signals2::connection NetworkEditorController::connectPortAdded(const PortAddedSignalType::slot_type& subscriber)
{
  return collabs_.dynamicPortManager_->connectPortAdded(subscriber);
}

boost::signals2::connection NetworkEditorController::connectPortRemoved(const PortRemovedSignalType::slot_type& subscriber)
{
  return collabs_.dynamicPortManager_->connectPortRemoved(subscriber);
}

boost::signals2::connection NetworkEditorController::connectNetworkDoneLoading(const NetworkDoneLoadingSignalType::slot_type& subscriber)
{
  return signals_.networkDoneLoading_.connect(subscriber);
}

boost::signals2::connection NetworkEditorController::connectConnectionStatusChanged(const ConnectionStatusChangedSignalType::slot_type& subscriber)
{
  return signals_.connectionStatusChanged_.connect(subscriber);
}

std::string NetworkEditorController::setConnectionStatus(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex, bool enable)
{
  auto conn = getNetwork()->lookupConnection(moduleIdFrom, fromIndex, moduleIdTo, toIndex);
  if (conn)
  {
    conn->setDisable(!enable);
    signals_.connectionStatusChanged_(conn->id_, enable);
    return std::string("Connection status ") + (enable ? "enabled" : "disabled") + ": " + moduleIdFrom + "(" + std::to_string(fromIndex) + ")->" + moduleIdTo + "(" + std::to_string(toIndex) + ")";
  }
  else
  {
    return "Connection not found: " + moduleIdFrom + "(" + std::to_string(fromIndex) + ")->" + moduleIdTo + "(" + std::to_string(toIndex) + ")";
  }
}

NetworkFileHandle NetworkEditorController::serializeNetworkFragment(ModuleFilter modFilter, ConnectionFilter connFilter) const
{
  NetworkToXML conv(collabs_.serializationManager_);
  return conv.to_xml_data(collabs_.theNetwork_, modFilter, connFilter);
}

NetworkFileHandle NetworkEditorController::saveNetwork() const
{
  NetworkToXML conv(collabs_.serializationManager_);
  return conv.to_xml_data(collabs_.theNetwork_);
}

NetworkSignalManager::LoadingContext::LoadingContext(bool& load) : load_(load)
{
  load_ = true;
}

NetworkSignalManager::LoadingContext::~LoadingContext()
{
  load_ = false;
}

////////
// TODO: refactor the next two functions into one
///////

void NetworkEditorController::loadNetwork(const NetworkFileHandle& xml)
{
  NetworkSignalManager::LoadingContext ctx(signals_.loadingContext_);
  if (xml)
  {
    try
    {
      collabs_.theNetwork_ = makeShared<Network>(collabs_.moduleFactory_, collabs_.stateFactory_,
        collabs_.algoFactory_, collabs_.reexFactory_);
      loadXmlDataIntoNetwork(xml->network.data());

      ModuleCounter modulesDone;
      for (size_t i = 0; i < collabs_.theNetwork_->nmodules(); ++i)
      {
        auto module = collabs_.theNetwork_->module(i);
        signals_.moduleAdded_(module->name(), module, modulesDone);
        signals_.networkDoneLoading_(static_cast<int>(i));
      }

      {
        auto disable(createDynamicPortSwitch());
        //this is handled by NetworkXMLConverter now--but now the logic is convoluted.
        //They need to be signaled again after the modules are signaled to alert the GUI. Hence the disabling of DPM
        for (const auto& cd : collabs_.theNetwork_->connections(true))
        {
          auto id = ConnectionId::create(cd);
          signals_.connectionAdded_(cd);
        }
      }
      if (collabs_.serializationManager_)
      {
        collabs_.serializationManager_->updateModuleNotes(xml->moduleNotes);
        collabs_.serializationManager_->updateConnectionNotes(xml->connectionNotes);
        collabs_.serializationManager_->updateDisabledComponents(xml->disabledComponents);
        //collabs_.serializationManager_->updateSubnetworks(xml->subnetworks);
        collabs_.serializationManager_->updateModulePositions(xml->modulePositions, false);
        collabs_.serializationManager_->updateModuleTags(xml->moduleTags);
      }
      else
      {
#ifndef BUILD_HEADLESS
        logInfo("module position editor unavailable, module positions at default");
#endif
      }
      signals_.networkDoneLoading_(static_cast<int>(collabs_.theNetwork_->nmodules()) + 1);
    }
    catch (ExceptionBase& e)
    {
      logError("File load failed: exception while processing xml network data: {}", e.what());
      collabs_.theNetwork_->clear();
      throw;
    }
    catch (std::exception& ex)
    {
      logError("File load failed: exception while processing xml network data: {}", ex.what());
      collabs_.theNetwork_->clear();
      throw;
    }
    collabs_.eventCmdFactory_->create(NetworkEventCommands::OnNetworkLoad)->execute();
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
      auto originalConnections = collabs_.theNetwork_->connections(true);

      auto info = appendXmlData(xml->network.data());
      auto startIndex = info.newModuleStartIndex;
      ModuleCounter modulesDone;
      for (size_t i = startIndex; i < collabs_.theNetwork_->nmodules(); ++i)
      {
        auto module = collabs_.theNetwork_->module(i);
        signals_.moduleAdded_(module->name(), module, modulesDone);
      }

      {
        auto disable(createDynamicPortSwitch());
        //this is handled by NetworkXMLConverter now--but now the logic is convoluted.
        //They need to be signaled again after the modules are signaled to alert the GUI. Hence the disabling of DPM
        for (const auto& cd : collabs_.theNetwork_->connections(true))
        {
          if (std::find(originalConnections.begin(), originalConnections.end(), cd) == originalConnections.end())
          {
            auto id = ConnectionId::create(cd);
            signals_.connectionAdded_(cd);
          }
        }
      }

      if (collabs_.serializationManager_)
      {
        xml->modulePositions.modulePositions = remapIdBasedContainer(xml->modulePositions.modulePositions, info.moduleIdMapping);
        shiftAppendedModules(xml->modulePositions.modulePositions);
        collabs_.serializationManager_->updateModulePositions(xml->modulePositions, false); // need to shift everything.
        xml->moduleNotes.notes = remapIdBasedContainer(xml->moduleNotes.notes, info.moduleIdMapping);
        collabs_.serializationManager_->updateModuleNotes(xml->moduleNotes);
        xml->connectionNotes.notes = remapIdBasedContainer(xml->connectionNotes.notes, info.moduleIdMapping);
        collabs_.serializationManager_->updateConnectionNotes(xml->connectionNotes);
        xml->moduleTags.tags = remapIdBasedContainer(xml->moduleTags.tags, info.moduleIdMapping);
        collabs_.serializationManager_->updateModuleTags(xml->moduleTags);
        //TODO: need disabled here?
      }
      else
        logInfo("module position editor unavailable, module positions at default");
    }
    catch (ExceptionBase& e)
    {
      logError("File load failed: exception while processing xml network data: {}", e.what());
      collabs_.theNetwork_->clear();
      throw;
    }
  }
}

class ScopedControllerSignalDisabler
{
public:
  explicit ScopedControllerSignalDisabler(NetworkInterface* nec) : nec_(nec)
  {
    nec_->disableSignals();
  }
  ~ScopedControllerSignalDisabler()
  {
    nec_->enableSignals();
  }
private:
  NetworkInterface* nec_;
};

////////
// TODO: refactor the next two functions into one
///////

void NetworkEditorController::loadXmlDataIntoNetwork(NetworkSerializationInterfaceHandle data)
{
  /// @todo: need to use NEC here to manage signal/slots for dynamic ports.
  {
    ScopedControllerSignalDisabler scsd(this);
    for (const auto& modPair : data->modules())
    {
      try
      {
        auto module = addModule(modPair.second.first);
        module->setId(modPair.first);
        module->setState(modPair.second.second);
      }
      catch (Core::InvalidArgumentException& e)
      {
        static std::ofstream missingModulesFile(
          (Core::Logging::LogSettings::Instance().logDirectory() / "missingModules.log").string(), std::ios_base::out | std::ios_base::app);
        missingModulesFile << "File load problem: " << e.what() << std::endl;
        logCritical("File load problem: {}", e.what());
        throw;
      }
    }
  }

  auto connectionsSorted(data->sortedConnections());
  auto network = getNetwork();
  for (const auto& conn : connectionsSorted)
  {
    auto from = network->lookupModule(conn.out_.moduleId_);
    auto to = network->lookupModule(conn.in_.moduleId_);

    if (from && to)
      requestConnection(from->getOutputPort(conn.out_.portId_).get(), to->getInputPort(conn.in_.portId_).get());
    else
    {
      logError(
        "File load error: connection not created between modules {} and {}.",
        conn.out_.moduleId_.id_, conn.in_.moduleId_.id_);
    }
  }
}

NetworkAppendInfo NetworkEditorController::appendXmlData(NetworkSerializationInterfaceHandle data)
{
  const auto network = getNetwork();
  NetworkAppendInfo info;
  info.newModuleStartIndex = network->nmodules();
  {
    ScopedControllerSignalDisabler scsd(this);
    for (const auto& modPair : data->modules())
    {
      ModuleId newId(modPair.first);
      while (network->lookupModule(newId))
      {
        ++newId;
      }

      auto module = addModule(modPair.second.first);

      info.moduleIdMapping[modPair.first] = newId;
      module->setId(newId);
      module->setState(modPair.second.second);
    }
  }

  auto connectionsSorted(data->sortedConnections());

  for (const auto& conn : connectionsSorted)
  {
    auto modOut = info.moduleIdMapping.find(conn.out_.moduleId_);
    auto modIn = info.moduleIdMapping.find(conn.in_.moduleId_);
    if (modOut != info.moduleIdMapping.end() && modIn != info.moduleIdMapping.end())
    {
      const auto from = network->lookupModule(ModuleId(modOut->second));
      const auto to = network->lookupModule(ModuleId(modIn->second));
      if (from && to)
        requestConnection(from->getOutputPort(conn.out_.portId_).get(), to->getInputPort(conn.in_.portId_).get());
    }
  }
  return info;
}

void NetworkEditorController::clear()
{
  collabs_.theNetwork_->clear();
  LOG_DEBUG("NetworkEditorController::clear()");
}

// TODO:
// - [X] refactor duplication
// - [X] set up execution context queue
// - [X] separate threads for looping through queue: another producer/consumer pair

std::future<int> NetworkEditorController::executeAll()
{
  return executeGeneric(ExecuteAllModules::Instance());
}

void NetworkEditorController::setExecutableLookup(const ExecutableLookup* lookup)
{
  collabs_.lookup_ = lookup;
}

void NetworkEditorController::executeModule(const ModuleHandle& module, bool executeUpstream)
{
  try
  {
    const ExecuteSingleModule filter(module, *collabs_.theNetwork_, executeUpstream);
    executeGeneric(filter);
  }
  catch (NetworkHasCyclesException&)
  {
    logError("Cannot schedule execution: network has cycles. Please break all cycles and try again.");
    ExecutionContext::globalExecutionBounds().executeFinishes_(-1);
  }
}

void NetworkEditorController::initExecutor()
{
  collabs_.executionManager_->initExecutor(collabs_.executorFactory_);
}

ExecutionContextHandle NetworkEditorController::createExecutionContext(ModuleFilter filter) const
{
  return makeShared<ExecutionContext>(*collabs_.theNetwork_,
    collabs_.lookup_ ? collabs_.lookup_ : collabs_.theNetwork_.get(), filter);
}

std::future<int> NetworkEditorController::executeGeneric(ModuleFilter filter)
{
  initExecutor();
  const auto context = createExecutionContext(filter);
  return collabs_.executionManager_->execute(context);
}

void NetworkEditorController::stopExecutionContextLoopWhenExecutionFinishes()
{
  connectStaticNetworkExecutionFinished([this](int)
  {
    //std::cout << "Execution manager thread stopped." << std::endl;
    collabs_.executionManager_->stopExecution();
  });
}

NetworkStateHandle NetworkEditorController::getNetwork() const
{
  return collabs_.theNetwork_;
}

NetworkGlobalSettings& NetworkEditorController::getSettings()
{
  return collabs_.theNetwork_->settings();
}

void NetworkEditorController::setExecutorType(int type)
{
  collabs_.executionManager_->setExecutionStrategy(collabs_.executorFactory_->create(static_cast<ExecutionStrategy::Type>(type)));
}

const ModuleDescriptionMap& NetworkEditorController::getAllAvailableModuleDescriptions() const
{
  return collabs_.moduleFactory_->getAllAvailableModuleDescriptions();
}

SharedPointer<DisableDynamicPortSwitch> NetworkEditorController::createDynamicPortSwitch()
{
  return makeShared<DisableDynamicPortSwitch>(collabs_.dynamicPortManager_);
}

DisableDynamicPortSwitch::DisableDynamicPortSwitch(SharedPointer<DynamicPortManager> dpm) : first_(true), dpm_(dpm)
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
  signals_.signalSwitch_ = false;
}

void NetworkEditorController::enableSignals()
{
  signals_.signalSwitch_ = true;
}

const ModuleLookupInfoSet& NetworkEditorController::possibleReplacements(ModuleHandle module)
{
  if (!collabs_.replacementFilter_)
  {
    auto descMap = collabs_.moduleFactory_->getDirectModuleDescriptionLookupMap();
    ModuleReplacementFilterBuilder builder(descMap);
    collabs_.replacementFilter_ = builder.build();
  }
  return collabs_.replacementFilter_->findReplacements(makeConnectedPortInfo(module));
}

void NetworkEditorController::updateModulePositions(const ModulePositions& modulePositions, bool selectAll)
{
  if (collabs_.serializationManager_)
  {
    collabs_.serializationManager_->updateModulePositions(modulePositions, selectAll);
  }
}

void NetworkEditorController::cleanUpNetwork()
{
  auto all = [](ModuleHandle) { return true; };
  NetworkGraphAnalyzer analyze(*collabs_.theNetwork_, all, true);
  auto connected = analyze.connectedComponents();

  std::map<int, std::map<int, std::string>> modulesByComponentAndGroup;

  BoostGraphParallelScheduler scheduleAll(all);
  auto order = scheduleAll.schedule(*collabs_.theNetwork_);
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
  return collabs_.theNetwork_ ? collabs_.theNetwork_->moduleExecutionStates() : std::vector<ModuleExecutionState::Value>();
}

NetworkHandle NetworkEditorController::createSubnetwork() const
{
  return NetworkHandle(new NetworkEditorController(*this));
}
