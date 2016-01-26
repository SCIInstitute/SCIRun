/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
/// @todo Documentation Dataflow/Network/ModuleInterface.h

#ifndef DATAFLOW_NETWORK_MODULE_INTERFACE_H
#define DATAFLOW_NETWORK_MODULE_INTERFACE_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Base/AlgorithmFactory.h>
#include <Dataflow/Network/ExecutableObject.h>
#include <Core/Logging/LoggerFwd.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  //TODO: refactor with this type
  template <class PortType>
  class SCISHARE PortView
  {
  public:
    virtual ~PortView() {}
    virtual bool hasPort(const PortId& id) const = 0;
    virtual boost::shared_ptr<PortType> getPort(const PortId& id) const = 0;
    virtual std::vector<boost::shared_ptr<PortType>> findPortsWithName(const std::string& name) const = 0;
    virtual size_t numPorts() const = 0;
    virtual std::vector<boost::shared_ptr<PortType>> ports() const = 0;
  };

  class SCISHARE ModuleInfoProvider
  {
  public:
    virtual ~ModuleInfoProvider() {}

    /// @todo: kind of ridiculous interface/duplication. Should pull out a subinterface for "PortView" and just return one of those for input/output
    virtual bool hasOutputPort(const PortId& id) const = 0;
    virtual OutputPortHandle getOutputPort(const PortId& id) const = 0;
    virtual std::vector<OutputPortHandle> findOutputPortsWithName(const std::string& name) const = 0;
    virtual size_t num_output_ports() const = 0;
    virtual std::vector<OutputPortHandle> outputPorts() const = 0;

    virtual bool hasInputPort(const PortId& id) const = 0;
    virtual InputPortHandle getInputPort(const PortId& id) = 0;
    virtual std::vector<InputPortHandle> findInputPortsWithName(const std::string& name) const = 0;
    virtual size_t num_input_ports() const = 0;
    virtual std::vector<InputPortHandle> inputPorts() const = 0;

    virtual std::string get_module_name() const = 0;
    virtual ModuleId get_id() const = 0;
    virtual bool has_ui() const = 0;
    virtual const ModuleLookupInfo& get_info() const = 0;
    virtual bool hasDynamicPorts() const = 0;
  };

  class SCISHARE ModuleDisplayInterface
  {
  public:
    virtual ~ModuleDisplayInterface() {}
    virtual void setUiVisible(bool visible) = 0;
  };

  SCISHARE std::string to_string(const ModuleInfoProvider&);

  typedef boost::function<void(bool)> UiToggleFunc;

  class SCISHARE ModuleReexecutionStrategy
  {
  public:
    virtual ~ModuleReexecutionStrategy() {}

    virtual bool needToExecute() const = 0;
  };

  typedef boost::shared_ptr<ModuleReexecutionStrategy> ModuleReexecutionStrategyHandle;

  class SCISHARE ModuleExecutionState
  {
  public:
    enum Value
    {
      NotExecuted,
      Waiting,
      Executing,
      Completed,
      Errored
    };
    virtual Value currentState() const = 0;

    typedef boost::signals2::signal<void(int)> ExecutionStateChangedSignalType;

    virtual boost::signals2::connection connectExecutionStateChanged(const ExecutionStateChangedSignalType::slot_type& subscriber) = 0;
    virtual bool transitionTo(Value state) = 0;
    virtual std::string currentColor() const = 0;
    virtual ~ModuleExecutionState() {}
  };

  typedef boost::shared_ptr<ModuleExecutionState> ModuleExecutionStateHandle;

  /// @todo: interface is getting bloated, segregate it.
  class SCISHARE ModuleInterface : public ModuleInfoProvider, public ModuleDisplayInterface,
    public ExecutableObject, public Core::Algorithms::AlgorithmCollaborator
  {
  public:
    virtual ~ModuleInterface();

    virtual ModuleStateHandle get_state() = 0;
    virtual const ModuleStateHandle get_state() const = 0;

    virtual bool do_execute() = 0;

    typedef boost::signals2::signal<void()> ExecutionSelfRequestSignalType;
    virtual boost::signals2::connection connectExecuteSelfRequest(const ExecutionSelfRequestSignalType::slot_type& subscriber) = 0;

    virtual ModuleExecutionState& executionState() = 0;

    /// @todo for deserialization
    virtual void set_id(const std::string& id) = 0;
    virtual void set_state(ModuleStateHandle state) = 0;

    virtual SCIRun::Core::Datatypes::DatatypeHandleOption get_input_handle(const PortId& id) = 0;
    virtual std::vector<SCIRun::Core::Datatypes::DatatypeHandleOption> get_dynamic_input_handles(const PortId& id) = 0;
    virtual void send_output_handle(const PortId& id, SCIRun::Core::Datatypes::DatatypeHandle data) = 0;

    virtual void setLogger(SCIRun::Core::Logging::LoggerHandle log) = 0;
    virtual SCIRun::Core::Logging::LoggerHandle getLogger() const override = 0;

    /// @todo functions
    virtual SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc getUpdaterFunc() const override = 0;
    virtual void setUpdaterFunc(SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc func) = 0;
    virtual void setUiToggleFunc(UiToggleFunc func) = 0;

    /// @todo:
    // need to hook up output ports for cached state.
    virtual bool needToExecute() const = 0;

    virtual ModuleReexecutionStrategyHandle getReexecutionStrategy() const = 0;
    virtual void setReexecutionStrategy(ModuleReexecutionStrategyHandle caching) = 0;

    virtual void setStateDefaults() = 0;

    virtual Core::Algorithms::AlgorithmHandle getAlgorithm() const = 0;

    virtual void portAddedSlot(const Networks::ModuleId& mid, const Networks::PortId& pid) {}
    virtual void portRemovedSlot(const Networks::ModuleId& mid, const Networks::PortId& pid) {}
    virtual void addPortConnection(const boost::signals2::connection& con) = 0;

    virtual void enqueueExecuteAgain() = 0;

    virtual const MetadataMap& metadata() const = 0;

    virtual bool isStoppable() const = 0;
  };

  struct SCISHARE DataPortException : virtual Core::ExceptionBase {};
  struct SCISHARE NoHandleOnPortException : virtual DataPortException {};
  struct SCISHARE NullHandleOnPortException : virtual DataPortException {};
  struct SCISHARE WrongDatatypeOnPortException : virtual DataPortException {};
  struct SCISHARE PortNotFoundException : virtual DataPortException {};
  struct SCISHARE InvalidInputPortRequestException : virtual DataPortException {};

  #define MODULE_ERROR_WITH_TYPE(type, message) { error(message); BOOST_THROW_EXCEPTION(type() << SCIRun::Core::ErrorMessage(message)); }

  class SCISHARE ReexecuteStrategyFactory
  {
  public:
    virtual ~ReexecuteStrategyFactory() {}
    virtual ModuleReexecutionStrategyHandle create(const class Module& module) const = 0;
  };

  class SCISHARE ModuleIdGenerator
  {
  public:
    virtual ~ModuleIdGenerator() {}
    virtual int makeId(const std::string& name) = 0;
    virtual bool takeId(const std::string& name, int id) = 0;
    virtual void reset() = 0; //for unit testing
  };
  typedef boost::shared_ptr<ModuleIdGenerator> ModuleIdGeneratorHandle;

}}}

#endif
