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


/// @todo Documentation Dataflow/Network/ConnectionId.h

#ifndef DATAFLOW_NETWORK_CONNECTIONID_H
#define DATAFLOW_NETWORK_CONNECTIONID_H

#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  struct SCISHARE OutgoingConnectionDescription
  {
    OutgoingConnectionDescription() {}
    OutgoingConnectionDescription(const ModuleId& id, const PortId& p)
      : moduleId_(id), portId_(p) {}
    ModuleId moduleId_;
    PortId portId_;
  };

  struct SCISHARE IncomingConnectionDescription
  {
    IncomingConnectionDescription() {}
    IncomingConnectionDescription(const ModuleId& id, const PortId& p)
      : moduleId_(id), portId_(p) {}
    ModuleId moduleId_;
    PortId portId_;
  };

  struct SCISHARE ConnectionDescription
  {
    ConnectionDescription() {}
    ConnectionDescription(const OutgoingConnectionDescription& out, const IncomingConnectionDescription& in)
      : out_(out), in_(in) {}
    OutgoingConnectionDescription out_;
    IncomingConnectionDescription in_;
  };

  struct SCISHARE ConnectionId
  {
    ConnectionId() {}
    ConnectionId(const char* s) : id_(s) {}
    ConnectionId(const std::string& s) : id_(s) {}
    static ConnectionId create(const ConnectionDescription& desc);
    ConnectionDescription describe() const;

    std::string id_;
    operator std::string() const { return id_; }
  };

  SCISHARE bool operator==(const ConnectionId& lhs, const ConnectionId& rhs);
  SCISHARE bool operator!=(const ConnectionId& lhs, const ConnectionId& rhs);
  SCISHARE bool operator==(const OutgoingConnectionDescription& lhs, const OutgoingConnectionDescription& rhs);
  SCISHARE bool operator!=(const OutgoingConnectionDescription& lhs, const OutgoingConnectionDescription& rhs);
  SCISHARE bool operator==(const IncomingConnectionDescription& lhs, const IncomingConnectionDescription& rhs);
  SCISHARE bool operator!=(const IncomingConnectionDescription& lhs, const IncomingConnectionDescription& rhs);
  SCISHARE bool operator==(const ConnectionDescription& lhs, const ConnectionDescription& rhs);
  SCISHARE bool operator!=(const ConnectionDescription& lhs, const ConnectionDescription& rhs);
  SCISHARE std::ostream& operator<<(std::ostream& o, const ConnectionId& cid);
  SCISHARE std::ostream& operator<<(std::ostream& o, const ConnectionDescription& cd);

  struct SCISHARE OrderedByConnectionId
  {
    bool operator()(const ConnectionId& lhs, const ConnectionId& rhs) const
    {
      return lhs.id_ < rhs.id_;
    }
  };

}}}


#endif
