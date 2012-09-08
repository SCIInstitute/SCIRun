/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

using namespace SCIRun::Core;

namespace SCIRun
{
  namespace Core
  {
    class ApplicationPrivate
    {
    public:
      boost::filesystem::path app_filepath_;
      boost::filesystem::path app_filename_;	
    };
  }
}

CORE_SINGLETON_IMPLEMENTATION( Application );

Application::Application() :
	private_( new ApplicationPrivate )
{
}

Application::~Application()
{
}

bool Application::is_command_line_parameter( const std::string &key )
{
	return true;
}

bool Application::check_command_line_parameter( const std::string &key, std::string& value )
{
  return true;
}

void Application::parse_command_line_parameters( int argc, char **argv )
{

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