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

#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Module.h>

#include <stdexcept>

using namespace SCIRun::Domain::Networks;

Connection::Connection(ModuleHandle omod, size_t oportno, ModuleHandle imod, size_t iportno, const ConnectionId& id)
  : omod_(omod), imod_(imod), id_(id)
{
  if (!omod)
    throw std::invalid_argument("output module is null");
  if (!imod)
    throw std::invalid_argument("input module is null");
  oport_ = omod_->get_output_port(oportno);
  if (!oport_)
    throw std::invalid_argument("output port is null");
  iport_ = imod_->get_input_port(iportno);
  if (!iport_)
    throw std::invalid_argument("input port is null");

  //TODO: check for matching types here

  oport_->attach(this);
  iport_->attach(this);
}

Connection::~Connection()
{
  oport_->detach(this);
  iport_->detach(this);
}

