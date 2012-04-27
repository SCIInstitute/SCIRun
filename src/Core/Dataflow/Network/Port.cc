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
#include <Core/Dataflow/Network/Port.h>
#include <Core/Datatypes/Datatype.h>

#include <stdexcept>

using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Domain::Datatypes;

Port::Port(ModuleInterface* module, const ConstructionParams& params)
  : module_(module), typeName_(params.type_name), portName_(params.port_name), colorName_(params.color_name)
{
  if (!module_)
    throw std::invalid_argument("port cannot have null module");
  if (typeName_.empty() || portName_.empty() || colorName_.empty())
    throw std::invalid_argument("port has empty metadata");
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
  std::vector<Connection*>::iterator pos = std::find(connections_.begin(), connections_.end(), conn);
  if (pos == connections_.end())
  {
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

InputPort::InputPort(ModuleInterface* module, const ConstructionParams& params)
  : Port(module, params)
{

}

InputPort::~InputPort()
{

}

DatatypeHandle InputPort::get()
{
  throw "TDD";
}

OutputPort::OutputPort(ModuleInterface* module, const ConstructionParams& params)
  : Port(module, params)
{

}

OutputPort::~OutputPort()
{

}

void OutputPort::send(DatatypeHandle data)
{
  throw "TDD";
}