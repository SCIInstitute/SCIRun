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


#ifndef CORE_DATAFLOW_NETWORK_CONNECTIONID_H
#define CORE_DATAFLOW_NETWORK_CONNECTIONID_H 

#include <string>
#include <Core/Dataflow/Network/Share.h>

namespace SCIRun {
namespace Domain {
namespace Networks {

  struct SCISHARE ConnectionDescription
  {
    ConnectionDescription() : port1_(-1), port2_(-1) {}
    ConnectionDescription(const std::string& id1, size_t p1, const std::string& id2, size_t p2)
      : moduleId1_(id1), port1_(p1), moduleId2_(id2), port2_(p2) {}
    std::string moduleId1_;
    size_t port1_;
    std::string moduleId2_;
    size_t port2_;
  };

  struct SCISHARE ConnectionId
  {
    ConnectionId(const char* s) : id_(s) {}
    ConnectionId(const std::string& s) : id_(s) {}
    static ConnectionId create(const ConnectionDescription& desc);
    ConnectionDescription describe() const;

    std::string id_;
    operator std::string() const { return id_; }
  };

  SCISHARE bool operator==(const ConnectionId& lhs, const ConnectionId& rhs);
  SCISHARE bool operator!=(const ConnectionId& lhs, const ConnectionId& rhs);
  SCISHARE bool operator==(const ConnectionDescription& lhs, const ConnectionDescription& rhs);
  SCISHARE bool operator!=(const ConnectionDescription& lhs, const ConnectionDescription& rhs);

  struct SCISHARE OrderedByConnectionId
  {
    bool operator()(const ConnectionId& lhs, const ConnectionId& rhs) const
    {
      return lhs.id_ < rhs.id_;
    }
  };

}}}


#endif
