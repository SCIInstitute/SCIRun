/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <iostream>
#include <memory>
#include <numeric>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include <Dataflow/Network/PortManager.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/Network/DataflowInterfaces.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/NullModuleState.h>
#include <Core/Logging/ConsoleLogger.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Engine::State;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

std::string SCIRun::Dataflow::Networks::to_string(const ModuleInfoProvider& m)
{
  return m.get_module_name() + " [" + m.get_id().id_ + "]";
}

/*static*/ int Module::instanceCount_ = 0;
/*static*/ LoggerHandle Module::defaultLogger_(new ConsoleLogger);

/*static*/ void Module::resetInstanceCount() { instanceCount_ = 0; }

Module::Module(const ModuleLookupInfo& info,
  bool hasUi,
  AlgorithmFactoryHandle algoFactory,
  ModuleStateFactoryHandle stateFactory,
  ReexecuteStrategyFactoryHandle reexFactory,
  const std::string& version)
  : info_(info),
  id_(info_.module_name_, instanceCount_++),
  inputsChanged_(false),
  has_ui_(hasUi),
  state_(stateFactory ? stateFactory->make_state(info.module_name_) : new NullModuleState),
  executionState_(ModuleInterface::NotExecuted)
{
  iports_.set_module(this);
  oports_.set_module(this);
  setLogger(defaultLogger_);

  Log& log = Log::get();

  log << DEBUG_LOG << "Module created: " << info.module_name_ << " with id: " << id_;

  if (algoFactory)
  {
    algo_ = algoFactory->create(get_module_name(), this);
    if (algo_)
      log << DEBUG_LOG << "Module algorithm initialized: " << info.module_name_;
  }
  log.flush();

  initStateObserver(state_.get());

  if (reexFactory)
    setReexecutionStrategy(reexFactory->create(*this));
}

Module::~Module()
{
}

ModuleStateFactoryHandle Module::defaultStateFactory_;
AlgorithmFactoryHandle Module::defaultAlgoFactory_;
ReexecuteStrategyFactoryHandle Module::defaultReexFactory_;

LoggerHandle Module::getLogger() const { return log_ ? log_ : defaultLogger_; }

OutputPortHandle Module::getOutputPort(const PortId& id) const
{
  return oports_[id];
}

InputPortHandle Module::getInputPort(const PortId& id)
{
  return iports_[id];
}

size_t Module::num_input_ports() const
{
  return iports_.size();
}

size_t Module::num_output_ports() const
{
  return oports_.size();
}

bool Module::do_execute() throw()
{
  //Log::get() << INFO << "executing module: " << id_ << std::endl;
  executeBegins_(id_);
  /// @todo: status() calls should be logged everywhere, need to change legacy loggers. issue #nnn
  status("STARTING MODULE: " + id_.id_);
  /// @todo: need separate logger per module
  //LOG_DEBUG("STARTING MODULE: " << id_.id_);
  setExecutionState(ModuleInterface::Executing);
  bool returnCode = false;

  try
  {
    //TODO: could we call needToExecute() here?
    execute();
    returnCode = true;
  }
  catch(const std::bad_alloc&)
  {
    error("MODULE ERROR: bad_alloc caught");
  }
  catch (Core::ExceptionBase& e)
  {
    /// @todo: this block is repetitive (logging-wise) if the macros are used to log AND throw an exception with the same message. Figure out a reasonable condition to enable it.
    if (Core::Logging::Log::get().verbose())
    {
      std::ostringstream ostr;
      ostr << "Caught exception: " << e.typeName() << std::endl << "Message: " << e.what() << std::endl;
      error(ostr.str());
    }

    if (Core::Logging::Log::get().verbose())
    {
      std::ostringstream ostrExtra;
      ostrExtra << boost::diagnostic_information(e) << std::endl;
      error(ostrExtra.str());
    }
  }
  catch (const std::exception& e)
  {
    error(std::string("MODULE ERROR: std::exception caught: ") + e.what());
  }
  catch (...)
  {
    error("MODULE ERROR: unhandled exception caught");
  }

  // Call finish on all ports.
  //iports_.apply(boost::bind(&PortInterface::finish, _1));
  //oports_.apply(boost::bind(&PortInterface::finish, _1));

  status("MODULE FINISHED: " + id_.id_);
  /// @todo: need separate logger per module
  //LOG_DEBUG("MODULE FINISHED: " << id_.id_);
  setExecutionState(ModuleInterface::Completed);
  resetStateChanged();
  //std::cout << id_ << " inputsChanged set to false post-execute" << std::endl;
  inputsChanged_ = false;
  executeEnds_(id_);
  return returnCode;
}

ModuleStateHandle Module::get_state()
{
  return state_;
}

const ModuleStateHandle Module::get_state() const
{
  return state_;
}

void Module::set_state(ModuleStateHandle state)
{
  state_ = state;
  initStateObserver(state_.get());
}

AlgorithmBase& Module::algo()
{
  if (!algo_)
    error("Null algorithm object, make sure AlgorithmFactory knows about this module's algorithm types.");
  ENSURE_NOT_NULL(algo_, "Null algorithm!");

  return *algo_;
}

size_t Module::add_input_port(InputPortHandle h)
{
  return iports_.add(h);
}

size_t Module::add_output_port(OutputPortHandle h)
{
  return oports_.add(h);
}

bool Module::hasInputPort(const PortId& id) const
{
  return iports_.hasPort(id);
}

bool Module::hasOutputPort(const PortId& id) const
{
  return oports_.hasPort(id);
}

DatatypeHandleOption Module::get_input_handle(const PortId& id)
{
  /// @todo test...
  if (!iports_.hasPort(id))
  {
    BOOST_THROW_EXCEPTION(PortNotFoundException() << Core::ErrorMessage("Input port not found: " + id.toString()));
  }

  auto port = iports_[id];
  if (port->isDynamic())
  {
    BOOST_THROW_EXCEPTION(InvalidInputPortRequestException() << Core::ErrorMessage("Input port " + id.toString() + " is dynamic, get_dynamic_input_handles must be called."));
  }

  {
    //Log::get() << DEBUG_LOG << id_ << " :: inputsChanged is " << inputsChanged_ << ", querying port for value." << std::endl;
    // NOTE: don't use short-circuited boolean OR here, we need to call hasChanged each time since it updates the port's cache flag.
    inputsChanged_ = port->hasChanged() || inputsChanged_;
    //Log::get() << DEBUG_LOG << id_ << ":: inputsChanged is now " << inputsChanged_ << std::endl;
  }

  auto data = port->getData();

  return data;
}

std::vector<DatatypeHandleOption> Module::get_dynamic_input_handles(const PortId& id)
{
  /// @todo test...
  auto portsWithName = iports_[id.name];  //will throw if empty
  if (!portsWithName[0]->isDynamic())
  {
    BOOST_THROW_EXCEPTION(InvalidInputPortRequestException() << Core::ErrorMessage("Input port " + id.toString() + " is static, get_input_handle must be called."));
  }

  {
    LOG_DEBUG(id_ << " :: inputsChanged is " << inputsChanged_ << ", querying port for value.");
    // NOTE: don't use short-circuited boolean OR here, we need to call hasChanged each time since it updates the port's cache flag.
    bool startingVal = inputsChanged_;
    inputsChanged_ = std::accumulate(portsWithName.begin(), portsWithName.end(), startingVal, [](bool acc, InputPortHandle input) { return input->hasChanged() || acc; });
    LOG_DEBUG(id_ << ":: inputsChanged is now " << inputsChanged_);
  }

  std::vector<DatatypeHandleOption> options;



  auto getData = [](InputPortHandle input) { return input->getData(); };
  std::transform(portsWithName.begin(), portsWithName.end(), std::back_inserter(options), getData);

  return options;
}

void Module::send_output_handle(const PortId& id, DatatypeHandle data)
{
  /// @todo test...
  if (!oports_.hasPort(id))
  {
    THROW_OUT_OF_RANGE("Output port does not exist: " + id.toString());
  }

  oports_[id]->sendData(data);
}

std::vector<InputPortHandle> Module::findInputPortsWithName(const std::string& name) const
{
  return iports_[name];
}

std::vector<OutputPortHandle> Module::findOutputPortsWithName(const std::string& name) const
{
  return oports_[name];
}

std::vector<InputPortHandle> Module::inputPorts() const
{
  return iports_.view();
}

std::vector<OutputPortHandle> Module::outputPorts() const
{
  return oports_.view();
}

Module::Builder::Builder()
{
}

Module::Builder::SinkMaker Module::Builder::sink_maker_;
Module::Builder::SourceMaker Module::Builder::source_maker_;

/*static*/ void Module::Builder::use_sink_type(Module::Builder::SinkMaker func) { sink_maker_ = func; }
/*static*/ void Module::Builder::use_source_type(Module::Builder::SourceMaker func) { source_maker_ = func; }

class DummyModule : public Module
{
public:
  explicit DummyModule(const ModuleLookupInfo& info) : Module(info) {}
  virtual void execute()
  {
    std::ostringstream ostr;
    ostr << "Module " << get_module_name() << " executing for " << 3.14 << " seconds." << std::endl;
    status(ostr.str());
  }
  virtual void setStateDefaults() {}
};

Module::Builder& Module::Builder::with_name(const std::string& name)
{
  if (!module_)
  {
    ModuleLookupInfo info;
    info.module_name_ = name;
    module_.reset(new DummyModule(info));
  }
  return *this;
}

Module::Builder& Module::Builder::using_func(ModuleMaker create)
{
  if (!module_)
    module_.reset(create());
  return *this;
}

Module::Builder& Module::Builder::setStateDefaults()
{
  if (module_)
    module_->setStateDefaults();
  return *this;
}

Module::Builder& Module::Builder::add_input_port(const Port::ConstructionParams& params)
{
  if (module_)
  {
    addInputPortImpl(*module_, params);
  }
  return *this;
}

void Module::Builder::addInputPortImpl(Module& module, const Port::ConstructionParams& params)
{
  DatatypeSinkInterfaceHandle sink(sink_maker_ ? sink_maker_() : 0);
  InputPortHandle port(boost::make_shared<InputPort>(module_.get(), params, sink));
  port->setIndex(module_->add_input_port(port));
}

Module::Builder& Module::Builder::add_output_port(const Port::ConstructionParams& params)
{
  if (module_)
  {
    DatatypeSourceInterfaceHandle source(source_maker_ ? source_maker_() : 0);
    OutputPortHandle port(boost::make_shared<OutputPort>(module_.get(), params, source));
    port->setIndex(module_->add_output_port(port));
  }
  return *this;
}

PortId Module::Builder::cloneInputPort(ModuleHandle module, const PortId& id)
{
  Module* m = dynamic_cast<Module*>(module.get());
  if (m)
  {
    InputPortHandle newPort(m->getInputPort(id)->clone());
    newPort->setIndex(m->add_input_port(newPort));
    return newPort->id();
  }
  THROW_INVALID_ARGUMENT("Don't know how to clone ports on other Module types");
}

void Module::Builder::removeInputPort(ModuleHandle module, const PortId& id)
{
  Module* m = dynamic_cast<Module*>(module.get());
  if (m)
  {
    m->removeInputPort(id);
  }
}

ModuleHandle Module::Builder::build()
{
  return module_;
}

boost::signals2::connection Module::connectExecuteBegins(const ExecuteBeginsSignalType::slot_type& subscriber)
{
  return executeBegins_.connect(subscriber);
}

boost::signals2::connection Module::connectExecuteEnds(const ExecuteEndsSignalType::slot_type& subscriber)
{
  return executeEnds_.connect(subscriber);
}

boost::signals2::connection Module::connectErrorListener(const ErrorSignalType::slot_type& subscriber)
{
  return errorSignal_.connect(subscriber);
}

boost::signals2::connection Module::connectExecutionStateChanged(const ExecutionStateChangedSignalType::slot_type& subscriber)
{
  return executionStateChanged_.connect(subscriber);
}

void Module::setUiVisible(bool visible)
{
  if (uiToggleFunc_)
    uiToggleFunc_(visible);
}

void Module::setLogger(SCIRun::Core::Logging::LoggerHandle log)
{
  log_ = log;
  if (algo_)
    algo_->setLogger(log);
}

void Module::setUpdaterFunc(SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc func)
{
  updaterFunc_ = func;
  if (algo_)
    algo_->setUpdaterFunc(func);
}

bool Module::oport_connected(const PortId& id) const
{
  if (!oports_.hasPort(id))
    return false;

  auto port = oports_[id];
  return port->nconnections() > 0;
}

void Module::removeInputPort(const PortId& id)
{
  iports_.remove(id);
}

void Module::setStateBoolFromAlgo(const AlgorithmParameterName& name)
{
  get_state()->setValue(name, algo().get(name).toBool());
}

void Module::setAlgoIntFromState(const AlgorithmParameterName& name)
{
  algo().set(name, get_state()->getValue(name).toInt());
}

void Module::setAlgoBoolFromState(const AlgorithmParameterName& name)
{
  algo().set(name, get_state()->getValue(name).toBool());
}

void Module::setStateIntFromAlgo(const AlgorithmParameterName& name)
{
  get_state()->setValue(name, algo().get(name).toInt());
}

void Module::setStateDoubleFromAlgo(const AlgorithmParameterName& name)
{
  get_state()->setValue(name, algo().get(name).toDouble());
}

void Module::setStateListFromAlgo(const AlgorithmParameterName& name)
{
  get_state()->setValue(name, algo().get(name).toVector());
}

void Module::setAlgoDoubleFromState(const AlgorithmParameterName& name)
{
  algo().set(name, get_state()->getValue(name).toDouble());
}

void Module::setAlgoOptionFromState(const AlgorithmParameterName& name)
{
	algo().set_option(name, get_state()->getValue(name).toString());
}

void Module::setStateStringFromAlgoOption(const AlgorithmParameterName& name)
{
  get_state()->setValue(name, algo().get_option(name));
}

void Module::setAlgoListFromState(const AlgorithmParameterName& name)
{
  algo().set(name, get_state()->getValue(name).toVector());
}

ModuleInterface::ExecutionState Module::executionState() const
{
  return executionState_;
}

void Module::setExecutionState(ModuleInterface::ExecutionState state)
{
  //std::cout << get_id() << " setExecutionState old " << executionState_ << " new " << state << std::endl;
  if (state != executionState_)
    executionStateChanged_(state);
  executionState_ = state;
}

bool Module::needToExecute() const
{
  if (reexecute_)
  {
    auto val = reexecute_->needToExecute();
    //Log::get() << DEBUG_LOG << id_ << " Using real needToExecute strategy object, value is: " << val << std::endl;
    return val;
  }

  return true;
}

ModuleReexecutionStrategyHandle Module::getReexecutionStrategy() const
{
  return reexecute_;
}

void Module::setReexecutionStrategy(ModuleReexecutionStrategyHandle caching)
{
  reexecute_ = caching;
}

bool Module::inputsChanged() const
{
  return inputsChanged_;
}

void Module::addPortConnection(const boost::signals2::connection& con)
{
  portConnections_.emplace_back(new boost::signals2::scoped_connection(con));
}

ModuleWithAsyncDynamicPorts::ModuleWithAsyncDynamicPorts(const ModuleLookupInfo& info) : Module(info)
{
}

void ModuleWithAsyncDynamicPorts::execute()
{
}

size_t ModuleWithAsyncDynamicPorts::add_input_port(InputPortHandle h)
{
  h->connectDataOnPortHasChanged(boost::bind(&ModuleWithAsyncDynamicPorts::asyncExecute, this, _1, _2));
  return Module::add_input_port(h);
}

void ModuleWithAsyncDynamicPorts::portRemovedSlot(const ModuleId& mid, const PortId& pid)
{
  //TODO: redesign with non-virtual slot method and virtual hook that ensures module id is the same as this
  if (mid == id_)
  {
    portRemovedSlotImpl(pid);
  }
}

DynamicReexecutionStrategy::DynamicReexecutionStrategy(
  InputsChangedCheckerHandle inputsChanged,
  StateChangedCheckerHandle stateChanged,
  OutputPortsCachedCheckerHandle outputsCached) : inputsChanged_(inputsChanged), stateChanged_(stateChanged), outputsCached_(outputsCached)
{
  ENSURE_NOT_NULL(inputsChanged_, "InputsChangedChecker");
  ENSURE_NOT_NULL(stateChanged_, "StateChangedChecker");
  ENSURE_NOT_NULL(outputsCached_, "OutputPortsCachedChecker");
}

bool DynamicReexecutionStrategy::needToExecute() const
{
  return inputsChanged_->inputsChanged() || stateChanged_->newStatePresent() || !outputsCached_->outputPortsCached();
}

InputsChangedCheckerImpl::InputsChangedCheckerImpl(const Module& module) : module_(module)
{
}

bool InputsChangedCheckerImpl::inputsChanged() const
{
  auto ret = module_.inputsChanged();
  //Log::get() << DEBUG_LOG << module_.get_id() << " InputsChangedCheckerImpl returns " << ret << std::endl;
  return ret;
}

StateChangedCheckerImpl::StateChangedCheckerImpl(const Module& module) : module_(module)
{
}

bool StateChangedCheckerImpl::newStatePresent() const
{
  auto ret = module_.newStatePresent();
  //Log::get() << DEBUG_LOG << module_.get_id() << " StateChangedCheckerImpl returns " << ret << std::endl;
  return ret;
}

OutputPortsCachedCheckerImpl::OutputPortsCachedCheckerImpl(const Module& module) : module_(module)
{
}

bool OutputPortsCachedCheckerImpl::outputPortsCached() const
{
  return true;
  //TODO: need a way to filter optional input ports
  /*
  auto outputs = module_.outputPorts();
  auto ret = std::all_of(outputs.begin(), outputs.end(), [](OutputPortHandle out) { return out-> out->hasData(); });
  Log::get() << DEBUG_LOG << module_.get_id() << " OutputPortsCachedCheckerImpl, returns " << ret << std::endl;
  return ret;
  */
}

DynamicReexecutionStrategyFactory::DynamicReexecutionStrategyFactory(const boost::optional<std::string>& reexMode)
  : reexecuteMode_(reexMode)
{
}

ModuleReexecutionStrategyHandle DynamicReexecutionStrategyFactory::create(const Module& module) const
{
  if (reexecuteMode_ && ((*reexecuteMode_) == "always"))
  {
    LOG_DEBUG("Using Always reexecute mode for module execution.");
    return boost::make_shared<AlwaysReexecuteStrategy>();
  }

  return boost::make_shared<DynamicReexecutionStrategy>(
    boost::make_shared<InputsChangedCheckerImpl>(module),
    boost::make_shared<StateChangedCheckerImpl>(module),
    boost::make_shared<OutputPortsCachedCheckerImpl>(module));
}

bool SCIRun::Dataflow::Networks::canReplaceWith(ModuleHandle module, const ModuleDescription& potentialReplacement)
{
  if (module->get_module_name() == potentialReplacement.lookupInfo_.module_name_)
    return false;

  {
    auto inputs = module->inputPorts();
    for (size_t i = 0; i < inputs.size(); ++i)
    {
      auto toMatch = inputs[i];
      if (toMatch->nconnections() > 0)
      {
        if (i >= potentialReplacement.input_ports_.size())
          return false;

        const InputPortDescription& input = potentialReplacement.input_ports_[i];
        if (input.datatype != toMatch->get_typename())
          return false;
      }
    }
  }
  {
    auto outputs = module->outputPorts();
    for (size_t i = 0; i < outputs.size(); ++i)
    {
      auto toMatch = outputs[i];
      if (toMatch->nconnections() > 0)
      {
        if (i >= potentialReplacement.output_ports_.size())
          return false;

        const OutputPortDescription& output = potentialReplacement.output_ports_[i];
        if (output.datatype != toMatch->get_typename())
          return false;
      }
    }
  }
  //LOG_DEBUG("\tFound replacement: " << potentialReplacement.lookupInfo_.module_name_ << std::endl);
  return true;
}

void Module::enqueueExecuteAgain()
{
  std::cout << "TODO: Module needs to execute again" << std::endl;
}
