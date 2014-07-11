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
    MOCK_CONST_METHOD0(stateChanged, bool());
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
  sendModule->get_state()->setTransientValue("MatrixToSend", input, true);

  Testing::MockModuleReexecutionStrategyPtr mockNeedToExecute(new NiceMock<Testing::MockModuleReexecutionStrategy>);
  process->setRexecutionStrategy(mockNeedToExecute);

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
  
  std::cout << "Rest of test" << std::endl;
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
  ON_CALL(*mockStateChanged, stateChanged()).WillByDefault(Return(stateChanged_));
  Testing::MockOutputPortsCachedCheckerPtr mockOutputPortsCached(new NiceMock<Testing::MockOutputPortsCachedChecker>);
  ON_CALL(*mockOutputPortsCached, outputPortsCached()).WillByDefault(Return(oportsCached_));
  ModuleReexecutionStrategyHandle realNeedToExecute(new DynamicReexecutionStrategy(mockInputsChanged, mockStateChanged, mockOutputPortsCached));

  std::cout << "NeedToExecute = " << true << 
    ", inputsChanged_ = " << inputsChanged_ <<
    ", stateChanged_ = " << stateChanged_ <<
    ", oportsCached_ = " << oportsCached_ << std::endl;
  EXPECT_EQ(inputsChanged_ || stateChanged_ || !oportsCached_, realNeedToExecute->needToExecute());
}

class InputsChangedCheckerImpl : public InputsChangedChecker
{
public:
  explicit InputsChangedCheckerImpl(Module& module) : module_(module) 
  {
  }
  virtual bool inputsChanged() const override 
  { 
    return module_.inputsChanged(); 
  }
private:
  Module& module_;
};

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
  sendModule->get_state()->setTransientValue("MatrixToSend", input, true);

  std::cout << "RealInputsChanged, stateChanged = " << stateChanged_ << " oportsCached = " << oportsCached_ << std::endl;
  InputsChangedCheckerHandle realInputsChanged(new InputsChangedCheckerImpl(*evalModule));
  Testing::MockStateChangedCheckerPtr mockStateChanged(new NiceMock<Testing::MockStateChangedChecker>);
  ON_CALL(*mockStateChanged, stateChanged()).WillByDefault(Return(stateChanged_));
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
      //EXPECT_FALSE(realNeedToExecuteWithPartialMocks->needToExecute());

      EXPECT_TRUE(evalModule->executeCalled_);
      EXPECT_FALSE(evalModule->expensiveComputationDone_);

      DenseMatrixHandle input = matrix2();
      sendModule->get_state()->setTransientValue("MatrixToSend", input, true);

      //inputs have changed
      evalModule->resetFlags();
      send->execute();
      process->execute();
      receive->execute();

      EXPECT_TRUE(evalModule->executeCalled_);
      EXPECT_TRUE(evalModule->expensiveComputationDone_);
    }
  }

//   std::cout << "Rest of test" << std::endl;
// 
//   ReceiveTestMatrixModule* receiveModule = dynamic_cast<ReceiveTestMatrixModule*>(receive.get());
//   ASSERT_TRUE(receiveModule != nullptr);
// 
//   if (evalModule->expensiveComputationDone_)
//   {
//     ASSERT_TRUE(receiveModule->latestReceivedMatrix().get() != nullptr);
//   }
// 
//   evalModule->resetFlags();
//   send->execute();
//   process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE);
//   process->execute();
//   receive->execute();
//   EXPECT_EQ(*input, *receiveModule->latestReceivedMatrix());
// 
//   evalModule->resetFlags();
//   send->execute();
//   process->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY);
//   process->get_state()->setValue(Variables::ScalarValue, 2.0);
//   process->execute();
//   receive->execute();
//   EXPECT_EQ(*input, *receiveModule->latestReceivedMatrix());
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
  sendModule->get_state()->setTransientValue("MatrixToSend", input, true);

  Testing::MockInputsChangedCheckerPtr mockInputsChanged(new NiceMock<Testing::MockInputsChangedChecker>);
  ON_CALL(*mockInputsChanged, inputsChanged()).WillByDefault(Return(inputsChanged_));
  Testing::MockStateChangedCheckerPtr mockStateChanged(new NiceMock<Testing::MockStateChangedChecker>);
  ON_CALL(*mockStateChanged, stateChanged()).WillByDefault(Return(stateChanged_));
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

  std::cout << "Rest of test" << std::endl;

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
  sendModule->get_state()->setTransientValue("MatrixToSend", input, true);

  Testing::MockInputsChangedCheckerPtr mockInputsChanged(new NiceMock<Testing::MockInputsChangedChecker>);
  ON_CALL(*mockInputsChanged, inputsChanged()).WillByDefault(Return(inputsChanged_));
  Testing::MockStateChangedCheckerPtr mockStateChanged(new NiceMock<Testing::MockStateChangedChecker>);
  ON_CALL(*mockStateChanged, stateChanged()).WillByDefault(Return(stateChanged_));
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

  std::cout << "Rest of test" << std::endl;

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