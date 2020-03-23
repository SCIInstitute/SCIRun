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


#include <iostream>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <Dataflow/Network/ConnectionId.h>

using namespace SCIRun::Dataflow::Networks;

bool SCIRun::Dataflow::Networks::operator==(const ConnectionId& lhs, const ConnectionId& rhs)
{
  return lhs.id_ == rhs.id_;
}

bool SCIRun::Dataflow::Networks::operator!=(const ConnectionId& lhs, const ConnectionId& rhs)
{
  return !(lhs == rhs);
}

bool SCIRun::Dataflow::Networks::operator==(const OutgoingConnectionDescription& lhs, const OutgoingConnectionDescription& rhs)
{
  return lhs.moduleId_ == rhs.moduleId_
    && lhs.portId_.name == rhs.portId_.name;
}

bool SCIRun::Dataflow::Networks::operator!=(const OutgoingConnectionDescription& lhs, const OutgoingConnectionDescription& rhs)
{
  return !(lhs == rhs);
}

bool SCIRun::Dataflow::Networks::operator==(const IncomingConnectionDescription& lhs, const IncomingConnectionDescription& rhs)
{
  return lhs.moduleId_ == rhs.moduleId_
    && lhs.portId_.name == rhs.portId_.name;
}

bool SCIRun::Dataflow::Networks::operator!=(const IncomingConnectionDescription& lhs, const IncomingConnectionDescription& rhs)
{
  return !(lhs == rhs);
}

bool SCIRun::Dataflow::Networks::operator==(const ConnectionDescription& lhs, const ConnectionDescription& rhs)
{
  return lhs.in_ == rhs.in_
    && lhs.out_ == rhs.out_;
}

bool SCIRun::Dataflow::Networks::operator!=(const ConnectionDescription& lhs, const ConnectionDescription& rhs)
{
  return !(lhs == rhs);
}

std::ostream& SCIRun::Dataflow::Networks::operator<<(std::ostream& o, const ConnectionId& cid)
{
  return o << cid.id_;
}

std::ostream& SCIRun::Dataflow::Networks::operator<<(std::ostream& o, const ConnectionDescription& cd)
{
  return o << ConnectionId::create(cd);
}

/*static*/ ConnectionId ConnectionId::create(const ConnectionDescription& desc)
{
  std::ostringstream cid;
  cid << desc.out_.moduleId_ << "_p#" << desc.out_.portId_ << "#_@to@_" << desc.in_.moduleId_ << "_p#" << desc.in_.portId_ << "#";
  return ConnectionId(cid.str());
}

ConnectionDescription ConnectionId::describe() const
{
  static boost::regex r("(.+)_p#(.+):(\\d+)#_@to@_(.+)_p#(.+):(\\d+)#");
  boost::smatch what;
  regex_match(id_, what, r);
  return ConnectionDescription(
    OutgoingConnectionDescription(ModuleId(what[1]), PortId(boost::lexical_cast<size_t>(what[3]), (std::string)what[2])),
    IncomingConnectionDescription(ModuleId(what[4]), PortId(boost::lexical_cast<size_t>(what[6]), (std::string)what[5])));
}
