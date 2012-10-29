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
#include <boost/foreach.hpp>
#include <Dataflow/Network/Port.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Utils/Exception.h>
#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/DataflowInterfaces.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

Port::Port(ModuleInterface* module, const ConstructionParams& params)
  : module_(module), typeName_(params.type_name), portName_(params.port_name), colorName_(params.color_name)
{
  ENSURE_NOT_NULL1(module_, "port cannot have null module");
  if (typeName_.empty() || portName_.empty() || colorName_.empty())
    THROW_INVALID_ARGUMENT("port has empty metadata");
}

Port::~Port()
{

}

void Port::attach(Connection* conn)
{
  connections_.push_back(conn);
}

void Port::detach(Connection* conn)
{
  auto pos = std::find(connections_.begin(), connections_.end(), conn);
  if (pos == connections_.end())
  {
    //TODO: use real logger here
    std::cerr << "Port::detach: Connection not found";
  }
  connections_.erase(pos);
}

const Connection* Port::connection(size_t i) const
{
  return connections_[i];
}

size_t Port::nconnections() const
{
  return connections_.size();
}

InputPort::InputPort(ModuleInterface* module, const ConstructionParams& params, DatatypeSinkInterfaceHandle sink)
  : Port(module, params), sink_(sink)
{

}

InputPort::~InputPort()
{

}

DatatypeSinkInterfaceHandle InputPort::sink()
{
  return sink_;
}

DatatypeHandleOption InputPort::getData()
{
  if (0 == nconnections())
    return DatatypeHandleOption();

  sink_->waitForData();
  return sink_->receive();
}

void InputPort::attach(Connection* conn)
{
  if (connections_.size() > 0)
    THROW_INVALID_ARGUMENT("input ports accept at most one connection");
  Port::attach(conn);
}

OutputPort::OutputPort(ModuleInterface* module, const ConstructionParams& params, DatatypeSourceInterfaceHandle source)
  : Port(module, params), source_(source)
{

}

OutputPort::~OutputPort()
{

}

void OutputPort::sendData(DatatypeHandle data)
{
  if (0 == nconnections())
    return;
  BOOST_FOREACH(Connection* c, connections_)
    source_->send(c->iport_->sink(), data);
}