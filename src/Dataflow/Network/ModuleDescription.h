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
/// @todo Documentation Dataflow/Network/ModuleDescription.h

#ifndef DATAFLOW_NETWORK_MODULE_DESCRIPTION_H
#define DATAFLOW_NETWORK_MODULE_DESCRIPTION_H

#include <string>
#include <vector>
#include <iosfwd>
#include <boost/function.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  struct SCISHARE PortId
  {
    explicit PortId(size_t num = 0, const std::string& n = "[undefined]") : name(n), id(num)
    {
      //std::cout << "PortId(" << num << "," << n << "," << dyn << ")" << std::endl;
    }
    std::string name;
    size_t id; /// @todo: need smart way to set
    std::string toString() const;
  };

  struct SCISHARE PortDescription
  {
    PortDescription(const PortId& p, const std::string& d, bool dyn) :
      id(p), datatype(d), isDynamic(dyn) {}
    PortId id;
    std::string datatype;
    bool isDynamic;
  };

  class SCISHARE PortColorLookup
  {
  public:
    static std::string toColor(const std::string& portDatatype);
  private:
    static std::map<std::string, std::string> portColorMap_;
    static void init();
  };

  typedef PortDescription InputPortDescription;
  typedef PortDescription OutputPortDescription;
  typedef std::vector<InputPortDescription> InputPortDescriptionList;
  typedef std::vector<OutputPortDescription> OutputPortDescriptionList;

  struct SCISHARE ModuleId
  {
    std::string name_, id_;
    int idNumber_;

    ModuleId();
    ModuleId(const std::string& name, int idNumber);
    explicit ModuleId(const std::string& nameIdStr);

    operator std::string() const { return id_; }

    ModuleId& operator++();
    ModuleId operator++(int);
  private:
    void setIdString();
  };

  SCISHARE bool operator==(const ModuleId& lhs, const ModuleId& rhs);
  SCISHARE bool operator!=(const ModuleId& lhs, const ModuleId& rhs);
  SCISHARE bool operator<(const ModuleId& lhs, const ModuleId& rhs);
  SCISHARE std::ostream& operator<<(std::ostream& o, const ModuleId& id);

  SCISHARE bool operator==(const PortId& lhs, const PortId& rhs);
  SCISHARE bool operator!=(const PortId& lhs, const PortId& rhs);
  SCISHARE bool operator<(const PortId& lhs, const PortId& rhs);
  SCISHARE std::ostream& operator<<(std::ostream& o, const PortId& id);

  struct SCISHARE ModuleLookupInfo
  {
    ModuleLookupInfo();
    ModuleLookupInfo(const std::string& mod, const std::string& cat, const std::string& pack);
    std::string package_name_;
    std::string category_name_;
    std::string module_name_;
  };

  SCISHARE bool operator==(const ModuleLookupInfo& lhs, const ModuleLookupInfo& rhs);
  SCISHARE bool operator!=(const ModuleLookupInfo& lhs, const ModuleLookupInfo& rhs);

  typedef boost::function<class Module*()> ModuleMaker;

  struct SCISHARE ModuleDescription
  {
    ModuleDescription();
    ~ModuleDescription();
    ModuleLookupInfo lookupInfo_;
    std::string module_version_;
    std::string moduleStatus_;
    std::string moduleInfo_;
    InputPortDescriptionList input_ports_;
    OutputPortDescriptionList output_ports_;
    //bool                              optional_;
    //bool                              hide_;
    //bool                              dynamic_;
    //std::vector<std::string>          authors_;
    //std::string                       summary_;
    ModuleMaker                       maker_;
    //bool last_port_dynamic_;
    //bool                              has_gui_node_;
  };

  SCISHARE std::ostream& operator<<(std::ostream& o, const ModuleLookupInfo& mli);
  SCISHARE std::ostream& operator<<(std::ostream& o, const ModuleDescription& desc);
  SCISHARE bool canReplaceWith(ModuleHandle module, const ModuleDescription& potentialReplacement);

  struct SCISHARE ModuleLookupInfoLess
  {
    bool operator()(const ModuleLookupInfo& lhs, const ModuleLookupInfo& rhs) const;
  };

  typedef std::map<ModuleLookupInfo, ModuleDescription, ModuleLookupInfoLess> DirectModuleDescriptionLookupMap;

}}}

#endif
