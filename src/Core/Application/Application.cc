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
#include <Dataflow/State/SimpleMapModuleState.h>
#include <Dataflow/Engine/Scheduler/LinearSerialNetworkExecutor.h>

using namespace SCIRun::Core;
using namespace SCIRun::Core::CommandLine;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Dataflow::State;

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
      boost::shared_ptr<NetworkEditorController> controller_;
    };
  }
}

CORE_SINGLETON_IMPLEMENTATION( Application )

Application::Application() :
	private_( new ApplicationPrivate )
{
}

Application::~Application()
{
}

ApplicationParametersHandle Application::parameters()
{
  return private_->parameters_;
}

void Application::readCommandLine(int argc, const char* argv[])
{
  private_->parameters_ = private_->parser.parse(argc, argv);
}

boost::shared_ptr<SCIRun::Dataflow::Engine::NetworkEditorController> Application::controller()
{
  if (!private_->controller_)
  {
    //TODO: these all get configured
    ModuleFactoryHandle moduleFactory(new HardCodedModuleFactory);
    ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
    NetworkExecutorHandle exe(new LinearSerialNetworkExecutor);
    private_->controller_.reset(new NetworkEditorController(moduleFactory, sf, exe));
  }
  return private_->controller_;
}

/*
std::string Application::GetVersion()
{
	return CORE_APPLICATION_VERSION;
}

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