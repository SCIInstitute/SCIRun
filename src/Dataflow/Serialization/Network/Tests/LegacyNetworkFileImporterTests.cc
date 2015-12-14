/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Dataflow/Serialization/Network/Importer/NetworkIO.h>
#include <Dataflow/Serialization/Network/ModuleDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/NetworkXMLSerializer.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Dataflow/Network/Tests/MockNetwork.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Modules/Basic/SendTestMatrix.h>
#include <Modules/Basic/ReceiveTestMatrix.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Dataflow/Network/Tests/MockModuleState.h>
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Engine/Scheduler/DesktopExecutionStrategyFactory.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Testing/Utils/SCIRunUnitTests.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;

#include <stdexcept>
#include <fstream>
#include <boost/assign.hpp>

using namespace SCIRun::Dataflow::Networks;
using namespace boost::assign;

namespace
{
  HardCodedModuleFactory mf;
  std::ostringstream dummyLog;
  NetworkFileHandle load(const std::string& file)
  {
    auto dtdpath = TestResources::rootDir() / "Other";
    LegacyNetworkIO lnio(dtdpath.string(), mf, dummyLog);
    auto v4file1 = TestResources::rootDir() / "Other" / "v4nets" / file;
    return lnio.load_net(v4file1.string());
  }
}

TEST(LegacyNetworkFileImporterTests, CanLoadEmptyNetworkFile)
{
  auto networkFile = load("empty.srn");
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(0, networkFile->network.modules.size());
  EXPECT_EQ(0, networkFile->network.connections.size());
  EXPECT_EQ(0, networkFile->modulePositions.modulePositions.size());
  EXPECT_EQ(0, networkFile->moduleNotes.notes.size());
  EXPECT_EQ(0, networkFile->connectionNotes.notes.size());
  EXPECT_EQ(0, networkFile->moduleTags.tags.size());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithSingleModuleNoState)
{
  auto dtdpath = TestResources::rootDir() / "Other";
  LegacyNetworkIO lnio(dtdpath.string(), mf, dummyLog);
  auto v4file1 = TestResources::rootDir() / "Other" / "v4nets" / "oneModule.srn";
  auto networkFile = lnio.load_net(v4file1.string());
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(1, networkFile->network.modules.size());
  auto mod = *networkFile->network.modules.begin();
  EXPECT_EQ("CreateLatVol:0", mod.first);
  EXPECT_EQ("CreateLatVol", mod.second.module.module_name_);
  EXPECT_EQ("NewField", mod.second.module.category_name_);
  EXPECT_EQ("SCIRun", mod.second.module.package_name_);

  EXPECT_EQ(0, networkFile->network.connections.size());

  EXPECT_EQ(1, networkFile->modulePositions.modulePositions.size());
  EXPECT_EQ(std::make_pair(433.5,226.5), networkFile->modulePositions.modulePositions.begin()->second);

  EXPECT_EQ(0, networkFile->moduleNotes.notes.size());
  EXPECT_EQ(0, networkFile->connectionNotes.notes.size());
  EXPECT_EQ(0, networkFile->moduleTags.tags.size());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithSingleModuleWithState)
{
  auto networkFile = load("clvState.srn");
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(3, networkFile->network.modules.size());

  auto mod = networkFile->network.modules.begin();
  EXPECT_EQ("CreateLatVol:0", mod->first);
  EXPECT_TRUE(mod->second.state.containsKey(Name("XSize")));
  EXPECT_TRUE(mod->second.state.containsKey(Name("PadPercent")));
  EXPECT_EQ(2, mod->second.state.getValue(Name("XSize")).toInt());
  EXPECT_EQ(3, mod->second.state.getValue(Name("YSize")).toInt());
  EXPECT_EQ(4, mod->second.state.getValue(Name("ZSize")).toInt());
  EXPECT_EQ(0.0, mod->second.state.getValue(Name("PadPercent")).toDouble());
  EXPECT_EQ(0, mod->second.state.getValue(Name("DataAtLocation")).toInt());
  EXPECT_EQ(0, mod->second.state.getValue(Name("ElementSizeNormalized")).toInt());
  ++mod;
  EXPECT_EQ("CreateLatVol:1", mod->first);
  EXPECT_EQ(10, mod->second.state.getValue(Name("XSize")).toInt());
  EXPECT_EQ(10, mod->second.state.getValue(Name("YSize")).toInt());
  EXPECT_EQ(10, mod->second.state.getValue(Name("ZSize")).toInt());
  EXPECT_EQ(0.1, mod->second.state.getValue(Name("PadPercent")).toDouble());
  EXPECT_EQ(0, mod->second.state.getValue(Name("DataAtLocation")).toInt());
  EXPECT_EQ(0, mod->second.state.getValue(Name("ElementSizeNormalized")).toInt());
  ++mod;
  EXPECT_EQ("CreateLatVol:2", mod->first);
  EXPECT_EQ(10, mod->second.state.getValue(Name("XSize")).toInt());
  EXPECT_EQ(10, mod->second.state.getValue(Name("YSize")).toInt());
  EXPECT_EQ(10, mod->second.state.getValue(Name("ZSize")).toInt());
  EXPECT_EQ(0.1, mod->second.state.getValue(Name("PadPercent")).toDouble());
  EXPECT_EQ(1, mod->second.state.getValue(Name("DataAtLocation")).toInt());
  EXPECT_EQ(1, mod->second.state.getValue(Name("ElementSizeNormalized")).toInt());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithTwoModulesNoConnections)
{
  auto dtdpath = TestResources::rootDir() / "Other";
  LegacyNetworkIO lnio(dtdpath.string(), mf, dummyLog);
  auto v4file1 = TestResources::rootDir() / "Other" / "v4nets" / "threeModulesNoConnections.srn";
  auto networkFile = lnio.load_net(v4file1.string());
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(3, networkFile->network.modules.size());
  auto modIter = networkFile->network.modules.begin();
  EXPECT_EQ("ComputeSVD:0", modIter->first);
  EXPECT_EQ("ComputeSVD", modIter->second.module.module_name_);
  EXPECT_EQ("Math", modIter->second.module.category_name_);
  EXPECT_EQ("SCIRun", modIter->second.module.package_name_);
  ++modIter;
  EXPECT_EQ("CreateLatVol:0", modIter->first);
  EXPECT_EQ("CreateLatVol", modIter->second.module.module_name_);
  EXPECT_EQ("NewField", modIter->second.module.category_name_);
  EXPECT_EQ("SCIRun", modIter->second.module.package_name_);
  ++modIter;
  EXPECT_EQ("ShowField:0", modIter->first);
  EXPECT_EQ("ShowField", modIter->second.module.module_name_);
  EXPECT_EQ("Visualization", modIter->second.module.category_name_);
  EXPECT_EQ("SCIRun", modIter->second.module.package_name_);

  EXPECT_EQ(0, networkFile->network.connections.size());

  EXPECT_EQ(3, networkFile->modulePositions.modulePositions.size());
  auto posIter = networkFile->modulePositions.modulePositions.begin();
  EXPECT_EQ(std::make_pair(535.5,201.0), posIter->second); ++posIter;
  EXPECT_EQ(std::make_pair(456.0,387.0), posIter->second); ++posIter;
  EXPECT_EQ(std::make_pair(579.0,564.0), posIter->second);

  EXPECT_EQ(0, networkFile->moduleNotes.notes.size());
  EXPECT_EQ(0, networkFile->connectionNotes.notes.size());
  EXPECT_EQ(0, networkFile->moduleTags.tags.size());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithTwoModulesOneConnection)
{
  auto dtdpath = TestResources::rootDir() / "Other";
  LegacyNetworkIO lnio(dtdpath.string(), mf, dummyLog);
  auto v4file1 = TestResources::rootDir() / "Other" / "v4nets" / "twoModsOneConnection.srn";
  auto networkFile = lnio.load_net(v4file1.string());
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(2, networkFile->network.modules.size());
  auto modIter = networkFile->network.modules.begin();
  EXPECT_EQ("ComputeSVD:0", modIter->first);
  EXPECT_EQ("ComputeSVD", modIter->second.module.module_name_);
  EXPECT_EQ("Math", modIter->second.module.category_name_);
  EXPECT_EQ("SCIRun", modIter->second.module.package_name_);
  ++modIter;
  EXPECT_EQ("CreateLatVol:0", modIter->first);
  EXPECT_EQ("CreateLatVol", modIter->second.module.module_name_);
  EXPECT_EQ("NewField", modIter->second.module.category_name_);
  EXPECT_EQ("SCIRun", modIter->second.module.package_name_);

  ASSERT_EQ(1, networkFile->network.connections.size());
  auto conn = networkFile->network.connections[0];
  std::cout << "CONN LOOKS LIKE " << static_cast<std::string>(ConnectionId::create(conn)) << std::endl;

  EXPECT_EQ(2, networkFile->modulePositions.modulePositions.size());

  EXPECT_EQ(0, networkFile->moduleNotes.notes.size());
  EXPECT_EQ(0, networkFile->connectionNotes.notes.size());
  EXPECT_EQ(0, networkFile->moduleTags.tags.size());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithThreeModulesSameType)
{
  auto dtdpath = TestResources::rootDir() / "Other";
  LegacyNetworkIO lnio(dtdpath.string(), mf, dummyLog);
  auto v4file1 = TestResources::rootDir() / "Other" / "v4nets" / "threeSameModulesIDTest.srn";
  auto networkFile = lnio.load_net(v4file1.string());
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(3, networkFile->network.modules.size());
  auto modIter = networkFile->network.modules.begin();
  EXPECT_EQ("CreateLatVol:0", modIter->first);
  EXPECT_EQ("CreateLatVol", modIter->second.module.module_name_);
  EXPECT_EQ("NewField", modIter->second.module.category_name_);
  EXPECT_EQ("SCIRun", modIter->second.module.package_name_);
  ++modIter;
  EXPECT_EQ("CreateLatVol:1", modIter->first);
  EXPECT_EQ("CreateLatVol", modIter->second.module.module_name_);
  EXPECT_EQ("NewField", modIter->second.module.category_name_);
  EXPECT_EQ("SCIRun", modIter->second.module.package_name_);
  ++modIter;
  EXPECT_EQ("CreateLatVol:2", modIter->first);
  EXPECT_EQ("CreateLatVol", modIter->second.module.module_name_);
  EXPECT_EQ("NewField", modIter->second.module.category_name_);
  EXPECT_EQ("SCIRun", modIter->second.module.package_name_);

  EXPECT_EQ(0, networkFile->network.connections.size());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithMultipleConnections)
{
  auto dtdpath = TestResources::rootDir() / "Other";
  LegacyNetworkIO lnio(dtdpath.string(), mf, dummyLog);
  auto v4file1 = TestResources::rootDir() / "Other" / "v4nets" / "multiConnections.srn";
  auto networkFile = lnio.load_net(v4file1.string());
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(5, networkFile->network.modules.size());

  EXPECT_EQ(4, networkFile->network.connections.size());
  std::string expected[] = {
  "ReadMatrix:0_p#Matrix:0#_@to@_ComputeSVD:0_p#InputMatrix:0#",
  "FairMesh:0_p#Faired_Mesh:0#_@to@_BuildFEMatrix:0_p#InputField:0#",
  "FairMesh:0_p#Faired_Mesh:0#_@to@_ReportFieldInfo:0_p#InputField:0#",
  "ComputeSVD:0_p#SingularValues:0#_@to@_BuildFEMatrix:0_p#Conductivity_Table:0#" };
  std::string* expectedPtr = expected;
  for (const auto& c : networkFile->network.connections)
  {
    std::cout << static_cast<std::string>(ConnectionId::create(c)) << std::endl;
    EXPECT_EQ(static_cast<std::string>(ConnectionId::create(c)), *expectedPtr++);
  }
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithRenamedModules)
{
  auto networkFile = load("forward_problem.srn");
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(30, networkFile->network.modules.size());
  EXPECT_EQ(39, networkFile->network.connections.size());
  EXPECT_EQ(30, networkFile->modulePositions.modulePositions.size());
  EXPECT_EQ(6, networkFile->moduleNotes.notes.size());
  EXPECT_EQ(0, networkFile->connectionNotes.notes.size());
  EXPECT_EQ(0, networkFile->moduleTags.tags.size());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithDynamicPorts)
{
  auto networkFile = load("dynamicPorts.srn");
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(2, networkFile->network.modules.size());
  EXPECT_EQ(3, networkFile->network.connections.size());
  EXPECT_EQ(2, networkFile->modulePositions.modulePositions.size());
  EXPECT_EQ(0, networkFile->moduleNotes.notes.size());
  EXPECT_EQ(0, networkFile->connectionNotes.notes.size());
  EXPECT_EQ(0, networkFile->moduleTags.tags.size());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithLotsOfState)
{
  FAIL() << "todo";
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithModuleNotes)
{
  auto networkFile = load("notes.srn");
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(2, networkFile->network.modules.size());
  EXPECT_EQ(1, networkFile->network.connections.size());
  EXPECT_EQ(2, networkFile->modulePositions.modulePositions.size());
  EXPECT_EQ(2, networkFile->moduleNotes.notes.size());
  EXPECT_EQ(1, networkFile->connectionNotes.notes.size());
  EXPECT_EQ(0, networkFile->moduleTags.tags.size());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithConnectionNotes)
{
  auto networkFile = load("notes.srn");
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(2, networkFile->network.modules.size());
  EXPECT_EQ(1, networkFile->network.connections.size());
  EXPECT_EQ(2, networkFile->modulePositions.modulePositions.size());
  EXPECT_EQ(2, networkFile->moduleNotes.notes.size());
  EXPECT_EQ(1, networkFile->connectionNotes.notes.size());
  EXPECT_EQ(0, networkFile->moduleTags.tags.size());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithModuleNotesInFivePositions)
{
  auto networkFile = load("notePositions.srn");
  ASSERT_TRUE(networkFile != nullptr);

  EXPECT_EQ(5, networkFile->network.modules.size());
  EXPECT_EQ(0, networkFile->network.connections.size());
  EXPECT_EQ(5, networkFile->modulePositions.modulePositions.size());
  EXPECT_EQ(5, networkFile->moduleNotes.notes.size());
  EXPECT_EQ(0, networkFile->connectionNotes.notes.size());
  EXPECT_EQ(0, networkFile->moduleTags.tags.size());

  const auto& moduleNotesMap = networkFile->moduleNotes.notes;
  /*
  enum NotePosition
  {
    Default,  0
    None,     1
    Tooltip,  2
    Top,      3
    Left,     4
    Right,    5
    Bottom    6
  };
  */
  auto noteIter = moduleNotesMap.begin();
  EXPECT_TRUE(noteIter->second.noteText.find("Left") != std::string::npos);
  EXPECT_EQ(4, noteIter->second.position);
  ++noteIter;
  EXPECT_TRUE(noteIter->second.noteText.find("Right") != std::string::npos);
  EXPECT_EQ(5, noteIter->second.position);
  ++noteIter;
  EXPECT_TRUE(noteIter->second.noteText.find("Top") != std::string::npos);
  EXPECT_EQ(3, noteIter->second.position);
  ++noteIter;
  EXPECT_TRUE(noteIter->second.noteText.find("Bottom") != std::string::npos);
  EXPECT_EQ(6, noteIter->second.position);
  ++noteIter;
  EXPECT_TRUE(noteIter->second.noteText.find("tooltip") != std::string::npos);
  EXPECT_EQ(2, noteIter->second.position);
  ++noteIter;
  EXPECT_TRUE(noteIter == moduleNotesMap.end());
}
