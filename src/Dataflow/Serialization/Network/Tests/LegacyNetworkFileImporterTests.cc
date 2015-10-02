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

TEST(LegacyNetworkFileImporterTests, CanLoadEmptyNetworkFile)
{
  auto dtdpath = TestResources::rootDir() / "Other";
  LegacyNetworkIO lnio(dtdpath.string());;
  auto v4file1 = TestResources::rootDir() / "Other" / "v4nets" / "empty.srn";
  auto networkFile = lnio.load_net(v4file1.string());
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
  LegacyNetworkIO lnio(dtdpath.string());
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
  EXPECT_EQ(std::make_pair(289.0,151.0), networkFile->modulePositions.modulePositions.begin()->second);

  EXPECT_EQ(0, networkFile->moduleNotes.notes.size());
  EXPECT_EQ(0, networkFile->connectionNotes.notes.size());
  EXPECT_EQ(0, networkFile->moduleTags.tags.size());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithSingleModuleWithState)
{
  FAIL() << "todo";
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithTwoModulesNoConnections)
{
  auto dtdpath = TestResources::rootDir() / "Other";
  LegacyNetworkIO lnio(dtdpath.string());
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
  EXPECT_EQ(std::make_pair(357.0,134.0), posIter->second); ++posIter;
  EXPECT_EQ(std::make_pair(304.0,258.0), posIter->second); ++posIter;
  EXPECT_EQ(std::make_pair(386.0,376.0), posIter->second);

  EXPECT_EQ(0, networkFile->moduleNotes.notes.size());
  EXPECT_EQ(0, networkFile->connectionNotes.notes.size());
  EXPECT_EQ(0, networkFile->moduleTags.tags.size());
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithTwoModulesOneConnection)
{
  auto dtdpath = TestResources::rootDir() / "Other";
  LegacyNetworkIO lnio(dtdpath.string());
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

  FAIL() << "todo";
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithThreeModulesSameType)
{
  auto dtdpath = TestResources::rootDir() / "Other";
  LegacyNetworkIO lnio(dtdpath.string());
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

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithTwoModulesTwoConnections)
{
  FAIL() << "todo";
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithLotsOfObjects)
{
  FAIL() << "todo";
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithModuleNotes)
{
  FAIL() << "todo";
}

TEST(LegacyNetworkFileImporterTests, CanLoadNetworkFileWithConnectionNotes)
{
  FAIL() << "todo";
}
