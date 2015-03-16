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

/// @todo Documentation HardCodedModuleFactory.cc

#include <iostream>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/SimpleSourceSink.h>
#include <Modules/Factory/ModuleDescriptionLookup.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::Factory;
//
using namespace boost::assign;

ModuleDescriptionLookup::ModuleDescriptionLookup() : includeTestingModules_(false)
{
  /// @todo: is BUILD_TESTING off when we build releases?
#ifdef BUILD_TESTING
  includeTestingModules_ = true;
#endif
  /// @todo: make EVEN MORE generic...macros? xml?
  /// @todo: at least remove duplication of Name,Package,Category here since we should be able to infer from header somehow.

  addEssentialModules();

  addBundleModules();

  /// @todo: possibly use different build setting for these.
  if (includeTestingModules_)
  {
    addTestingModules();
  }

  addBrainSpecificModules();
  addMoreModules();
}


ModuleDescription ModuleDescriptionLookup::lookupDescription(const ModuleLookupInfo& info) const
{
  auto iter = lookup_.find(info);
  if (iter == lookup_.end())
  {
    /// @todo: log
    std::ostringstream ostr;
    ostr << "Error: Undefined module \"" << info.module_name_ << "\"";
    THROW_INVALID_ARGUMENT(ostr.str());
  }
  return iter->second;
}

namespace SCIRun {
  namespace Modules {
    namespace Factory {

      class HardCodedModuleFactoryImpl
      {
      public:
        ModuleDescriptionLookup lookup;
      };

    }}}

HardCodedModuleFactory::HardCodedModuleFactory() : impl_(new HardCodedModuleFactoryImpl)
{
  Module::Builder::use_sink_type(boost::factory<SimpleSink*>());
  Module::Builder::use_source_type(boost::factory<SimpleSource*>());
}

void HardCodedModuleFactory::setStateFactory(SCIRun::Dataflow::Networks::ModuleStateFactoryHandle stateFactory)
{
  stateFactory_ = stateFactory;
  Module::defaultStateFactory_ = stateFactory_;
}

void HardCodedModuleFactory::setAlgorithmFactory(SCIRun::Core::Algorithms::AlgorithmFactoryHandle algoFactory)
{
  Module::defaultAlgoFactory_ = algoFactory;
}

void HardCodedModuleFactory::setReexecutionFactory(SCIRun::Dataflow::Networks::ReexecuteStrategyFactoryHandle reexFactory)
{
  Module::defaultReexFactory_ = reexFactory;
}

ModuleHandle HardCodedModuleFactory::create(const ModuleDescription& desc)
{
  Module::Builder builder;

  if (desc.maker_)
    builder.using_func(desc.maker_);
  else
    builder.with_name(desc.lookupInfo_.module_name_);

  BOOST_FOREACH(const InputPortDescription& input, desc.input_ports_)
  {
    builder.add_input_port(Port::ConstructionParams(input.id, input.datatype, input.isDynamic));
  }
  BOOST_FOREACH(const OutputPortDescription& output, desc.output_ports_)
  {
    builder.add_output_port(Port::ConstructionParams(output.id, output.datatype, output.isDynamic));
  }

  ModuleHandle module = builder.setStateDefaults().build();

  return module;
}

ModuleDescription HardCodedModuleFactory::lookupDescription(const ModuleLookupInfo& info)
{
  return impl_->lookup.lookupDescription(info);
}

const ModuleDescriptionMap& HardCodedModuleFactory::getAllAvailableModuleDescriptions() const
{
  return impl_->lookup.descMap_;
}

const DirectModuleDescriptionLookupMap& HardCodedModuleFactory::getDirectModuleDescriptionLookupMap() const
{
  return impl_->lookup.lookup_;
}

bool SCIRun::Modules::Factory::operator==(const ConnectedPortInfo& lhs, const ConnectedPortInfo& rhs)
{
  return lhs.input == rhs.input && lhs.output == rhs.output;
}

bool SCIRun::Modules::Factory::operator!=(const ConnectedPortInfo& lhs, const ConnectedPortInfo& rhs)
{
  return !(lhs == rhs);
}

std::ostream& SCIRun::Modules::Factory::operator<<(std::ostream& o, const ConnectedPortInfo& cpi)
{
  o << "Connected port info: ";
  if (cpi.input.empty())
    o << "[No inputs connected] ";
  else
    for (const auto& p : cpi.input)
    {
      o << "Input[" << p.first << ":" << p.second << "] ";
    }
  if (cpi.output.empty())
    o << "[No outputs connected] ";
  else
    for (const auto& p : cpi.output)
    {
      o << "Output[" << p.first << ":" << p.second << "] ";
    }
  return o;
}

ConnectedPortInfo SCIRun::Modules::Factory::makeConnectedPortInfo(ModuleHandle module)
{
  ConnectedPortInfo cpi;
  if (!module)
    return cpi;
  for (const auto& in : module->inputPorts())
  {
    if (in->nconnections() > 0)
      cpi.input[in->get_typename()]++;
  }
  for (const auto& out : module->outputPorts())
  {
    if (out->nconnections() > 0)
      cpi.output[out->get_typename()]++;
  }
  return cpi;
}

boost::shared_ptr<ModuleReplacementFilter> ModuleReplacementFilterBuilder::build()
{
  ModuleReplacementFilter::ReplaceMap replaceMap;

  for (const auto& infoDesc : descMap_)
  {
    if (false)
      std::cout << infoDesc.first << " --> " << infoDesc.second << std::endl;

    registerModule(infoDesc.first, infoDesc.second.input_ports_, infoDesc.second.output_ports_);
  }

  return boost::make_shared<ModuleReplacementFilter>(std::move(replaceMap));
}

void ModuleReplacementFilterBuilder::registerModule(const ModuleLookupInfo& info, const InputPortDescriptionList& inputPorts, const OutputPortDescriptionList& outputPorts)
{
}
