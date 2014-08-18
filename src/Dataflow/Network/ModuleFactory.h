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
/// @todo Documentation Dataflow/Network/ModuleFactory.h

#ifndef DATAFLOW_NETWORK_MODULE_FACTORY_H
#define DATAFLOW_NETWORK_MODULE_FACTORY_H 

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE ModuleFactory
  {
  public:
    virtual ~ModuleFactory();
    virtual ModuleDescription lookupDescription(const ModuleLookupInfo& info) = 0;
    virtual ModuleHandle create(const ModuleDescription& desc) = 0;
    virtual void setStateFactory(ModuleStateFactoryHandle stateFactory) = 0;
    virtual void setAlgorithmFactory(Core::Algorithms::AlgorithmFactoryHandle algoFactory) = 0;
    virtual void setReexecutionFactory(ReexecuteStrategyFactoryHandle reexFactory) = 0;
    virtual const ModuleDescriptionMap& getAllAvailableModuleDescriptions() const = 0;
  };

  SCISHARE SCIRun::Dataflow::Networks::ModuleHandle CreateModuleFromUniqueName(ModuleFactory& factory, const std::string& moduleName);

}}}

#endif
