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

#ifndef CORE_APPLICATION_APPLICATION_H
#define CORE_APPLICATION_APPLICATION_H

#include <boost/filesystem/path.hpp>

#include <Core/Utils/Singleton.h>
#include <Core/CommandLine/CommandLine.h>
#include <Core/Application/Share.h>

namespace SCIRun
{
namespace Core
{

// CLASS APPLICATION:
/// Application is the thread that processes all the actions in the program.

class Application;
class ApplicationPrivate;
typedef boost::shared_ptr<ApplicationPrivate> ApplicationPrivateHandle;


class SCISHARE Application : boost::noncopyable //: public EventHandler, public RecursiveLockable
{
	CORE_SINGLETON( Application );

private:
	Application();
	virtual ~Application();

public:

  CommandLine::ApplicationParametersHandle parameters();


	bool is_command_line_parameter( const std::string& key );
	bool check_command_line_parameter( const std::string& key, std::string& value );
	void readCommandLine(int argc, char* argv[]);

  //TODO: following will be useful later
#if 0
public:
	void log_start();
	void log_finish();

  bool get_user_directory( boost::filesystem::path& user_dir, bool config_path = false );
	bool get_config_directory( boost::filesystem::path& config_dir );
	bool get_user_desktop_directory( boost::filesystem::path& user_desktop_dir );
	bool get_user_name( std::string& user_name );
	bool get_application_filepath( boost::filesystem::path& app_filepath );
	bool get_application_filename( boost::filesystem::path& app_filename );

	int get_process_id();

public:
	boost::signals2::signal< void () > reset_signal_;
	boost::signals2::signal< void () > application_start_signal_;
	boost::signals2::signal< void () > application_stop_signal_;
#endif

	// -- internals --
private:
	// internals of this class
	ApplicationPrivateHandle private_;
  
//public:
//	static bool IsApplicationThread();
//	static std::string GetVersion();
//	static int GetMajorVersion();
//	static int GetMinorVersion();
//	static int GetPatchVersion();
//	static bool Is64Bit();
//	static bool Is32Bit();
//	static std::string GetApplicationName();
//	static std::string GetReleaseName();
//	static std::string GetApplicationNameAndVersion();
//	static std::string GetAbout();
};

}}

//#define ASSERT_IS_APPLICATION_THREAD() assert( Core::Application::IsApplicationThread() )

#endif
