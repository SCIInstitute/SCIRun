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


#ifndef DATAFLOW_NETWORK_DATAFLOW_INTERFACES_H
#define DATAFLOW_NETWORK_DATAFLOW_INTERFACES_H 

#include <boost/optional.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Datatypes/Datatype.h>
#include <Dataflow/Network/Share.h>

namespace SCIRun {
namespace Domain {
namespace Networks {

  //template <class Receiver> switch to run-time compatibility checking...
  class SCISHARE DatatypeSourceInterface
  {
  public:
    virtual ~DatatypeSourceInterface() {}
    virtual void send(DatatypeSinkInterfaceHandle receiver, Datatypes::DatatypeHandle data) = 0;
  };

  class SCISHARE DatatypeSinkInterface
  {
  public:
    virtual ~DatatypeSinkInterface() {}
    virtual void waitForData() = 0;
    virtual Datatypes::DatatypeHandleOption receive() = 0;
  };

}}}


#endif
