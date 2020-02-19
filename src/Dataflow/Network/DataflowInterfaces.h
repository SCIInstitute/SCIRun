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


/// @todo Documentation Dataflow/Network/DataflowInterfaces.h

#ifndef DATAFLOW_NETWORK_DATAFLOW_INTERFACES_H
#define DATAFLOW_NETWORK_DATAFLOW_INTERFACES_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Datatypes/Datatype.h>
#include <boost/signals2/signal.hpp>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  //template <class Receiver> switch to run-time compatibility checking...
  class SCISHARE DatatypeSourceInterface
  {
  public:
    virtual ~DatatypeSourceInterface() {}
    virtual void cacheData(Core::Datatypes::DatatypeHandle data) = 0;
    virtual void send(DatatypeSinkInterfaceHandle receiver) const = 0;
    virtual bool hasData() const = 0;
    virtual std::string describeData() const = 0;
  };

  typedef boost::signals2::signal<void(SCIRun::Core::Datatypes::DatatypeHandle)> DataHasChangedSignalType;

  class SCISHARE DatatypeSinkInterface
  {
  public:
    virtual ~DatatypeSinkInterface() {}

    // "mailbox" interface
    //virtual bool hasData() const = 0;
    //virtual void setHasData(bool dataPresent) = 0;

    virtual DatatypeSinkInterface* clone() const = 0;

    virtual void waitForData() = 0;
    virtual Core::Datatypes::DatatypeHandleOption receive() = 0;
    virtual bool hasChanged() const = 0;
    virtual void invalidateProvider() = 0;
    virtual boost::signals2::connection connectDataHasChanged(const DataHasChangedSignalType::slot_type& subscriber) = 0;
    virtual void forceFireDataHasChanged() = 0;
  };

}}}


#endif
