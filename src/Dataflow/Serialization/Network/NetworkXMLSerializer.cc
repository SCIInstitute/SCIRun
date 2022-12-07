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
#include <Core/Logging/Log.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Algorithms;

NetworkToXML::NetworkToXML(NetworkEditorSerializationManager* nesm)
  : nesm_(nesm)
{}

NetworkFileHandle NetworkToXML::to_xml_data(const NetworkStateHandle& network)
{
  return to_xml_data(network, [](ModuleHandle) { return true; }, [](const ConnectionDescription&) { return true; });
}

NetworkFileHandle NetworkToXML::to_xml_data(const NetworkStateHandle& network, ModuleFilter modFilter, ConnectionFilter connFilter)
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

  auto file(makeShared<NetworkFile>());
  file->network = networkXML;
  if (nesm_)
  {
    file->modulePositions = *nesm_->dumpModulePositions(modFilter);
    file->moduleNotes = *nesm_->dumpModuleNotes(modFilter);
    file->connectionNotes = *nesm_->dumpConnectionNotes(connFilter);
    file->moduleTags = *nesm_->dumpModuleTags(modFilter);
    file->disabledComponents = *nesm_->dumpDisabledComponents(modFilter, connFilter);
    //file->subnetworks = *nesm_->dumpSubnetworks(modFilter);
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
