/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2026 Scientific Computing and Imaging Institute,
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


#include <gtest/gtest.h>
#include <Dataflow/Serialization/Network/NetworkToPython.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Engine/Scheduler/DesktopExecutionStrategyFactory.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/Network/PortInterface.h>
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/Option.h>
#include <limits>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Core::Algorithms;

namespace
{
  std::string literal(const Variable::Value& v)
  {
    return pythonLiteral(Variable(Name("x"), v)).value_or("<none>");
  }
}

TEST(PythonLiteralTests, Scalars)
{
  EXPECT_EQ("42", literal(42));
  EXPECT_EQ("-7", literal(-7));
  EXPECT_EQ("True", literal(true));
  EXPECT_EQ("False", literal(false));
}

TEST(PythonLiteralTests, DoublesAreShortestRoundTrip)
{
  EXPECT_EQ("0.1", literal(0.1));
  EXPECT_EQ("4.0", literal(4.0));
  EXPECT_EQ("-2.5", literal(-2.5));
  EXPECT_EQ("1e+300", literal(1e300));
  EXPECT_EQ("0.30000000000000004", literal(0.1 + 0.2));
  EXPECT_EQ("float('inf')", literal(std::numeric_limits<double>::infinity()));
  EXPECT_EQ("-float('inf')", literal(-std::numeric_limits<double>::infinity()));
  // Variable stores NaN as the string "NaN", and the python setter accepts it back.
  EXPECT_EQ("\"NaN\"", literal(std::numeric_limits<double>::quiet_NaN()));
}

TEST(PythonLiteralTests, StringsAreEscaped)
{
  EXPECT_EQ("\"plain\"", literal(std::string("plain")));
  EXPECT_EQ("\"C:\\\\data\\\\x.fld\"", literal(std::string("C:\\data\\x.fld")));
  EXPECT_EQ("\"say \\\"hi\\\"\"", literal(std::string("say \"hi\"")));
  EXPECT_EQ("\"1 2\\n3 4\\r\\t\"", literal(std::string("1 2\n3 4\r\t")));
  EXPECT_EQ("\"\\x01\\x7f\"", literal(std::string("\x01\x7f")));
  EXPECT_EQ("\"caf\xc3\xa9\"", literal(std::string("caf\xc3\xa9")));
}

TEST(PythonLiteralTests, ListsNest)
{
  Variable::List inner{ Variable(Name("a"), 1), Variable(Name("b"), std::string("s")) };
  Variable::List outer{ Variable(Name("l"), inner), Variable(Name("d"), 0.5) };
  EXPECT_EQ("[[1, \"s\"], 0.5]", literal(outer));
  EXPECT_EQ("[]", literal(Variable::List{}));
}

TEST(PythonLiteralTests, AlgoOptionHasNoLiteral)
{
  EXPECT_FALSE(pythonLiteral(Variable(Name("x"), AlgoOption("a", { "a", "b" }))));
  Variable::List withOption{ Variable(Name("o"), AlgoOption("a", { "a" })) };
  EXPECT_FALSE(pythonLiteral(Variable(Name("x"), withOption)));
}

class NetworkToPythonTests : public ::testing::Test
{
protected:
  NetworkToPythonTests() :
    controller_(ModuleFactoryHandle(new HardCodedModuleFactory),
      ModuleStateFactoryHandle(new SimpleMapModuleStateFactory),
      ExecutionStrategyFactoryHandle(new DesktopExecutionStrategyFactory(std::nullopt)),
      nullptr, nullptr, nullptr, nullptr)
  {
    Module::resetIdGenerator();
  }

  void connect(const ModuleHandle& from, size_t out, const ModuleHandle& to, size_t in)
  {
    ASSERT_TRUE(controller_.requestConnection(from->outputPorts().at(out).get(), to->inputPorts().at(in).get()));
  }

  std::string script(const NetworkFile* layout = nullptr)
  {
    return networkToPythonScript(*controller_.getNetwork(), layout, "test.srn5");
  }

  NetworkEditorController controller_;
};

TEST_F(NetworkToPythonTests, EmptyNetworkIsJustTheHeader)
{
  const auto py = script();
  EXPECT_NE(std::string::npos, py.find("from test.srn5"));
  EXPECT_EQ(std::string::npos, py.find("scirun_add_module"));
  EXPECT_EQ(std::string::npos, py.find("scirun_connect_modules"));
}

TEST_F(NetworkToPythonTests, AddsModulesSetsStateAndConnects)
{
  auto create = controller_.addModule("CreateMatrix");
  auto unary = controller_.addModule("EvaluateLinearAlgebraUnary");
  auto report = controller_.addModule("ReportMatrixInfo");
  connect(create, 0, unary, 0);
  connect(unary, 0, report, 0);
  unary->get_state()->setValue(Variables::ScalarValue, 0.1);

  const auto py = script();
  EXPECT_NE(std::string::npos, py.find("createMatrix_0 = scirun_add_module(\"CreateMatrix\")\n"));
  EXPECT_NE(std::string::npos, py.find("evaluateLinearAlgebraUnary_0 = scirun_add_module(\"EvaluateLinearAlgebraUnary\")\n"));
  EXPECT_NE(std::string::npos, py.find("set_state(evaluateLinearAlgebraUnary_0, \"ScalarValue\", 0.1)\n"));
  EXPECT_NE(std::string::npos, py.find("scirun_connect_modules(createMatrix_0, 0, evaluateLinearAlgebraUnary_0, 0)\n"));
  EXPECT_NE(std::string::npos, py.find("scirun_connect_modules(evaluateLinearAlgebraUnary_0, 0, reportMatrixInfo_0, 0)\n"));
  EXPECT_LT(py.find("scirun_add_module(\"ReportMatrixInfo\")"), py.find("set_state(createMatrix_0"));
  EXPECT_LT(py.rfind("set_state("), py.find("scirun_connect_modules"));
}

TEST_F(NetworkToPythonTests, DynamicPortsConnectInPortOrder)
{
  auto a = controller_.addModule("CreateMatrix");
  auto b = controller_.addModule("CreateMatrix");
  auto c = controller_.addModule("CreateMatrix");
  auto append = controller_.addModule("AppendMatrix");
  connect(a, 0, append, 0);
  connect(c, 0, append, 2);
  connect(b, 0, append, 3);

  const auto py = script();
  const auto first = py.find("scirun_connect_modules(createMatrix_0, 0, appendMatrix_0, 0)\n");
  const auto second = py.find("scirun_connect_modules(createMatrix_2, 0, appendMatrix_0, 2)\n");
  const auto third = py.find("scirun_connect_modules(createMatrix_1, 0, appendMatrix_0, 3)\n");
  ASSERT_NE(std::string::npos, first);
  ASSERT_NE(std::string::npos, second);
  ASSERT_NE(std::string::npos, third);
  EXPECT_LT(first, second);
  EXPECT_LT(second, third);
}

TEST_F(NetworkToPythonTests, LayoutAddsPositionsAndDisabledComponents)
{
  auto create = controller_.addModule("CreateMatrix");
  auto report = controller_.addModule("ReportMatrixInfo");
  connect(create, 0, report, 0);

  NetworkFile layout;
  layout.modulePositions.modulePositions["CreateMatrix:0"] = { 10.5, -20 };
  layout.disabledComponents.disabledConnections.push_back("ReportMatrixInfo:0--CreateMatrix:0");
  layout.disabledComponents.disabledModules.push_back("ReportMatrixInfo:0");

  const auto py = script(&layout);
  EXPECT_NE(std::string::npos, py.find("scirun_move_module(createMatrix_0, 10.5, -20.0)\n"));
  EXPECT_EQ(std::string::npos, py.find("scirun_move_module(reportMatrixInfo_0"));
  EXPECT_NE(std::string::npos, py.find("scirun_disable_connection(createMatrix_0, 0, reportMatrixInfo_0, 0)\n"));
  EXPECT_NE(std::string::npos, py.find("# ReportMatrixInfo:0 is disabled"));
}

TEST_F(NetworkToPythonTests, PortLabelStateComesAfterConnections)
{
  auto a = controller_.addModule("CreateMatrix");
  auto append = controller_.addModule("AppendMatrix");
  connect(a, 0, append, 2);
  const auto label = append->inputPorts().at(2)->externalId().toString();
  append->get_state()->setValue(Name(label), std::string("first"));

  const auto py = script();
  const auto labelLine = py.find("set_state(appendMatrix_0, \"" + label + "\", \"first\")\n");
  ASSERT_NE(std::string::npos, labelLine);
  EXPECT_LT(py.find("scirun_connect_modules"), labelLine);
  EXPECT_LT(py.find("set_state(appendMatrix_0, \"RowsOrColumns\""), py.find("scirun_connect_modules"));
}

// Renumbered IDs (ShowField:9 vs ShowField:10) must not reorder connections.
TEST_F(NetworkToPythonTests, ConnectionsFollowModuleOrderNotIdText)
{
  std::vector<ModuleHandle> creates;
  for (int i = 0; i < 11; ++i)
    creates.push_back(controller_.addModule("CreateMatrix"));
  std::vector<ModuleHandle> reports;
  for (int i = 0; i < 11; ++i)
    reports.push_back(controller_.addModule("ReportMatrixInfo"));
  connect(creates[10], 0, reports[10], 0);
  connect(creates[9], 0, reports[9], 0);

  const auto py = script();
  const auto nine = py.find("scirun_connect_modules(createMatrix_9, 0, reportMatrixInfo_9, 0)");
  const auto ten = py.find("scirun_connect_modules(createMatrix_10, 0, reportMatrixInfo_10, 0)");
  ASSERT_NE(std::string::npos, nine);
  ASSERT_NE(std::string::npos, ten);
  EXPECT_LT(nine, ten);
}
