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


#ifndef DATAFLOW_NETWORK_ModuleWithAsyncDynamicPorts_H
#define DATAFLOW_NETWORK_ModuleWithAsyncDynamicPorts_H

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE ModuleWithAsyncDynamicPorts : public Module
  {
  public:
    explicit ModuleWithAsyncDynamicPorts(const ModuleLookupInfo& info, bool hasUI);
    virtual bool hasDynamicPorts() const override { return true; }
    virtual void execute() override;
    virtual void asyncExecute(const PortId& pid, Core::Datatypes::DatatypeHandle data) = 0;
    virtual void portRemovedSlot(const ModuleId& mid, const PortId& pid) override;
  protected:
    virtual void portRemovedSlotImpl(const PortId& pid) = 0;
    virtual size_t add_input_port(InputPortHandle h) override;
  };

}}}

#endif
