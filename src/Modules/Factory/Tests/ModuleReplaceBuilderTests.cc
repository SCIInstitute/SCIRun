/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Network/ConnectionId.h>

using namespace SCIRun;
using namespace Testing;
using namespace Modules::Factory;
using namespace Dataflow::Networks;
using namespace ReplacementImpl;
using namespace Dataflow::Engine;
using namespace Core::Algorithms;

class ModuleReplaceTests : public ModuleTest
{
};

TEST(HardCodedModuleFactoryTests, ListAllModules)
{
  HardCodedModuleFactory factory;

  auto descMap = factory.getDirectModuleDescriptionLookupMap();

  EXPECT_GE(descMap.size(), 105);
}

TEST_F(ModuleReplaceTests, CanComputeConnectedPortInfoFromModule)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  auto network = controller.getNetwork();

  ModuleHandle send = controller.addModule("SendTestMatrix");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReceiveTestMatrix");

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
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);
  auto network = controller.getNetwork();
  ModuleHandle send = controller.addModule("SendTestMatrix");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReceiveTestMatrix");

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
