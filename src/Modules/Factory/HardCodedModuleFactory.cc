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


/// @todo Documentation HardCodedModuleFactory.cc

#include <iostream>
#include <boost/assign.hpp>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/ModuleBuilder.h>
#include <Dataflow/Network/SimpleSourceSink.h>
#include <Modules/Factory/ModuleDescriptionLookup.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules;
using namespace Factory;

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

  /// @todo: possibly use different build setting for these.
  if (includeTestingModules_)
  {
    addTestingModules();
  }

  addBrainSpecificModules();
  addMoreModules();
  addGeneratedModules();
}


ModuleDescription ModuleDescriptionLookup::lookupDescription(const ModuleLookupInfo& info) const
{
  auto iter = moduleLookup_.find(info);
  if (iter == moduleLookup_.end())
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
  ModuleBuilder::use_sink_type(boost::factory<SimpleSink*>());
  ModuleBuilder::use_source_type(boost::factory<SimpleSource*>());
}

void HardCodedModuleFactory::setStateFactory(ModuleStateFactoryHandle stateFactory)
{
  DefaultModuleFactories::defaultStateFactory_ = stateFactory_ = stateFactory;
}

void HardCodedModuleFactory::setAlgorithmFactory(AlgorithmFactoryHandle algoFactory)
{
  DefaultModuleFactories::defaultAlgoFactory_ = algoFactory;
}

void HardCodedModuleFactory::setReexecutionFactory(ReexecuteStrategyFactoryHandle reexFactory)
{
  DefaultModuleFactories::defaultReexFactory_ = reexFactory;
}

ModuleHandle HardCodedModuleFactory::create(const ModuleDescription& desc) const
{
  ModuleBuilder builder;

  if (desc.maker_)
    builder.using_func(desc.maker_);
  else
    builder.with_name(desc.lookupInfo_.module_name_);

  for (const auto& input : desc.input_ports_)
  {
    builder.add_input_port(Port::ConstructionParams(input.id, input.datatype, input.isDynamic));
  }
  builder.add_input_port(Port::ConstructionParams(ProgrammablePortId(), "MetadataObject", false));
  for (const auto& output : desc.output_ports_)
  {
    builder.add_output_port(Port::ConstructionParams(output.id, output.datatype, output.isDynamic));
  }

  builder.setInfoStrings(desc);

  return builder.setStateDefaults().build();
}

ModuleDescription HardCodedModuleFactory::lookupDescription(const ModuleLookupInfo& info) const
{
  return impl_->lookup.lookupDescription(info);
}

const ModuleDescriptionMap& HardCodedModuleFactory::getAllAvailableModuleDescriptions() const
{
  return impl_->lookup.descMap_;
}

const DirectModuleDescriptionLookupMap& HardCodedModuleFactory::getDirectModuleDescriptionLookupMap() const
{
  return impl_->lookup.moduleLookup_;
}

bool HardCodedModuleFactory::moduleImplementationExists(const std::string& name) const
{
  auto map = getDirectModuleDescriptionLookupMap();
  return map.find(ModuleLookupInfo(name, "", "")) != map.end();
}
