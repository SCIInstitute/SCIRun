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


#ifndef CORE_DATAFLOW_NETWORK_NETWORK_FWD_H
#define CORE_DATAFLOW_NETWORK_NETWORK_FWD_H 

#include <boost/shared_ptr.hpp>

namespace SCIRun {
namespace Domain {
namespace Networks {

class Network;
class Module;
class Port;
class Connection;
class PortManager;
class InputPort;
class OutputPort;

typedef boost::shared_ptr<Network> NetworkHandle;
typedef boost::shared_ptr<Module> ModuleHandle;
typedef boost::shared_ptr<Port> PortHandle;
typedef boost::shared_ptr<Connection> ConnectionHandle;
typedef boost::shared_ptr<PortManager> PortManagerHandle;
typedef boost::shared_ptr<InputPort> InputPortHandle;
typedef boost::shared_ptr<OutputPort> OutputPortHandle;
  
}}}


#endif
