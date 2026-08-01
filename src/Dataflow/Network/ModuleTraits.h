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
  enum class ModuleFlags
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
  const bool HasUI<ModuleType>::value = (ModuleTraits<ModuleType>::Flags & static_cast<int>(ModuleFlags::ModuleHasUI)) != 0;

  template <class ModuleType>
  struct HasAlgorithm
  {
    static const int ensureModuleDefinesFlags[ModuleTraits<ModuleType>::Flags];
    static const bool value;
  };

  template <class ModuleType>
  const bool HasAlgorithm<ModuleType>::value = (ModuleTraits<ModuleType>::Flags & static_cast<int>(ModuleFlags::ModuleHasAlgorithm)) != 0;

  // 1-arg form (legacy): flags only.  staticInfo_ is declared but not defined
  // here; MODULE_INFO_DEF must still appear in the module's .cc file.
  #define MODULE_TRAITS_AND_INFO_1(flags) \
    public: static const int TraitFlags = static_cast<int>(flags); \
    static const Dataflow::Networks::ModuleLookupInfo staticInfo_;

  // 4-arg form (preferred): flags + registered name + category + package.
  // staticInfo_ is defined inline — no MODULE_INFO_DEF in the .cc needed.
  // Note: registered name may differ from the C++ class name (e.g. ShowColorMap
  // vs ShowColorMapModule), so it must be supplied explicitly.
  #define MODULE_TRAITS_AND_INFO_4(flags, regName, category, package) \
    public: static const int TraitFlags = static_cast<int>(flags); \
    inline static const Dataflow::Networks::ModuleLookupInfo staticInfo_{ \
      #regName, #category, #package };

  #define MODULE_TRAITS_AND_INFO_PICK(_1, _2, _3, _4, WHICH, ...) WHICH
  #define MODULE_TRAITS_AND_INFO(...) \
    MODULE_TRAITS_AND_INFO_PICK(__VA_ARGS__, \
      MODULE_TRAITS_AND_INFO_4, _unused3, _unused2, \
      MODULE_TRAITS_AND_INFO_1)(__VA_ARGS__)

  // Legacy .cc definition — still needed for modules using the 1-arg form.
  // Becomes a no-op comment for modules that have migrated to the 4-arg form.
  #define MODULE_INFO_DEF(moduleName, category, package) \
    const SCIRun::Dataflow::Networks::ModuleLookupInfo \
      moduleName::staticInfo_(#moduleName, #category, #package);

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
