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


#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Modules/Legacy/Fields/CreateLatVol.h>
#include <Modules/Legacy/Math/SolveMinNormLeastSqSystem.h>
#include <Modules/Legacy/Fields/GetMeshQualityField.h>
#include <Modules/Legacy/Fields/GetFieldData.h>
#include <Interface/Modules/Factory/ModuleDialogFactory.h>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Modules;
using namespace Testing;
using namespace Modules::Factory;
using namespace Dataflow::Networks;

const int NUM_DIALOGS = 131;
const int EXPECTED_RANGE = 5;   // Require updating these numbers every few modules

TEST(ModuleDialogFactoryTests, ListAllDialogs)
{
  ModuleDialogFactory factory(nullptr, {}, {});

  auto descMap = factory.getMap();

  std::cout << "dialog factory size: " << descMap.size() << std::endl;
  EXPECT_GE(descMap.size(), NUM_DIALOGS);
  EXPECT_LE(descMap.size(), NUM_DIALOGS + EXPECTED_RANGE);

  // for (const auto& m : descMap)
  // {
  //   std::cout << m.first << " -> " << m.second << std::endl;
  // }
}

TEST(ModuleDialogFactoryTests, ModuleTraitHasUIMatchesDialogFactory)
{
  HardCodedModuleFactory moduleFactory;
  auto modules = moduleFactory.getDirectModuleDescriptionLookupMap();
  ModuleDialogFactory dialogFactory(nullptr, {}, {});
  auto dialogs = dialogFactory.getMap();

  std::set<std::string> modulesWithUIs;

  for (const auto& d : dialogs)
  {
    auto moduleName = d.first;
    auto modFactIter = std::find_if(modules.cbegin(), modules.cend(),
      [&moduleName](const DirectModuleDescriptionLookupMap::value_type& p) { return p.first.module_name_ == moduleName; });
    if (modFactIter != modules.end())
    {
      if (!modFactIter->second.hasUI_)
        std::cout << moduleName << " is missing trait HasUI" << std::endl;
      EXPECT_TRUE(modFactIter->second.hasUI_);
      modulesWithUIs.insert(moduleName);
    }
    else
      FAIL() << "Module found in dialog factory but not module factory: " << moduleName;
  }

  for (const auto& m : modules)
  {
    if (modulesWithUIs.find(m.first.module_name_) != modulesWithUIs.end())
    {
      if (!m.second.hasUI_)
        std::cout << m.first.module_name_ << " is missing trait HasUI" << std::endl;
      EXPECT_TRUE(m.second.hasUI_);
    }
    else
    {
      if (m.second.hasUI_)
        std::cout << m.first.module_name_ << " has trait HasUI, when it should not" << std::endl;
      EXPECT_FALSE(m.second.hasUI_);
    }
  }
}
