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


#include <atomic>
#include <cstdlib>
#include <future>
#include <memory>
#include <Core/ConsoleApplication/ConsoleCommands.h>
#include <Core/Utils/QuickExit.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Core/Application/Application.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/Importer/NetworkIO.h>
#include <Dataflow/Network/Module.h>
#include <Core/Logging/ConsoleLogger.h>
#include <Core/Python/PythonInterpreter.h>
#include <boost/algorithm/string.hpp>
#include <Core/Application/Preferences/Preferences.h>

using namespace SCIRun::Core;
using namespace Commands;
using namespace Console;
using namespace Logging;
using namespace SCIRun::Dataflow::Networks;
using namespace Algorithms;

LoadFileCommandConsole::LoadFileCommandConsole()
{
  addParameter(Name("FileNum"), 0);
}

//TODO: find a better place for this function
namespace
{
  void quietModulesIfNotVerbose()
  {
    if (!Application::Instance().parameters()->verboseMode())
      DefaultModuleFactories::defaultLogger_.reset(new Logging::NullLogger);
  }
}

/// @todo: real logger
#define LOG_CONSOLE(x) std::cout << "[SCIRun] " << x << std::endl;

std::string NetworkFileProcessCommandConsole::resolveFilename() const
{
  return get(Variables::Filename).toFilename().string();
}

bool NetworkFileProcessCommandConsole::execute()
{
  quietModulesIfNotVerbose();

  auto filename = resolveFilename();
  const auto verb = actionVerb();

  LOG_CONSOLE("Attempting " << verb << " of " << filename);
  if (!boost::filesystem::exists(filename))
  {
    LOG_CONSOLE("File does not exist: " << filename);
    return false;
  }
  try
  {
    auto file = processFile(filename);

    if (file)
    {
      Application::Instance().controller()->clear();
      Application::Instance().controller()->loadNetwork(file);
      LOG_CONSOLE("File " << verb << " done: " << filename);
      return true;
    }
    LOG_CONSOLE("File " << verb << " failed: " << filename);
  }
  catch (std::exception& e)
  {
    LOG_CONSOLE("File " << verb << " failed: " << filename << ", exception: " << e.what());
  }
  catch (...)
  {
    LOG_CONSOLE("File " << verb << " failed: " << filename);
  }

  // Mirrors FileImportCommand/NetworkFileProcessCommand (GUI): a failure that
  // matters in regression mode must exit non-zero so ctest reports it,
  // instead of silently continuing.
  if (failTestOnErrorInRegressionMode() && Application::Instance().parameters()->isRegressionMode())
  {
    LOG_CONSOLE("Regression " << verb << " failed, exiting non-zero: " << filename);
    // quickExit rather than quick_exit: the latter has no declaration on macOS
    // SDKs < 15 and emits a load-time-fatal reference on the 15 SDK (see #2564).
    quickExit(1);
  }
  return false;
}

std::string LoadFileCommandConsole::resolveFilename() const
{
  auto inputFiles = Application::Instance().parameters()->inputFiles();
  if (!inputFiles.empty())
    return inputFiles[0];
  return NetworkFileProcessCommandConsole::resolveFilename();
}

NetworkFileHandle LoadFileCommandConsole::processFile(const std::string& filename) const
{
  return XMLSerializer::load_xml<NetworkFile>(filename);
}

NetworkFileHandle ImportFileCommandConsole::processFile(const std::string& filename) const
{
  auto dtdPath = Core::Application::Instance().executablePath();
  const auto& modFactory = Core::Application::Instance().controller()->moduleFactory();
  std::ostringstream legacyImportLog;
  LegacyNetworkIO lnio(dtdPath.string(), modFactory, legacyImportLog);
  return lnio.load_net(filename);
}

bool SaveFileCommandConsole::execute()
{
  return !saveImpl(get(Variables::Filename).toFilename().string()).empty();
}

namespace
{
  /// Blocks until a network execution finishes, however the execution manager
  /// chooses to report it: ExecutionQueueManager signals completion and returns
  /// no future at all, so waiting on the future alone dereferences null (#2688).
  /// Construct before starting execution -- the slot has to be armed first.
  class ExecutionCompletionWaiter
  {
  public:
    explicit ExecutionCompletionWaiter(SCIRun::Dataflow::Engine::NetworkEditorController& controller)
      : future_(finished_->get_future())
    {
      auto finished = finished_;
      auto alreadyFinished = alreadyFinished_;
      connection_ = controller.connectStaticNetworkExecutionFinished(
        [finished, alreadyFinished](int code)
        {
          LOG_CONSOLE("Execution finished with code " << code);
          // The signal can fire more than once; a second set_value would throw.
          if (!alreadyFinished->exchange(true))
            finished->set_value(code);
        });
    }

    ~ExecutionCompletionWaiter() { connection_.disconnect(); }

    ExecutionCompletionWaiter(const ExecutionCompletionWaiter&) = delete;
    ExecutionCompletionWaiter& operator=(const ExecutionCompletionWaiter&) = delete;

    void wait(std::future<int>& executionResult)
    {
      if (executionResult.valid())
        executionResult.wait();
      else
        future_.wait();
    }

  private:
    // Shared with the slot, which may outlive this object if the signal fires
    // while it is being destroyed.
    std::shared_ptr<std::promise<int>> finished_{std::make_shared<std::promise<int>>()};
    std::shared_ptr<std::atomic<bool>> alreadyFinished_{std::make_shared<std::atomic<bool>>(false)};
    std::future<int> future_;
    boost::signals2::connection connection_;
  };
}

bool ExecuteCurrentNetworkCommandConsole::execute()
{
  LOG_CONSOLE("Executing network...");
  auto& controller = *Application::Instance().controller();

  {
    // Scoped so the slot is disconnected before interactive mode, which can
    // start executions of its own.
    ExecutionCompletionWaiter waiter(controller);
    controller.stopExecutionContextLoopWhenExecutionFinishes();
    auto result = controller.executeAll();
    LOG_CONSOLE("Execution started.");
    waiter.wait(result);
  }

  LOG_CONSOLE("Execute thread stopped. Entering interactive mode.");

  InteractiveModeCommandConsole interactive;
  return interactive.execute();
}

QuitAfterExecuteCommandConsole::QuitAfterExecuteCommandConsole()
{
  addParameter(Name("RunningPython"), false);
}

bool QuitAfterExecuteCommandConsole::execute()
{
  LOG_CONSOLE("Quit after execute is set.");
  Application::Instance().controller()->connectStaticNetworkExecutionFinished([](int code)
  {
    LOG_CONSOLE("Goodbye! Exit code: " << code);
    exit(code);
  });
  return true;
}

QuitCommandConsole::QuitCommandConsole()
{
  addParameter(Name("RunningPython"), false);
}

bool QuitCommandConsole::execute()
{
  LOG_CONSOLE("Goodbye!");
  exit(0);
  return true;
}

bool PrintHelpCommand::execute()
{
  std::cout << Application::Instance().commandHelpString() << std::endl;
  return true;
}

bool PrintVersionCommand::execute()
{
  std::cout << Application::Instance().version() << std::endl;
  return true;
}

bool PrintModulesCommand::execute()
{
  std::cout << "MODULE LIST as of " << Application::Instance().version() << "\n" << Application::Instance().moduleList() << std::endl;
  return true;
}

bool InteractiveModeCommandConsole::execute()
{
#ifdef BUILD_WITH_PYTHON
  quietModulesIfNotVerbose();
  PythonInterpreter::Instance().importSCIRunLibrary();
  std::string line;

#ifndef WIN32
  LOG_CONSOLE("\033[1; 31mEntering interactive mode, type quit or hit ^C to exit.\033[0m");
#else
  LOG_CONSOLE("Entering interactive mode, type quit or hit ^C to exit.");
#endif
  while (true)
  {
    std::cout << "scirun5> " << std::flush;
    std::getline(std::cin, line);
    if (line == "quit")
      break;
    if (std::cin.eof())
      break;
    if (!PythonInterpreter::Instance().run_string(line))
      break;
  }
  std::cout << std::endl;
  LOG_CONSOLE("~~~~~~~");
  LOG_CONSOLE("Goodbye!");
  LOG_CONSOLE("~~~~~~~");
  exit(0);
#endif
  return true;
}

bool RunPythonScriptCommandConsole::execute()
{
  quietModulesIfNotVerbose();

  auto& app = Application::Instance();
  auto script = app.parameters()->pythonScriptFile();
  if (script)
  {
#ifdef BUILD_WITH_PYTHON
    LOG_CONSOLE("RUNNING PYTHON SCRIPT: " << *script);

    app.controller()->clear();
    PythonInterpreter::Instance().importSCIRunLibrary();

    if (app.parameters()->quitAfterOneScriptedExecution())
    {
      app.controller()->connectStaticNetworkExecutionFinished([](int code){ LOG_CONSOLE("Execution finished with code " << code); exit(code); });
      app.controller()->stopExecutionContextLoopWhenExecutionFinishes();
    }

    if (!PythonInterpreter::Instance().run_file(script->string()))
    {
      return false;
    }

    LOG_CONSOLE("Done running Python script.");

    if (!app.parameters()->quitAfterOneScriptedExecution())
    {
      InteractiveModeCommandConsole interactive;
      return interactive.execute();
    }

    return true;
#else
    LOG_CONSOLE("Python disabled, cannot run script " << *script);
    return false;
#endif
  }
  return false;
}

bool SetupDataDirectoryCommand::execute()
{
  auto dir = *Application::Instance().parameters()->dataDirectory();
  LOG_DEBUG("Data dir set to: {}", dir.string());

  runPythonString(Preferences::Instance().setDataDirectory(dir));

  return true;
}
