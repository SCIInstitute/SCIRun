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


/// @todo Documentation Modules/Factory/HardCodedModuleFactory.h

#ifndef HARD_CODED_MODULE_FACTORY_H
#define HARD_CODED_MODULE_FACTORY_H

#include <Dataflow/Network/ModuleFactory.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Modules/Factory/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Factory {

      class SCISHARE HardCodedModuleFactory : public Dataflow::Networks::ModuleFactory
      {
      public:
        HardCodedModuleFactory();
        virtual Dataflow::Networks::ModuleDescription lookupDescription(const Dataflow::Networks::ModuleLookupInfo& info) const override;
        virtual Dataflow::Networks::ModuleHandle create(const Dataflow::Networks::ModuleDescription& info) const override;
        virtual void setStateFactory(Dataflow::Networks::ModuleStateFactoryHandle stateFactory) override;
        virtual void setAlgorithmFactory(Core::Algorithms::AlgorithmFactoryHandle algoFactory) override;
        virtual void setReexecutionFactory(Dataflow::Networks::ReexecuteStrategyFactoryHandle reexFactory) override;
        virtual const Dataflow::Networks::ModuleDescriptionMap& getAllAvailableModuleDescriptions() const override;
        virtual const Dataflow::Networks::DirectModuleDescriptionLookupMap& getDirectModuleDescriptionLookupMap() const override;
        virtual bool moduleImplementationExists(const std::string& name) const override;
      private:
        Dataflow::Networks::ModuleStateFactoryHandle stateFactory_;
        boost::shared_ptr<class HardCodedModuleFactoryImpl> impl_;
      };
    }
  }
}

#endif
