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


#include <boost/filesystem.hpp>

#include <Core/Application/Application.h>
#include <Core/CommandLine/CommandLine.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Dataflow/Network/ModuleReexecutionStrategies.h>
#include <Dataflow/Engine/Scheduler/DesktopExecutionStrategyFactory.h>
#include <Core/Command/GlobalCommandBuilderFromCommandLine.h>
#include <Core/Logging/Log.h>
#include <Core/Logging/ApplicationHelper.h>
#include <Core/IEPlugin/IEPluginInit.h>
#include <Core/Utils/Exception.h>
#include <Core/Application/Session/Session.h>
#include <Core/Application/Version.h>
#include <Core/Python/PythonInterpreter.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <boost/algorithm/string.hpp>
#include <Core/Thread/Parallel.h>

using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::CommandLine;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Algorithms;

namespace SCIRun
{
  namespace Core
  {
    struct ApplicationPrivate
    {
      CommandLineParser parser;
      boost::filesystem::path app_filepath_;
      boost::filesystem::path app_filename_;
      ApplicationParametersHandle parameters_;
      NetworkEditorControllerHandle controller_;
      GlobalCommandFactoryHandle cmdFactory_;
    };
  }
}

CORE_SINGLETON_IMPLEMENTATION( Application )

Application::Application() :
	private_( new ApplicationPrivate )
{
  private_->app_filepath_ = boost::filesystem::current_path();
  //std::cout << "exec path set to: " << private_->app_filepath_ << std::endl;
  auto configDir = configDirectory();
  LogSettings::Instance().setLogDirectory(configDir);
  SessionManager::Instance().initialize(configDir);
  SessionManager::Instance().session()->beginSession();
}

Application::~Application()
{
  SessionManager::Instance().session()->endSession();
}

void Application::shutdown()
{
  if (!private_)
    logInfo("Application shutdown called with null internals");
  try
  {
    private_.reset();
  }
  catch (std::exception& e)
  {
    logCritical("Unhandled exception during application shutdown: {}", e.what());
  }
  catch (...)
  {
    logCritical("Unknown unhandled exception during application shutdown");
  }
}

static ApplicationHelper applicationHelper;

std::string Application::applicationName() const
{
  return applicationHelper.applicationName();
}

void Application::setCommandFactory(GlobalCommandFactoryHandle cmdFactory)
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");
  private_->cmdFactory_ = cmdFactory;
}

ApplicationParametersHandle Application::parameters() const
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");
  return private_->parameters_;
}

void Application::readCommandLine(int argc, const char* argv[])
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");

  //std::cout << "argv[0]: " << argv[0] << std::endl;
  private_->app_filename_ = boost::filesystem::path(argv[0]);
  //std::cout << "app_filename_: " << private_->app_filename_ << std::endl;

  if (!private_->app_filename_.parent_path().empty())
  {
    //std::cout << "app_filename_ parent path: " << private_->app_filename_.parent_path() << std::endl;
    auto oldPath = private_->app_filepath_;
    private_->app_filepath_ = boost::filesystem::system_complete(private_->app_filename_.parent_path());
    //std::cout << "exec path reset to: " << private_->app_filepath_ << std::endl;
    if (!boost::filesystem::exists(private_->app_filepath_))
    {
      private_->app_filepath_ = oldPath;
      //std::cout << "reset to old path: " << oldPath << std::endl;
    }
  }

  private_->parameters_ = private_->parser.parse(argc, argv);

  //TODO: move this special logic somewhere else
  {
    auto maxCoresOption = private_->parameters_->developerParameters()->maxCores();
    if (maxCoresOption)
      Thread::Parallel::SetMaximumCores(*maxCoresOption);

    LogSettings::Instance().setVerbose(parameters()->verboseMode());
  }
}

namespace
{
#ifdef BUILD_WITH_PYTHON

  //TODO: obviously will need a better way to communicate the user-entered script string.
  class HardCodedPythonTestCommand : public ParameterizedCommand
  {
  public:
    HardCodedPythonTestCommand(const std::string& script, bool enabled) : script_(script), enabled_(enabled) {}
    virtual bool execute() override
    {
      if (!enabled_)
        return false;

      if (!script_.empty())
      {
        PythonInterpreter::Instance().importSCIRunLibrary();
        PythonInterpreter::Instance().run_script(script_);
      }
      return true;
    }
  private:
    std::string script_;
    bool enabled_;
  };

  class HardCodedPythonFactory : public NetworkEventCommandFactory
  {
  public:
    virtual CommandHandle create(NetworkEventCommands type) const override
    {
      const auto& prefs = Preferences::Instance();
      switch (type)
      {
      case NetworkEventCommands::PostModuleAdd:
        return boost::make_shared<HardCodedPythonTestCommand>(
          prefs.postModuleAdd.script.val(),
          prefs.postModuleAdd.enabled.val());
      case NetworkEventCommands::OnNetworkLoad:
        return boost::make_shared<HardCodedPythonTestCommand>(
          prefs.onNetworkLoad.script.val(),
          prefs.onNetworkLoad.enabled.val());
      case NetworkEventCommands::ApplicationStart:
        return boost::make_shared<HardCodedPythonTestCommand>(
          prefs.applicationStart.script.val(),
          prefs.applicationStart.enabled.val());
      }
      return nullptr;
    }
  };


#endif

  NetworkEventCommandFactoryHandle makeNetworkEventCommandFactory()
  {
#ifdef BUILD_WITH_PYTHON
    return boost::make_shared<HardCodedPythonFactory>();
#else
    return boost::make_shared<NullCommandFactory>();
#endif
  }
}

NetworkEditorControllerHandle Application::controller()
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");
  ENSURE_NOT_NULL(private_->cmdFactory_, "Application internals are uninitialized!");

  if (!private_->controller_)
  {
    /// @todo: these all get configured
    ModuleFactoryHandle moduleFactory(new HardCodedModuleFactory);
    ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
    ExecutionStrategyFactoryHandle exe(new DesktopExecutionStrategyFactory(parameters()->developerParameters()->threadMode()));
    AlgorithmFactoryHandle algoFactory(new HardCodedAlgorithmFactory);
    ReexecuteStrategyFactoryHandle reexFactory(new DynamicReexecutionStrategyFactory(parameters()->developerParameters()->reexecuteMode()));
    auto eventCmdFactory(makeNetworkEventCommandFactory());
    private_->controller_.reset(new NetworkEditorController(moduleFactory, sf, exe, algoFactory, reexFactory, private_->cmdFactory_, eventCmdFactory));

    /// @todo: sloppy way to initialize this but similar to v4, oh well
    IEPluginManager::Initialize();
  }
  return private_->controller_;
}

void Application::executeCommandLineRequests()
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");
  ENSURE_NOT_NULL(private_->cmdFactory_, "Application internals (command factory) are uninitialized!");

  GlobalCommandBuilderFromCommandLine builder(private_->cmdFactory_);
  auto queue = builder.build(parameters());
  queue->runAll();
}

boost::filesystem::path Application::executablePath() const
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");
  //std::cout << "exec path: " << private_->app_filepath_ << std::endl;
  return private_->app_filepath_;
}

std::string Application::commandHelpString() const
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");

  return private_->parser.describe();
}

std::string Application::version() const
{
  auto version = VersionInfo::GIT_VERSION_TAG;
  if (version.empty())
    return "5.0.0 developer version";
  if (version.find("NOTFOUND") != std::string::npos) // source zip build most likely
    return "TODO";
  return version;
}

std::string Application::moduleList()
{
  std::ostringstream ostr;
  auto map = controller()->getAllAvailableModuleDescriptions();
  for (const auto& p1 : map)
  {
    for (const auto& p2 : p1.second)
    {
      for (const auto& p3 : p2.second)
      {
        ostr << p1.first << "::" << p2.first << "::" << p3.first << std::endl;
      }
    }
  }
  return ostr.str();
}

bool Application::moduleNameExists(const std::string& name)
{
  auto map = controller()->getAllAvailableModuleDescriptions();
  for (const auto& p1 : map)
  {
    for (const auto& p2 : p1.second)
    {
      for (const auto& p3 : p2.second)
      {
        if (boost::iequals(name, p3.first))
          return true;
      }
    }
  }
  return false;
}

boost::filesystem::path Application::configDirectory() const
{
  return applicationHelper.configDirectory();
}

bool Application::get_user_directory( boost::filesystem::path& user_dir, bool config_path) const
{
  return applicationHelper.get_user_directory(user_dir, config_path);
}

bool Application::get_config_directory( boost::filesystem::path& config_dir ) const
{
  return applicationHelper.get_config_directory(config_dir);
}

bool Application::get_user_name( std::string& user_name ) const
{
  return applicationHelper.get_user_name(user_name);
}

std::string SaveFileCommandHelper::saveImpl(const std::string& filename)
{
  auto fileNameWithExtension = filename;
  if (!boost::algorithm::ends_with(fileNameWithExtension, ".srn5"))
    fileNameWithExtension += ".srn5";

  auto file = Application::Instance().controller()->saveNetwork();

  if (!XMLSerializer::save_xml(*file, fileNameWithExtension, "networkFile"))
    return "";

  return fileNameWithExtension;
}
