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
#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Dataflow/Network/Tests/MockModuleState.h>
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Algorithms;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

namespace
{
  DenseMatrixHandle matrix1()
  {
    DenseMatrixHandle m(new DenseMatrix(3, 3));
    for (int i = 0; i < m->rows(); ++i)
      for (int j = 0; j < m->cols(); ++j)
        (*m)(i, j) = 3.0 * i + j;
    return m;
  }
  DenseMatrixHandle matrix2()
  {
    DenseMatrixHandle m(new DenseMatrix(3, 3));
    for (int i = 0; i < m->rows(); ++i)
      for (int j = 0; j < m->cols(); ++j)
        (*m)(i, j) = -2.0 * i + j;
    return m;
  }
  const DenseMatrix Zero(DenseMatrix::Zero(3,3));

  ModuleHandle addModuleToNetwork(Network& network, const std::string& moduleName)
  {
    ModuleLookupInfo info;
    info.module_name_ = moduleName;
    return network.add_module(info);
  }
}

TEST(EvaluateLinearAlgebraUnaryFunctionalTest, CanExecuteManuallyWithChoiceOfOperation)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  Network matrixUnaryNetwork(mf, sf, af, ReexecuteStrategyFactoryHandle());

  ModuleHandle send = addModuleToNetwork(matrixUnaryNetwork, "SendTestMatrix");
  ModuleHandle process = addModuleToNetwork(matrixUnaryNetwork, "EvaluateLinearAlgebraUnary");
  ModuleHandle receive = addModuleToNetwork(matrixUnaryNetwork, "ReceiveTestMatrix");

  EXPECT_EQ(3, matrixUnaryNetwork.nmodules());

  matrixUnaryNetwork.connect(ConnectionOutputPort(send, 0), ConnectionInputPort(process, 0));
  EXPECT_EQ(1, matrixUnaryNetwork.nconnections());
  matrixUnaryNetwork.connect(ConnectionOutputPort(process, 0), ConnectionInputPort(receive, 0));
  EXPECT_EQ(2, matrixUnaryNetwork.nconnections());

  SendTestMatrixModule* sendModule = dynamic_cast<SendTestMatrixModule*>(send.get());
  ASSERT_TRUE(sendModule != nullptr);
  EvaluateLinearAlgebraUnaryModule* evalModule = dynamic_cast<EvaluateLinearAlgebraUnaryModule*>(process.get());
  ASSERT_TRUE(evalModule != nullptr);

  DenseMatrixHandle input = matrix1();
  sendModule->get_state()->setTransientValue("MatrixToSend", input);

  process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);
  //manually execute the network, in the correct order.
  send->execute();
  process->execute();
  receive->execute();

  ReceiveTestMatrixModule* receiveModule = dynamic_cast<ReceiveTestMatrixModule*>(receive.get());
  ASSERT_TRUE(receiveModule != nullptr);
  ASSERT_TRUE(receiveModule->latestReceivedMatrix().get() != nullptr);

  EXPECT_EQ(-*input, *receiveModule->latestReceivedMatrix());

  send->execute();
  process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE);
  process->execute();
  receive->execute();
  EXPECT_EQ(input->transpose(), *receiveModule->latestReceivedMatrix());

  send->execute();
  process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY);
  process->get_state()->setValue(Variables::ScalarValue, 2.0);
  process->execute();
  receive->execute();
  EXPECT_EQ(2.0 * *input, *receiveModule->latestReceivedMatrix());
}


TEST(MatrixCalculatorFunctionalTest, ManualExecutionOfMultiNodeNetwork)
{
  DenseMatrix expected = (-*matrix1()) * (4* *matrix2()) + matrix1()->transpose();

  //Test network:
  /* 
  send m1             send m2
  |         |         |
  transpose negate    scalar mult *4
  |         |         |
  |           multiply
  |           |
        add
        |      |
        report receive
  */

  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  Network matrixMathNetwork(mf, sf, af, ReexecuteStrategyFactoryHandle());
  ModuleHandle matrix1Send = addModuleToNetwork(matrixMathNetwork, "SendTestMatrix");
  ModuleHandle matrix2Send = addModuleToNetwork(matrixMathNetwork, "SendTestMatrix");
  
  ModuleHandle transpose = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraUnary");
  ModuleHandle negate = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraUnary");
  ModuleHandle scalar = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraUnary");

  ModuleHandle multiply = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraBinary");
  ModuleHandle add = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraBinary");

  ModuleHandle report = addModuleToNetwork(matrixMathNetwork, "ReportMatrixInfo");
  ModuleHandle receive = addModuleToNetwork(matrixMathNetwork, "ReceiveTestMatrix");
  
  EXPECT_EQ(9, matrixMathNetwork.nmodules());

  /// @todo: turn this into a convenience network printing function
  //for (size_t i = 0; i < matrixMathNetwork.nmodules(); ++i)
  //{
  //  ModuleHandle m = matrixMathNetwork.module(i);
  //  std::cout << m->get_module_name() << std::endl;
  //  std::cout << "inputs: " << m->num_input_ports() << std::endl;
  //  std::cout << "outputs: " << m->num_output_ports() << std::endl;
  //  std::cout << "has ui: " << m->has_ui() << std::endl;
  //}

  EXPECT_EQ(0, matrixMathNetwork.nconnections());
  matrixMathNetwork.connect(ConnectionOutputPort(matrix1Send, 0), ConnectionInputPort(transpose, 0));
  matrixMathNetwork.connect(ConnectionOutputPort(matrix1Send, 0), ConnectionInputPort(negate, 0));
  matrixMathNetwork.connect(ConnectionOutputPort(matrix2Send, 0), ConnectionInputPort(scalar, 0));
  matrixMathNetwork.connect(ConnectionOutputPort(negate, 0), ConnectionInputPort(multiply, 0));
  matrixMathNetwork.connect(ConnectionOutputPort(scalar, 0), ConnectionInputPort(multiply, 1));
  matrixMathNetwork.connect(ConnectionOutputPort(transpose, 0), ConnectionInputPort(add, 0));
  matrixMathNetwork.connect(ConnectionOutputPort(multiply, 0), ConnectionInputPort(add, 1));
  matrixMathNetwork.connect(ConnectionOutputPort(add, 0), ConnectionInputPort(report, 0));
  matrixMathNetwork.connect(ConnectionOutputPort(add, 0), ConnectionInputPort(receive, 0));
  EXPECT_EQ(9, matrixMathNetwork.nconnections());

  //Set module parameters.
  matrix1Send->get_state()->setTransientValue("MatrixToSend", matrix1());
  matrix2Send->get_state()->setTransientValue("MatrixToSend", matrix2());
  transpose->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE);
  negate->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);
  scalar->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY);
  scalar->get_state()->setValue(Variables::ScalarValue, 4.0);
  multiply->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraBinaryAlgorithm::MULTIPLY);
  add->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraBinaryAlgorithm::ADD);
  
  //execute all manually, in order
  matrix1Send->execute();
  matrix2Send->execute();
  transpose->execute();
  scalar->execute();
  negate->execute();
  multiply->execute();
  add->execute();
  report->execute();
  receive->execute();

  //grab reporting module state
  ReportMatrixInfoAlgorithm::Outputs reportOutput = transient_value_cast<ReportMatrixInfoAlgorithm::Outputs>(report->get_state()->getTransientValue("ReportedInfo"));
  DenseMatrixHandle receivedMatrix = transient_value_cast<DenseMatrixHandle>(receive->get_state()->getTransientValue("ReceivedMatrix"));

  ASSERT_TRUE(receivedMatrix.get() != nullptr);
  //verify results
  EXPECT_EQ(expected, *receivedMatrix);
  EXPECT_EQ(3, reportOutput.get<1>());
  EXPECT_EQ(3, reportOutput.get<2>());
  EXPECT_EQ(9, reportOutput.get<3>());
  EXPECT_EQ(22, reportOutput.get<4>());
  EXPECT_EQ(186, reportOutput.get<5>());
}