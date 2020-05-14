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


#ifndef DATAFLOW_NETWORK_MODULE_INTERFACE_H
#define DATAFLOW_NETWORK_MODULE_INTERFACE_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Base/AlgorithmFactory.h>
#include <Dataflow/Network/ExecutableObject.h>
#include <Dataflow/Network/ModuleInfoProvider.h>
#include <Dataflow/Network/ModuleExceptions.h>
#include <Dataflow/Network/ModuleExecutionInterfaces.h>
#include <Dataflow/Network/ModuleIdGenerator.h>
#include <Dataflow/Network/ModuleDisplayInterface.h>
#include <Core/Logging/LoggerFwd.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  // Methods a module writer needs to know/use/override
  class SCISHARE ModuleUserInterface
  {
  public:
    virtual ~ModuleUserInterface() {}

    // These two functions must be implemented:
    virtual void execute() = 0;
    virtual void setStateDefaults() = 0;

    // These two functions must be understood and used correctly:
    virtual ModuleStateHandle get_state() = 0;
    virtual bool needToExecute() const = 0;
  };

  // Methods for internal developer use/testing
  class SCISHARE ModuleInternalsInterface
  {
  public:
    virtual ~ModuleInternalsInterface() {}
    virtual const ModuleStateHandle cstate() const = 0;
    typedef boost::signals2::signal<void(bool)> ExecutionSelfRequestSignalType;
    virtual boost::signals2::connection connectExecuteSelfRequest(const ExecutionSelfRequestSignalType::slot_type& subscriber) = 0;
    virtual ModuleExecutionState& executionState() = 0;
    /// @todo for deserialization
    virtual void setId(const std::string& id) = 0;
    virtual void setState(ModuleStateHandle state) = 0;
    virtual SCIRun::Core::Datatypes::DatatypeHandleOption get_input_handle(const PortId& id) = 0;
    virtual std::vector<SCIRun::Core::Datatypes::DatatypeHandleOption> get_dynamic_input_handles(const PortId& id) = 0;
    virtual void send_output_handle(const PortId& id, SCIRun::Core::Datatypes::DatatypeHandle data) = 0;
    virtual void setLogger(SCIRun::Core::Logging::LoggerHandle log) = 0;
    virtual void setUpdaterFunc(SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc func) = 0;
    virtual void setUiToggleFunc(UiToggleFunc func) = 0;
    virtual ModuleReexecutionStrategyHandle getReexecutionStrategy() const = 0;
    virtual void setReexecutionStrategy(ModuleReexecutionStrategyHandle caching) = 0;
    virtual Core::Algorithms::AlgorithmHandle getAlgorithm() const = 0;
    virtual void portAddedSlot(const Networks::ModuleId& mid, const Networks::PortId& pid) {}
    virtual void portRemovedSlot(const Networks::ModuleId& mid, const Networks::PortId& pid) {}
    virtual void addPortConnection(const boost::signals2::connection& con) = 0;
    virtual void enqueueExecuteAgain(bool upstream) = 0;
    virtual const MetadataMap& metadata() const = 0;
    virtual bool isStoppable() const = 0;
    virtual bool executionDisabled() const = 0;
    virtual void setExecutionDisabled(bool disable) = 0;
    virtual bool isImplementationDisabled() const = 0;
    virtual void setProgrammableInputPortEnabled(bool enable) = 0;
    virtual bool checkForVirtualConnection(const ModuleInterface& downstream) const = 0;
  };

  class SCISHARE ModuleInterface :
    public ModuleUserInterface,
    public ModuleInternalsInterface,
    public ModuleInfoProvider,
    public ModuleDisplayInterface,
    public ExecutableObject,
    public Core::Algorithms::AlgorithmCollaborator
  {
  public:
    virtual ~ModuleInterface();
  };

}}}

#endif
