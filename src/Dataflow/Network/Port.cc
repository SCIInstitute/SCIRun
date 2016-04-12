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

/// @todo Documentation Dataflow/Network/Port.cc

#include <Dataflow/Network/Port.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Utils/Exception.h>
#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/DataflowInterfaces.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

Port::Port(ModuleInterface* module, const ConstructionParams& params)
  : module_(module), index_(0), id_(params.id_), typeName_(params.type_name), portName_(params.port_name), colorName_(PortColorLookup::toColor(params.type_name))
{
  ENSURE_NOT_NULL(module_, "port cannot have null module");
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
    LOG_DEBUG(id() << " Port::detach: Connection not found" << std::endl);
  }
  connections_.erase(pos);
}

const Connection* Port::connection(size_t i) const
{
  return connections_[i];
}

boost::optional<ConnectionId> Port::firstConnectionId() const
{
  return !connections_.empty() ? connections_[0]->id_ : boost::optional<ConnectionId>();
}

void Port::setIndex(size_t index)
{
  index_ = index;
}

size_t Port::nconnections() const
{
  return connections_.size();
}

ModuleId Port::getUnderlyingModuleId() const
{
  return module_->get_id();
}

size_t Port::getIndex() const
{
  return index_;
}

InputPort::InputPort(ModuleInterface* module, const ConstructionParams& params, DatatypeSinkInterfaceHandle sink)
  : Port(module, params), sink_(sink), isDynamic_(params.isDynamic_)
{
}

InputPort::~InputPort()
{

}

DatatypeSinkInterfaceHandle InputPort::sink() const
{
  return sink_;
}

DatatypeHandleOption InputPort::getData() const
{
  if (0 == nconnections())
    return DatatypeHandleOption();

  sink_->waitForData();
  return sink_->receive();
}

void InputPort::attach(Connection* conn)
{
  if (connections_.size() > 0)
    THROW_INVALID_ARGUMENT("static input ports accept at most one connection");
  Port::attach(conn);
}

void InputPort::detach(Connection* conn)
{
  if (sink_)
    sink_->invalidateProvider();
  Port::detach(conn);
}

InputPortInterface* InputPort::clone() const
{
  DatatypeSinkInterfaceHandle sink(sink_->clone());
  if (!isDynamic_)
    THROW_INVALID_ARGUMENT("Cannot clone non-dynamic port.");
  PortId cloneId(id_.id + 1, id_.name);
  return new InputPort(module_, ConstructionParams(cloneId, typeName_, isDynamic_), sink);
}

bool InputPort::hasChanged() const
{
  return sink()->hasChanged();
}

boost::signals2::connection InputPort::connectDataOnPortHasChanged(const DataOnPortHasChangedSignalType::slot_type& subscriber)
{
  return sink()->connectDataHasChanged([this, subscriber] (DatatypeHandle data)
  {
    subscriber(this->id(), data);
  });
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
  source_->cacheData(data);

  if (0 == nconnections())
    return;

  for (Connection* c : connections_)
  {
    if (c && !c->disabled() && c->iport_)
      source_->send(c->iport_->sink());
  }
}

bool OutputPort::hasData() const
{
  if (!source_)
    return false;
  auto ret = source_->hasData();
  LOG_DEBUG(id() << " OutputPort::hasData returns " << ret << std::endl);
  return ret;
}

void OutputPort::attach(Connection* conn)
{
  if (hasData() && conn && conn->iport_)
    source_->send(conn->iport_->sink());

  Port::attach(conn);
}

PortDataDescriber OutputPort::getPortDataDescriber() const
{
  return [this]() { return source_->describeData(); };
}

boost::signals2::connection OutputPort::connectConnectionFeedbackListener(const ConnectionFeedbackSignalType::slot_type& subscriber)
{
  return cxnFeedback_.connect(subscriber);
}

void OutputPort::sendConnectionFeedback(const ModuleFeedback& info)
{
  cxnFeedback_(info);
}
