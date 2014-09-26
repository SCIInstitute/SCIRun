/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

// Includes for platform specific functions to get directory to store temp files and user data
#ifdef _WIN32
#include <shlobj.h>
#include <tlhelp32.h>
#include <windows.h>
#include <LMCons.h>
#include <psapi.h>
#else
#include <stdlib.h>
#include <sys/types.h>
#ifndef __APPLE__
#include <unistd.h>
#include <sys/sysinfo.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <sys/param.h>
#include <sys/mount.h>
#endif
#endif

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
    };
  }
}

CORE_SINGLETON_IMPLEMENTATION( Application )

Application::Application() :
	private_( new ApplicationPrivate )
{
  private_->app_filepath_ = boost::filesystem::current_path();
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

std::string Application::applicationName() const
{
  return "SCIRun";
}

ApplicationParametersHandle Application::parameters() const
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");
  return private_->parameters_;
}

void Application::readCommandLine(int argc, const char* argv[])
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");

  private_->app_filename_ = boost::filesystem::path( argv[0] );
  private_->app_filepath_ = private_->app_filename_.parent_path();
  private_->parameters_ = private_->parser.parse(argc, argv);

  Logging::Log::get().setVerbose(parameters()->verboseMode());
}

NetworkEditorControllerHandle Application::controller()
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");

  if (!private_->controller_)
  {
    /// @todo: these all get configured
    ModuleFactoryHandle moduleFactory(new HardCodedModuleFactory);
    ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
    ExecutionStrategyFactoryHandle exe(new DesktopExecutionStrategyFactory(parameters()->threadMode()));
    AlgorithmFactoryHandle algoFactory(new HardCodedAlgorithmFactory);
    ReexecuteStrategyFactoryHandle reexFactory(new DynamicReexecutionStrategyFactory(parameters()->reexecuteMode()));
    private_->controller_.reset(new NetworkEditorController(moduleFactory, sf, exe, algoFactory, reexFactory));

    /// @todo: sloppy way to initialize this but similar to v4, oh well
    IEPluginManager::Initialize();
  }
  return private_->controller_;
}

void Application::executeCommandLineRequests(Commands::GlobalCommandFactoryHandle cmdFactory)
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");

  GlobalCommandBuilderFromCommandLine builder(cmdFactory);
  auto queue = builder.build(parameters());
  queue->runAll();
}

boost::filesystem::path Application::executablePath() const
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");

  return private_->app_filepath_;
}

std::string Application::commandHelpString() const
{
  ENSURE_NOT_NULL(private_, "Application internals are uninitialized!");

  return private_->parser.describe();
}

std::string Application::version() const
{
	/// @todo:
  ///return CORE_APPLICATION_VERSION;
  return "5.0.0 developer version";
}

boost::filesystem::path Application::configDirectory() const
{
  boost::filesystem::path config;
  if (!get_config_directory(config))
    return executablePath();
  return config;
}

// following ugly code copied from Seg3D.

bool Application::get_user_directory( boost::filesystem::path& user_dir, bool config_path) const
{
#ifdef _WIN32
  TCHAR dir[MAX_PATH];

  // Try to create the local application directory
  // If it already exists return the name of the directory.

  if( config_path )
  {
    if ( SUCCEEDED( SHGetFolderPath( 0, CSIDL_LOCAL_APPDATA, 0, 0, dir ) ) )
    {
      user_dir = boost::filesystem::path( dir );
      return true;
    }
    else
    {
      Log::get() << ERROR_LOG << "Could not get user directory.";
      return false;
    }
  }
  else
  {
    if ( SUCCEEDED( SHGetFolderPath( 0, CSIDL_MYDOCUMENTS, 0, 0, dir ) ) )
    {
      user_dir = boost::filesystem::path( dir );
      return true;
    }
    else
    {
      Log::get() << ERROR_LOG << "Could not get user directory.";
      return false;
    }
  }
#else

  if ( getenv( "HOME" ) )
  {
    user_dir = boost::filesystem::path( getenv( "HOME" ) );

    if (! boost::filesystem::exists( user_dir ) )
    {
      Log::get() << ERROR_LOG << "Could not get user directory.";
      return false;
    }

    return true;
  }
  else
  {
    Log::get() << ERROR_LOG << "Could not get user directory.";
    return false;
  }
#endif
}


bool Application::get_user_desktop_directory( boost::filesystem::path& user_desktop_dir ) const
{
#ifdef _WIN32
  TCHAR dir[MAX_PATH];

  // Try to create the local application directory
  // If it already exists return the name of the directory.

  if ( SUCCEEDED( SHGetFolderPath( 0, CSIDL_DESKTOPDIRECTORY, 0, 0, dir ) ) )
  {
    user_desktop_dir = boost::filesystem::path( dir );
    return true;
  }
  else
  {
    Log::get() << ERROR_LOG << "Could not get user desktop directory.";
    return false;
  }


#else

  if ( getenv( "HOME" ) )
  {
    user_desktop_dir = boost::filesystem::path( getenv( "HOME" ) ) / "Desktop" / "";

    if (! boost::filesystem::exists( user_desktop_dir ) )
    {
      Log::get() << ERROR_LOG << "Could not get user desktop directory.";
      return false;
    }


    return true;
  }
  else
  {
    Log::get() << ERROR_LOG << "Could not get user desktop directory.";
    return false;
  }
#endif
}

bool Application::get_config_directory( boost::filesystem::path& config_dir ) const
{
  boost::filesystem::path user_dir;
  if ( !( get_user_directory( user_dir, true ) ) ) return false;

#ifdef _WIN32
  config_dir = user_dir / applicationName();
#else
  std::string dot_app_name = std::string( "." ) + applicationName();
  config_dir = user_dir / dot_app_name;
#endif

  if ( !( boost::filesystem::exists( config_dir ) ) )
  {
    if ( !( boost::filesystem::create_directory( config_dir ) ) )
    {
      Log::get() << ERROR_LOG << "Could not create directory: " << config_dir.string();
      return false;
    }

    Log::get() << INFO << "Created directory: " << config_dir.string();
  }

  return true;
}

bool Application::get_user_name( std::string& user_name ) const
{
#ifdef _WIN32
  TCHAR name[UNLEN+1];
  DWORD length = UNLEN;

  if ( GetUserName( name, &length ) )
  {
    user_name = std::string( name );
    return true;
  }
  else
  {
    Log::get() << ERROR_LOG << "Could not resolve user name.";
    return false;
  }
#else
  if ( getenv( "USER" ) )
  {
    user_name = std::string( getenv( "USER" ) );
    return true;
  }
  else
  {
    Log::get() << ERROR_LOG << "Could not resolve user name.";
    return false;
  }
#endif

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
