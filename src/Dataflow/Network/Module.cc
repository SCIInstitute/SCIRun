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
  const std::string& version)
  : info_(info),
  id_(info_.module_name_, instanceCount_++),
  inputsChanged_(true),
  has_ui_(hasUi),
  state_(stateFactory ? stateFactory->make_state(info.module_name_) : new NullModuleState),
  executionState_(ModuleInterface::Waiting)
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

  //TODO: configure via command line flag as advanced option.
  setReexecutionStrategy(boost::make_shared<AlwaysReexecuteStrategy>());
  //setReexecutionStrategy(makeDynamicReexecutionStrategy(*this));
}

Module::~Module()
{
}

ModuleStateFactoryHandle Module::defaultStateFactory_;
AlgorithmFactoryHandle Module::defaultAlgoFactory_;

LoggerHandle Module::getLogger() const { return log_ ? log_ : defaultLogger_; }

OutputPortHandle Module::getOutputPort(const PortId& id) const
{
  return oports_[id];
}

InputPortHandle Module::getInputPort(const PortId& id) const
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
  executeBegins_(id_);
  /// @todo: status() calls should be logged everywhere, need to change legacy loggers. issue #nnn
  status("STARTING MODULE: " + id_.id_);
  /// @todo: need separate logger per module
  //LOG_DEBUG("STARTING MODULE: " << id_.id_);
  setExecutionState(ModuleInterface::Executing);
  bool returnCode = false;

  try
  {
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

  if (!inputsChanged_)
  {
    //LOG_DEBUG(id_ << " :: inputsChanged is false, querying port for value.");
    inputsChanged_ = port->hasChanged();
    //LOG_DEBUG(id_ << ":: inputsChanged is now " << inputsChanged_);
  }
  return port->getData();
}

std::vector<DatatypeHandleOption> Module::get_dynamic_input_handles(const PortId& id)
{
  /// @todo test...
  auto portsWithName = iports_[id.name];  //will throw if empty
  if (!portsWithName[0]->isDynamic())
  {
    BOOST_THROW_EXCEPTION(InvalidInputPortRequestException() << Core::ErrorMessage("Input port " + id.toString() + " is static, get_input_handle must be called."));
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

void Module::setStateBoolFromAlgo(AlgorithmParameterName name)
{
  get_state()->setValue(name, algo().get(name).getBool());
}

void Module::setAlgoIntFromState(AlgorithmParameterName name)
{
  algo().set(name, get_state()->getValue(name).getInt());
}

void Module::setAlgoBoolFromState(AlgorithmParameterName name)
{
  algo().set(name, get_state()->getValue(name).getBool());
}

void Module::setStateIntFromAlgo(AlgorithmParameterName name)
{
  get_state()->setValue(name, algo().get(name).getInt());
}

void Module::setStateDoubleFromAlgo(AlgorithmParameterName name)
{
  get_state()->setValue(name, algo().get(name).getDouble());
}

void Module::setAlgoDoubleFromState(AlgorithmParameterName name)
{
  algo().set(name, get_state()->getValue(name).getDouble());
}

void Module::setAlgoOptionFromState(AlgorithmParameterName name)
{
  algo().set_option(name, get_state()->getValue(name).getString());
}

void Module::setStateStringFromAlgoOption(AlgorithmParameterName name)
{
  get_state()->setValue(name, algo().get_option(name));
}

ModuleInterface::ExecutionState Module::executionState() const
{
  return executionState_;
}

void Module::setExecutionState(ModuleInterface::ExecutionState state)
{
  executionState_ = state;
}

bool Module::needToExecute() const
{
  if (reexecute_)
  {
    auto val = reexecute_->needToExecute();
    LOG_DEBUG(id_ << " Using real needToExecute strategy object, value is: " << val << std::endl);
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

InputsChangedCheckerImpl::InputsChangedCheckerImpl(Module& module) : module_(module)
{
}

bool InputsChangedCheckerImpl::inputsChanged() const 
{
  LOG_DEBUG(module_.get_id() << " InputsChangedCheckerImpl returns " << module_.inputsChanged());
  return module_.inputsChanged();
}

StateChangedCheckerImpl::StateChangedCheckerImpl(Module& module) : module_(module)
{
}

bool StateChangedCheckerImpl::newStatePresent() const 
{
  LOG_DEBUG(module_.get_id() << " StateChangedCheckerImpl returns " << module_.newStatePresent());
  return module_.newStatePresent();
}

OutputPortsCachedCheckerImpl::OutputPortsCachedCheckerImpl(Module& module) : module_(module)
{
}

bool OutputPortsCachedCheckerImpl::outputPortsCached() const 
{
  auto outputs = module_.outputPorts();
  auto ret = std::all_of(outputs.begin(), outputs.end(), [](OutputPortHandle out) { return out->hasData(); });
  LOG_DEBUG(module_.get_id() << " OutputPortsCachedCheckerImpl, returns " << ret);
  return ret;
}

ModuleReexecutionStrategyHandle SCIRun::Dataflow::Networks::makeDynamicReexecutionStrategy(Module& module)
{
  return boost::make_shared<DynamicReexecutionStrategy>(
    boost::make_shared<InputsChangedCheckerImpl>(module),
    boost::make_shared<StateChangedCheckerImpl>(module),
    boost::make_shared<OutputPortsCachedCheckerImpl>(module));
}