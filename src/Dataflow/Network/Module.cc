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

#include <memory>
#include <numeric>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer.hpp>
#include <atomic>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Dataflow/Network/PortManager.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/NullModuleState.h>
#include <Dataflow/Network/ModuleReexecutionStrategies.h>
#include <Dataflow/Network/ModuleWithAsyncDynamicPorts.h>
#include <Dataflow/Network/GeometryGeneratingModule.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Dataflow/Network/DataflowInterfaces.h>
#include <Dataflow/Network/ModuleBuilder.h>
#include <Core/Logging/ConsoleLogger.h>
#include <Core/Logging/Log.h>
#include <Core/Thread/Mutex.h>
#include <Core/Thread/Interruptible.h>

//TODO remove once method is extracted below
#include <Dataflow/Network/Connection.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Engine::State;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;

std::string SCIRun::Dataflow::Networks::to_string(const ModuleInfoProvider& m)
{
  return m.get_module_name() + " [" + m.get_id().id_ + "]";
}

namespace detail
{
  class PerTypeInstanceCountIdGenerator : public ModuleIdGenerator
  {
  public:
    PerTypeInstanceCountIdGenerator() : mapLock_("moduleCounts") {}
    virtual int makeId(const std::string& name) override final
    {
      Guard g(mapLock_.get());
      return instanceCounts_[name]++;
    }
    virtual bool takeId(const std::string& name, int id) override final
    {
      Guard g(mapLock_.get());
      int next = instanceCounts_[name];
      instanceCounts_[name] = std::max(next, id + 1);
      return true;
    }
    virtual void reset() override final
    {
      Guard g(mapLock_.get());
      instanceCounts_.clear();
    }
  private:
    Mutex mapLock_;
    std::map<std::string, int> instanceCounts_;
  };

  // basic int version to start. next hookup state machine
  class ModuleExecutionStateImpl : public ModuleExecutionState
  {
  public:
    virtual Value currentState() const override
    {
      return current_;
    }
    virtual boost::signals2::connection connectExecutionStateChanged(const ExecutionStateChangedSignalType::slot_type& subscriber) override
    {
      return signal_.connect(subscriber);
    }
    virtual bool transitionTo(Value state) override
    {
      if (current_ != state)
      {
        //std::cout << "Transitioning to " << state << std::endl;
        signal_(static_cast<int>(state));
      }
      current_ = state;
      setExpandedState(state);
      return true;
    }
    virtual std::string currentColor() const override
    {
      return "not implemented";
    }
    virtual Value expandedState() const override
    {
      return expandedState_.value_or(currentState());
    }

    virtual void setExpandedState(Value state) override
    {
      expandedState_ = state;
    }

  private:
    Value current_;
    ExecutionStateChangedSignalType signal_;
    boost::optional<Value> expandedState_;
  };
}

namespace SCIRun
{
  namespace Dataflow
  {
    namespace Networks
    {
      class ModuleImpl
      {
      private:
        Module* module_;
      public:
        ModuleImpl(Module* module,
          const ModuleLookupInfo& info,
          bool hasUi,
          ModuleStateFactoryHandle stateFactory)
          : module_(module),
          info_(info),
          id_(info.module_name_, DefaultModuleFactories::idGenerator_->makeId(info.module_name_)),
          has_ui_(hasUi),
          state_(stateFactory ? stateFactory->make_state(info_.module_name_) : new NullModuleState),
          metadata_(state_),
          executionState_(boost::make_shared<detail::ModuleExecutionStateImpl>())
        {
          iports_.set_module(module_);
          oports_.set_module(module_);
        }

        boost::atomic<bool> inputsChanged_ { false };

        const ModuleLookupInfo info_;
        ModuleId id_;
        bool has_ui_;
        AlgorithmHandle algo_;

        ModuleStateHandle state_;
        MetadataMap metadata_;
        PortManager<OutputPortHandle> oports_;
        PortManager<InputPortHandle> iports_;

        ExecuteBeginsSignalType executeBegins_;
        ExecuteEndsSignalType executeEnds_;
        ErrorSignalType errorSignal_;
        std::vector<boost::shared_ptr<boost::signals2::scoped_connection>> portConnections_;
        ModuleInterface::ExecutionSelfRequestSignalType executionSelfRequested_;

        ModuleReexecutionStrategyHandle reexecute_;
        std::atomic<bool> threadStopped_ { false };

        ModuleExecutionStateHandle executionState_;
        std::atomic<bool> executionDisabled_ { false };

        LoggerHandle log_;
        AlgorithmStatusReporter::UpdaterFunc updaterFunc_;
        UiToggleFunc uiToggleFunc_;
      };
    }
  }
}

/*static*/ LoggerHandle DefaultModuleFactories::defaultLogger_(new ConsoleLogger);
/*static*/ ModuleIdGeneratorHandle DefaultModuleFactories::idGenerator_(new detail::PerTypeInstanceCountIdGenerator);

/*static*/ void Module::resetIdGenerator() { DefaultModuleFactories::idGenerator_->reset(); }

const int Module::TraitFlags = SCIRun::Modules::UNDEFINED_MODULE_FLAG;

Module::Module(const ModuleLookupInfo& info,
  bool hasUi,
  AlgorithmFactoryHandle algoFactory,
  ModuleStateFactoryHandle stateFactory,
  ReexecuteStrategyFactoryHandle reexFactory)
{
  impl_ = boost::make_shared<ModuleImpl>(this, info, hasUi, stateFactory);

  setLogger(DefaultModuleFactories::defaultLogger_);
  setUpdaterFunc([](double x) {});

  LOG_DEBUG("Module created: {} with id: {}", info.module_name_, impl_->id_.id_);

  if (algoFactory)
  {
    impl_->algo_ = algoFactory->create(get_module_name(), this);
    if (impl_->algo_)
      LOG_DEBUG("Module algorithm initialized: {}", info.module_name_);
  }

  initStateObserver(impl_->state_.get());

  if (reexFactory)
    setReexecutionStrategy(reexFactory->create(*this));

  impl_->executionState_->transitionTo(ModuleExecutionState::NotExecuted);
}

void Module::set_id(const std::string& id)
{
  ModuleId newId(id);
  if (!DefaultModuleFactories::idGenerator_->takeId(newId.name_, newId.idNumber_))
    THROW_INVALID_ARGUMENT("Duplicate module IDs, invalid network file.");
  impl_->id_ = newId;
}

Module::~Module()
{
}

ModuleStateFactoryHandle DefaultModuleFactories::defaultStateFactory_;
AlgorithmFactoryHandle DefaultModuleFactories::defaultAlgoFactory_;
ReexecuteStrategyFactoryHandle DefaultModuleFactories::defaultReexFactory_;

bool Module::has_ui() const
{
  return impl_->has_ui_;
}

const ModuleLookupInfo& Module::get_info() const
{
  return impl_->info_;
}

std::string Module::get_module_name() const
{
  return get_info().module_name_;
}

std::string Module::get_categoryname() const
{
  return get_info().category_name_;
}

std::string Module::get_packagename() const
{
  return get_info().package_name_;
}

ModuleId Module::get_id() const
{
  return impl_->id_;
}

bool Module::executionDisabled() const
{
  return impl_->executionDisabled_;
}

void Module::setExecutionDisabled(bool disable)
{
  impl_->executionDisabled_ = disable;
}

void Module::error(const std::string& msg) const
{
  impl_->errorSignal_(get_id());
  getLogger()->error(msg);
}

AlgorithmStatusReporter::UpdaterFunc Module::getUpdaterFunc() const
{
  return impl_->updaterFunc_;
}

void Module::setUiToggleFunc(UiToggleFunc func)
{
  impl_->uiToggleFunc_ = func;
}

AlgorithmHandle Module::getAlgorithm() const
{
  return impl_->algo_;
}

LoggerHandle Module::getLogger() const
{
  return impl_->log_ ? impl_->log_ : DefaultModuleFactories::defaultLogger_;
}

OutputPortHandle Module::getOutputPort(const PortId& id) const
{
  return impl_->oports_[id];
}

InputPortHandle Module::getInputPort(const PortId& id)
{
  return impl_->iports_[id];
}

size_t Module::num_input_ports() const
{
  return impl_->iports_.size();
}

size_t Module::num_output_ports() const
{
  return impl_->oports_.size();
}

//TODO requirements for state metadata reporting
std::string Module::stateMetaInfo() const
{
  if (!cstate())
    return "Null state map.";
  auto keys = cstate()->getKeys();
  size_t i = 0;
  std::ostringstream ostr;
  ostr << "\n\t{";
  for (const auto& key : keys)
  {
    ostr << "[" << key.name() << ", " << cstate()->getValue(key).value() << "]";
    i++;
    if (i < keys.size())
      ostr << ",\n\t";
  }
  ostr << "}";
  return ostr.str();
}

void Module::copyStateToMetadata()
{
  impl_->metadata_.setMetadata("Module state", stateMetaInfo());
}

bool Module::executeWithSignals() NOEXCEPT
{
  auto starting = "STARTING MODULE: " + get_id().id_;
#ifdef BUILD_HEADLESS //TODO: better headless logging
  static Mutex executeLogLock("headlessExecution");
  if (!LogSettings::Instance().verbose())
  {
    Guard g(executeLogLock.get());
    std::cout << starting << std::endl;
  }
#endif
  impl_->executeBegins_(get_id());
  boost::timer executionTimer;
  {
    auto isoString = boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::universal_time());
    impl_->metadata_.setMetadata("Last execution timestamp", isoString);
    copyStateToMetadata();
  }
  /// @todo: status() calls should be logged everywhere, need to change legacy loggers. issue #nnn
  status(starting);
  /// @todo: need separate logger per module
  //LOG_DEBUG("STARTING MODULE: " << id_.id_);
  impl_->executionState_->transitionTo(ModuleExecutionState::Executing);
  bool returnCode = false;
  bool threadStopValue = false;

  try
  {
    if (!executionDisabled())
      execute();
    returnCode = true;
  }
  catch (const std::bad_alloc&)
  {
    error("MODULE ERROR: bad_alloc caught");
  }
  catch (PortNotFoundException& e)
  {
    std::ostringstream ostr;
    ostr << "Port not found, it may need initializing the module constructor. " << std::endl << "Message: " << e.what() << std::endl;
    error(ostr.str());
  }
  catch (AlgorithmParameterNotFound& e)
  {
    std::ostringstream ostr;
    ostr << "State key not found, it may need initializing in ModuleClass::setStateDefaults(). " << std::endl << "Message: " << e.what() << std::endl;
    error(ostr.str());
  }
  catch (Core::ExceptionBase& e)
  {
    /// @todo: this block is repetitive (logging-wise) if the macros are used to log AND throw an exception with the same message. Figure out a reasonable condition to enable it.
    if (LogSettings::Instance().verbose())
    {
      std::ostringstream ostr;
      ostr << "Caught exception: " << e.typeName() << std::endl << "Message: " << e.what() << "\n" << boost::diagnostic_information(e) << std::endl;
      error(ostr.str());
    }
  }
  catch (const std::exception& e)
  {
    error(std::string("MODULE ERROR: std::exception caught: ") + e.what());
  }
  catch (const boost::thread_interrupted&)
  {
    error("MODULE ERROR: execution thread interrupted by user.");
    threadStopValue = true;
  }
  catch (...)
  {
    error("MODULE ERROR: unhandled exception caught");
  }
  impl_->threadStopped_ = threadStopValue;

  auto executionTime = executionTimer.elapsed();
  {
    std::ostringstream ostr;
    ostr << executionTime;
    impl_->metadata_.setMetadata("Last execution duration (seconds)", ostr.str());
  }

  std::ostringstream finished;
  finished << "MODULE " << get_id().id_ << " FINISHED " <<
    (returnCode ? "successfully " : "with errors ") << "in " << executionTime << " seconds.";
  status(finished.str());
#ifdef BUILD_HEADLESS //TODO: better headless logging
  if (!LogSettings::Instance().verbose())
  {
    Guard g(executeLogLock.get());
    std::cout << finished.str() << std::endl;
  }
#endif

  //TODO: brittle dependency on Completed with executor
  impl_->executionState_->transitionTo(ModuleExecutionState::Completed);

  auto expandedEndState = returnCode ? ModuleExecutionState::Completed : ModuleExecutionState::Errored;
  impl_->executionState_->setExpandedState(expandedEndState);

  if (!executionDisabled())
  {
    resetStateChanged();
    impl_->inputsChanged_ = false;
  }

  impl_->executeEnds_(executionTime, get_id());
  return returnCode;
}

ModuleStateHandle Module::get_state()
{
  return impl_->state_;
}

const ModuleStateHandle Module::cstate() const
{
  return impl_->state_;
}

void Module::set_state(ModuleStateHandle state)
{
  if (!get_state())
    impl_->state_ = state;
  else if (state) // merge/overwrite
  {
    impl_->state_->overwriteWith(*state);
  }
  initStateObserver(impl_->state_.get());
  postStateChangeInternalSignalHookup();
  copyStateToMetadata();
}

AlgorithmBase& Module::algo()
{
  if (!impl_->algo_)
    error("Null algorithm object, make sure AlgorithmFactory knows about this module's algorithm types.");
  ENSURE_NOT_NULL(impl_->algo_, "Null algorithm!");

  return *impl_->algo_;
}

size_t Module::add_input_port(InputPortHandle h)
{
  return impl_->iports_.add(h);
}

size_t Module::add_output_port(OutputPortHandle h)
{
  return impl_->oports_.add(h);
}

bool Module::hasInputPort(const PortId& id) const
{
  return impl_->iports_.hasPort(id);
}

bool Module::hasOutputPort(const PortId& id) const
{
  return impl_->oports_.hasPort(id);
}

namespace //TODO: flesh out requirements for metadata on input handles.
{
  std::string metaInfo(DatatypeHandleOption data)
  {
    if (!data)
      return "Not connected";
    if (!*data)
      return "Null data handle";
    return "Datatype id# " + boost::lexical_cast<std::string>((*data)->id());
  }
}

DatatypeHandleOption Module::get_input_handle(const PortId& id)
{
  /// @todo test...
  if (!impl_->iports_.hasPort(id))
  {
    BOOST_THROW_EXCEPTION(PortNotFoundException() << Core::ErrorMessage("Input port not found: " + id.toString()));
  }

  auto port = impl_->iports_[id];
  if (port->isDynamic())
  {
    BOOST_THROW_EXCEPTION(InvalidInputPortRequestException() << Core::ErrorMessage("Input port " + id.toString() + " is dynamic, get_dynamic_input_handles must be called."));
  }

  {
    //Log::get() << DEBUG_LOG << id_ << " :: inputsChanged is " << inputsChanged_ << ", querying port for value." << std::endl;
    // NOTE: don't use short-circuited boolean OR here, we need to call hasChanged each time since it updates the port's cache flag.
    impl_->inputsChanged_ = port->hasChanged() || impl_->inputsChanged_;
    //Log::get() << DEBUG_LOG << id_ << ":: inputsChanged is now " << inputsChanged_ << std::endl;
  }

  auto data = port->getData();
  impl_->metadata_.setMetadata("Input " + id.toString(), metaInfo(data));
  return data;
}

std::vector<DatatypeHandleOption> Module::get_dynamic_input_handles(const PortId& id)
{
  /// @todo test...
  auto portsWithName = impl_->iports_[id.name];  //will throw if empty
  if (!portsWithName[0]->isDynamic())
  {
    BOOST_THROW_EXCEPTION(InvalidInputPortRequestException() << Core::ErrorMessage("Input port " + id.toString() + " is static, get_input_handle must be called."));
  }

  {
    LOG_DEBUG("{} :: inputsChanged is {}, querying port for value.", get_id().id_, impl_->inputsChanged_);
    // NOTE: don't use short-circuited boolean OR here, we need to call hasChanged each time since it updates the port's cache flag.
    bool startingVal = impl_->inputsChanged_;
    impl_->inputsChanged_ = std::accumulate(portsWithName.begin(), portsWithName.end(), startingVal, [](bool acc, InputPortHandle input) { return input->hasChanged() || acc; });
    LOG_DEBUG("{} :: inputsChanged is now {}.", get_id().id_, impl_->inputsChanged_);
  }

  std::vector<DatatypeHandleOption> options;
  auto getData = [](InputPortHandle input) { return input->getData(); };
  std::transform(portsWithName.begin(), portsWithName.end(), std::back_inserter(options), getData);

  impl_->metadata_.setMetadata("Input " + id.toString(), metaInfo(options.empty() ? boost::none : options[0]));

  return options;
}

void Module::send_output_handle(const PortId& id, DatatypeHandle data)
{
  /// @todo test...
  if (!impl_->oports_.hasPort(id))
  {
    THROW_OUT_OF_RANGE("Output port does not exist: " + id.toString());
  }

  impl_->oports_[id]->sendData(data);
}

std::vector<InputPortHandle> Module::findInputPortsWithName(const std::string& name) const
{
  return impl_->iports_[name];
}

std::vector<OutputPortHandle> Module::findOutputPortsWithName(const std::string& name) const
{
  return impl_->oports_[name];
}

std::vector<InputPortHandle> Module::inputPorts() const
{
  return impl_->iports_.view();
}

std::vector<OutputPortHandle> Module::outputPorts() const
{
  return impl_->oports_.view();
}

ModuleBuilder::ModuleBuilder()
{
}

ModuleBuilder::SinkMaker ModuleBuilder::sink_maker_;
ModuleBuilder::SourceMaker ModuleBuilder::source_maker_;

/*static*/ void ModuleBuilder::use_sink_type(SinkMaker func) { sink_maker_ = func; }
/*static*/ void ModuleBuilder::use_source_type(SourceMaker func) { source_maker_ = func; }

class DummyModule : public Module
{
public:
  explicit DummyModule(const ModuleLookupInfo& info) : Module(info) {}
  virtual void execute() override
  {
    std::ostringstream ostr;
    ostr << "Module " << get_module_name() << " executing for " << 3.14 << " seconds." << std::endl;
    status(ostr.str());
  }
  virtual void setStateDefaults() override
  {}
};

ModuleBuilder& ModuleBuilder::with_name(const std::string& name)
{
  if (!module_)
  {
    ModuleLookupInfo info;
    info.module_name_ = name;
    module_.reset(new DummyModule(info));
  }
  return *this;
}

ModuleBuilder& ModuleBuilder::using_func(ModuleMaker create)
{
  if (!module_)
    module_.reset(create());
  return *this;
}

ModuleBuilder& ModuleBuilder::setStateDefaults()
{
  if (module_)
  {
    module_->setStateDefaults();
    module_->copyStateToMetadata();
  }
  return *this;
}

ModuleBuilder& ModuleBuilder::add_input_port(const Port::ConstructionParams& params)
{
  if (module_)
  {
    addInputPortImpl(params);
  }
  return *this;
}

void ModuleBuilder::addInputPortImpl(const Port::ConstructionParams& params) const
{
  DatatypeSinkInterfaceHandle sink(sink_maker_ ? sink_maker_() : nullptr);
  auto port(boost::make_shared<InputPort>(module_.get(), params, sink));
  port->setIndex(module_->add_input_port(port));
}

ModuleBuilder& ModuleBuilder::add_output_port(const Port::ConstructionParams& params)
{
  if (module_)
  {
    DatatypeSourceInterfaceHandle source(source_maker_ ? source_maker_() : nullptr);
    auto port(boost::make_shared<OutputPort>(module_.get(), params, source));
    port->setIndex(module_->add_output_port(port));
  }
  return *this;
}

PortId ModuleBuilder::cloneInputPort(ModuleHandle module, const PortId& id) const
{
  auto m = dynamic_cast<Module*>(module.get());
  if (m)
  {
    InputPortHandle newPort(m->getInputPort(id)->clone());
    newPort->setIndex(m->add_input_port(newPort));
    return newPort->id();
  }
  THROW_INVALID_ARGUMENT("Don't know how to clone ports on other Module types");
}

void ModuleBuilder::removeInputPort(ModuleHandle module, const PortId& id) const
{
  auto m = dynamic_cast<Module*>(module.get());
  if (m)
  {
    m->removeInputPort(id);
  }
}

ModuleHandle ModuleBuilder::build() const
{
  return module_;
}

boost::signals2::connection Module::connectExecuteBegins(const ExecuteBeginsSignalType::slot_type& subscriber)
{
  return impl_->executeBegins_.connect(subscriber);
}

boost::signals2::connection Module::connectExecuteEnds(const ExecuteEndsSignalType::slot_type& subscriber)
{
  return impl_->executeEnds_.connect(subscriber);
}

boost::signals2::connection Module::connectErrorListener(const ErrorSignalType::slot_type& subscriber)
{
  return impl_->errorSignal_.connect(subscriber);
}

void Module::setUiVisible(bool visible)
{
  if (impl_->uiToggleFunc_)
    impl_->uiToggleFunc_(visible);
}

void Module::setLogger(LoggerHandle log)
{
  impl_->log_ = log;
  if (impl_->algo_)
    impl_->algo_->setLogger(log);
}

void Module::setUpdaterFunc(AlgorithmStatusReporter::UpdaterFunc func)
{
  impl_->updaterFunc_ = func;
  if (impl_->algo_)
    impl_->algo_->setUpdaterFunc(func);
}

bool Module::oport_connected(const PortId& id) const
{
  if (!impl_->oports_.hasPort(id))
    return false;

  auto port = impl_->oports_[id];
  return port->nconnections() > 0;
}

void Module::removeInputPort(const PortId& id)
{
  impl_->iports_.remove(id);
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

void Module::setStateStringFromAlgo(const AlgorithmParameterName& name)
{
  get_state()->setValue(name, algo().get(name).toString());
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

void Module::setAlgoStringFromState(const AlgorithmParameterName& name)
{
  algo().set(name, get_state()->getValue(name).toString());
}

void Module::setAlgoOptionFromState(const AlgorithmParameterName& name)
{
	algo().setOption(name, get_state()->getValue(name).toString());
}

void Module::setStateStringFromAlgoOption(const AlgorithmParameterName& name)
{
  get_state()->setValue(name, algo().getOption(name));
}

void Module::setAlgoListFromState(const AlgorithmParameterName& name)
{
  algo().set(name, get_state()->getValue(name).toVector());
}

ModuleExecutionState& Module::executionState()
{
  return *impl_->executionState_;
}

/// @todo:
// need to hook up output ports for cached state.
bool Module::needToExecute() const
{
  static Mutex needToExecuteLock("needToExecute");
  if (impl_->reexecute_)
  {
    //Test fix for reexecute problem. Seems like it could be a race condition, but not sure.
    Guard g(needToExecuteLock.get());
    if (impl_->threadStopped_)
      return true;
    auto val = impl_->reexecute_->needToExecute();
    //Log::get() << DEBUG_LOG << id_ << " Using real needToExecute strategy object, value is: " << val << std::endl;
    return val;
  }

  return true;
}

bool Module::hasDynamicPorts() const
{
  return false; /// @todo: need to examine HasPorts base classes
}

const MetadataMap& Module::metadata() const
{
  return impl_->metadata_;
}

ModuleReexecutionStrategyHandle Module::getReexecutionStrategy() const
{
  return impl_->reexecute_;
}

void Module::setReexecutionStrategy(ModuleReexecutionStrategyHandle caching)
{
  impl_->reexecute_ = caching;
}

bool Module::inputsChanged() const
{
  return impl_->inputsChanged_;
}

void Module::addPortConnection(const boost::signals2::connection& con)
{
  impl_->portConnections_.emplace_back(new boost::signals2::scoped_connection(con));
}

ModuleWithAsyncDynamicPorts::ModuleWithAsyncDynamicPorts(const ModuleLookupInfo& info, bool hasUI) : Module(info, hasUI)
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
  if (mid == get_id())
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
  std::cout << "hello is this called?" << std::endl;
  //module_.outputPorts();
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
  if (reexecuteMode_ && *reexecuteMode_ == "always")
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

        const auto& input = potentialReplacement.input_ports_[i];
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

        const auto& output = potentialReplacement.output_ports_[i];
        if (output.datatype != toMatch->get_typename())
          return false;
      }
    }
  }
  //LOG_DEBUG("\tFound replacement: " << potentialReplacement.lookupInfo_.module_name_ << std::endl);
  return true;
}

void Module::enqueueExecuteAgain(bool upstream)
{
  impl_->executionSelfRequested_(upstream);
}

boost::signals2::connection Module::connectExecuteSelfRequest(const ExecutionSelfRequestSignalType::slot_type& subscriber)
{
  return impl_->executionSelfRequested_.connect(subscriber);
}

std::hash<std::string> ModuleLevelUniqueIDGenerator::hash_;

std::string ModuleLevelUniqueIDGenerator::generateModuleLevelUniqueID(const ModuleInterface& module, const std::string& name) const
{
  std::ostringstream ostr;
  ostr << name << GeometryObject::delimiter << module.get_id() << GeometryObject::delimiter << "_";

  std::ostringstream toHash;
  toHash << "Data{";
  for (const auto& input : module.inputPorts())
  {
    auto data = input->getData();
    auto dataID = data ? (*data ? (*data)->id() : -1) : -2;
    toHash << "[" << input->get_portname() << "]:" << dataID << "_";
  }

  toHash << "}__State{";
  auto state = module.cstate();
  for (const auto& key : state->getKeys())
  {
    toHash << key << "->" << state->getValue(key).value() << "_";
  }
  toHash << "}";

  ostr << hash_(toHash.str());

  return ostr.str();
}

GeometryGeneratingModule::GeometryGeneratingModule(const ModuleLookupInfo& info) : Module(info)
{}

std::string GeometryGeneratingModule::generateGeometryID(const std::string& tag) const
{
  ModuleLevelUniqueIDGenerator gen(*this, tag);
  return gen();
}

bool Module::isStoppable() const
{
  return dynamic_cast<const Interruptible*>(this) != nullptr;
}

void Module::sendFeedbackUpstreamAlongIncomingConnections(const ModuleFeedback& feedback) const
{
  for (const auto& inputPort : inputPorts())
  {
    if (inputPort->nconnections() > 0)
    {
      auto connection = inputPort->connection(0); // only one incoming connection for input ports
      //TODO: extract port method
      connection->oport_->sendConnectionFeedback(feedback);
    }
  }
}

std::string Module::helpPageUrl() const
{
  auto url = "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php/CIBC:Documentation:SCIRun:Reference:"
    + legacyPackageName() + ":" + legacyModuleName();
  return url;
}

std::string Module::newHelpPageUrl() const
{
  return "https://cibctest.github.io/scirun-pages/modules.html#" + legacyModuleName();
}
