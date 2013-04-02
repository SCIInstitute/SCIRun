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


#ifndef DATAFLOW_NETWORK_MODULE_INTERFACE_H
#define DATAFLOW_NETWORK_MODULE_INTERFACE_H 

#include <boost/signals2.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Dataflow/Network/ExecutableObject.h>
#include <Core/Logging/LoggerFwd.h>
#include <Dataflow/Network/Share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE ModuleInfoProvider
  {
  public:
    virtual ~ModuleInfoProvider() {}
    virtual OutputPortHandle get_output_port(size_t idx) const = 0;
    virtual InputPortHandle get_input_port(size_t idx) const = 0;
    virtual size_t num_input_ports() const = 0;
    virtual size_t num_output_ports() const = 0;
    virtual std::string get_module_name() const = 0;
    virtual ModuleId get_id() const = 0;
    virtual bool has_ui() const = 0;
    virtual const ModuleLookupInfo& get_info() const = 0;
  };

  class SCISHARE ModuleDisplayInterface 
  {
  public:
    virtual ~ModuleDisplayInterface() {}
    virtual void setUiVisible(bool visible) = 0;
  };

  SCISHARE std::string to_string(const ModuleInfoProvider&);
  
  typedef boost::signals2::signal<void (const ModuleId&)> ExecuteBeginsSignalType;
  typedef boost::signals2::signal<void (const ModuleId&)> ExecuteEndsSignalType;
  typedef boost::signals2::signal<void (const ModuleId&)> ErrorSignalType;

  //TODO: interface is getting bloated, segregate it.
  class SCISHARE ModuleInterface : public ModuleInfoProvider, public ModuleDisplayInterface, public ExecutableObject
  {
  public:
    virtual ~ModuleInterface();

    virtual ModuleStateHandle get_state() = 0;

    virtual void do_execute() = 0;

    //TODO for deserialization
    virtual void set_id(const std::string& id) = 0;
    virtual void set_state(ModuleStateHandle state) = 0;

    virtual SCIRun::Core::Datatypes::DatatypeHandleOption get_input_handle(size_t idx) = 0;
    virtual void send_output_handle(size_t idx, SCIRun::Core::Datatypes::DatatypeHandle data) = 0;

    virtual void setLogger(SCIRun::Core::Logging::LoggerHandle log) = 0;
    virtual SCIRun::Core::Logging::LoggerHandle getLogger() const = 0;
    virtual void setUpdaterFunc(SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc func) = 0;

    // TODO: name too clunky.
    virtual void preExecutionInitialization() {}

    // TODO: name too clunky.
    /// Called before the module is to be destroyed. More importantly, called
    /// before the UI widget is to be destroyed.
    virtual void preDestruction() {}

    virtual boost::signals2::connection connectExecuteBegins(const ExecuteBeginsSignalType::slot_type& subscriber) = 0;
    virtual boost::signals2::connection connectExecuteEnds(const ExecuteEndsSignalType::slot_type& subscriber) = 0;
    virtual boost::signals2::connection connectErrorListener(const ErrorSignalType::slot_type& subscriber) = 0;
  };

  #define MODULE_ERROR_WITH_TYPE(type, message) { error(message); BOOST_THROW_EXCEPTION(type() << SCIRun::Core::ErrorMessage(message)); }
}}}

#endif
