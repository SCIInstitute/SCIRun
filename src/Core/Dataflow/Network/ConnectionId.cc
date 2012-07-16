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

#include <iostream>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <Core/Dataflow/Network/ConnectionId.h>

using namespace SCIRun::Domain::Networks;

bool SCIRun::Domain::Networks::operator==(const ConnectionId& lhs, const ConnectionId& rhs)
{
  return lhs.id_ == rhs.id_;
}

bool SCIRun::Domain::Networks::operator!=(const ConnectionId& lhs, const ConnectionId& rhs)
{
  return !(lhs == rhs);
}

bool SCIRun::Domain::Networks::operator==(const ConnectionDescription& lhs, const ConnectionDescription& rhs)
{
  return lhs.moduleId1_ == rhs.moduleId1_
    && lhs.moduleId2_ == rhs.moduleId2_
    && lhs.port1_ == rhs.port1_
    && lhs.port2_== rhs.port2_;
}

bool SCIRun::Domain::Networks::operator!=(const ConnectionDescription& lhs, const ConnectionDescription& rhs)
{
  return !(lhs == rhs);
}


/*static*/ ConnectionId ConnectionId::create(const ConnectionDescription& desc)
{
  std::ostringstream cid;
  cid << desc.moduleId1_ << "_p#" << desc.port1_ << "_@to@_" << desc.moduleId2_ << "_p#" << desc.port2_;
  return ConnectionId(cid.str());
}

ConnectionDescription ConnectionId::describe() const
{
  static boost::regex r("(.+)_p#(\\d+)_@to@_(.+)_p#(\\d+)");
  boost::smatch what;
  regex_match(id_, what, r);
  return ConnectionDescription(what[1], 
    boost::lexical_cast<size_t>((std::string)what[2]), 
    what[3], 
    boost::lexical_cast<size_t>((std::string)what[4]));
}
