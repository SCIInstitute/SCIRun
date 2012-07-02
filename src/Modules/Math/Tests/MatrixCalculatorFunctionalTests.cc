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
#include <Core/Dataflow/Network/HardCodedModuleFactory.h>
#include <Core/Dataflow/Network/ModuleInterface.h>
#include <Core/Dataflow/Network/ConnectionId.h>
#include <Core/Dataflow/Network/Tests/MockNetwork.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Modules/Basic/SendTestMatrix.h>
#include <Modules/Basic/ReceiveTestMatrix.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Algorithms/Math/EvaluateLinearAlgebraUnary.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Domain::Datatypes;
using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Domain::Networks::Mocks;
using namespace SCIRun::Algorithms::Math;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

//TODO DAN

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
}

TEST(EvaluateLinearAlgebraUnaryFunctionalTest, CanExecuteManuallyWithChoiceOfOperation)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  Network matrixUnaryNetwork(mf);

  ModuleLookupInfo sendTestMatrixInfo;
  sendTestMatrixInfo.module_name_ = "SendTestMatrix";
  ModuleHandle send = matrixUnaryNetwork.add_module(sendTestMatrixInfo);

  ModuleLookupInfo processMatrixInfo;
  processMatrixInfo.module_name_ = "EvaluateLinearAlgebraUnary";
  ModuleHandle process = matrixUnaryNetwork.add_module(processMatrixInfo);

  ModuleLookupInfo receiveTestMatrixInfo;
  receiveTestMatrixInfo.module_name_ = "ReceiveTestMatrix";
  ModuleHandle receive = matrixUnaryNetwork.add_module(receiveTestMatrixInfo);

  EXPECT_EQ(3, matrixUnaryNetwork.nmodules());

  matrixUnaryNetwork.connect(send, 0, process, 0);
  EXPECT_EQ(1, matrixUnaryNetwork.nconnections());
  matrixUnaryNetwork.connect(process, 0, receive, 0);
  EXPECT_EQ(2, matrixUnaryNetwork.nconnections());

  SendTestMatrixModule* sendModule = dynamic_cast<SendTestMatrixModule*>(send.get());
  ASSERT_TRUE(sendModule != 0);
  EvaluateLinearAlgebraUnaryModule* evalModule = dynamic_cast<EvaluateLinearAlgebraUnaryModule*>(process.get());
  ASSERT_TRUE(evalModule != 0);

  DenseMatrixHandle input = matrix1();
  sendModule->setMatrix(input);

  //evalModule->set_operation(EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);
  //manually execute the network, in the correct order.
  send->execute();
  process->execute();
  receive->execute();

  ReceiveTestMatrixModule* receiveModule = dynamic_cast<ReceiveTestMatrixModule*>(receive.get());
  ASSERT_TRUE(receiveModule != 0);
  ASSERT_TRUE(receiveModule->latestReceivedMatrix());

  EXPECT_EQ(-*input, *receiveModule->latestReceivedMatrix());

  //evalModule->set_operation(EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE);
  process->execute();
  receive->execute();
  EXPECT_EQ(transpose(*input), *receiveModule->latestReceivedMatrix());

  //evalModule->set_operation(EvaluateLinearAlgebraUnaryAlgorithm::Parameters(EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY, 2.0));
  process->execute();
  receive->execute();
  EXPECT_EQ(2.0 * *input, *receiveModule->latestReceivedMatrix());
}


TEST(MatrixCalculatorFunctionalTest, ManualExecutionOfMultiNodeNetwork)
{
  std::cout << "m1" << std::endl;
  std::cout << *matrix1() << std::endl;
  std::cout << "m2" << std::endl;
  std::cout << *matrix2() << std::endl;
  std::cout << "(-m1 * 4m2) + trans(m1)" << std::endl;
  std::cout << (-*matrix1()) * (4* *matrix2()) + transpose(*matrix1()) << std::endl;

  //Test network:
  /*
  send m1             send m2
  |         |         |
  transpose negate    scalar mult *4
  |         |         |
  |           multiply
  |           |
        add
        |
        report
  */







}