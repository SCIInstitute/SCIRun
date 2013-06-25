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

#include <log4cpp/Category.hh>
#include <log4cpp/CategoryStream.hh>
#include <log4cpp/Priority.hh>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Engine::State;
using namespace SCIRun::Core::Logging;

std::string SCIRun::Dataflow::Networks::to_string(const ModuleInfoProvider& m)
{
  return m.get_module_name() + " [" + m.get_id().id_ + "]";
}

/*static*/ int Module::instanceCount_ = 0;
/*static*/ LoggerHandle Module::defaultLogger_(new ConsoleLogger);

Module::Module(const ModuleLookupInfo& info,
  bool hasUi,
  ModuleStateFactoryHandle stateFactory,
  const std::string& version)
  : info_(info), has_ui_(hasUi), 
  state_(stateFactory ? stateFactory->make_state(info.module_name_) : new NullModuleState),
  id_(info_.module_name_, instanceCount_++)
{
  iports_.set_module(this);
  oports_.set_module(this);
  setLogger(defaultLogger_);

  log4cpp::Category& root = log4cpp::Category::getRoot();

  root << log4cpp::Priority::INFO << "Module created: " << info.module_name_ << " with id: " << id_;
}

Module::~Module()
{
  instanceCount_--;
}

ModuleStateFactoryHandle Module::defaultStateFactory_;

LoggerHandle Module::getLogger() const { return log_ ? log_ : defaultLogger_; }

OutputPortHandle Module::get_output_port(size_t idx) const
{
  return oports_[idx];
}

InputPortHandle Module::get_input_port(size_t idx) const
{
  return iports_[idx];
}

size_t Module::num_input_ports() const
{
  return iports_.size();
}

size_t Module::num_output_ports() const
{
  return oports_.size();
}

void Module::do_execute() throw()
{
  executeBegins_(id_);
  status("STARTING MODULE: " + id_.id_);

  // Reset all of the ports.
  oports_.resetAll();
  iports_.resetAll();

  try 
  {
    execute();
  }
  catch(const std::bad_alloc&)
  {
    error("MODULE ERROR: bad_alloc caught");
  }
  catch (Core::ExceptionBase& e)
  {
    //TODO: this block is repetitive (logging-wise) if the macros are used to log AND throw an exception with the same message. Figure out a reasonable condition to enable it.
    if (false)
    {
      std::ostringstream ostr;
      ostr << "Caught exception: " << e.typeName();
      ostr << "\n";
      ostr << "Message: " << e.what() << std::endl;
      error(ostr.str());
    }

    //TODO: condition this block on logging level
    if (false)
    {
      std::ostringstream ostrExtra;
      ostrExtra << "TODO! Following error info will be filtered later, it's too technical for general audiences.\n";
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
  executeEnds_(id_);
}

ModuleStateHandle Module::get_state() 
{
  return state_;
}

void Module::set_state(ModuleStateHandle state) 
{
  state_ = state;
}

size_t Module::add_input_port(InputPortHandle h)
{
  iports_.add(h);
  return iports_.size() - 1;
}

size_t Module::add_output_port(OutputPortHandle h)
{
  oports_.add(h);
  return oports_.size() - 1;
}

SCIRun::Core::Datatypes::DatatypeHandleOption Module::get_input_handle(size_t idx)
{
  //TODO test...
  if (idx >= iports_.size())
  {
    BOOST_THROW_EXCEPTION(PortNotFoundException() << Core::ErrorMessage("Port not found: " + boost::lexical_cast<std::string>(idx)));
  }

  return iports_[idx]->getData();
}

void Module::send_output_handle(size_t idx, SCIRun::Core::Datatypes::DatatypeHandle data)
{
  //TODO test...
  if (idx >= oports_.size())
  {
    THROW_OUT_OF_RANGE("port does not exist at index " + boost::lexical_cast<std::string>(idx));
  }

  oports_[idx]->sendData(data);
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

Module::Builder& Module::Builder::add_input_port(const Port::ConstructionParams& params)
{
  if (module_)
  {
    DatatypeSinkInterfaceHandle sink(sink_maker_ ? sink_maker_() : 0);
    InputPortHandle port(boost::make_shared<InputPort>(module_.get(), params, sink));
    port->setIndex(module_->add_input_port(port));
  }
  return *this;
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

Module::Builder& Module::Builder::disable_ui()
{
  module_->has_ui_ = false;
  return *this;
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
