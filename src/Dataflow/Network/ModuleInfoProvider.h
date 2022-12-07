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


#ifndef DATAFLOW_NETWORK_MODULEINFOPROVIDER_H
#define DATAFLOW_NETWORK_MODULEINFOPROVIDER_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Datatypes/Datatype.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE ModuleInfoProvider
  {
  public:
    virtual ~ModuleInfoProvider() {}

    /// @todo: kind of ridiculous interface/duplication. Should pull out a subinterface for "PortView" and just return one of those for input/output
    virtual bool hasOutputPort(const PortId& id) const = 0;
    virtual OutputPortHandle getOutputPort(const PortId& id) const = 0;
    virtual std::vector<OutputPortHandle> findOutputPortsWithName(const std::string& name) const = 0;
    virtual size_t numOutputPorts() const = 0;
    virtual std::vector<OutputPortHandle> outputPorts() const = 0;

    virtual bool hasInputPort(const PortId& id) const = 0;
    virtual InputPortHandle getInputPort(const PortId& id) = 0;
    virtual std::vector<InputPortHandle> findInputPortsWithName(const std::string& name) const = 0;
    virtual size_t numInputPorts() const = 0;
    virtual std::vector<InputPortHandle> inputPorts() const = 0;

    virtual std::string name() const = 0;
    virtual ModuleId id() const = 0;
    virtual bool hasUI() const = 0;
    virtual const ModuleLookupInfo& info() const = 0;
    virtual bool hasDynamicPorts() const = 0;

    virtual std::string helpPageUrl() const = 0;
    virtual std::string newHelpPageUrl() const = 0;
    virtual std::string legacyPackageName() const = 0;
    virtual std::string legacyModuleName() const = 0;
    virtual bool isDeprecated() const = 0;
    virtual std::string replacementModuleName() const = 0;
    virtual std::string description() const = 0;
  };

  SCISHARE std::string to_string(const ModuleInfoProvider&);

}}}

#endif
