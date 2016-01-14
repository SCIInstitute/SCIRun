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
#include <Core/Datatypes/DenseMatrix.h>
#include <Modules/Basic/SendTestMatrix.h>
#include <Modules/Basic/ReceiveTestMatrix.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Dataflow/Network/Tests/MockModuleState.h>
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Dataflow/Engine/Scheduler/BoostGraphSerialScheduler.h>
#include <Dataflow/Engine/Scheduler/LinearSerialNetworkExecutor.h>
#include <Dataflow/Engine/Scheduler/BoostGraphParallelScheduler.h>
#include <Dataflow/Engine/Scheduler/BasicMultithreadedNetworkExecutor.h>
#include <Dataflow/Engine/Scheduler/BasicParallelExecutionStrategy.h>
#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Logging/Log.h>

#include <boost/assign.hpp>
#include <boost/config.hpp> // put this first to suppress some VC++ warnings
#include <boost/lockfree/spsc_queue.hpp>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <ctime>

#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/visitors.hpp>
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
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Thread;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

using namespace std;
using namespace boost;
using namespace boost::assign;

namespace
{
  const DenseMatrix Zero(DenseMatrix::Zero(3,3));
}


class SchedulingWithBoostGraph : public ::testing::Test
{
public:
  SchedulingWithBoostGraph() :
    mf(new HardCodedModuleFactory),
    sf(new SimpleMapModuleStateFactory),
    af(new HardCodedAlgorithmFactory),
    matrixMathNetwork(mf, sf, af, ReexecuteStrategyFactoryHandle())
  {
  }
protected:
  ModuleFactoryHandle mf;
  ModuleStateFactoryHandle sf;
  AlgorithmFactoryHandle af;
  Network matrixMathNetwork;
  ModuleHandle receive, report;
  DenseMatrix expected;
  UseGlobalInstanceCountIdGenerator switcher;

  virtual void SetUp()
  {

  }

  ModuleHandle addModuleToNetwork(Network& network, const std::string& moduleName)
  {
    ModuleLookupInfo info;
    info.module_name_ = moduleName;
    return network.add_module(info);
  }

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

  void setupBasicNetwork()
  {
    Module::resetIdGenerator();
    //Test network:
    /*
    send m1(0)          send m2(0)
    |            |             |
    transpose(1) negate(1)    scalar mult *4(1)
    |            |             |
    |           multiply(2)
    |           |
          add(3)
          |      |
          report(4) receive(4)
    */

    expected = (-*matrix1()) * (4* *matrix2()) + matrix1()->transpose();

    ModuleHandle matrix1Send = addModuleToNetwork(matrixMathNetwork, "SendTestMatrix");
    ModuleHandle matrix2Send = addModuleToNetwork(matrixMathNetwork, "SendTestMatrix");

    ModuleHandle transpose = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraUnary");
    ModuleHandle negate = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraUnary");
    ModuleHandle scalar = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraUnary");

    ModuleHandle multiply = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraBinary");
    ModuleHandle add = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraBinary");

    report = addModuleToNetwork(matrixMathNetwork, "ReportMatrixInfo");
    receive = addModuleToNetwork(matrixMathNetwork, "ReceiveTestMatrix");

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
  }
};

TEST_F(SchedulingWithBoostGraph, NetworkFromMatrixCalculator)
{
  setupBasicNetwork();

  BoostGraphSerialScheduler scheduler;
  ModuleExecutionOrder order = scheduler.schedule(matrixMathNetwork);
  LinearSerialNetworkExecutor executor;
  ExecutionContext context(matrixMathNetwork);
  Mutex m("exec");
  executor.execute(context, order, m);

  /// @todo: let executor thread finish.  should be an event generated or something.
  boost::this_thread::sleep(boost::posix_time::milliseconds(800));

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

TEST_F(SchedulingWithBoostGraph, CanDetectConnectionCycles)
{
  ModuleHandle negate = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraUnary");
  ModuleHandle scalar = addModuleToNetwork(matrixMathNetwork, "EvaluateLinearAlgebraUnary");

  EXPECT_EQ(2, matrixMathNetwork.nmodules());

  EXPECT_EQ(0, matrixMathNetwork.nconnections());
  matrixMathNetwork.connect(ConnectionOutputPort(negate, 0), ConnectionInputPort(scalar, 0));
  matrixMathNetwork.connect(ConnectionOutputPort(scalar, 0), ConnectionInputPort(negate, 0));
  EXPECT_EQ(2, matrixMathNetwork.nconnections());

  //Set module parameters.
  negate->get_state()->setValue(Variables::Operator,
    EvaluateLinearAlgebraUnaryAlgorithm::NEGATE);
  scalar->get_state()->setValue(Variables::Operator, EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY);
  scalar->get_state()->setValue(Variables::ScalarValue, 4.0);

  BoostGraphSerialScheduler scheduler;

  EXPECT_THROW(scheduler.schedule(matrixMathNetwork), NetworkHasCyclesException);
}

TEST_F(SchedulingWithBoostGraph, NetworkFromMatrixCalculatorMultiThreaded)
{
  setupBasicNetwork();

  //BoostGraphParallelScheduler scheduler;
  //auto order = scheduler.schedule(matrixMathNetwork);
  //BasicMultithreadedNetworkExecutor executor;
  //executor.executeAll(matrixMathNetwork, order, ExecutionBounds());
  BasicParallelExecutionStrategy strategy;
  ExecutionContext context(matrixMathNetwork, matrixMathNetwork);
  Mutex m("exec");
  strategy.execute(context, m);

  /// @todo: let executor thread finish.  should be an event generated or something.
  boost::this_thread::sleep(boost::posix_time::milliseconds(800));

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

TEST_F(SchedulingWithBoostGraph, SerialNetworkOrder)
{
  setupBasicNetwork();

  BoostGraphSerialScheduler scheduler;
  ModuleExecutionOrder order = scheduler.schedule(matrixMathNetwork);

  std::list<ModuleId> expected = list_of
    (ModuleId("SendTestMatrix:1"))
    (ModuleId("EvaluateLinearAlgebraUnary:4"))
    (ModuleId("SendTestMatrix:0"))
    (ModuleId("EvaluateLinearAlgebraUnary:3"))
    (ModuleId("EvaluateLinearAlgebraBinary:5"))
    (ModuleId("EvaluateLinearAlgebraUnary:2"))
    (ModuleId("EvaluateLinearAlgebraBinary:6"))
    (ModuleId("ReportMatrixInfo:7"))
    (ModuleId("ReceiveTestMatrix:8"));
  EXPECT_EQ(ModuleExecutionOrder(expected), order);
}

TEST_F(SchedulingWithBoostGraph, ParallelNetworkOrder)
{
  setupBasicNetwork();

  BoostGraphParallelScheduler scheduler(ExecuteAllModules::Instance());
  auto order = scheduler.schedule(matrixMathNetwork);
  std::ostringstream ostr;
  ostr << order;

  std::string expected =
    "0 SendTestMatrix:0\n"
    "0 SendTestMatrix:1\n"
    "1 EvaluateLinearAlgebraUnary:2\n"
    "1 EvaluateLinearAlgebraUnary:3\n"
    "1 EvaluateLinearAlgebraUnary:4\n"
    "2 EvaluateLinearAlgebraBinary:5\n"
    "3 EvaluateLinearAlgebraBinary:6\n"
    "4 ReceiveTestMatrix:8\n"
    "4 ReportMatrixInfo:7\n";

  EXPECT_EQ(expected, ostr.str());
}

TEST_F(SchedulingWithBoostGraph, ParallelNetworkOrderWithSomeModulesDone)
{
  setupBasicNetwork();

  ModuleFilter filter = [](ModuleHandle mh) { return mh->get_module_name().find("Unary") == std::string::npos; };
  BoostGraphParallelScheduler scheduler(filter);
  auto order = scheduler.schedule(matrixMathNetwork);
  std::ostringstream ostr;
  ostr << order;

  std::string expected =
    "0 EvaluateLinearAlgebraBinary:5\n"
    "0 SendTestMatrix:0\n"
    "0 SendTestMatrix:1\n"
    "1 EvaluateLinearAlgebraBinary:6\n"
    "2 ReceiveTestMatrix:8\n"
    "2 ReportMatrixInfo:7\n";

  EXPECT_EQ(expected, ostr.str());
}

TEST_F(SchedulingWithBoostGraph, ParallelNetworkOrderExecutedFromAModuleInADisjointSubnetwork)
{
  setupBasicNetwork();

  ModuleHandle create2 = addModuleToNetwork(matrixMathNetwork, "CreateMatrix");
  ModuleHandle report2 = addModuleToNetwork(matrixMathNetwork, "ReportMatrixInfo");

  EXPECT_EQ(11, matrixMathNetwork.nmodules());
  matrixMathNetwork.connect(ConnectionOutputPort(create2, 0), ConnectionInputPort(report2, 0));
  EXPECT_EQ(10, matrixMathNetwork.nconnections());

  {
    BoostGraphParallelScheduler scheduler(ExecuteAllModules::Instance());
    auto order = scheduler.schedule(matrixMathNetwork);
    std::ostringstream ostr;
    ostr << order;

    std::string expected =
      "0 CreateMatrix:9\n"
      "0 SendTestMatrix:0\n"
      "0 SendTestMatrix:1\n"
      "1 EvaluateLinearAlgebraUnary:2\n"
      "1 EvaluateLinearAlgebraUnary:3\n"
      "1 EvaluateLinearAlgebraUnary:4\n"
      "1 ReportMatrixInfo:10\n"
      "2 EvaluateLinearAlgebraBinary:5\n"
      "3 EvaluateLinearAlgebraBinary:6\n"
      "4 ReceiveTestMatrix:8\n"
      "4 ReportMatrixInfo:7\n";

    EXPECT_EQ(expected, ostr.str());
  }

  {
    ExecuteSingleModule filterByCreate(create2, matrixMathNetwork);
    BoostGraphParallelScheduler scheduler(filterByCreate);
    auto order = scheduler.schedule(matrixMathNetwork);
    std::ostringstream ostr;
    ostr << order;

    std::string expected =
      "0 CreateMatrix:9\n"
      "1 ReportMatrixInfo:10\n";

    EXPECT_EQ(expected, ostr.str());
  }

  {
    ExecuteSingleModule filterByReceive(receive, matrixMathNetwork);
    BoostGraphParallelScheduler scheduler(filterByReceive);
    auto order = scheduler.schedule(matrixMathNetwork);
    std::ostringstream ostr;
    ostr << order;

    std::string expected =
      "0 SendTestMatrix:0\n"
      "0 SendTestMatrix:1\n"
      "1 EvaluateLinearAlgebraUnary:2\n"
      "1 EvaluateLinearAlgebraUnary:3\n"
      "1 EvaluateLinearAlgebraUnary:4\n"
      "2 EvaluateLinearAlgebraBinary:5\n"
      "3 EvaluateLinearAlgebraBinary:6\n"
      "4 ReceiveTestMatrix:8\n"
      "4 ReportMatrixInfo:7\n";

    EXPECT_EQ(expected, ostr.str());
  }
}

namespace ThreadingPrototype
{
  struct Unit
  {
    Unit(const std::string& s) : id(s),
      priority(rand() % 4),
      ready(0 == priority), done(false),
      runtime(rand() % 1000 + 1)
    {
    }
    std::string id;
    int priority;
    bool ready;
    bool done;
    int runtime;

    void run()
    {
      boost::this_thread::sleep(boost::posix_time::milliseconds(runtime));
      done = true;
    }
  };

  typedef boost::shared_ptr<Unit> UnitPtr;

  bool operator<(const Unit& lhs, const Unit& rhs)
  {
    return lhs.priority < rhs.priority;
  }

  bool operator<(const UnitPtr& lhs, const UnitPtr& rhs)
  {
    return lhs->priority < rhs->priority;
  }

  std::ostream& operator<<(std::ostream& o, const Unit& u)
  {
    return o << u.id << " : "
      << u.priority << ":"
      << u.ready << ":" << u.done << ":"
      << u.runtime;
  }

  Log::Stream& operator<<(Log::Stream& o, const Unit& u)
  {
    return o << u.id << " : "
      << u.priority << ":"
      << u.ready << ":" << u.done << ":"
      << u.runtime;
  }

  typedef std::queue<UnitPtr> WorkQueue;  /// @todo: will need to be thread-safe
  typedef std::list<UnitPtr> WaitingList;
  typedef std::list<UnitPtr> DoneList;

  UnitPtr makeUnit()
  {
    return UnitPtr(new Unit(boost::lexical_cast<std::string>(rand())));
  }

  std::ostream& operator<<(std::ostream& o, const UnitPtr& u)
  {
    if (u)
      o << *u;
    return o;
  }

  TEST(MultiExecutorPrototypeTest, GenerateListOfUnits)
  {
    WaitingList list;
    std::generate_n(std::back_inserter(list), 10, makeUnit);
    std::copy(list.begin(), list.end(), std::ostream_iterator<UnitPtr>(std::cout, "\n"));
  }

  typedef boost::mutex Mutex;

  class WorkUnitProducer
  {
  public:
    WorkUnitProducer(WorkQueue& workQueue, WaitingList& list, Mutex& mutex) : work_(workQueue), waiting_(list),
      currentPriority_(0), mutex_(mutex)
    {
      waiting_.sort();
      log_ << INFO << "WorkUnitProducer starting. Sorted work list:" << std::endl;
      std::for_each(list.begin(), list.end(), [](UnitPtr u) { log_ << INFO << *u << "\n"; });
    }
    void run()
    {
      log_ << INFO << "Producer started." << std::endl;
      while (!waiting_.empty())
      {
        for (auto i = waiting_.begin(); i != waiting_.end(); )
        {
          if ((*i)->ready)
          {
            log_ << INFO << "\tProducer: Transferring ready unit " << (*i)->id << std::endl;

            mutex_.lock();
            work_.push(*i);
            mutex_.unlock();

            log_ << INFO << "\tProducer: Done transferring ready unit " << (*i)->id << std::endl;

            i = waiting_.erase(i);
          }
          else
            ++i;
        }
        if (workDone() && !waiting_.empty() && (*waiting_.begin())->priority > currentPriority_)
        {
          currentPriority_ = (*waiting_.begin())->priority;
          log_ << INFO << "\tProducer: Setting as ready units with priority = " << currentPriority_ << std::endl;
          for (auto i = waiting_.begin(); i != waiting_.end(); ++i)
          {
            if ((*i)->priority == currentPriority_)
              (*i)->ready = true;
          }
        }
      }
      log_ << INFO << "Producer done." << std::endl;
    }
    bool isDone() const
    {
      boost::lock_guard<Mutex> lock(mutex_);
      return waiting_.empty();
    }
    bool workDone() const
    {
      boost::lock_guard<Mutex> lock(mutex_);
      return work_.empty();
    }
  private:
    WorkQueue& work_;
    WaitingList& waiting_;
    int currentPriority_;
    Mutex& mutex_;
    static Log& log_;
  };



  class WorkUnitConsumer
  {
  public:
    explicit WorkUnitConsumer(WorkQueue& workQueue, const WorkUnitProducer& producer, DoneList& done, Mutex& mutex) :
    work_(workQueue), producer_(producer), done_(done), mutex_(mutex)
    {
    }
    void run()
    {
      log_ << INFO << "Consumer started." << std::endl;
      while (!producer_.isDone())
      {
        log_ << INFO << "\tConsumer thinks producer is not done." << std::endl;
        while (moreWork())
        {
          log_ << INFO << "\tConsumer thinks work queue is not empty." << std::endl;

          mutex_.lock();
          log_ << INFO << "\tConsumer accessing front of work queue." << std::endl;
          UnitPtr unit = work_.front();
          log_ << INFO << "\tConsumer popping front of work queue." << std::endl;
          work_.pop();
          mutex_.unlock();

          log_ << INFO << "~~~Processing " << unit->id << ": sleeping for " << unit->runtime << " ms" << std::endl;
          unit->run();

          done_.push_back(unit);

          log_ << INFO << "\tConsumer: adding done unit, done size = " << done_.size() << std::endl;
        }
      }
      log_ << INFO << "Consumer done." << std::endl;
    }

    bool moreWork()
    {
      boost::lock_guard<Mutex> lock(mutex_);
      return !work_.empty();
    }


  private:
    WorkQueue& work_;
    const WorkUnitProducer& producer_;
    DoneList& done_;
    Mutex& mutex_;
    static Log& log_;
  };

  Log& WorkUnitProducer::log_ = Log::get();
  Log& WorkUnitConsumer::log_ = Log::get();


  TEST(MultiExecutorPrototypeTest, DISABLED_Run1)
  {
    const int size = 50;
    WaitingList list;
    std::generate_n(std::back_inserter(list), size, makeUnit);

    int totalSleepTime = std::accumulate(list.begin(), list.end(), 0, [](int total, UnitPtr u){ return total + u->runtime; });
    std::cout << size << " work units, will sleep for total of " << totalSleepTime / 1000 << " seconds" << std::endl;

    Mutex mutex;
    WorkQueue workQ;
    WorkUnitProducer producer(workQ, list, mutex);

    DoneList done;
    WorkUnitConsumer consumer(workQ, producer, done, mutex);

    boost::thread tR = boost::thread(boost::bind(&WorkUnitProducer::run, producer));
    boost::thread tC = boost::thread(boost::bind(&WorkUnitConsumer::run, consumer));

    tR.join();
    tC.join();

    EXPECT_EQ(size, done.size());
    std::cout << "DONE. Finished list first 10:" << std::endl;
    auto end = done.begin();
    std::advance(end, 10);
    std::copy(done.begin(), end, std::ostream_iterator<UnitPtr>(std::cout, "\n"));
    for (const UnitPtr& u : done)
    {
      EXPECT_TRUE(u->done);
    }
  }

  typedef boost::lockfree::spsc_queue<UnitPtr> WorkQueue2;

  class WorkUnitProducer2
  {
  public:
    WorkUnitProducer2(WorkQueue2& workQueue, WaitingList& list) : work_(workQueue), waiting_(list),
      currentPriority_(0)
    {
      waiting_.sort();
      log_ << INFO << "WorkUnitProducer starting. Sorted work list:" << std::endl;
      std::for_each(list.begin(), list.end(), [](UnitPtr u) { log_ << INFO << *u << "\n"; });
    }
    void run()
    {
      log_ << INFO << "Producer started." << std::endl;
      while (!waiting_.empty())
      {
        for (auto i = waiting_.begin(); i != waiting_.end(); )
        {
          if ((*i)->ready)
          {
            log_ << INFO << "\tProducer: Transferring ready unit " << (*i)->id << std::endl;

            work_.push(*i);

            log_ << INFO << "\tProducer: Done transferring ready unit " << (*i)->id << std::endl;

            i = waiting_.erase(i);
          }
          else
            ++i;
        }
        if (workDone() && !waiting_.empty() && (*waiting_.begin())->priority > currentPriority_)
        {
          currentPriority_ = (*waiting_.begin())->priority;
          log_ << INFO << "\tProducer: Setting as ready units with priority = " << currentPriority_ << std::endl;
          for (auto i = waiting_.begin(); i != waiting_.end(); ++i)
          {
            if ((*i)->priority == currentPriority_)
              (*i)->ready = true;
          }
        }
      }
      log_ << INFO << "Producer done." << std::endl;
    }
    bool isDone() const
    {
      return waiting_.empty();
    }
    bool workDone() const
    {
      return work_.empty();
    }
  private:
    WorkQueue2& work_;
    WaitingList& waiting_;
    int currentPriority_;
    static Log& log_;
  };



  class WorkUnitConsumer2
  {
  public:
    explicit WorkUnitConsumer2(WorkQueue2& workQueue, const WorkUnitProducer2& producer, DoneList& done) :
    work_(workQueue), producer_(producer), done_(done)
    {
    }
    void run()
    {
      log_ << INFO << "Consumer started." << std::endl;
      while (!producer_.isDone())
      {
        log_ << INFO << "\tConsumer thinks producer is not done." << std::endl;
        while (moreWork())
        {
          log_ << INFO << "\tConsumer thinks work queue is not empty." << std::endl;

          log_ << INFO << "\tConsumer accessing front of work queue." << std::endl;
          UnitPtr unit;
          work_.pop(unit);
          log_ << INFO << "\tConsumer popping front of work queue." << std::endl;

          log_ << INFO << "~~~Processing " << unit->id << ": sleeping for " << unit->runtime << " ms" << std::endl;

          unit->run();

          done_.push_back(unit);

          log_ << INFO << "\tConsumer: adding done unit, done size = " << done_.size() << std::endl;
        }
      }
      log_ << INFO << "Consumer done." << std::endl;
    }

    bool moreWork()
    {
      return !work_.empty();
    }

  private:
    WorkQueue2& work_;
    const WorkUnitProducer2& producer_;
    DoneList& done_;
    static Log& log_;
  };

  Log& WorkUnitProducer2::log_ = Log::get();
  Log& WorkUnitConsumer2::log_ = Log::get();


  TEST(MultiExecutorPrototypeTest, DISABLED_Run2)
  {
    const int size = 20;
    WaitingList list;
    std::generate_n(std::back_inserter(list), size, makeUnit);

    int totalSleepTime = std::accumulate(list.begin(), list.end(), 0, [](int total, UnitPtr u){ return total + u->runtime; });
    std::cout << size << " work units, will sleep for total of " << totalSleepTime / 1000 << " seconds" << std::endl;

    WorkQueue2 workQ(list.size());
    WorkUnitProducer2 producer(workQ, list);

    DoneList done;
    WorkUnitConsumer2 consumer(workQ, producer, done);

    boost::thread tR = boost::thread(boost::bind(&WorkUnitProducer2::run, producer));
    boost::thread tC = boost::thread(boost::bind(&WorkUnitConsumer2::run, consumer));

    tR.join();
    tC.join();

    EXPECT_EQ(size, done.size());
    std::cout << "DONE. Finished list first 10:" << std::endl;
    auto end = done.begin();
    std::advance(end, 10);
    std::copy(done.begin(), end, std::ostream_iterator<UnitPtr>(std::cout, "\n"));
    for (const UnitPtr& u : done)
    {
      EXPECT_TRUE(u->done);
    }
  }
}
