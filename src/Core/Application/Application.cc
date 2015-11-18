/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
#include <Dataflow/Network/Module.h>  //TODO move Reex
#include <Dataflow/Engine/Scheduler/DesktopExecutionStrategyFactory.h>
#include <Core/Command/GlobalCommandBuilderFromCommandLine.h>
#include <Core/Logging/Log.h>
#include <Core/IEPlugin/IEPluginInit.h>
#include <Core/Utils/Exception.h>
#include <Core/Application/Session/Session.h>
#include <Core/Application/Version.h>
#include <Core/Services/ServiceLog.h>
#include <Core/Services/ServiceDB.h>
#include <Core/Services/ServiceManager.h>

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
    class ApplicationPrivate
    {
    public:
      CommandLineParser parser;
      boost::filesystem::path app_filepath_;
      boost::filesystem::path app_filename_;
      ApplicationParametersHandle parameters_;
      NetworkEditorControllerHandle controller_;
      GlobalCommandFactoryHandle cmdFactory_;
      void start_eai();
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
  Log::setLogDirectory(configDir);
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
    Log::get() << NOTICE << "Application shutdown called with null internals" << std::endl;
  try
  {
    private_.reset();
  }
  catch (std::exception& e)
  {
    Log::get() << EMERG << "Unhandled exception during application shutdown: " << e.what() << std::endl;
  }
  catch (...)
  {
    Log::get() << EMERG << "Unknown unhandled exception during application shutdown" << std::endl;
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

  Logging::Log::get().setVerbose(parameters()->verboseMode());
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
    ExecutionStrategyFactoryHandle exe(new DesktopExecutionStrategyFactory(parameters()->threadMode()));
    AlgorithmFactoryHandle algoFactory(new HardCodedAlgorithmFactory);
    ReexecuteStrategyFactoryHandle reexFactory(new DynamicReexecutionStrategyFactory(parameters()->reexecuteMode()));
    private_->controller_.reset(new NetworkEditorController(moduleFactory, sf, exe, algoFactory, reexFactory, private_->cmdFactory_));

    /// @todo: sloppy way to initialize this but similar to v4, oh well
    IEPluginManager::Initialize();

    /// @todo split out into separate piece
    // TODO: turn off until Matlab services are converted.
    // private_->start_eai();
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
  return VersionInfo::GIT_VERSION_TAG.empty() ? "5.0.0 developer version" : VersionInfo::GIT_VERSION_TAG;
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
  return ostr.str();;
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

/*
int Application::GetMajorVersion()
{
	return CORE_APPLICATION_MAJOR_VERSION;
}

int Application::GetMinorVersion()
{
	return CORE_APPLICATION_MINOR_VERSION;
}

int Application::GetPatchVersion()
{
	return CORE_APPLICATION_PATCH_VERSION;
}

bool Application::Is64Bit()
{
	return ( sizeof(void *) == 8 );
}

bool Application::Is32Bit()
{
	return ( sizeof(void *) == 4 );
}

std::string Application::GetApplicationName()
{
	return CORE_APPLICATION_NAME;
}

std::string Application::GetReleaseName()
{
	return CORE_APPLICATION_RELEASE;
}

std::string Application::GetApplicationNameAndVersion()
{
	return GetApplicationName() + " " + GetReleaseName() + " " + GetVersion();
}

std::string Application::GetAbout()
{
	return CORE_APPLICATION_ABOUT;
}
*/


// Services start up...
void ApplicationPrivate::start_eai()
{
  // Create a database of all available services. The next piece of code
  // scans both the SCIRun as well as the Packages directories to find
  // services that need to be started. Services allow communication with
  // thirdparty software and are Threads that run asychronously with
  // with the rest of SCIRun. Since the thirdparty software may be running
  // on a different platform it allows for connecting to remote machines
  // and running the service on a different machine
  ServiceDBHandle servicedb(new ServiceDB);
  // load all services and find all makers
  servicedb->loadpackages();
  // activate all services
  servicedb->activateall();

  // Services are started and created by the ServiceManager,
  // which will be launched here
  // Two competing managers will be started,
  // one for purely internal usage and one that
  // communicates over a socket.
  // The latter will only be created if a port is set.
  // If the current instance of SCIRun should not provide any services
  // to other instances of SCIRun over the internet,
  // the second manager will not be launched

  IComAddressHandle internaladdress(new IComAddress("internal","servicemanager"));

// Only build log file if needed for debugging
#ifdef DEBUG
  const char *chome = sci_getenv("HOME");
  std::string scidir("");
  if (chome)
    scidir = chome+std::string("/SCIRun/");

  // A log file is not necessary but handy for debugging purposes
  ServiceLogHandle internallogfile(new ServiceLog(scidir+"scirun_internal_servicemanager.log"));

  ServiceManagerHandle internal_service_manager(new ServiceManager(servicedb, internaladdress, internallogfile));
#else
  ServiceManager internal_service_manager(servicedb, internaladdress);
#endif

  boost::thread t_int(internal_service_manager);
  t_int.detach();


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  // Use the following environment setting to switch on IPv6 support
  // Most machines should be running a dual-host stack for the internet
  // connections, so it should not hurt to run in IPv6 mode. In most case
  // ipv4 address will work as well.
  // It might be useful
  std::string ipstr(sci_getenv_p("SCIRUN_SERVICE_IPV6")?"ipv6":"");

  // Start an external service as well
  const char *serviceport_str = sci_getenv("SCIRUN_SERVICE_PORT");
  // If its not set in the env, we're done
  if (!serviceport_str) return;

  // The protocol for conencting has been called "scirun"
  // In the near future this should be replaced with "sciruns" for
  // a secure version which will run over ssl.

  // A log file is not necessary but handy for debugging purposes

  IComAddress externaladdress("scirun","",serviceport_str,ipstr);

#ifdef DEBUG
  ServiceLogHandle externallogfile =
    new ServiceLog(scidir+"scirun_external_servicemanager.log");

  ServiceManager* external_service_manager =
    new ServiceManager(servicedb,externaladdress,externallogfile);
#else
  ServiceManager* external_service_manager =
    new ServiceManager(servicedb,externaladdress);

#endif

  Thread* t_ext =
    new Thread(external_service_manager,"external service manager",
		  0, Thread::NotActivated);
  t_ext->setStackSize(1024*20);
  t_ext->activate(false);
  t_ext->detach();
#endif
}
