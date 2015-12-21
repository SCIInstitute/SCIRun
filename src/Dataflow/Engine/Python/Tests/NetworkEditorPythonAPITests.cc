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

#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Modules/Legacy/Fields/CreateLatVol.h>
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Dataflow/Engine/Scheduler/DesktopExecutionStrategyFactory.h>
#include <Core/Python/PythonInterpreter.h>

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
    PythonInterpreter::Instance().initialize(false);
    PythonInterpreter::Instance().run_string("import SCIRunPythonAPI; from SCIRunPythonAPI import *");
  }
};

TEST_F(PythonControllerFunctionalTests, CanAddModule)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  std::string command = "addModule(\"CreateLatVol\")";
  PythonInterpreter::Instance().run_string(command);
  //TODO: expose API directly on NEC?
  //controller.runPython("addModule(\"CreateLatVol\")");

  ASSERT_EQ(1, controller.getNetwork()->nmodules());
}

TEST_F(PythonControllerFunctionalTests, CanAddMultipleModule)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  std::string command = "addModule(\"CreateLatVol\")";
  PythonInterpreter::Instance().run_string(command);
  PythonInterpreter::Instance().run_string(command);

  ASSERT_EQ(2, controller.getNetwork()->nmodules());
}

TEST_F(PythonControllerFunctionalTests, CanChangeModuleState)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  NetworkEditorController controller(mf, sf, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  std::string command = "m = addModule(\"CreateLatVol\")";
  PythonInterpreter::Instance().run_string(command);

  ASSERT_EQ(1, controller.getNetwork()->nmodules());
  auto mod = controller.getNetwork()->module(0);
  ASSERT_TRUE(mod != nullptr);
  EXPECT_EQ(16, mod->get_state()->getValue(CreateLatVol::XSize).toInt());
  command = "m.XSize = 14";
  PythonInterpreter::Instance().run_string(command);
  EXPECT_EQ(14, mod->get_state()->getValue(CreateLatVol::XSize).toInt());
}

TEST_F(PythonControllerFunctionalTests, CanConnectModules)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  PythonInterpreter::Instance().run_string("m1 = addModule(\"CreateLatVol\")");
  PythonInterpreter::Instance().run_string("m2 = addModule(\"CreateLatVol\")");

  ASSERT_EQ(2, controller.getNetwork()->nmodules());

  ASSERT_EQ(0, controller.getNetwork()->nconnections());

  PythonInterpreter::Instance().run_string("m1.output[0] >> m2.input[0]");
  ASSERT_EQ(1, controller.getNetwork()->nconnections());
}

//TODO: this test is unstable
TEST_F(PythonControllerFunctionalTests, DISABLED_CanExecuteNetwork)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  ExecutionStrategyFactoryHandle exe(new DesktopExecutionStrategyFactory(boost::none));
  NetworkEditorController controller(mf, sf, exe, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  PythonInterpreter::Instance().run_string("m1 = addModule(\"CreateLatVol\")");
  ASSERT_TRUE(controller.getNetwork()->module(0)->executionState().currentState() == ModuleExecutionState::NotExecuted);
  PythonInterpreter::Instance().run_string("m2 = addModule(\"CreateLatVol\")");
  PythonInterpreter::Instance().run_string("m1.output[0] >> m2.input[0]");
  PythonInterpreter::Instance().run_string("executeAll()");
 // boost::this_thread::sleep(boost::posix_time::milliseconds(500));
  ASSERT_TRUE(controller.getNetwork()->module(0)->executionState().currentState() == ModuleExecutionState::Completed);
  //TODO: how do i assert on
}

TEST_F(PythonControllerFunctionalTests, CanAddModuleWithStaticFunction)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr);
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
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr);
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
  NetworkEditorController controller(mf, sf, nullptr, nullptr, nullptr, nullptr);
  initModuleParameters(false);

  ASSERT_EQ(0, controller.getNetwork()->nmodules());

  std::string command = "m = scirun_add_module(\"CreateLatVol\")";
  PythonInterpreter::Instance().run_string(command);

  ASSERT_EQ(1, controller.getNetwork()->nmodules());
  auto mod = controller.getNetwork()->module(0);
  ASSERT_TRUE(mod != nullptr);
  EXPECT_EQ(16, mod->get_state()->getValue(CreateLatVol::XSize).toInt());
  command = "xs = scirun_get_module_state(m, \"XSize\")";
  PythonInterpreter::Instance().run_string(command);

  ////???? need to get value back!!! how??

  FAIL() << "todo";
}

TEST_F(PythonControllerFunctionalTests, CanChangeModuleStateWithStaticFunction)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  NetworkEditorController controller(mf, sf, nullptr, nullptr, nullptr, nullptr);
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
  FAIL() << "todo";
}

TEST_F(PythonControllerFunctionalTests, CanConnectModulesWithStaticFunction)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr);
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
  NetworkEditorController controller(mf, nullptr, nullptr, nullptr, nullptr, nullptr);
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