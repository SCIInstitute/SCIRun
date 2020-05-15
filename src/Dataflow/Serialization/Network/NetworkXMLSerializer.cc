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


/// @todo Documentation Dataflow/Serialization/Network/NetworkXMLSerializer.cc

#include <Dataflow/Serialization/Network/NetworkXMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Network/Network.h> /// @todo: need network factory??
#include <Dataflow/Network/ModuleInterface.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Dataflow/Network/PortInterface.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <fstream>
#include <boost/lambda/lambda.hpp>
#include <Core/Logging/Log.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Algorithms;

class ScopedControllerSignalDisabler
{
public:
  explicit ScopedControllerSignalDisabler(NetworkEditorControllerInterface* nec) : nec_(nec)
  {
    nec_->disableSignals();
  }
  ~ScopedControllerSignalDisabler()
  {
    nec_->enableSignals();
  }
private:
  NetworkEditorControllerInterface* nec_;
};

NetworkXMLConverter::NetworkXMLConverter(ModuleFactoryHandle moduleFactory, ModuleStateFactoryHandle stateFactory, AlgorithmFactoryHandle algoFactory,
  ReexecuteStrategyFactoryHandle reexFactory,
  NetworkEditorControllerInterface* nec, NetworkEditorSerializationManager* nesm)
  : moduleFactory_(moduleFactory), stateFactory_(stateFactory), algoFactory_(algoFactory),
  reexFactory_(reexFactory),
  controller_(nec), nesm_(nesm)
{
}

////////
// TODO: refactor the next two functions into one
///////

NetworkHandle NetworkXMLConverter::from_xml_data(const NetworkXML& data)
{
  /// @todo: need to use NEC here to manage signal/slots for dynamic ports.
  NetworkHandle network(boost::make_shared<Network>(moduleFactory_, stateFactory_, algoFactory_, reexFactory_));
  controller_->setNetwork(network);

  {
    ScopedControllerSignalDisabler scsd(controller_);
    for (const auto& modPair : data.modules)
    {
      try
      {
        auto module = controller_->addModule(modPair.second.module);
        module->setId(modPair.first);
        ModuleStateHandle state(new SimpleMapModuleState(std::move(modPair.second.state)));
        module->setState(state);
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

  std::vector<ConnectionDescriptionXML> connectionsSorted(data.connections);
  std::sort(connectionsSorted.begin(), connectionsSorted.end());
  for (const auto& conn : connectionsSorted)
  {
    auto from = network->lookupModule(conn.out_.moduleId_);
    auto to = network->lookupModule(conn.in_.moduleId_);

    if (from && to)
      controller_->requestConnection(from->getOutputPort(conn.out_.portId_).get(), to->getInputPort(conn.in_.portId_).get());
    else
    {
      logError(
        "File load error: connection not created between modules {} and {}.",
        conn.out_.moduleId_.id_, conn.in_.moduleId_.id_);
    }
  }

  return network;
}

NetworkXMLConverter::NetworkAppendInfo NetworkXMLConverter::appendXmlData(const NetworkXML& data)
{
  auto network = controller_->getNetwork();
  NetworkAppendInfo info;
  info.newModuleStartIndex = network->nmodules();
  {
    ScopedControllerSignalDisabler scsd(controller_);
    for (const auto& modPair : data.modules)
    {
      ModuleId newId(modPair.first);
      while (network->lookupModule(newId))
      {
        //std::cout << "found module by ID : " << modPair.first << std::endl;
        ++newId;
      }

      auto module = controller_->addModule(modPair.second.module);

      //std::cout << "setting module id to " << newId << std::endl;
      info.moduleIdMapping[modPair.first] = newId;
      module->setId(newId);
      ModuleStateHandle state(new SimpleMapModuleState(std::move(modPair.second.state)));
      module->setState(state);
    }
  }

  auto connectionsSorted(data.connections);
  std::sort(connectionsSorted.begin(), connectionsSorted.end());

  for (const auto& conn : connectionsSorted)
  {
    auto modOut = info.moduleIdMapping.find(conn.out_.moduleId_);
    auto modIn = info.moduleIdMapping.find(conn.in_.moduleId_);
    if (modOut != info.moduleIdMapping.end() && modIn != info.moduleIdMapping.end())
    {
      auto from = network->lookupModule(ModuleId(modOut->second));
      auto to = network->lookupModule(ModuleId(modIn->second));
      if (from && to)
        controller_->requestConnection(from->getOutputPort(conn.out_.portId_).get(), to->getInputPort(conn.in_.portId_).get());
    }
  }
  return info;
}

NetworkToXML::NetworkToXML(NetworkEditorSerializationManager* nesm)
  : nesm_(nesm)
{}

NetworkFileHandle NetworkXMLConverter::to_xml_data(const NetworkHandle& network)
{
  return NetworkToXML(nesm_).to_xml_data(network);
}

NetworkFileHandle NetworkToXML::to_xml_data(const NetworkHandle& network)
{
  return to_xml_data(network, boost::lambda::constant(true), boost::lambda::constant(true));
}

NetworkFileHandle NetworkToXML::to_xml_data(const NetworkHandle& network, ModuleFilter modFilter, ConnectionFilter connFilter)
{
  NetworkXML networkXML;
  auto conns = network->connections(true);
  for (const auto& desc : conns)
  {
    if (connFilter(desc))
      networkXML.connections.push_back(ConnectionDescriptionXML(desc));
  }
  for (size_t i = 0; i < network->nmodules(); ++i)
  {
    auto module = network->module(i);
    if (modFilter(module))
    {
      auto state = module->get_state();
      auto stateXML = make_state_xml(state);
      networkXML.modules[module->id()] = ModuleWithState(module->info(), stateXML ? *stateXML : SimpleMapModuleStateXML());
    }
  }

  auto file(boost::make_shared<NetworkFile>());
  file->network = networkXML;
  if (nesm_)
  {
    file->modulePositions = *nesm_->dumpModulePositions(modFilter);
    file->moduleNotes = *nesm_->dumpModuleNotes(modFilter);
    file->connectionNotes = *nesm_->dumpConnectionNotes(connFilter);
    file->moduleTags = *nesm_->dumpModuleTags(modFilter);
    file->disabledComponents = *nesm_->dumpDisabledComponents(modFilter, connFilter);
    file->subnetworks = *nesm_->dumpSubnetworks(modFilter);
  }
  return file;
}

void NetworkXMLSerializer::save_xml(const NetworkXML& data, const std::string& filename)
{
  XMLSerializer::save_xml(data, filename, "network");
}

void NetworkXMLSerializer::save_xml(const NetworkXML& data, std::ostream& ostr)
{
  XMLSerializer::save_xml(data, ostr, "network");
}

NetworkXMLHandle NetworkXMLSerializer::load_xml(const std::string& filename)
{
  return XMLSerializer::load_xml<NetworkXML>(filename);
}

NetworkXMLHandle NetworkXMLSerializer::load_xml(std::istream& istr)
{
  return XMLSerializer::load_xml<NetworkXML>(istr);
}
