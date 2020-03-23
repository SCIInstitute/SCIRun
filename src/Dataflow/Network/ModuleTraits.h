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


#ifndef DATAFLOW_NETWORK_MODULETRAITS_H
#define DATAFLOW_NETWORK_MODULETRAITS_H

#include <boost/static_assert.hpp>

namespace SCIRun {
namespace Modules
{
  enum ModuleFlags
  {
    NoAlgoOrUI              = 0,
    ModuleHasAlgorithm      = 1 << 0,
    ModuleHasUI             = 1 << 1,
    ModuleHasUIAndAlgorithm = ModuleHasAlgorithm + ModuleHasUI,
    UNDEFINED_MODULE_FLAG   = -1
  };

  template <class ModuleType>
  struct ModuleTraits
  {
    static const int Flags = ModuleType::TraitFlags;
  };

  DEFINE_MEMBER_CHECKER(Flags)

  template <class ModuleType>
  struct HasUI
  {
    static const int ensureModuleDefinesFlags[ModuleTraits<ModuleType>::Flags];
    static const bool value;
  };

  template <class ModuleType>
  const bool HasUI<ModuleType>::value = (ModuleTraits<ModuleType>::Flags & ModuleHasUI) != 0;

  template <class ModuleType>
  struct HasAlgorithm
  {
    static const int ensureModuleDefinesFlags[ModuleTraits<ModuleType>::Flags];
    static const bool value;
  };

  template <class ModuleType>
  const bool HasAlgorithm<ModuleType>::value = (ModuleTraits<ModuleType>::Flags & ModuleHasAlgorithm) != 0;

  #define MODULE_TRAITS_AND_INFO(value) public: static const int TraitFlags = value;\
    static const Dataflow::Networks::ModuleLookupInfo staticInfo_;\

  #define MODULE_INFO_DEF(moduleName, category, package) const SCIRun::Dataflow::Networks::ModuleLookupInfo moduleName::staticInfo_(#moduleName, #category, #package);

  #define HAS_DYNAMIC_PORTS public: bool hasDynamicPorts() const override { return true; }

  #define LEGACY_BIOPSE_MODULE public: std::string legacyPackageName() const override { return "BioPSE"; }
  #define LEGACY_MATLAB_MODULE public: std::string legacyPackageName() const override { return "MatlabInterface"; }
  #define CONVERTED_VERSION_OF_MODULE(modName) public: std::string legacyModuleName() const override { return #modName; }
  #define NEW_HELP_WEBPAGE_ONLY public: std::string helpPageUrl() const override { return newHelpPageUrl(); }
  #define DEPRECATED_MODULE_REPLACE_WITH(modName) public: bool isDeprecated() const override { return true; } std::string replacementModuleName() const override { return #modName; }
  #define DISABLED_WITHOUT_ABOVE_COMPILE_FLAG public: bool isImplementationDisabled() const override { return true; }
}
}

#endif
