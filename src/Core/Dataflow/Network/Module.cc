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
#include <Core/Dataflow/Network/Module.h>
#include <Core/Dataflow/Network/PortManager.h>
#include <Core/Dataflow/Network/ModuleStateInterface.h>
#include <Core/Dataflow/Network/DataflowInterfaces.h>

using namespace SCIRun::Domain::Networks;

std::string SCIRun::Domain::Networks::to_string(const ModuleInfoProvider& m)
{
  return m.get_module_name() + " [" + m.get_id() + "]";
}

/*static*/ int Module::instanceCount_ = 0;

//TODO MAKE NEW FILE 
class NullModuleState : public ModuleStateInterface
{
public:
  virtual boost::any& operator[](const std::string& parameterName)
  {
    return dummy_;
  }

  //virtual boost::signals::connection connect_state_changed_signal(state_changed_sig_t::slot_function_type subscriber)
  //{
  //  return boost::signals::connection();
  //}
private:
  boost::any dummy_;
};


Module::Module(const std::string& name, 
  ModuleStateFactoryHandle stateMaker,
  bool hasUi,
  const std::string& cat/* ="unknown" */, const std::string& pack/* ="unknown" */, const std::string& version/* ="1.0" */)
  : has_ui_(hasUi), executionTime_(1.0), state_(stateMaker ? stateMaker->make_state(name) : new NullModuleState)
{
  set_modulename(name);
  id_ = name + boost::lexical_cast<std::string>(instanceCount_++);
  iports_.set_module(this);
  oports_.set_module(this);
}

Module::~Module()
{
  instanceCount_--;
}

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

void Module::do_execute()
{
  //abort_flag_=0;

  //TODO: soon
  //Logger::Instance->log("STARTING MODULE: "+id_);

  // Reset all of the ports.
  oports_.resetAll();
  iports_.resetAll();

  // Reset the TCL variables.
  //reset_vars();

  // This gets flagged if the data on any input port has changed.
  // Also used in the various execute functions to note gui_vars changing.
  //inputs_changed_ = false;

  // Call the User's execute function.
  //update_msg_state(Reset);
  //timer_ = Time::currentTicks();

  //update_state(JustStarted);

  try 
  {
    execute();
  }
  catch(const std::bad_alloc& /*ba*/)
  {
    std::cerr << "MODULE ERROR: bad_alloc caught" << std::endl;
    //std::ostringstream command;
    //command << "createSciDialog -error -title \"Memory Allocation Error\""
    //  << " -message \"" << module_name_ << " tried to allocate too much memory.\n"
    //  << "You network may be in an unstable state.  Please consider saving it\n"
    //  << "if necessary and restarting SCIRun using a smaller data set.\n"
    //  << "(You may need to use ^u to unlock the Network Editor (if SCIRun didn't\n"
    //  << "finish executing) before you can save your network.)\"";
    //error( "Module tried to allocate too much memory... Be careful, your network"
    //  " may be unstable now." );
    //TCLInterface::eval( command.str() );
  }
  catch (const std::exception& /*e*/)
  {
    std::cerr << "MODULE ERROR: std::exception caught" << std::endl;
    //error(std::string("Module crashed with the following exception:\n  ")+
    //  e.message());
    //if (e.stackTrace())
    //{
    //  error("Thread Stacktrace:");
    //  error(e.stackTrace());
    //}
  }
  catch (const std::string& /*a*/)
  {
    std::cerr << "MODULE ERROR: std::string caught" << std::endl;
    //error(a);
  }
  catch (const char * /*a*/)
  {
    std::cerr << "MODULE ERROR: const char* caught" << std::endl;
    //error(a);
  }
  catch (...)
  {
    std::cerr << "MODULE ERROR: unhandled exception caught" << std::endl;
    //error("Module.cc: Module caught unhandled exception.");
  }

  //update_state(Completed);

  // Call finish on all ports.
  iports_.apply(boost::bind(&PortInterface::finish, _1));
  oports_.apply(boost::bind(&PortInterface::finish, _1));

  //TODO: soon
  //Logger::Instance->log("MODULE FINISHED: " + id_);  
}

ModuleStateHandle Module::get_state() 
{
  return state_;
}

void Module::add_input_port(InputPortHandle h)
{
  iports_.add(h);
}

void Module::add_output_port(OutputPortHandle h)
{
  oports_.add(h);
}

SCIRun::Domain::Datatypes::DatatypeHandleOption Module::get_input_handle(size_t idx)
{
  //TODO test...
  if (idx >= iports_.size())
  {
    throw std::invalid_argument("port does not exist at index " + boost::lexical_cast<std::string>(idx));
  }

  return iports_[idx]->getData();
}

void Module::send_output_handle(size_t idx, SCIRun::Domain::Datatypes::DatatypeHandle data)
{
  //TODO test...
  if (idx >= oports_.size())
  {
    throw std::invalid_argument("port does not exist at index " + boost::lexical_cast<std::string>(idx));
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
  explicit DummyModule(const std::string& name) : Module(name) {}
  virtual void execute() 
  {
    std::cout << "Module " << get_module_name() << " executing for " << executionTime_ << " seconds." << std::endl;
  }
};

Module::Builder& Module::Builder::with_name(const std::string& name)
{
  if (!module_)
    module_.reset(new DummyModule(name));
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
    InputPortHandle port(new InputPort(module_.get(), params, sink));
    module_->add_input_port(port);
  }
  return *this;
}

Module::Builder& Module::Builder::add_output_port(const Port::ConstructionParams& params)
{
  if (module_)
  {
    DatatypeSourceInterfaceHandle source(source_maker_ ? source_maker_() : 0);
    OutputPortHandle port(new OutputPort(module_.get(), params, source));
    module_->add_output_port(port);
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
