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

using namespace SCIRun;
using namespace SCIRun::Modules;
using namespace Testing;
using namespace Modules::Factory;
using namespace Dataflow::Networks;
using namespace ReplacementImpl;
using namespace Dataflow::Engine;
using namespace Core::Algorithms;

class ModuleReplaceTests : public ModuleTest
{
};

#ifdef BUILD_TESTING
const int NUM_MODULES = 188;
#else
const int NUM_MODULES = 179;
#endif
const int NUM_ALGORITHMS = 93;

const int EXPECTED_RANGE = 5;   // Require updating these numbers every few modules

TEST(HardCodedModuleFactoryTests, ListAllModules)
{
  HardCodedModuleFactory factory;

  auto descMap = factory.getDirectModuleDescriptionLookupMap();

  std::cout << "descMap size: " << descMap.size() << std::endl;
  EXPECT_GE(descMap.size(), NUM_MODULES);
  EXPECT_LE(descMap.size(), NUM_MODULES + EXPECTED_RANGE);

  // for (const auto& m : descMap)
  // {
  //   std::cout << m.first << " -> " << m.second << std::endl;
  // }
}

TEST(HardCodedModuleFactoryTests, ListAllAlgorithms)
{
  HardCodedAlgorithmFactory factory;

  std::cout << "algorithm factory size: " << factory.numAlgorithms() << std::endl;
  EXPECT_GE(factory.numAlgorithms(), NUM_ALGORITHMS);
  EXPECT_LE(factory.numAlgorithms(), NUM_ALGORITHMS + EXPECTED_RANGE);

  // for (const auto& a : factory)
  // {
  //   std::cout << a.first << " -> " << a.second.first << std::endl;
  // }
}

TEST(HardCodedModuleFactoryTests, ModuleTraitHasAlgorithmMatchesAlgoFactory)
{
  HardCodedModuleFactory moduleFactory;

  auto modules = moduleFactory.getDirectModuleDescriptionLookupMap();

  HardCodedAlgorithmFactory algoFactory;

  std::set<std::string> modulesWithAlgorithms;

  for (const auto& a : algoFactory)
  {
    auto moduleName = a.first;
    auto modFactIter = std::find_if(modules.cbegin(), modules.cend(),
      [&moduleName](const DirectModuleDescriptionLookupMap::value_type& p) { return p.first.module_name_ == moduleName; });
    if (modFactIter != modules.end())
    {
      if (!modFactIter->second.hasAlgo_)
        std::cout << moduleName << " is missing trait HasAlgorithm" << std::endl;
      EXPECT_TRUE(modFactIter->second.hasAlgo_);
      modulesWithAlgorithms.insert(moduleName);
    }
    else
      FAIL() << "Module found in algorithm factory but not module factory: " << moduleName;
  }

  for (const auto& m : modules)
  {
    if (modulesWithAlgorithms.find(m.first.module_name_) != modulesWithAlgorithms.end())
    {
      if (!m.second.hasAlgo_)
        std::cout << m.first.module_name_ << " is missing trait HasAlgorithm" << std::endl;
      EXPECT_TRUE(m.second.hasAlgo_);
    }
    else
    {
      if (m.second.hasAlgo_)
        std::cout << m.first.module_name_ << " has trait HasAlgorithm, when it should not" << std::endl;
      EXPECT_FALSE(m.second.hasAlgo_);
    }
  }
}

TEST(ModuleTraitsTest, CanDetermineUIAlgoStatically)
{
  ASSERT_TRUE(HasUI<Fields::CreateLatVol>::value);
  ASSERT_FALSE(HasAlgorithm<Fields::CreateLatVol>::value);

  ASSERT_FALSE(HasUI<Fields::GetFieldData>::value);
  ASSERT_TRUE(HasAlgorithm<Fields::GetFieldData>::value);

  ASSERT_TRUE(HasUI<Fields::GetMeshQualityField>::value);
  ASSERT_TRUE(HasAlgorithm<Fields::GetMeshQualityField>::value);

  ASSERT_FALSE(HasUI<Math::SolveMinNormLeastSqSystem>::value);
  ASSERT_FALSE(HasAlgorithm<Math::SolveMinNormLeastSqSystem>::value);
}

TEST_F(ModuleReplaceTests, CanComputeConnectedPortInfoFromModule)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  auto network = controller.getNetwork();

  ModuleHandle send = controller.addModule("CreateMatrix");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReportMatrixInfo");

  ASSERT_EQ(3, network->nmodules());

  {
    auto sendInfo = makeConnectedPortInfo(send);
    ConnectedPortInfo expectedSendInfo; // empty maps
    EXPECT_EQ(expectedSendInfo, sendInfo);
    auto procInfo = makeConnectedPortInfo(process);
    ConnectedPortInfo expectedProcInfo; // empty maps
    EXPECT_EQ(expectedProcInfo, procInfo);
    auto recInfo = makeConnectedPortInfo(receive);
    ConnectedPortInfo expectedRecInfo; // empty maps
    EXPECT_EQ(expectedRecInfo, recInfo);
  }

  auto cid1 = network->connect(ConnectionOutputPort(send, 0), ConnectionInputPort(process, 0));
  auto cid2 = network->connect(ConnectionOutputPort(process, 0), ConnectionInputPort(receive, 0));
  ASSERT_EQ(2, network->nconnections());

  //clang on mac doesn't like "{}" for empty std::map initialization in a struct.
  const ConnectedPortTypesWithCount empty;

  {
    auto sendInfo = makeConnectedPortInfo(send);
    ConnectedPortInfo expectedSendInfo { empty, { { "Matrix", 1 } } };
    EXPECT_EQ(expectedSendInfo, sendInfo);
    auto procInfo = makeConnectedPortInfo(process);
    ConnectedPortInfo expectedProcInfo { { { "Matrix", 1 } }, { { "Matrix", 1 } } };
    EXPECT_EQ(expectedProcInfo, procInfo);
    auto recInfo = makeConnectedPortInfo(receive);
    ConnectedPortInfo expectedRecInfo { { { "Matrix", 1 } }, empty };
    EXPECT_EQ(expectedRecInfo, recInfo);
  }

  network->disconnect(cid1);
  ASSERT_EQ(1, network->nconnections());

  {
    auto sendInfo = makeConnectedPortInfo(send);
    ConnectedPortInfo expectedSendInfo; // empty maps
    EXPECT_EQ(expectedSendInfo, sendInfo);
    auto procInfo = makeConnectedPortInfo(process);
    ConnectedPortInfo expectedProcInfo { empty, { { "Matrix", 1 } } };
    EXPECT_EQ(expectedProcInfo, procInfo);
    auto recInfo = makeConnectedPortInfo(receive);
    ConnectedPortInfo expectedRecInfo { { { "Matrix", 1 } }, empty };
    EXPECT_EQ(expectedRecInfo, recInfo);
  }

  network->disconnect(cid2);
  ASSERT_EQ(0, network->nconnections());

  {
    auto sendInfo = makeConnectedPortInfo(send);
    ConnectedPortInfo expectedSendInfo; // empty maps
    EXPECT_EQ(expectedSendInfo, sendInfo);
    auto procInfo = makeConnectedPortInfo(process);
    ConnectedPortInfo expectedProcInfo; // empty maps
    EXPECT_EQ(expectedProcInfo, procInfo);
    auto recInfo = makeConnectedPortInfo(receive);
    ConnectedPortInfo expectedRecInfo; // empty maps
    EXPECT_EQ(expectedRecInfo, recInfo);
  }

  ModuleHandle create = controller.addModule("CreateLatVol");
  ModuleHandle setFD = controller.addModule("SetFieldData");
  ModuleHandle report = controller.addModule("ReportFieldInfo");

  cid1 = network->connect(ConnectionOutputPort(create, 0), ConnectionInputPort(setFD, 0));
  cid2 = network->connect(ConnectionOutputPort(process, 0), ConnectionInputPort(setFD, 1));
  auto cid3 = network->connect(ConnectionOutputPort(setFD, 0), ConnectionInputPort(report, 0));
  ASSERT_EQ(3, network->nconnections());

  {
    auto createInfo = makeConnectedPortInfo(create);
    ConnectedPortInfo expectedCreateInfo { empty, { { "Field", 1 } } };
    EXPECT_EQ(expectedCreateInfo, createInfo);
    auto setInfo = makeConnectedPortInfo(setFD);
    ConnectedPortInfo expectedSetInfo { { { "Matrix", 1 }, { "Field", 1 } }, { { "Field", 1 } } };
    EXPECT_EQ(expectedSetInfo, setInfo);
    auto reportInfo = makeConnectedPortInfo(report);
    ConnectedPortInfo expectedReportInfo { { { "Field", 1 } }, empty };
    EXPECT_EQ(expectedReportInfo, reportInfo);
  }
}

TEST_F(ModuleReplaceTests, NoConnectedPortsCanBeReplacedWithAnything)
{
  HardCodedModuleFactory factory;

  auto descMap = factory.getDirectModuleDescriptionLookupMap();

  ModuleReplacementFilterBuilder builder(descMap);
  auto filter = builder.build();

  ConnectedPortInfo noConnections;
  auto noConnectionReplacements = filter->findReplacements(noConnections);
  EXPECT_EQ(descMap.size(), noConnectionReplacements.size());

  for (const auto& p : descMap)
  {
    if (find(noConnectionReplacements.begin(), noConnectionReplacements.end(), p.first) == noConnectionReplacements.end())
      std::cout << "replacements list did not contain " << p.first << std::endl;
  }
}

TEST(ReplacementFilterBuilderTests, CanBuild)
{
  HardCodedModuleFactory factory;

  auto descMap = factory.getDirectModuleDescriptionLookupMap();

  ModuleReplacementFilterBuilder builder(descMap);
  auto filter = builder.build();

  ASSERT_TRUE(filter != nullptr);
}

TEST_F(ModuleReplaceTests, CurrentConnectionsFilterReplacements)
{
  HardCodedModuleFactory factory;
  auto descMap = factory.getDirectModuleDescriptionLookupMap();
  ModuleReplacementFilterBuilder builder(descMap);
  auto filter = builder.build();

  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);
  auto network = controller.getNetwork();
  ModuleHandle send = controller.addModule("CreateMatrix");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReportMatrixInfo");

  ASSERT_EQ(3, network->nmodules());

  auto repl = filter->findReplacements(makeConnectedPortInfo(send));
  EXPECT_EQ(descMap.size(), repl.size()); //everything

  auto cid1 = network->connect(ConnectionOutputPort(send, 0), ConnectionInputPort(process, 0));
  auto cid2 = network->connect(ConnectionOutputPort(process, 0), ConnectionInputPort(receive, 0));
  ASSERT_EQ(2, network->nconnections());

  repl = filter->findReplacements(makeConnectedPortInfo(send));
  EXPECT_LE(repl.size(), descMap.size()); // some modules have been filtered out

  network->disconnect(cid1);
  repl = filter->findReplacements(makeConnectedPortInfo(send));
  EXPECT_EQ(descMap.size(), repl.size()); //everything again
}
