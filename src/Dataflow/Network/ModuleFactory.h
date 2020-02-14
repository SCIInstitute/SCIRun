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


/// @todo Documentation Dataflow/Network/ModuleFactory.h

#ifndef DATAFLOW_NETWORK_MODULE_FACTORY_H
#define DATAFLOW_NETWORK_MODULE_FACTORY_H

#include <vector>
#include <set>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE ModuleFactory
  {
  public:
    virtual ~ModuleFactory();
    virtual ModuleDescription lookupDescription(const ModuleLookupInfo& info) const = 0;
    virtual ModuleHandle create(const ModuleDescription& desc) const = 0;
    virtual void setStateFactory(ModuleStateFactoryHandle stateFactory) = 0;
    virtual void setAlgorithmFactory(Core::Algorithms::AlgorithmFactoryHandle algoFactory) = 0;
    virtual void setReexecutionFactory(ReexecuteStrategyFactoryHandle reexFactory) = 0;
    virtual const ModuleDescriptionMap& getAllAvailableModuleDescriptions() const = 0;
    virtual const DirectModuleDescriptionLookupMap& getDirectModuleDescriptionLookupMap() const = 0;
    virtual bool moduleImplementationExists(const std::string& name) const = 0;
  };

  SCISHARE ModuleHandle CreateModuleFromUniqueName(ModuleFactory& factory, const std::string& moduleName);

  namespace ReplacementImpl
  {
    //loose replace interpretation: order of ports doesn't matter, only number. could use multiset here, but not as easy to deal with.
    using ConnectedPortTypesWithCount = std::map<std::string, int>;
    struct SCISHARE ConnectedPortInfo
    {
      ConnectedPortTypesWithCount input, output;
    };

    SCISHARE bool operator==(const ConnectedPortInfo& lhs, const ConnectedPortInfo& rhs);
    SCISHARE bool operator!=(const ConnectedPortInfo& lhs, const ConnectedPortInfo& rhs);
    SCISHARE bool operator<(const ConnectedPortInfo& lhs, const ConnectedPortInfo& rhs);
    SCISHARE std::ostream& operator<<(std::ostream& o, const ConnectedPortInfo& cpi);
    SCISHARE ConnectedPortInfo makeConnectedPortInfo(ModuleHandle module);

    SCISHARE std::vector<ConnectedPortInfo> allPossibleConnectedPortConfigs(
      const InputPortDescriptionList& inputPorts,
      const OutputPortDescriptionList& outputPorts);

    using ModuleLookupInfoSet = std::set<ModuleLookupInfo, ModuleLookupInfoLess>;

    class SCISHARE ModuleReplacementFilter
    {
    public:
      using ReplaceMap = std::map<ConnectedPortInfo, ModuleLookupInfoSet>;
      explicit ModuleReplacementFilter(ReplaceMap&& map) : replaceMap_(map) {}
      const ModuleLookupInfoSet& findReplacements(const ConnectedPortInfo& ports) const;
    private:
      ReplaceMap replaceMap_;
    };

    class SCISHARE ModuleReplacementFilterBuilder
    {
    public:
      explicit ModuleReplacementFilterBuilder(const DirectModuleDescriptionLookupMap& map) : descMap_(map) {}
      boost::shared_ptr<ModuleReplacementFilter> build();
    private:
      void registerModule(ModuleReplacementFilter::ReplaceMap& replaceMap, const ModuleLookupInfo& info,
        const InputPortDescriptionList& inputPorts,
        const OutputPortDescriptionList& outputPorts);
      const DirectModuleDescriptionLookupMap& descMap_;
    };
  }

}}}

#endif
