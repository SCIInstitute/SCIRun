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


#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/assign.hpp>
#include <Dataflow/Network/ModuleDescription.h>
#include <Core/Utils/Exception.h>

using namespace SCIRun::Dataflow::Networks;
using namespace boost::assign;

std::string PortColorLookup::toColor(const std::string& portDatatype)
{
  if (portColorMap_.empty())
    init();

  auto it = portColorMap_.find(portDatatype);
  return it != portColorMap_.end() ? it->second : "<undefined>";
}

std::map<std::string, std::string> PortColorLookup::portColorMap_;

void PortColorLookup::init()
{
  insert(portColorMap_)
    ("Matrix", "blue")
    ("Scalar", "lightblue")
    ("Double", "lightblue")
    ("Int32", "lightblue")
    ("String", "darkGreen")
    ("Field", "yellow")
    ("Geometry", "magenta")
    ("OsprayGeometry", "darkMagenta")
    ("ColorMap", "purple")
    ("Bundle", "orange")
    ("Nrrd", "cyan") // not quite right, it's bluer than the highlight cyan
    ("ComplexMatrix", "brown")
    ("MetadataObject", "darkGray")
    ("Datatype", "white");
}

ModuleLookupInfo::ModuleLookupInfo() {}

ModuleLookupInfo::ModuleLookupInfo(const std::string& mod, const std::string& cat, const std::string& pack)
  : package_name_(pack), category_name_(cat), module_name_(mod)
{}

ModuleId::ModuleId() : name_("<Unknown>"), id_("<Invalid>"), idNumber_(-1) {}

ModuleId::ModuleId(const std::string& name, int idNumber)
  : name_(name), idNumber_(idNumber)
{
  setIdString();
}

ModuleId::ModuleId(const std::string& nameIdStr)
  : id_(nameIdStr)
{
  static boost::regex r("(.+?):?(\\d+)");
  boost::smatch what;
  if (!regex_match(id_, what, r))
    THROW_INVALID_ARGUMENT("Invalid Module Id: " + nameIdStr);
  name_ = std::string(what[1]);
  idNumber_ = boost::lexical_cast<int>(static_cast<std::string>(what[2]));
}

void ModuleId::setIdString()
{
  id_ = name_ + ':' + boost::lexical_cast<std::string>(idNumber_);
}

ModuleId& ModuleId::operator++()
{
  ++idNumber_;
  setIdString();
  return *this;
}

ModuleId ModuleId::operator++(int)
{
  ModuleId copy(*this);
  ++(*this);
  return copy;
}

bool SCIRun::Dataflow::Networks::operator==(const ModuleId& lhs, const ModuleId& rhs)
{
  return lhs.id_ == rhs.id_;
}

bool SCIRun::Dataflow::Networks::operator!=(const ModuleId& lhs, const ModuleId& rhs)
{
  return !(lhs == rhs);
}

bool SCIRun::Dataflow::Networks::operator<(const ModuleId& lhs, const ModuleId& rhs)
{
  return lhs.id_ < rhs.id_;
}

std::ostream& SCIRun::Dataflow::Networks::operator<<(std::ostream& o, const ModuleId& id)
{
  return o << id.id_;
}

bool SCIRun::Dataflow::Networks::operator==(const PortId& lhs, const PortId& rhs)
{
  return lhs.name == rhs.name && lhs.id == rhs.id;
}

bool SCIRun::Dataflow::Networks::operator!=(const PortId& lhs, const PortId& rhs)
{
  return !(lhs == rhs);
}

bool SCIRun::Dataflow::Networks::operator<(const PortId& lhs, const PortId& rhs)
{
  if (lhs.name == rhs.name)
    return lhs.id < rhs.id;
  return lhs.name < rhs.name;
}

std::ostream& SCIRun::Dataflow::Networks::operator<<(std::ostream& o, const PortId& id)
{
  return o << id.name << ":" << id.id;
}

std::string PortId::toString() const
{
  std::ostringstream ostr;
  ostr << *this;
  return ostr.str();
}

std::ostream& SCIRun::Dataflow::Networks::operator<<(std::ostream& o, const ModuleLookupInfo& mli)
{
  return o << "Module[" << mli.package_name_ << "::" << mli.category_name_ << "::" << mli.module_name_ << "]";

}

std::ostream& SCIRun::Dataflow::Networks::operator<<(std::ostream& o, const ModuleDescription& desc)
{
  return o << desc.lookupInfo_ << " status: " << desc.moduleStatus_ << " info: " << desc.moduleInfo_
    << " num inputs: " << desc.input_ports_.size() << " num outputs: " << desc.output_ports_.size()
    << " has ui: " << desc.hasUI_ << " has algo: " << desc.hasAlgo_;
}

bool ModuleLookupInfoLess::operator()(const ModuleLookupInfo& lhs, const ModuleLookupInfo& rhs) const
{
  //TODO: this is the correct ordering, but right now the package and category are not passed in when adding modules to the network. For now, just compare by module name.
  //return std::tie(lhs.package_name_, lhs.category_name_, lhs.module_name_) < std::tie(rhs.package_name_, rhs.category_name_, rhs.module_name_);
  return lhs.module_name_ < rhs.module_name_;
}

bool SCIRun::Dataflow::Networks::operator==(const ModuleLookupInfo& lhs, const ModuleLookupInfo& rhs)
{
  return lhs.module_name_ == rhs.module_name_;// && lhs.id == rhs.id;
}

bool SCIRun::Dataflow::Networks::operator!=(const ModuleLookupInfo& lhs, const ModuleLookupInfo& rhs)
{
  return !(lhs == rhs);
}
