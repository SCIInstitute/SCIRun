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


#include <Core/Logging/ApplicationHelper.h>
#include <boost/filesystem.hpp>
#include <iostream>

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

using namespace SCIRun::Core::Logging;

// following ugly code copied from Seg3D.

bool ApplicationHelper::get_user_directory( boost::filesystem::path& user_dir, bool config_path)
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
      std::cerr << "Could not get user directory.";
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
      std::cerr << "Could not get user directory.";
      return false;
    }
  }
#else

  if ( getenv( "HOME" ) )
  {
    user_dir = boost::filesystem::path( getenv( "HOME" ) );

    if (! boost::filesystem::exists( user_dir ) )
    {
      std::cerr << "Could not get user directory.";
      return false;
    }

    return true;
  }
  else
  {
    std::cerr << "Could not get user directory.";
    return false;
  }
#endif
}

bool ApplicationHelper::get_config_directory( boost::filesystem::path& config_dir )
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
      std::cerr << "Could not create directory: " << config_dir.string() << std::endl;
      return false;
    }

    std::cerr << "Created directory: " << config_dir.string() << std::endl;
  }

  return true;
}

bool ApplicationHelper::get_user_name( std::string& user_name )
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
    std::cerr << "Could not resolve user name.";
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
    std::cerr << "Could not resolve user name.";
    return false;
  }
#endif

}

boost::filesystem::path ApplicationHelper::configDirectory()
{
  boost::filesystem::path config;
  if (!get_config_directory(config))
    return boost::filesystem::current_path();
  return config;
}

std::string ApplicationHelper::applicationName()
{
  return "SCIRun";
}

ApplicationHelper::ApplicationHelper()
{
#if WIN32
  boost::filesystem::path::imbue( std::locale( "" ) );
#endif
  boost::filesystem::path dummy("boost bug workaround");
  if (dummy.string().empty())
    std::cout << dummy.string() << std::endl;
}
