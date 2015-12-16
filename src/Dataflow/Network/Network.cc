/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

/// @todo Documentation Dataflow/Network/Network.cc

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/thread.hpp>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/ModuleFactory.h>
#include <Core/Utils/Exception.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

Network::Network(ModuleFactoryHandle moduleFactory, ModuleStateFactoryHandle stateFactory, AlgorithmFactoryHandle algoFactory, ReexecuteStrategyFactoryHandle reexFactory)
  : moduleFactory_(moduleFactory), stateFactory_(stateFactory), errorCode_(0)
{
  moduleFactory_->setStateFactory(stateFactory_);
  moduleFactory_->setAlgorithmFactory(algoFactory);
  moduleFactory_->setReexecutionFactory(reexFactory);
}

Network::~Network()
{
}

ModuleHandle Network::add_module(const ModuleLookupInfo& info)
{
  ModuleHandle module = moduleFactory_->create(moduleFactory_->lookupDescription(info));
  modules_.push_back(module);
  if (module)
  {
    module->connectErrorListener(boost::bind(&NetworkInterface::incrementErrorCode, this, _1));
  }
  return module;
}

bool Network::remove_module(const ModuleId& id)
{
  Modules::iterator loc = std::find_if(modules_.begin(), modules_.end(), boost::lambda::bind(&ModuleInterface::get_id, *boost::lambda::_1) == id);
  if (loc != modules_.end())
  {
    // Inform the module that it is about to be erased from the network...
    modules_.erase(loc);
    return true;
  }
  return false;
}

ConnectionId Network::connect(const ConnectionOutputPort& out, const ConnectionInputPort& in)
{
  ModuleHandle outputModule = out.first;
  ModuleHandle inputModule = in.first;
  auto outputPortId = out.second;
  auto inputPortId = in.second;

  ENSURE_NOT_NULL(outputModule, "cannot connect null output module");
  ENSURE_NOT_NULL(inputModule, "cannot connect null input module");

  // assure that the ports are not altered while connecting
  //m1->oports_.lock();
  //m2->iports_.lock();

  if (!outputModule->hasOutputPort(outputPortId))
  {
    std::ostringstream ostr;
    ostr << "Unknown output port: " << outputPortId;
    THROW_INVALID_ARGUMENT(ostr.str());
  }
  if (!inputModule->hasInputPort(inputPortId))
  {
    std::ostringstream ostr;
    ostr << "Unknown input port: " << inputPortId;
    THROW_INVALID_ARGUMENT(ostr.str());
  }

  ConnectionId id = ConnectionId::create(ConnectionDescription(
    OutgoingConnectionDescription(outputModule->get_id(), outputPortId),
    IncomingConnectionDescription(inputModule->get_id(), inputPortId)));
  if (connections_.find(id) == connections_.end())
  {
    try
    {
      ConnectionHandle conn(boost::make_shared<Connection>(outputModule->getOutputPort(outputPortId), inputModule->getInputPort(inputPortId), id));

      connections_[id] = conn;

      return id;
    }
    catch (const SCIRun::Core::ExceptionBase& e)
    {
      std::cout << "Caught exception making a connection: " << e.what() << std::endl;
      ///????????
      return ConnectionId(""); //??
    }
  }
  return ConnectionId(""); //??
}

bool Network::disconnect(const ConnectionId& id)
{
  Connections::iterator loc = connections_.find(id);
  if (loc != connections_.end())
  {
    connections_.erase(loc);
    return true;
  }
  return false;
}

void Network::disable_connection(const ConnectionId&)
{
  /// @todo
}

size_t Network::nmodules() const
{
  return modules_.size();
}

ModuleHandle Network::module(size_t i) const
{
  if (i >= nmodules())
    THROW_OUT_OF_RANGE("Module index out of range");

  return modules_[i];
}

ModuleHandle Network::lookupModule(const ModuleId& id) const
{
  Modules::const_iterator i = std::find_if(modules_.begin(), modules_.end(), boost::lambda::bind(&ModuleInterface::get_id, *boost::lambda::_1) == id);
  return i == modules_.end() ? nullptr : *i;
}

ExecutableObject* Network::lookupExecutable(const ModuleId& id) const
{
  return lookupModule(id).get();
}

size_t Network::nconnections() const
{
  return connections_.size();
}

struct GetConnectionIds
{
  std::string operator()(const Network::Connections::value_type& pair) const
  {
    return pair.first.id_;
  }
};

std::string Network::toString() const
{
  using boost::lambda::bind;
  std::ostringstream ostr;
  ostr << "~~~NETWORK DESCRIPTION~~~\n";
  ostr << "Modules:\n";
  std::transform(modules_.begin(), modules_.end(), std::ostream_iterator<std::string>(ostr, ", "), [](ModuleHandle m) { return to_string(*m);});
  ostr << "\nConnections:\n";
  std::transform(connections_.begin(), connections_.end(), std::ostream_iterator<std::string>(ostr, ", "), GetConnectionIds());
  return ostr.str();
}

struct Describe
{
  ConnectionDescription operator()(const Network::Connections::value_type& p) const
  {
    return p.first.describe();
  }
};

NetworkInterface::ConnectionDescriptionList Network::connections() const
{
  ConnectionDescriptionList conns;
  std::transform(connections_.begin(), connections_.end(), std::back_inserter(conns), Describe());
  return conns;
}

int Network::errorCode() const
{
  return errorCode_;
}

void Network::incrementErrorCode(const ModuleId& moduleId)
{
  errorCode_++;
  /// @todo: store errored modules in a list or something
}

NetworkGlobalSettings& Network::settings()
{
  return settings_;
}

void Network::setModuleExecutionState(ModuleExecutionState::Value state, ModuleFilter filter)
{
  for (ModuleHandle module : modules_ | boost::adaptors::filtered(filter))
  {
    module->executionState().transitionTo(state);
  }
}

void Network::clear()
{
  connections_.clear();
  modules_.clear();
}

bool Network::containsViewScene() const
{
  return std::find_if(modules_.begin(), modules_.end(), [](ModuleHandle m) { return m->get_module_name() == "ViewScene"; }) != modules_.end();
}

boost::signals2::connection Network::connectModuleInterrupted(ModuleInterruptedSignal::slot_function_type subscriber) const
{
  return interruptModule_.connect(subscriber);
}

void Network::interruptModuleRequest(const ModuleId& id)
{
  interruptModule_(id.id_);
}

ConnectionOutputPort::ConnectionOutputPort(ModuleHandle m, size_t index) : ModulePortIdPair(m, m->outputPorts().at(index)->id())
{
}

ConnectionInputPort::ConnectionInputPort(ModuleHandle m, size_t index) : ModulePortIdPair(m, m->inputPorts().at(index)->id())
{
}
