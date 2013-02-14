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
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Modules/Basic/SendTestMatrix.h>
#include <Modules/Basic/ReceiveTestMatrix.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnary.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinary.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Dataflow/Network/Tests/MockModuleState.h>
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Dataflow/Engine/Scheduler/BoostGraphSerialScheduler.h>
#include <Dataflow/Engine/Scheduler/LinearSerialNetworkExecutor.h>
#include <boost/config.hpp> // put this first to suppress some VC++ warnings

#include <iostream>
#include <iterator>
#include <algorithm>
#include <time.h>

#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

using namespace SCIRun;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Dataflow::Engine;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

using namespace std;
using namespace boost;

namespace
{
  DenseMatrixHandle matrix1()
  {
    DenseMatrixHandle m(new DenseMatrix(3, 3));
    for (size_t i = 0; i < m->rows(); ++i)
      for (size_t j = 0; j < m->cols(); ++j)
        (*m)(i, j) = 3.0 * i + j;
    return m;
  }
  DenseMatrixHandle matrix2()
  {
    DenseMatrixHandle m(new DenseMatrix(3, 3));
    for (size_t i = 0; i < m->rows(); ++i)
      for (size_t j = 0; j < m->cols(); ++j)
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

TEST(SchedulingWithBoostGraph, NetworkFromMatrixCalculator)
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
  Network matrixMathNetwork(mf, sf);
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

  //TODO: turn this into a convenience network printing function
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
  transpose->get_state()->setValue(EvaluateLinearAlgebraUnaryAlgorithm::OperatorName, EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE);
  negate->get_state()->setValue(EvaluateLinearAlgebraUnaryAlgorithm::OperatorName, EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);
  scalar->get_state()->setValue(EvaluateLinearAlgebraUnaryAlgorithm::OperatorName, EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY);
  scalar->get_state()->setValue(EvaluateLinearAlgebraUnaryAlgorithm::ScalarValue, 4.0);
  multiply->get_state()->setValue(EvaluateLinearAlgebraBinaryAlgorithm::OperatorName, EvaluateLinearAlgebraBinaryAlgorithm::MULTIPLY);
  add->get_state()->setValue(EvaluateLinearAlgebraBinaryAlgorithm::OperatorName, EvaluateLinearAlgebraBinaryAlgorithm::ADD);

  BoostGraphSerialScheduler scheduler;
  ModuleExecutionOrder order = scheduler.schedule(matrixMathNetwork);
  LinearSerialNetworkExecutor executor;
  executor.executeAll(matrixMathNetwork, order);

  //TODO: let executor thread finish.  should be an event generated or something.
  boost::this_thread::sleep(boost::posix_time::milliseconds(100));

  //grab reporting module state
  ReportMatrixInfoAlgorithm::Outputs reportOutput = any_cast_or_default<ReportMatrixInfoAlgorithm::Outputs>(report->get_state()->getTransientValue("ReportedInfo"));
  DenseMatrixHandle receivedMatrix = any_cast_or_default<DenseMatrixHandle>(receive->get_state()->getTransientValue("ReceivedMatrix"));

  ASSERT_TRUE(receivedMatrix);
  //verify results
  EXPECT_EQ(expected, *receivedMatrix);
  EXPECT_EQ(3, reportOutput.get<1>());
  EXPECT_EQ(3, reportOutput.get<2>());
  EXPECT_EQ(9, reportOutput.get<3>());
  EXPECT_EQ(22, reportOutput.get<4>());
  EXPECT_EQ(186, reportOutput.get<5>());
}

TEST(SchedulingWithBoostGraph, CanDetectConnectionCycles)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  Network matrixMathNetwork(mf, sf);
  
  ModuleHandle negate = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraUnary");
  ModuleHandle scalar = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraUnary");

  EXPECT_EQ(2, matrixMathNetwork.nmodules());

  EXPECT_EQ(0, matrixMathNetwork.nconnections());
  matrixMathNetwork.connect(ConnectionOutputPort(negate, 0), ConnectionInputPort(scalar, 0));
  matrixMathNetwork.connect(ConnectionOutputPort(scalar, 0), ConnectionInputPort(negate, 0));
  EXPECT_EQ(2, matrixMathNetwork.nconnections());

  //Set module parameters.
  negate->get_state()->setValue(EvaluateLinearAlgebraUnaryAlgorithm::OperatorName,
    EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);
  scalar->get_state()->setValue(EvaluateLinearAlgebraUnaryAlgorithm::OperatorName, EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY);
  scalar->get_state()->setValue(EvaluateLinearAlgebraUnaryAlgorithm::ScalarValue, 4.0);

  BoostGraphSerialScheduler scheduler;
  
  EXPECT_THROW(scheduler.schedule(matrixMathNetwork), NetworkHasCyclesException);
}