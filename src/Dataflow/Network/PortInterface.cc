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


/// @todo Documentation Dataflow/Network/PortInterface.cc

#include <Dataflow/Network/PortInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/DataflowInterfaces.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;

PortDescriptionInterface::~PortDescriptionInterface()
{
}

PortInterface::~PortInterface()
{
}

InputPortInterface::~InputPortInterface()
{
}

OutputPortInterface::~OutputPortInterface()
{
}

namespace
{
  bool isFullInputPort(const PortDescriptionInterface& port)
  {
    return port.isInput() && port.nconnections() == 1;
  }

  bool sharesParentModule(const PortDescriptionInterface& port1, const PortDescriptionInterface& port2)
  {
    return port1.getUnderlyingModuleId() == port2.getUnderlyingModuleId();
  }

  bool isWildPort(const PortDescriptionInterface& port)
  {
    return port.get_typename() == "Datatype";
  }
}



/// @todo: unit test
bool PortConnectionDeterminer::canBeConnected(const PortDescriptionInterface& port1, const PortDescriptionInterface& port2) const
{
  if (isFullInputPort(port1) || isFullInputPort(port2))
  {
    LOG_TRACE("can't connect since input ports can only take one connection");
    return false;
  }
  if (port1.isInput() == port2.isInput())
  {
    LOG_TRACE("can't connect since input/output not compatible");
    return false;
  }
  if (sharesParentModule(port1, port2))
  {
    LOG_TRACE("can't connect since it's the same module");
    return false;
  }
  if (isWildPort(port1) || isWildPort(port2))
  {
    LOG_TRACE("found wild port");
    /// @todo: trying out "wildcard" ports
    return true;
  }
  if (port1.get_typename() != port2.get_typename())
  {
    LOG_TRACE("can't connect since colors don't match");
    return false;
  }
  return true;
}
