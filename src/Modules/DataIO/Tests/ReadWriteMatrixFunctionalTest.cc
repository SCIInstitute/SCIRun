/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Core/Dataflow/Network/Network.h>
#include <Core/Dataflow/Network/ModuleInterface.h>
#include <Core/Dataflow/Network/ModuleStateInterface.h>
#include <Core/Dataflow/Network/ConnectionId.h>
#include <Core/Dataflow/Network/Tests/MockNetwork.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Modules/DataIO/ReadMatrix.h>
#include <Modules/DataIO/WriteMatrix.h>
#include <Modules/Basic/SendTestMatrix.h>
#include <Modules/Basic/ReceiveTestMatrix.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Algorithms/Math/EvaluateLinearAlgebraUnary.h>
#include <Algorithms/Math/EvaluateLinearAlgebraBinary.h>
#include <Algorithms/Math/ReportMatrixInfo.h>
#include <Algorithms/DataIO/WriteMatrix.h>
#include <Core/Dataflow/Network/Tests/MockModuleState.h>
#include <Engine/State/SimpleMapModuleState.h>
#include <boost/filesystem.hpp>

using namespace SCIRun;
using namespace SCIRun::Algorithms::DataIO;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Domain::Datatypes;
using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Domain::Networks::Mocks;
using namespace SCIRun::Algorithms::Math;
using namespace SCIRun::Domain::State;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

namespace
{
  DenseMatrixHandle matrix1()
  {
    DenseMatrixHandle m(new DenseMatrix(3, 3));
    for (size_t i = 0; i < m->nrows(); ++i)
      for (size_t j = 0; j < m->ncols(); ++j)
        (*m)(i, j) = 3.0 * i + j;
    return m;
  }
  DenseMatrixHandle matrix2()
  {
    DenseMatrixHandle m(new DenseMatrix(3, 3));
    for (size_t i = 0; i < m->nrows(); ++i)
      for (size_t j = 0; j < m->ncols(); ++j)
        (*m)(i, j) = -2.0 * i + j;
    return m;
  }
  const DenseMatrix Zero(DenseMatrix::zero_matrix(3,3));

  ModuleHandle addModuleToNetwork(Network& network, const std::string& moduleName)
  {
    ModuleLookupInfo info;
    info.module_name_ = moduleName;
    return network.add_module(info);
  }
}

TEST(ReadWriteMatrixFunctionalTest, ManualExecution)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  Network writeReadMatrixNetwork(mf, sf);

  ModuleHandle send = addModuleToNetwork(writeReadMatrixNetwork, "SendTestMatrix");
  ModuleHandle write = addModuleToNetwork(writeReadMatrixNetwork, "WriteMatrix");
  ModuleHandle read = addModuleToNetwork(writeReadMatrixNetwork, "ReadMatrix");
  ModuleHandle receive = addModuleToNetwork(writeReadMatrixNetwork, "ReceiveTestMatrix");

  EXPECT_EQ(4, writeReadMatrixNetwork.nmodules());

  writeReadMatrixNetwork.connect(send, 0, write, 0);
  EXPECT_EQ(1, writeReadMatrixNetwork.nconnections());
  writeReadMatrixNetwork.connect(read, 0, receive, 0);
  EXPECT_EQ(2, writeReadMatrixNetwork.nconnections());

  DenseMatrixHandle input = matrix1();
  send->get_state()->setTransientValue("MatrixToSend", input);

  const std::string filename = "E:\\git\\SCIRunGUIPrototype\\src\\Samples\\moduleTestMatrix.txt";
  boost::filesystem3::remove(filename);

  write->get_state()->setValue(WriteMatrixAlgorithm::Filename, filename);
  WriteMatrixModule* writeModule = dynamic_cast<WriteMatrixModule*>(write.get());
  ASSERT_TRUE(writeModule != 0);
  read->get_state()->setValue(WriteMatrixAlgorithm::Filename, filename);
  ReadMatrixModule* readModule = dynamic_cast<ReadMatrixModule*>(read.get());
  ASSERT_TRUE(readModule != 0);

  //manually execute the network, in the correct order.
  send->execute();
  write->execute();
  read->execute();
  receive->execute();

  ReceiveTestMatrixModule* receiveModule = dynamic_cast<ReceiveTestMatrixModule*>(receive.get());
  ASSERT_TRUE(receiveModule != 0);
  ASSERT_TRUE(receiveModule->latestReceivedMatrix());

  EXPECT_EQ(*input, *receiveModule->latestReceivedMatrix());
}