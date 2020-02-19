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


#include <Testing/Utils/SCIRunUnitTests.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Modules/DataIO/ReadMatrixClassic.h>
#include <Modules/DataIO/WriteMatrix.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Modules/Math/ReportMatrixInfo.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Core/Algorithms/DataIO/WriteMatrix.h>
#include <Dataflow/Network/Tests/MockModuleState.h>
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <boost/filesystem.hpp>
#include <Dataflow/Network/Tests/MockNetwork.h>
#include <Modules/Math/CreateMatrix.h>
#include <Core/Datatypes/Tests/MatrixTestCases.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

//TODO: figure out this failure
TEST(ReadWriteMatrixFunctionalTest, DISABLED_ManualExecution)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  Network writeReadMatrixNetwork(mf, sf, af, ReexecuteStrategyFactoryHandle());

  auto send = addModuleToNetwork(writeReadMatrixNetwork, "CreateMatrix");
  auto write = addModuleToNetwork(writeReadMatrixNetwork, "WriteMatrix");
  auto read = addModuleToNetwork(writeReadMatrixNetwork, "ReadMatrix");
  auto receive = addModuleToNetwork(writeReadMatrixNetwork, "ReportMatrixInfo");

  EXPECT_EQ(4, writeReadMatrixNetwork.nmodules());

  writeReadMatrixNetwork.connect(ConnectionOutputPort(send, 0), ConnectionInputPort(write, 1));
  EXPECT_EQ(1, writeReadMatrixNetwork.nconnections());
  writeReadMatrixNetwork.connect(ConnectionOutputPort(read, 1), ConnectionInputPort(receive, 0));
  EXPECT_EQ(2, writeReadMatrixNetwork.nconnections());

  send->get_state()->setValue(Core::Algorithms::Math::Parameters::TextEntry, matrix1str());

  auto filename = TestResources::rootDir() / "moduleTestMatrix.txt";
  boost::filesystem::remove(filename);

  write->get_state()->setValue(Variables::Filename, filename.string());
  auto writeModule = dynamic_cast<WriteMatrix*>(write.get());
  ASSERT_TRUE(writeModule != nullptr);
  read->get_state()->setValue(Variables::Filename, filename.string());
  auto readModule = dynamic_cast<ReadMatrix*>(read.get());
  ASSERT_TRUE(readModule != nullptr);

  //manually execute the network, in the correct order.
  send->execute();
  write->execute();
  read->execute();
  receive->execute();

  auto receiveModule = dynamic_cast<ReportMatrixInfo*>(receive.get());
  ASSERT_TRUE(receiveModule != nullptr);
  FAIL() << "test needs rewrite";
  #if 0
  ASSERT_TRUE(receiveModule->latestReceivedMatrix().get() != nullptr);

  EXPECT_EQ(*input, *receiveModule->latestReceivedMatrix());
  #endif
}
