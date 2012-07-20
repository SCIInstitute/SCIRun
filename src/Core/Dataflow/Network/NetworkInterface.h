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


#ifndef CORE_DATAFLOW_NETWORK_NETWORK_INTERFACE_H
#define CORE_DATAFLOW_NETWORK_NETWORK_INTERFACE_H 

#include <Core/Dataflow/Network/NetworkFwd.h>
#include <string>
#include <vector>
#include <map>
#include <Core/Dataflow/Network/Share.h>

namespace SCIRun {
namespace Domain {
namespace Networks {

  class SCISHARE ModuleLookup
  {
  public:
    virtual ~ModuleLookup() {}
    virtual ModuleHandle lookupModule(const std::string& id) const = 0; 
  };

  class SCISHARE NetworkInterface : public ModuleLookup
  {
  public:
    typedef std::vector<ConnectionDescription> ConnectionDescriptionList;

    virtual ~NetworkInterface() {}
    virtual ModuleHandle add_module(const ModuleLookupInfo& info) = 0;
    virtual bool remove_module(const std::string& id) = 0;
    virtual size_t nmodules() const = 0;
    virtual ModuleHandle module(size_t i) const = 0;
    
    virtual ConnectionId connect(ModuleHandle, size_t, ModuleHandle, size_t) = 0;
    virtual bool disconnect(const ConnectionId&) = 0;
    virtual size_t nconnections() const = 0;
    virtual void disable_connection(const ConnectionId&) = 0;
    virtual ConnectionDescriptionList connections() const = 0;

    virtual std::string toString() const = 0;
  };
}}}


#endif
