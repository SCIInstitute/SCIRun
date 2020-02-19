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


#ifndef MODULE_DESCRIPTION_LOOKUP_H
#define MODULE_DESCRIPTION_LOOKUP_H

#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/Module.h>
#include <boost/functional/factory.hpp>
#include <Modules/Factory/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Factory {

      class SCISHARE ModuleDescriptionLookup
      {
      public:
        ModuleDescriptionLookup();
        Dataflow::Networks::ModuleDescription lookupDescription(const Dataflow::Networks::ModuleLookupInfo& info) const;
        Dataflow::Networks::ModuleDescriptionMap descMap_;
        Dataflow::Networks::DirectModuleDescriptionLookupMap moduleLookup_;
      private:
        bool includeTestingModules_;

        /// @todo: remove this function and use static MLI from each module
        template <class ModuleType>
        void addModuleDesc(const std::string& name, const std::string& category, const std::string& package, const std::string& status, const std::string& desc)
        {
          Dataflow::Networks::ModuleLookupInfo info(name, category, package);
          addModuleDesc<ModuleType>(info, status, desc);
        }

        template <class ModuleType>
        void addModuleDesc(const Dataflow::Networks::ModuleLookupInfo& info, const std::string& status, const std::string& desc)
        {
          Dataflow::Networks::ModuleDescription description;
          description.lookupInfo_ = info;

          description.input_ports_ = IPortDescriber<ModuleType::NumIPorts, ModuleType>::inputs();
          description.output_ports_ = OPortDescriber<ModuleType::NumOPorts, ModuleType>::outputs();
          description.maker_ = boost::factory<ModuleType*>();
          description.moduleStatus_ = status;
          description.moduleInfo_ = desc;
          description.hasUI_ = HasUI<ModuleType>::value;
          description.hasAlgo_ = HasAlgorithm<ModuleType>::value;

          moduleLookup_[info] = description;

          descMap_[info.package_name_][info.category_name_][info.module_name_] = description;
        }

        template <class ModuleType>
        void addModuleDesc(const std::string& status, const std::string& desc)
        {
          addModuleDesc<ModuleType>(ModuleType::staticInfo_, status, desc);
        }

        void addEssentialModules();
        void addTestingModules();
        void addBrainSpecificModules();
        void addMoreModules();
        void addGeneratedModules();
      };
    }
  }
}

#endif
