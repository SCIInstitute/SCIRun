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
#include <Modules/Basic/NeedToExecuteTester.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Dataflow/Network/Tests/MockModuleState.h>
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Network/SimpleSourceSink.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
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
}

namespace Testing
{
  class MockModuleReexecutionStrategy : public ModuleReexecutionStrategy
  {
  public:
    MOCK_CONST_METHOD0(needToExecute, bool());
  };

  typedef boost::shared_ptr<MockModuleReexecutionStrategy> MockModuleReexecutionStrategyPtr;

  class MockInputsChangedChecker : public InputsChangedChecker
  {
  public:
    MOCK_CONST_METHOD0(inputsChanged, bool());
  };

  typedef boost::shared_ptr<MockInputsChangedChecker> MockInputsChangedCheckerPtr;

  class MockStateChangedChecker : public StateChangedChecker
  {
  public:
    MOCK_CONST_METHOD0(newStatePresent, bool());
  };

  typedef boost::shared_ptr<MockStateChangedChecker> MockStateChangedCheckerPtr;

  class MockOutputPortsCachedChecker : public OutputPortsCachedChecker
  {
  public:
    MOCK_CONST_METHOD0(outputPortsCached, bool());
  };

  typedef boost::shared_ptr<MockOutputPortsCachedChecker> MockOutputPortsCachedCheckerPtr;

}

#if GTEST_HAS_COMBINE

using ::testing::Bool;
using ::testing::Values;
using ::testing::Combine;

class PortCachingUnitTest : public ::testing::TestWithParam < ::std::tr1::tuple<bool, bool> >
{
public:
  PortCachingUnitTest() :
    portCaching_(::std::tr1::get<0>(GetParam())),
    needToExecute_(::std::tr1::get<1>(GetParam()))
  {
  }
protected:
  bool portCaching_, needToExecute_;
};

INSTANTIATE_TEST_CASE_P(
  PortCachingUnitTestParameterized,
  PortCachingUnitTest,
  Combine(Bool(), Bool())
  );

TEST_P(PortCachingUnitTest, TestWithMockReexecute)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  NetworkEditorController controller(mf, sf, nullptr, af, nullptr, nullptr);

  auto network = controller.getNetwork();

  ModuleHandle send = controller.addModule("SendTestMatrix");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReceiveTestMatrix");

  EXPECT_EQ(3, network->nmodules());

  network->connect(ConnectionOutputPort(send, 0), ConnectionInputPort(process, 0));
  network->connect(ConnectionOutputPort(process, 0), ConnectionInputPort(receive, 0));
  EXPECT_EQ(2, network->nconnections());

  SendTestMatrixModule* sendModule = dynamic_cast<SendTestMatrixModule*>(send.get());
  ASSERT_TRUE(sendModule != nullptr);
  NeedToExecuteTester* evalModule = dynamic_cast<NeedToExecuteTester*>(process.get());
  ASSERT_TRUE(evalModule != nullptr);

  ASSERT_FALSE(evalModule->executeCalled_);

  DenseMatrixHandle input = matrix1();
  sendModule->get_state()->setTransientValue("MatrixToSend", input);

  Testing::MockModuleReexecutionStrategyPtr mockNeedToExecute(new NiceMock<Testing::MockModuleReexecutionStrategy>);
  process->setReexecutionStrategy(mockNeedToExecute);

  {
    evalModule->resetFlags();
    std::cout << "NeedToExecute = " << needToExecute_ << ", PortCaching = " << portCaching_ << std::endl;
    EXPECT_CALL(*mockNeedToExecute, needToExecute()).Times(1).WillOnce(Return(needToExecute_));
    SimpleSink::setGlobalPortCachingFlag(portCaching_);

    process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);

    send->execute();
    process->execute();
    if (needToExecute_)
      receive->execute();
    else
      EXPECT_THROW(receive->execute(), NoHandleOnPortException);

    EXPECT_TRUE(evalModule->executeCalled_);
    EXPECT_EQ(evalModule->expensiveComputationDone_, needToExecute_);

    if (portCaching_ && evalModule->expensiveComputationDone_)
    {
      // to simulate real life behavior
      EXPECT_CALL(*mockNeedToExecute, needToExecute()).Times(1).WillOnce(Return(false));
      evalModule->resetFlags();
      send->execute();
      process->execute();
      receive->execute();

      EXPECT_TRUE(evalModule->executeCalled_);
      EXPECT_FALSE(evalModule->expensiveComputationDone_);
    }
  }

  //std::cout << "Rest of test" << std::endl;
  EXPECT_CALL(*mockNeedToExecute, needToExecute()).WillRepeatedly(Return(true));

  ReceiveTestMatrixModule* receiveModule = dynamic_cast<ReceiveTestMatrixModule*>(receive.get());
  ASSERT_TRUE(receiveModule != nullptr);

  if (evalModule->expensiveComputationDone_)
  {
    ASSERT_TRUE(receiveModule->latestReceivedMatrix().get() != nullptr);
  }

  evalModule->resetFlags();
  send->execute();
  process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE);
  process->execute();
  receive->execute();
  EXPECT_EQ(*input, *receiveModule->latestReceivedMatrix());

  evalModule->resetFlags();
  send->execute();
  process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY);
  process->get_state()->setValue(Variables::ScalarValue, 2.0);
  process->execute();
  receive->execute();
  EXPECT_EQ(*input, *receiveModule->latestReceivedMatrix());
}

class ReexecuteStrategyUnitTest : public ::testing::TestWithParam < ::std::tr1::tuple<bool, bool, bool> >
{
public:
  ReexecuteStrategyUnitTest() :
    inputsChanged_(::std::tr1::get<0>(GetParam())),
    stateChanged_(::std::tr1::get<1>(GetParam())),
    oportsCached_(::std::tr1::get<2>(GetParam()))
  {
    SCIRun::Core::Logging::Log::get().setVerbose(true);
  }
protected:
  bool inputsChanged_, stateChanged_, oportsCached_;
};

INSTANTIATE_TEST_CASE_P(
  ReexecuteStrategyUnitTestParameterized,
  ReexecuteStrategyUnitTest,
  Combine(Bool(), Bool(), Bool())
  );

TEST_P(ReexecuteStrategyUnitTest, TestAllCombinationsWithMocks)
{
  // plug in 3 substrategies:
  //   StateChangedChecker
  //   InputsChangedChecker
  //   OPortCachedChecker
  // Class just does a disjunction of above 3 booleans

  Testing::MockInputsChangedCheckerPtr mockInputsChanged(new NiceMock<Testing::MockInputsChangedChecker>);
  ON_CALL(*mockInputsChanged, inputsChanged()).WillByDefault(Return(inputsChanged_));
  Testing::MockStateChangedCheckerPtr mockStateChanged(new NiceMock<Testing::MockStateChangedChecker>);
  ON_CALL(*mockStateChanged, newStatePresent()).WillByDefault(Return(stateChanged_));
  Testing::MockOutputPortsCachedCheckerPtr mockOutputPortsCached(new NiceMock<Testing::MockOutputPortsCachedChecker>);
  ON_CALL(*mockOutputPortsCached, outputPortsCached()).WillByDefault(Return(oportsCached_));
  ModuleReexecutionStrategyHandle realNeedToExecute(new DynamicReexecutionStrategy(mockInputsChanged, mockStateChanged, mockOutputPortsCached));

  std::cout << "NeedToExecute = " << true <<
    ", inputsChanged_ = " << inputsChanged_ <<
    ", stateChanged_ = " << stateChanged_ <<
    ", oportsCached_ = " << oportsCached_ << std::endl;
  EXPECT_EQ(inputsChanged_ || stateChanged_ || !oportsCached_, realNeedToExecute->needToExecute());
}
#if 0
TEST_P(ReexecuteStrategyUnitTest, TestNeedToExecuteWithRealInputsChanged)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  NetworkEditorController controller(mf, sf, ExecutionStrategyFactoryHandle(), af);

  auto network = controller.getNetwork();

  ModuleHandle send = controller.addModule("SendTestMatrix");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReceiveTestMatrix");

  EXPECT_EQ(3, network->nmodules());

  network->connect(ConnectionOutputPort(send, 0), ConnectionInputPort(process, 0));
  network->connect(ConnectionOutputPort(process, 0), ConnectionInputPort(receive, 0));
  EXPECT_EQ(2, network->nconnections());

  SendTestMatrixModule* sendModule = dynamic_cast<SendTestMatrixModule*>(send.get());
  ASSERT_TRUE(sendModule != nullptr);
  NeedToExecuteTester* evalModule = dynamic_cast<NeedToExecuteTester*>(process.get());
  ASSERT_TRUE(evalModule != nullptr);

  ASSERT_FALSE(evalModule->executeCalled_);

  DenseMatrixHandle input = matrix1();
  sendModule->get_state()->setTransientValue("MatrixToSend", input);

  std::cout << "RealInputsChanged, stateChanged = " << stateChanged_ << " oportsCached = " << oportsCached_ << std::endl;
  InputsChangedCheckerHandle realInputsChanged(new InputsChangedCheckerImpl(*evalModule));
  Testing::MockStateChangedCheckerPtr mockStateChanged(new NiceMock<Testing::MockStateChangedChecker>);
  ON_CALL(*mockStateChanged, newStatePresent()).WillByDefault(Return(stateChanged_));
  Testing::MockOutputPortsCachedCheckerPtr mockOutputPortsCached(new NiceMock<Testing::MockOutputPortsCachedChecker>);
  ON_CALL(*mockOutputPortsCached, outputPortsCached()).WillByDefault(Return(oportsCached_));
  ModuleReexecutionStrategyHandle realNeedToExecuteWithPartialMocks(new DynamicReexecutionStrategy(realInputsChanged, mockStateChanged, mockOutputPortsCached));

  process->setRexecutionStrategy(realNeedToExecuteWithPartialMocks);

  {
    SimpleSink::setGlobalPortCachingFlag(true);
    evalModule->resetFlags();

    process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);

    bool initialNeedToExecute = realNeedToExecuteWithPartialMocks->needToExecute();
    send->execute();
    process->execute();
    if (initialNeedToExecute)
      receive->execute();
    else
      EXPECT_THROW(receive->execute(), NoHandleOnPortException);

    EXPECT_TRUE(evalModule->executeCalled_);
    EXPECT_EQ(evalModule->expensiveComputationDone_, initialNeedToExecute);

    if (evalModule->expensiveComputationDone_)
    {
      //inputs haven't changed.
      evalModule->resetFlags();
      send->execute();
      process->execute();
      receive->execute();

      EXPECT_TRUE(evalModule->executeCalled_);
      EXPECT_FALSE(evalModule->expensiveComputationDone_);

      DenseMatrixHandle input = matrix2();
      sendModule->get_state()->setTransientValue("MatrixToSend", input);

      //inputs have changed
      evalModule->resetFlags();
      send->execute();
      process->execute();
      receive->execute();

      EXPECT_TRUE(evalModule->executeCalled_);
      EXPECT_TRUE(evalModule->expensiveComputationDone_);
    }
  }
}


TEST_P(ReexecuteStrategyUnitTest, TestNeedToExecuteWithRealStateChanged)
{
  FAIL() << "todo";
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  NetworkEditorController controller(mf, sf, ExecutionStrategyFactoryHandle(), af);

  auto network = controller.getNetwork();

  ModuleHandle send = controller.addModule("SendTestMatrix");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReceiveTestMatrix");

  EXPECT_EQ(3, network->nmodules());

  network->connect(ConnectionOutputPort(send, 0), ConnectionInputPort(process, 0));
  network->connect(ConnectionOutputPort(process, 0), ConnectionInputPort(receive, 0));
  EXPECT_EQ(2, network->nconnections());

  SendTestMatrixModule* sendModule = dynamic_cast<SendTestMatrixModule*>(send.get());
  ASSERT_TRUE(sendModule != nullptr);
  NeedToExecuteTester* evalModule = dynamic_cast<NeedToExecuteTester*>(process.get());
  ASSERT_TRUE(evalModule != nullptr);

  ASSERT_FALSE(evalModule->executeCalled_);

  DenseMatrixHandle input = matrix1();
  sendModule->get_state()->setTransientValue("MatrixToSend", input);

  Testing::MockInputsChangedCheckerPtr mockInputsChanged(new NiceMock<Testing::MockInputsChangedChecker>);
  ON_CALL(*mockInputsChanged, inputsChanged()).WillByDefault(Return(inputsChanged_));
  Testing::MockStateChangedCheckerPtr mockStateChanged(new NiceMock<Testing::MockStateChangedChecker>);
  ON_CALL(*mockStateChanged, newStatePresent()).WillByDefault(Return(stateChanged_));
  Testing::MockOutputPortsCachedCheckerPtr mockOutputPortsCached(new NiceMock<Testing::MockOutputPortsCachedChecker>);
  ON_CALL(*mockOutputPortsCached, outputPortsCached()).WillByDefault(Return(oportsCached_));
  ModuleReexecutionStrategyHandle realNeedToExecuteWithPartialMocks(new DynamicReexecutionStrategy(mockInputsChanged, mockStateChanged, mockOutputPortsCached));

  process->setRexecutionStrategy(realNeedToExecuteWithPartialMocks);

  {
    evalModule->resetFlags();

    process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);

    send->execute();
    process->execute();
    if (realNeedToExecuteWithPartialMocks->needToExecute())
      receive->execute();
    else
      EXPECT_THROW(receive->execute(), NoHandleOnPortException);

    EXPECT_TRUE(evalModule->executeCalled_);
    //EXPECT_EQ(evalModule->expensiveComputationDone_, needToExecute_);

    if (evalModule->expensiveComputationDone_)
    {
      // to simulate real life behavior
      evalModule->resetFlags();
      send->execute();
      process->execute();
      receive->execute();

      EXPECT_TRUE(evalModule->executeCalled_);
      EXPECT_FALSE(evalModule->expensiveComputationDone_);
    }
  }

  //std::cout << "Rest of test" << std::endl;

  ReceiveTestMatrixModule* receiveModule = dynamic_cast<ReceiveTestMatrixModule*>(receive.get());
  ASSERT_TRUE(receiveModule != nullptr);

  if (evalModule->expensiveComputationDone_)
  {
    ASSERT_TRUE(receiveModule->latestReceivedMatrix().get() != nullptr);
  }

  evalModule->resetFlags();
  send->execute();
  process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE);
  process->execute();
  receive->execute();
  EXPECT_EQ(*input, *receiveModule->latestReceivedMatrix());

  evalModule->resetFlags();
  send->execute();
  process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY);
  process->get_state()->setValue(Variables::ScalarValue, 2.0);
  process->execute();
  receive->execute();
  EXPECT_EQ(*input, *receiveModule->latestReceivedMatrix());
}
#endif
#if 0
TEST_P(ReexecuteStrategyUnitTest, TestNeedToExecuteWithRealOportsCached)
{
  FAIL() << "todo";
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  NetworkEditorController controller(mf, sf, ExecutionStrategyFactoryHandle(), af);

  auto network = controller.getNetwork();

  ModuleHandle send = controller.addModule("SendTestMatrix");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReceiveTestMatrix");

  EXPECT_EQ(3, network->nmodules());

  network->connect(ConnectionOutputPort(send, 0), ConnectionInputPort(process, 0));
  network->connect(ConnectionOutputPort(process, 0), ConnectionInputPort(receive, 0));
  EXPECT_EQ(2, network->nconnections());

  SendTestMatrixModule* sendModule = dynamic_cast<SendTestMatrixModule*>(send.get());
  ASSERT_TRUE(sendModule != nullptr);
  NeedToExecuteTester* evalModule = dynamic_cast<NeedToExecuteTester*>(process.get());
  ASSERT_TRUE(evalModule != nullptr);

  ASSERT_FALSE(evalModule->executeCalled_);

  DenseMatrixHandle input = matrix1();
  sendModule->get_state()->setTransientValue("MatrixToSend", input);

  Testing::MockInputsChangedCheckerPtr mockInputsChanged(new NiceMock<Testing::MockInputsChangedChecker>);
  ON_CALL(*mockInputsChanged, inputsChanged()).WillByDefault(Return(inputsChanged_));
  Testing::MockStateChangedCheckerPtr mockStateChanged(new NiceMock<Testing::MockStateChangedChecker>);
  ON_CALL(*mockStateChanged, newStatePresent()).WillByDefault(Return(stateChanged_));
  Testing::MockOutputPortsCachedCheckerPtr mockOutputPortsCached(new NiceMock<Testing::MockOutputPortsCachedChecker>);
  ON_CALL(*mockOutputPortsCached, outputPortsCached()).WillByDefault(Return(oportsCached_));
  ModuleReexecutionStrategyHandle realNeedToExecuteWithPartialMocks(new DynamicReexecutionStrategy(mockInputsChanged, mockStateChanged, mockOutputPortsCached));

  process->setRexecutionStrategy(realNeedToExecuteWithPartialMocks);

  {
    evalModule->resetFlags();

    process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);

    send->execute();
    process->execute();
    if (realNeedToExecuteWithPartialMocks->needToExecute())
      receive->execute();
    else
      EXPECT_THROW(receive->execute(), NoHandleOnPortException);

    EXPECT_TRUE(evalModule->executeCalled_);
    //EXPECT_EQ(evalModule->expensiveComputationDone_, needToExecute_);

    if (evalModule->expensiveComputationDone_)
    {
      // to simulate real life behavior
      evalModule->resetFlags();
      send->execute();
      process->execute();
      receive->execute();

      EXPECT_TRUE(evalModule->executeCalled_);
      EXPECT_FALSE(evalModule->expensiveComputationDone_);
    }
  }

  //std::cout << "Rest of test" << std::endl;

  ReceiveTestMatrixModule* receiveModule = dynamic_cast<ReceiveTestMatrixModule*>(receive.get());
  ASSERT_TRUE(receiveModule != nullptr);

  if (evalModule->expensiveComputationDone_)
  {
    ASSERT_TRUE(receiveModule->latestReceivedMatrix().get() != nullptr);
  }

  evalModule->resetFlags();
  send->execute();
  process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE);
  process->execute();
  receive->execute();
  EXPECT_EQ(*input, *receiveModule->latestReceivedMatrix());

  evalModule->resetFlags();
  send->execute();
  process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY);
  process->get_state()->setValue(Variables::ScalarValue, 2.0);
  process->execute();
  receive->execute();
  EXPECT_EQ(*input, *receiveModule->latestReceivedMatrix());
}
#endif
#endif

class ReexecuteStrategySimpleUnitTest : public ::testing::Test
{
public:
  ReexecuteStrategySimpleUnitTest() :
    inputsChanged_(false),
    stateChanged_(true),
    oportsCached_(true)
  {
    SCIRun::Core::Logging::Log::get().setVerbose(true);
  }
protected:
  bool inputsChanged_, stateChanged_, oportsCached_;
};

TEST_F(ReexecuteStrategySimpleUnitTest, JustInputsChanged)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  NetworkEditorController controller(mf, sf, nullptr, af, nullptr, nullptr);

  auto network = controller.getNetwork();

  ModuleHandle send = controller.addModule("SendTestMatrix");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReceiveTestMatrix");

  EXPECT_EQ(3, network->nmodules());

  network->connect(ConnectionOutputPort(send, 0), ConnectionInputPort(process, 0));
  network->connect(ConnectionOutputPort(process, 0), ConnectionInputPort(receive, 0));
  EXPECT_EQ(2, network->nconnections());

  SendTestMatrixModule* sendModule = dynamic_cast<SendTestMatrixModule*>(send.get());
  ASSERT_TRUE(sendModule != nullptr);
  NeedToExecuteTester* evalModule = dynamic_cast<NeedToExecuteTester*>(process.get());
  ASSERT_TRUE(evalModule != nullptr);

  ASSERT_FALSE(evalModule->executeCalled_);

  DenseMatrixHandle input = matrix1();
  std::cout << "### first input has id: " << input->id() << std::endl;
  sendModule->get_state()->setTransientValue("MatrixToSend", input);

  std::cout << "RealInputsChanged, stateChanged = " << stateChanged_ << " oportsCached = " << oportsCached_ << std::endl;
  InputsChangedCheckerHandle realInputsChanged(new InputsChangedCheckerImpl(*evalModule));
  Testing::MockStateChangedCheckerPtr mockStateChanged(new NiceMock<Testing::MockStateChangedChecker>);
  ON_CALL(*mockStateChanged, newStatePresent()).WillByDefault(Return(true));
  Testing::MockOutputPortsCachedCheckerPtr mockOutputPortsCached(new NiceMock<Testing::MockOutputPortsCachedChecker>);
  ON_CALL(*mockOutputPortsCached, outputPortsCached()).WillByDefault(Return(true));
  ModuleReexecutionStrategyHandle realNeedToExecuteWithPartialMocks(new DynamicReexecutionStrategy(realInputsChanged, mockStateChanged, mockOutputPortsCached));

  process->setReexecutionStrategy(realNeedToExecuteWithPartialMocks);

  {
    SimpleSink::setGlobalPortCachingFlag(true);
    evalModule->resetFlags();

    process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);

    bool initialNeedToExecute = realNeedToExecuteWithPartialMocks->needToExecute();
    ASSERT_TRUE(initialNeedToExecute);
    //std::cout << "EXECUTION 1 1 1 1 1 1 1" << std::endl;
    send->do_execute();
    process->do_execute();
    receive->do_execute();

    EXPECT_TRUE(evalModule->executeCalled_);
    EXPECT_EQ(evalModule->expensiveComputationDone_, initialNeedToExecute);

    ASSERT_TRUE(evalModule->expensiveComputationDone_);
    ON_CALL(*mockStateChanged, newStatePresent()).WillByDefault(Return(false));
    if (evalModule->expensiveComputationDone_)
    {
      //inputs haven't changed.
      evalModule->resetFlags();
      //std::cout << "EXECUTION 2 2 2 2 2 2 2" << std::endl;
      send->do_execute();
      process->do_execute();
      receive->do_execute();
      EXPECT_FALSE(realNeedToExecuteWithPartialMocks->needToExecute());

      EXPECT_TRUE(evalModule->executeCalled_);
      EXPECT_FALSE(evalModule->expensiveComputationDone_);

      DenseMatrixHandle input = matrix2();
      std::cout << "### second input has id: " << input->id() << std::endl;
      sendModule->get_state()->setTransientValue("MatrixToSend", input);

      //std::cout << "EXECUTION 3 3 3 3 3 3 3" << std::endl;
      //inputs have changed
      evalModule->resetFlags();
      send->do_execute();
      process->do_execute();
      receive->do_execute();

      EXPECT_TRUE(evalModule->executeCalled_);
      EXPECT_TRUE(evalModule->expensiveComputationDone_);
    }
  }
}

TEST_F(ReexecuteStrategySimpleUnitTest, JustStateChanged)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  NetworkEditorController controller(mf, sf, nullptr, af, nullptr, nullptr);

  auto network = controller.getNetwork();

  ModuleHandle send = controller.addModule("SendTestMatrix");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReceiveTestMatrix");

  EXPECT_EQ(3, network->nmodules());

  network->connect(ConnectionOutputPort(send, 0), ConnectionInputPort(process, 0));
  network->connect(ConnectionOutputPort(process, 0), ConnectionInputPort(receive, 0));
  EXPECT_EQ(2, network->nconnections());

  SendTestMatrixModule* sendModule = dynamic_cast<SendTestMatrixModule*>(send.get());
  ASSERT_TRUE(sendModule != nullptr);
  NeedToExecuteTester* evalModule = dynamic_cast<NeedToExecuteTester*>(process.get());
  ASSERT_TRUE(evalModule != nullptr);

  ASSERT_FALSE(evalModule->executeCalled_);

  DenseMatrixHandle input = matrix1();
  sendModule->get_state()->setTransientValue("MatrixToSend", input);

  std::cout << "RealStateChanged, inputsChanged = " << inputsChanged_ << " oportsCached = " << oportsCached_ << std::endl;
  StateChangedCheckerHandle realStateChanged(new StateChangedCheckerImpl(*evalModule));
  Testing::MockInputsChangedCheckerPtr mockInputsChanged(new NiceMock<Testing::MockInputsChangedChecker>);
  ON_CALL(*mockInputsChanged, inputsChanged()).WillByDefault(Return(inputsChanged_));
  Testing::MockOutputPortsCachedCheckerPtr mockOutputPortsCached(new NiceMock<Testing::MockOutputPortsCachedChecker>);
  ON_CALL(*mockOutputPortsCached, outputPortsCached()).WillByDefault(Return(true));
  ModuleReexecutionStrategyHandle realNeedToExecuteWithPartialMocks(new DynamicReexecutionStrategy(mockInputsChanged, realStateChanged, mockOutputPortsCached));

  process->setReexecutionStrategy(realNeedToExecuteWithPartialMocks);

  {
    SimpleSink::setGlobalPortCachingFlag(true);
    evalModule->resetFlags();

    process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);

    bool initialNeedToExecute = realNeedToExecuteWithPartialMocks->needToExecute();
    ASSERT_TRUE(initialNeedToExecute);
    //std::cout << "EXECUTION 1 1 1 1 1 1 1" << std::endl;
    send->do_execute();
    process->do_execute();
    receive->do_execute();

    EXPECT_TRUE(evalModule->executeCalled_);
    EXPECT_EQ(evalModule->expensiveComputationDone_, initialNeedToExecute);

    ASSERT_TRUE(evalModule->expensiveComputationDone_);
    if (evalModule->expensiveComputationDone_)
    {
      //state hasn't changed.
      evalModule->resetFlags();
      //std::cout << "EXECUTION 2 2 2 2 2 2 2" << std::endl;
      send->do_execute();
      process->do_execute();
      receive->do_execute();
      EXPECT_FALSE(realNeedToExecuteWithPartialMocks->needToExecute());

      EXPECT_TRUE(evalModule->executeCalled_);
      EXPECT_FALSE(evalModule->expensiveComputationDone_);

      process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE);

      //std::cout << "EXECUTION 3 3 3 3 3 3 3" << std::endl;
      //state has changed
      evalModule->resetFlags();
      send->do_execute();
      process->do_execute();
      receive->do_execute();

      EXPECT_TRUE(evalModule->executeCalled_);
      EXPECT_TRUE(evalModule->expensiveComputationDone_);
    }
  }
}

//TODO: port cache switch is not exposed, need to rework this anyway
TEST_F(ReexecuteStrategySimpleUnitTest, DISABLED_JustOportsCached)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  NetworkEditorController controller(mf, sf, nullptr, af, nullptr, nullptr);

  auto network = controller.getNetwork();

  ModuleHandle send = controller.addModule("SendTestMatrix");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReceiveTestMatrix");

  EXPECT_EQ(3, network->nmodules());

  network->connect(ConnectionOutputPort(send, 0), ConnectionInputPort(process, 0));
  auto oportId = network->connect(ConnectionOutputPort(process, 0), ConnectionInputPort(receive, 0));
  EXPECT_EQ(2, network->nconnections());

  SendTestMatrixModule* sendModule = dynamic_cast<SendTestMatrixModule*>(send.get());
  ASSERT_TRUE(sendModule != nullptr);
  NeedToExecuteTester* evalModule = dynamic_cast<NeedToExecuteTester*>(process.get());
  ASSERT_TRUE(evalModule != nullptr);

  ASSERT_FALSE(evalModule->executeCalled_);

  DenseMatrixHandle input = matrix1();
  sendModule->get_state()->setTransientValue("MatrixToSend", input);

  //std::cout << "RealOportsCached, inputsChanged = " << inputsChanged_ << " stateChanged = " << stateChanged_ << std::endl;
  Testing::MockStateChangedCheckerPtr mockStateChanged(new NiceMock<Testing::MockStateChangedChecker>);
  ON_CALL(*mockStateChanged, newStatePresent()).WillByDefault(Return(stateChanged_));
  Testing::MockInputsChangedCheckerPtr mockInputsChanged(new NiceMock<Testing::MockInputsChangedChecker>);
  ON_CALL(*mockInputsChanged, inputsChanged()).WillByDefault(Return(inputsChanged_));
  OutputPortsCachedCheckerHandle realOportsCached(new OutputPortsCachedCheckerImpl(*evalModule));

  ModuleReexecutionStrategyHandle realNeedToExecuteWithPartialMocks(new DynamicReexecutionStrategy(mockInputsChanged, mockStateChanged, realOportsCached));

  process->setReexecutionStrategy(realNeedToExecuteWithPartialMocks);

  {
    SimpleSink::setGlobalPortCachingFlag(true);
    evalModule->resetFlags();

    process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);

    bool initialNeedToExecute = realNeedToExecuteWithPartialMocks->needToExecute();
    ASSERT_TRUE(initialNeedToExecute);
    //std::cout << "@ @ @ @ @ @ @ @ @ @ EXECUTION 1 1 1 1 1 1 1" << std::endl;
    send->do_execute();
    process->do_execute();
    receive->do_execute();

    EXPECT_TRUE(evalModule->executeCalled_);
    EXPECT_EQ(evalModule->expensiveComputationDone_, initialNeedToExecute);
    ASSERT_TRUE(realOportsCached->outputPortsCached());

    ASSERT_TRUE(evalModule->expensiveComputationDone_);
    evalModule->resetFlags();
    //std::cout << "@ @ @ @ @ @ @ @ @ @ EXECUTION 2 2 2 2 2 2 2" << std::endl;
    send->do_execute();
    process->do_execute();
    receive->do_execute();
    EXPECT_FALSE(realNeedToExecuteWithPartialMocks->needToExecute());

    EXPECT_TRUE(evalModule->executeCalled_);
    EXPECT_FALSE(evalModule->expensiveComputationDone_);

    //Invalidate iport by disconnecting/reconnecting
    network->disconnect(oportId);
    EXPECT_EQ(1, network->nconnections());
    network->connect(ConnectionOutputPort(process, 0), ConnectionInputPort(receive, 0));
    EXPECT_EQ(2, network->nconnections());

    //std::cout << "@ @ @ @ @ @ @ @ @ @ EXECUTION 3 3 3 3 3 3 3" << std::endl;

    evalModule->resetFlags();
    EXPECT_TRUE(send->do_execute());
    EXPECT_TRUE(process->do_execute());
    EXPECT_TRUE(receive->do_execute());

    EXPECT_TRUE(evalModule->executeCalled_);
    EXPECT_FALSE(evalModule->expensiveComputationDone_);

    //Invalidate oport by changing flag
    SimpleSink::setGlobalPortCachingFlag(false);

    //std::cout << "@ @ @ @ @ @ @ @ @ @ EXECUTION 4 4 4 4 4 4" << std::endl;

    evalModule->resetFlags();
    send->do_execute();
    process->do_execute();
    receive->do_execute();

    EXPECT_TRUE(evalModule->executeCalled_);
    EXPECT_TRUE(evalModule->expensiveComputationDone_);
  }
}

TEST(PortCachingFunctionalTest, TestSourceSinkInputsChanged)
{
  Log::get().setVerbose(true);
  ReexecuteStrategyFactoryHandle re(new DynamicReexecutionStrategyFactory(std::string()));
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  NetworkEditorController controller(mf, sf, nullptr, af, re, nullptr);

  auto network = controller.getNetwork();

  ModuleHandle send = controller.addModule("CreateLatVol");
  ModuleHandle process = controller.addModule("NeedToExecuteTester");
  ModuleHandle receive = controller.addModule("ReportFieldInfo");

  EXPECT_EQ(3, network->nmodules());

  network->connect(ConnectionOutputPort(send, 0), ConnectionInputPort(process, 1));
  EXPECT_EQ(1, network->nconnections());
  network->connect(ConnectionOutputPort(process, 1), ConnectionInputPort(receive, 0));
  EXPECT_EQ(2, network->nconnections());

  NeedToExecuteTester* evalModule = dynamic_cast<NeedToExecuteTester*>(process.get());
  ASSERT_TRUE(evalModule != nullptr);

  EXPECT_FALSE(evalModule->executeCalled_);
  EXPECT_FALSE(evalModule->expensiveComputationDone_);

std::cout << "@ @ @ @ @ @ @ @ @ @ EXECUTION 1 1 1 1 1 1" << std::endl;
  send->do_execute();
  process->do_execute();
  receive->do_execute();

  EXPECT_TRUE(evalModule->executeCalled_);
  EXPECT_TRUE(evalModule->expensiveComputationDone_);

  evalModule->resetFlags();

std::cout << "@ @ @ @ @ @ @ @ @ @ EXECUTION 2 2 2 2 2" << std::endl;
  send->do_execute();
  process->do_execute();
  receive->do_execute();

  EXPECT_TRUE(evalModule->executeCalled_);
  EXPECT_FALSE(evalModule->expensiveComputationDone_);

  evalModule->resetFlags();

  std::cout << "@ @ @ @ @ @ @ @ @ @ EXECUTION 3 3 3 3 3 3 3" << std::endl;
  send->do_execute();
  process->do_execute();
  receive->do_execute();

  EXPECT_TRUE(evalModule->executeCalled_);
  EXPECT_FALSE(evalModule->expensiveComputationDone_);

}
