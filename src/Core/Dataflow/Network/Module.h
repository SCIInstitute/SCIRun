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


#ifndef CORE_DATAFLOW_NETWORK_MODULE_H
#define CORE_DATAFLOW_NETWORK_MODULE_H 

#include <boost/noncopyable.hpp>
#include <string>
#include <vector>
#include <map>

#include <Core/Dataflow/Network/NetworkFwd.h>
#include <Core/Dataflow/Network/ModuleInterface.h>
#include <Core/Dataflow/Network/ModuleDescription.h>
#include <Core/Dataflow/Network/PortManager.h>
#include <Core/Dataflow/Network/Share.h>

namespace SCIRun {
namespace Domain {
namespace Networks {
  
  class SCISHARE Module : public ModuleInterface, boost::noncopyable
  {
  public:
    //TODO replace params with ModuleLookupInfo
    Module(const std::string& name, bool hasUi = true, const std::string& cat="unknown", const std::string& pack="unknown", 
      const std::string& version="1.0");
    virtual ~Module();

    std::string get_module_name() const { return info_.module_name_; }
    std::string get_categoryname() const { return info_.category_name_; }
    std::string get_packagename() const { return info_.package_name_; }
    std::string get_id() const { return id_; }
    bool has_ui() const { return has_ui_; }

    size_t num_input_ports() const;
    size_t num_output_ports() const;

    InputPortHandle get_input_port(const std::string &name) const;
    OutputPortHandle get_output_port(const std::string &name) const;
    OutputPortHandle get_output_port(size_t idx) const;
    InputPortHandle get_input_port(size_t idx) const;

    void do_execute();
    virtual void execute();

    virtual SCIRun::Domain::Datatypes::DatatypeHandleOption get_input_handle(size_t idx);
    virtual void send_output_handle(size_t idx, SCIRun::Domain::Datatypes::DatatypeHandle data);

    class Builder : boost::noncopyable
    {
    public:
      Builder();
      Builder& with_name(const std::string& name);
      Builder& using_func(boost::function<Module*()> create);
      Builder& add_input_port(const Port::ConstructionParams& params);
      Builder& add_output_port(const Port::ConstructionParams& params);
      Builder& disable_ui();
      ModuleHandle build();

      typedef boost::function<SCIRun::Domain::Networks::DatatypeSinkInterface*()> SinkMaker;
      typedef boost::function<SCIRun::Domain::Networks::DatatypeSourceInterface*()> SourceMaker;
      static void use_sink_type(SinkMaker func) { sink_maker_ = func; }
      static void use_source_type(SourceMaker func) { source_maker_ = func; }
    private:
      boost::shared_ptr<Module> module_;
      static SinkMaker sink_maker_;
      static SourceMaker source_maker_;
    };

  protected:

    void set_modulename(const std::string& name)   { info_.module_name_ = name; }
    void set_categoryname(const std::string& name) { info_.category_name_ = name; }
    void set_packagename(const std::string& name)  { info_.package_name_ = name; }

    ModuleLookupInfo info_;

    std::string id_;

  private:
    friend class Builder;
    void add_input_port(InputPortHandle);
    void add_output_port(OutputPortHandle);
    bool has_ui_;

    double executionTime_;

    PortManager<OutputPortHandle> oports_;
    PortManager<InputPortHandle> iports_;
    static int instanceCount_;
  };

}}}

#endif
