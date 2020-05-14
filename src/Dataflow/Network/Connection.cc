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


#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Module.h>
#include <Core/Utils/Exception.h>

using namespace SCIRun::Dataflow::Networks;

Connection::Connection(OutputPortHandle oport, InputPortHandle iport, const ConnectionId& id, bool isVirtual)
  : oport_(oport), iport_(iport), id_(id), virtual_(isVirtual)
{
  ENSURE_NOT_NULL(oport_, "output port is null");
  ENSURE_NOT_NULL(iport_, "input port is null");

  /// @todo: this is already checked in the controller layer. Do we need a redundant check here?
  //if (oport_->get_colorname() != iport_->get_colorname())
  //  THROW_INVALID_ARGUMENT("Ports do not have matching type.");

  oport_->attach(this);
  iport_->attach(this);
}

Connection::~Connection()
{
  oport_->detach(this);
  iport_->detach(this);
}

std::string Connection::id() const
{
  return id_;
}

void Connection::setDisable(bool disable)
{
  disabled_ = disable;
  if (!disabled_)
    iport_->resendNewDataSignal();
}
