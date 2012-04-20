/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <Core/Dataflow/Network/Network.h>
#include <Core/Dataflow/Network/Connection.h>
#include <Core/Dataflow/Network/Module.h>
#include <Core/Dataflow/Network/ModuleDescription.h>
#include <Core/Dataflow/Network/ModuleFactory.h>

using namespace SCIRun::Domain::Networks;
using namespace boost::lambda;

Network::Network(ModuleFactoryHandle moduleFactory)
  : moduleFactory_(moduleFactory)
{

}

Network::~Network()
{

}

ModuleHandle Network::add_module(const ModuleLookupInfo& info)
{
  ModuleHandle module = moduleFactory_->create(moduleFactory_->lookupDescription(info));
  modules_.push_back(module);
  return module;
}

bool Network::remove_module(const std::string& id)
{
  Modules::iterator loc = std::find_if(modules_.begin(), modules_.end(), boost::lambda::bind(&ModuleInterface::get_id, *_1) == id);
  if (loc != modules_.end())
  {
    modules_.erase(loc);
    return true;
  }
  return false;
}

ConnectionId Network::connect(ModuleHandle m1, size_t p1, ModuleHandle m2, size_t p2)
{
  if (!m1 || !m2)
    throw std::invalid_argument("cannot connect null modules");
  // assure that the ports are not altered while connecting
  //m1->oports_.lock();
  //m2->iports_.lock();

  // dynamic port safeguard.
  //if (m2->lastportdynamic_ && p2 >= m2->iports_.size()) {
  //  p2 = m2->iports_.size() - 1;
  //}

  // TODO: release ports lock here?
  if (p1 >= m1->num_output_ports() || p2 >= m2->num_input_ports())
  {
    return ConnectionId("");
  }

  ConnectionId id = ConnectionId::create(ConnectionDescription(m1->get_id(), p1, m2->get_id(), p2));
  if (connections_.find(id) == connections_.end())
  {
    ConnectionHandle conn(new Connection(m1, p1, m2, p2, id));

  //lock.lock();

    connections_[id] = conn;

  // Reschedule next time we can.
  //reschedule=1;

  //lock.unlock();

  //m1->oports_.unlock();
  //m2->iports_.unlock();

    return id;
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
  //TODO
}

size_t Network::nmodules() const
{
  return modules_.size();
}

ModuleHandle Network::module(size_t i) const
{
  if (i >= nmodules())
    throw std::out_of_range("Module index out of range");

  return modules_[i];
}

ModuleHandle Network::lookupModule(const std::string& id) const
{
  Modules::const_iterator i = std::find_if(modules_.begin(), modules_.end(), boost::lambda::bind(&ModuleInterface::get_id, *_1) == id);
  return i == modules_.end() ? ModuleHandle() : *i;
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
  std::transform(modules_.begin(), modules_.end(), std::ostream_iterator<std::string>(ostr, ", "), bind(to_string, *_1));
  ostr << "\nConnections:\n";
  std::transform(connections_.begin(), connections_.end(), std::ostream_iterator<std::string>(ostr, ", "), GetConnectionIds());
  return ostr.str();
}