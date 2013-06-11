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


#ifndef DATAFLOW_NETWORK_NETWORK_H
#define DATAFLOW_NETWORK_NETWORK_H 

#include <boost/noncopyable.hpp>
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Dataflow/Network/NetworkSettings.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE Network : public NetworkInterface, boost::noncopyable
  {
  public:
    typedef std::map<ConnectionId, ConnectionHandle, OrderedByConnectionId> Connections;
    typedef std::vector<ModuleHandle> Modules;

    Network(ModuleFactoryHandle moduleFactory, ModuleStateFactoryHandle stateFactory);
    ~Network();

    virtual ModuleHandle add_module(const ModuleLookupInfo& info);
    virtual bool remove_module(const ModuleId& id);
    virtual size_t nmodules() const;
    virtual ModuleHandle module(size_t i) const;
    virtual ExecutableObject* lookupExecutable(const ModuleId& id) const;
    virtual ModuleHandle lookupModule(const ModuleId& id) const;
    virtual ConnectionId connect(const ConnectionOutputPort&, const ConnectionInputPort&);
    virtual bool disconnect(const ConnectionId&);
    virtual size_t nconnections() const;
    virtual void disable_connection(const ConnectionId&);
    virtual ConnectionDescriptionList connections() const;
    virtual int errorCode() const;
    virtual void incrementErrorCode(const ModuleId& moduleId);
    virtual NetworkGlobalSettings& settings();
    virtual std::string toString() const;
  private:
    ModuleFactoryHandle moduleFactory_;
    ModuleStateFactoryHandle stateFactory_;
    Modules modules_;
    Connections connections_;
    int errorCode_;
    NetworkGlobalSettings settings_;
  };

}}}


#endif
