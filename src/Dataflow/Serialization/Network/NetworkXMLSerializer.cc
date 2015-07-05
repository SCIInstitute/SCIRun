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

/// @todo Documentation Dataflow/Serialization/Network/NetworkXMLSerializer.cc

#include <Dataflow/Serialization/Network/NetworkXMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Network/Network.h> /// @todo: need network factory??
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/PortInterface.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <fstream>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/foreach.hpp>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Algorithms;

class ScopedControllerSignalDisabler
{
public:
  ScopedControllerSignalDisabler(NetworkEditorControllerInterface* nec) : nec_(nec)
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

NetworkHandle NetworkXMLConverter::from_xml_data(const NetworkXML& data)
{
  /// @todo: need to use NEC here to manage signal/slots for dynamic ports.
  NetworkHandle network(boost::make_shared<Network>(moduleFactory_, stateFactory_, algoFactory_, reexFactory_));
  controller_->setNetwork(network);

  {
    ScopedControllerSignalDisabler scsd(controller_);
    BOOST_FOREACH(const ModuleMapXML::value_type& modPair, data.modules)
    {
      ModuleHandle module = controller_->addModule(modPair.second.module);
      module->set_id(modPair.first);
      ModuleStateHandle state(new SimpleMapModuleState(std::move(modPair.second.state)));
      module->set_state(state);
    }
  }

  std::vector<ConnectionDescriptionXML> connectionsSorted(data.connections);
  std::sort(connectionsSorted.begin(), connectionsSorted.end());
  BOOST_FOREACH(const ConnectionDescriptionXML& conn, connectionsSorted)
  {
    ModuleHandle from = network->lookupModule(conn.out_.moduleId_);
    ModuleHandle to = network->lookupModule(conn.in_.moduleId_);

    controller_->requestConnection(from->getOutputPort(conn.out_.portId_).get(), to->getInputPort(conn.in_.portId_).get());
  }

  return network;
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
  NetworkXML networkXML;
  Network::ConnectionDescriptionList conns = network->connections();
  BOOST_FOREACH(ConnectionDescription& desc, conns)
    networkXML.connections.push_back(ConnectionDescriptionXML(desc));
  for (size_t i = 0; i < network->nmodules(); ++i)
  {
    ModuleHandle module = network->module(i);
    ModuleStateHandle state = module->get_state();
    boost::shared_ptr<SimpleMapModuleStateXML> stateXML = make_state_xml(state);
    networkXML.modules[module->get_id()] = ModuleWithState(module->get_info(), stateXML ? *stateXML : SimpleMapModuleStateXML());
  }

  NetworkFileHandle file(boost::make_shared<NetworkFile>());
  file->network = networkXML;
  if (nesm_)
  {
    file->modulePositions = *nesm_->dumpModulePositions();
    file->moduleNotes = *nesm_->dumpModuleNotes();
    file->connectionNotes = *nesm_->dumpConnectionNotes();
    file->moduleTags = *nesm_->dumpModuleTags();
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
