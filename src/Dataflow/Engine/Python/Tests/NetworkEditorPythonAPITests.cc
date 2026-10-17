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


#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Modules/Legacy/Fields/CreateLatVol.h>
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Dataflow/Engine/Scheduler/DesktopExecutionStrategyFactory.h>
#include <Core/Python/PythonInterpreter.h>
#include <boost/filesystem.hpp>
#include <Dataflow/Serialization/Network/NetworkToPython.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Modules/Math/CreateMatrix.h>
#include <limits>
#include <set>

using namespace SCIRun;
using namespace Core;
using namespace Testing;
using namespace Modules::Factory;
using namespace Modules::Fields;
using namespace Dataflow::Networks;
using namespace Dataflow::Engine;
using namespace ReplacementImpl;
using namespace Dataflow::Engine;
using namespace Dataflow::State;
using namespace Algorithms;

class PythonControllerFunctionalTests : public ModuleTest
{
public:
  PythonControllerFunctionalTests()
  {
    PythonInterpreter::Instance().initialize(false, "Engine_Python_Tests", boost::filesystem::current_path().string());
    PythonInterpreter::Instance().importSCIRunLibrary();
  }
};

TEST_F(PythonControllerFunctionalTests, CanAddModule)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  std::string command = "scirun_add_module(\"CreateLatVol\")";
  PythonInterpreter::Instance().run_string(command);
  //TODO: expose API directly on NEC?
  //controller.runPython("addModule(\"CreateLatVol\")");

  ASSERT_EQ(1, controller.getNetwork()->nmodules());
}

TEST_F(PythonControllerFunctionalTests, CanAddMultipleModule)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  std::string command = "scirun_add_module(\"CreateLatVol\")";
  PythonInterpreter::Instance().run_string(command);
  PythonInterpreter::Instance().run_string(command);

  ASSERT_EQ(2, controller.getNetwork()->nmodules());
}

TEST_F(PythonControllerFunctionalTests, CanChangeModuleState)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  NetworkEditorController controller(mf, sf, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  std::string command = "m = scirun_add_module(\"CreateLatVol\")";
  PythonInterpreter::Instance().run_string(command);

  ASSERT_EQ(1, controller.getNetwork()->nmodules());
  auto mod = controller.getNetwork()->module(0);
  ASSERT_TRUE(mod != nullptr);
  EXPECT_EQ(16, mod->get_state()->getValue(CreateLatVol::XSize).toInt());
  command = "scirun_set_module_state(m, \"XSize\", 14)";
  PythonInterpreter::Instance().run_string(command);
  EXPECT_EQ(14, mod->get_state()->getValue(CreateLatVol::XSize).toInt());
}

TEST_F(PythonControllerFunctionalTests, CanConnectModules)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  PythonInterpreter::Instance().run_string("m1 = scirun_add_module(\"CreateLatVol\")");
  PythonInterpreter::Instance().run_string("m2 = scirun_add_module(\"CreateLatVol\")");

  ASSERT_EQ(2, controller.getNetwork()->nmodules());

  ASSERT_EQ(0, controller.getNetwork()->nconnections());

  PythonInterpreter::Instance().run_string("scirun_connect_modules(m1, 0, m2, 0)");
  ASSERT_EQ(1, controller.getNetwork()->nconnections());
}

//TODO: this test is unstable
TEST_F(PythonControllerFunctionalTests, DISABLED_CanExecuteNetwork)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  ExecutionStrategyFactoryHandle exe(new DesktopExecutionStrategyFactory(std::nullopt));
  NetworkEditorController controller(mf, sf, exe, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  PythonInterpreter::Instance().run_string("m1 = scirun_add_module(\"CreateLatVol\")");
  ASSERT_TRUE(controller.getNetwork()->module(0)->executionState().currentState() == ModuleExecutionState::Value::NotExecuted);
  PythonInterpreter::Instance().run_string("m2 = scirun_add_module(\"CreateLatVol\")");
  PythonInterpreter::Instance().run_string("scirun_connect_modules(m1, 0, m2, 0)");
  PythonInterpreter::Instance().run_string("scirun_execute_all()");
 // boost::this_thread::sleep(boost::posix_time::milliseconds(500));
  ASSERT_TRUE(controller.getNetwork()->module(0)->executionState().currentState() == ModuleExecutionState::Value::Completed);
  //TODO: how do i assert on
}

TEST_F(PythonControllerFunctionalTests, CanAddModuleWithStaticFunction)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  std::string command = "scirun_add_module(\"CreateLatVol\")";
  PythonInterpreter::Instance().run_string(command);
  //TODO: expose API directly on NEC?
  //controller.runPython("addModule(\"CreateLatVol\")");

  ASSERT_EQ(1, controller.getNetwork()->nmodules());
}

TEST_F(PythonControllerFunctionalTests, CanAddMultipleModulesWithStaticFunction)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  std::string command = "scirun_add_module(\"CreateLatVol\")";
  PythonInterpreter::Instance().run_string(command);
  PythonInterpreter::Instance().run_string(command);

  ASSERT_EQ(2, controller.getNetwork()->nmodules());
}

TEST_F(PythonControllerFunctionalTests, CanGetModuleStateWithStaticFunction)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  NetworkEditorController controller(mf, sf, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  std::string command = "m = scirun_add_module(\"CreateLatVol\")";
  PythonInterpreter::Instance().run_string(command);

  ASSERT_EQ(1, controller.getNetwork()->nmodules());
  auto mod = controller.getNetwork()->module(0);
  ASSERT_TRUE(mod != nullptr);
  EXPECT_EQ(16, mod->get_state()->getValue(CreateLatVol::XSize).toInt());
  command = "scirun_get_module_state(m, \"XSize\")";
  PythonInterpreter::Instance().run_string(command);

  ////???? need to get value back!!! how??
}

TEST_F(PythonControllerFunctionalTests, CanChangeModuleStateWithStaticFunction)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  NetworkEditorController controller(mf, sf, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  std::string command = "m = scirun_add_module(\"CreateLatVol\")";
  PythonInterpreter::Instance().run_string(command);

  ASSERT_EQ(1, controller.getNetwork()->nmodules());
  auto mod = controller.getNetwork()->module(0);
  ASSERT_TRUE(mod != nullptr);
  EXPECT_EQ(16, mod->get_state()->getValue(CreateLatVol::XSize).toInt());
  command = "scirun_set_module_state(m, \"XSize\", 14)";
  PythonInterpreter::Instance().run_string(command);
  EXPECT_EQ(14, mod->get_state()->getValue(CreateLatVol::XSize).toInt());
 // FAIL() << "todo";
}

TEST_F(PythonControllerFunctionalTests, CanConnectModulesWithStaticFunction)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  PythonInterpreter::Instance().run_string("m1 = scirun_add_module(\"CreateLatVol\")");
  PythonInterpreter::Instance().run_string("m2 = scirun_add_module(\"CreateLatVol\")");

  ASSERT_EQ(2, controller.getNetwork()->nmodules());

  ASSERT_EQ(0, controller.getNetwork()->nconnections());

  PythonInterpreter::Instance().run_string("scirun_connect_modules(m1, 0, m2, 0)");
  ASSERT_EQ(1, controller.getNetwork()->nconnections());
}

TEST_F(PythonControllerFunctionalTests, CanDisconnectModulesWithStaticFunction)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  PythonInterpreter::Instance().run_string("m1 = scirun_add_module(\"CreateLatVol\")");
  PythonInterpreter::Instance().run_string("m2 = scirun_add_module(\"CreateLatVol\")");

  ASSERT_EQ(2, controller.getNetwork()->nmodules());

  ASSERT_EQ(0, controller.getNetwork()->nconnections());

  PythonInterpreter::Instance().run_string("c = scirun_connect_modules(m1, 0, m2, 0)");
  ASSERT_EQ(1, controller.getNetwork()->nconnections());

  PythonInterpreter::Instance().run_string("scirun_disconnect_modules(m1, 0, m2, 0)");

  ASSERT_EQ(0, controller.getNetwork()->nconnections());
}

TEST_F(PythonControllerFunctionalTests, RunScriptReportsFailure)
{
  auto& py = PythonInterpreter::Instance();
  EXPECT_TRUE(py.run_script("x = 1 + 1"));
  EXPECT_FALSE(py.run_script("raise RuntimeError('boom')"));
  EXPECT_FALSE(py.run_script("def broken(:\n  pass"));
  // A failure must not poison the next script.
  EXPECT_TRUE(py.run_script("y = 2"));
}

// Headless has no Python console to import the API; InterfaceWithPython relies
// on this being callable repeatedly and cheaply (#2699).
TEST_F(PythonControllerFunctionalTests, ScriptSeesSCIRunAPIAfterImport)
{
  auto& py = PythonInterpreter::Instance();
  py.importSCIRunLibrary();
  py.importSCIRunLibrary();
  EXPECT_TRUE(py.run_script("callable(scirun_get_module_input_value)"));
  EXPECT_TRUE(py.run_script("assert callable(scirun_get_module_input_value)"));
}

TEST_F(PythonControllerFunctionalTests, SetStateKeepsFullDoublePrecision)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  NetworkEditorController controller(mf, sf, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  auto& py = PythonInterpreter::Instance();
  ASSERT_TRUE(py.run_script("m = scirun_add_module(\"CreateLatVol\")\n"
    "scirun_set_module_state(m, \"PadPercent\", 0.1)\n"
    "scirun_set_module_state(m, \"XSize\", 7.0)"));
  auto state = controller.getNetwork()->module(0)->get_state();
  EXPECT_EQ(0.1, state->getValue(CreateLatVol::PadPercent).toDouble());
  EXPECT_EQ(7, state->getValue(CreateLatVol::XSize).toInt());
}

TEST_F(PythonControllerFunctionalTests, SetStateCoercesBoolsAndInts)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  NetworkEditorController controller(mf, sf, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_TRUE(PythonInterpreter::Instance().run_script(
    "m = scirun_add_module(\"CreateLatVol\")\n"
    "scirun_set_module_state(m, \"ElementSizeNormalized\", True)\n"
    "scirun_set_module_state(m, \"ProgrammableInputPortEnabled\", 0)"));
  auto state = controller.getNetwork()->module(0)->get_state();
  EXPECT_EQ(1, state->getValue(CreateLatVol::ElementSizeNormalized).toInt());
  EXPECT_FALSE(state->getValue(Name("ProgrammableInputPortEnabled")).toBool());
}

// EvaluateLinearAlgebraUnary defaults ScalarValue to int 0 but reads it as a double.
TEST_F(PythonControllerFunctionalTests, SetStateKeepsFractionInIntDefaultedState)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  NetworkEditorController controller(mf, sf, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_TRUE(PythonInterpreter::Instance().run_script(
    "m = scirun_add_module(\"EvaluateLinearAlgebraUnary\")\n"
    "scirun_set_module_state(m, \"ScalarValue\", 2.5)"));
  auto state = controller.getNetwork()->module(0)->get_state();
  EXPECT_EQ(2.5, state->getValue(Variables::ScalarValue).toDouble());
}

TEST_F(PythonControllerFunctionalTests, ExportedScriptRebuildsNetwork)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  initModuleParameters(false);

  std::string script;
  std::vector<std::pair<std::string, ModuleStateHandle>> originalStates;
  std::set<std::string> originalConnections;
  {
    // Scoped: the python API binds to the first live controller, so this one must be gone before the rebuild.
    NetworkEditorController original(mf, sf, nullptr, nullptr, nullptr, nullptr, nullptr);
    Module::resetIdGenerator();
    auto latVol = original.addModule("CreateLatVol");
    auto a = original.addModule("CreateMatrix");
    auto b = original.addModule("CreateMatrix");
    auto c = original.addModule("CreateMatrix");
    auto append = original.addModule("AppendMatrix");
    auto unary = original.addModule("EvaluateLinearAlgebraUnary");
    auto nanUnary = original.addModule("EvaluateLinearAlgebraUnary");
    auto connect = [&](const ModuleHandle& from, const ModuleHandle& to, size_t in)
    {
      EXPECT_TRUE(original.requestConnection(from->outputPorts().at(0).get(), to->inputPorts().at(in).get()));
    };
    connect(a, append, 0);
    connect(b, append, 1);
    connect(c, append, 2);
    connect(b, append, 3);
    connect(append, unary, 0);

    latVol->get_state()->setValue(CreateLatVol::XSize, 14);
    latVol->get_state()->setValue(CreateLatVol::PadPercent, 0.1);
    a->get_state()->setValue(Math::Parameters::TextEntry, std::string("1 2\n3 4"));
    b->get_state()->setValue(Math::Parameters::TextEntry, std::string("C:\\path \"quoted\""));
    unary->get_state()->setValue(Variables::ScalarValue, 0.1);
    nanUnary->get_state()->setValue(Variables::ScalarValue, std::numeric_limits<double>::quiet_NaN());

    script = networkToPythonScript(*original.getNetwork(), nullptr);
    for (size_t i = 0; i < original.getNetwork()->nmodules(); ++i)
    {
      auto m = original.getNetwork()->module(i);
      originalStates.emplace_back(m->id().id_, m->get_state());
    }
    for (const auto& desc : original.getNetwork()->connections(false))
      originalConnections.insert(ConnectionId::create(desc).id_);
  }

  NetworkEditorController rebuilt(mf, sf, nullptr, nullptr, nullptr, nullptr, nullptr);
  Module::resetIdGenerator();
  ASSERT_TRUE(PythonInterpreter::Instance().run_script(script)) << script;

  auto network = rebuilt.getNetwork();
  ASSERT_EQ(originalStates.size(), network->nmodules());
  for (const auto& [id, state] : originalStates)
  {
    auto module = network->lookupModule(ModuleId(id));
    ASSERT_TRUE(module) << id;
    for (const auto& key : state->getKeys())
      EXPECT_EQ(state->getValue(key), module->get_state()->getValue(key)) << id << " " << key.name();
  }

  std::set<std::string> rebuiltConnections;
  for (const auto& desc : network->connections(false))
    rebuiltConnections.insert(ConnectionId::create(desc).id_);
  EXPECT_EQ(originalConnections, rebuiltConnections);
}
