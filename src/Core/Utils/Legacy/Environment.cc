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
/// @todo Documentation Core/Utils/Legacy/Environment.cc

// Core SCIRun Includes

//#include <Core/Thread/Legacy/Mutex.h>
#include <Core/Utils/Legacy/Assert.h>
#include <Core/Utils/Legacy/FileUtils.h>
#include <Core/Utils/Legacy/sci_system.h>

#define SCI_OK_TO_INCLUDE_SCI_ENVIRONMENT_DEFS_H
#include <sci_defs/environment_defs.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <Core/Utils/Legacy/Environment.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <iostream>
#include <fstream>
#include <map>
#include <list>

using namespace SCIRun;
namespace bfs=boost::filesystem;
typedef bfs::path::string_type boost_string_type;
// value_type is char (POSIX) or wchar (Windows)
typedef bfs::path::value_type boost_value_type;
namespace bsys=boost::system;
namespace balgo=boost::algorithm;

/// This set stores all of the environemnt keys that were set when scirun was
/// started. Its checked by sci_putenv to ensure we don't overwrite variables
static std::map<std::string, std::string> scirun_env;

// MacroSubstitute takes var_value returns a string with the environment
// variables expanded out.  Performs one level of substitution

std::string
MacroSubstitute( const std::string& var_value )
{
  std::ostringstream newstring;
  const std::size_t macroFound = var_value.find('$');
  const std::size_t len = var_value.length();

  if ( (macroFound != std::string::npos) &&
      (macroFound+1 < len) &&
      (var_value[macroFound+1] == '(') )
  {
    const std::size_t macroEndFound = var_value.find(')');
    if (macroEndFound != std::string::npos)
    {
      std::size_t start = macroFound + 2;
      std::size_t end = macroEndFound - 1;
      std::string macro = var_value.substr(start, (end - start + 1));

      const char *env = sci_getenv(macro);
      if (env)
      {
        if (macroEndFound+1 < len)
          newstring << var_value.substr(0, macroFound) << env << var_value.substr(macroEndFound+1);
        else
          newstring << var_value.substr(0, macroFound) << env;
      }
      else
      {
        // macro not found, return unmodified string
        return var_value;
      }
      return newstring.str();
    }
  }
  // if macro not found, return unmodified string
  return var_value;
}

// WARNING: According to other software (specifically: tcl) you should
// lock before messing with the environment.

// Have to append 'SCIRun::' to these function names so that the
// compiler believes that they are in the SCIRun namespace (even
// though they are declared in the SCIRun namespace in the .h file...)
const char *
SCIRun::sci_getenv( const std::string & key )
{
  if (scirun_env.find(key) == scirun_env.end()) return 0;
  return scirun_env[key].c_str();
}

void
SCIRun::sci_putenv( const std::string &key, const std::string &val )
{
  scirun_env[key] = val;
}

const std::map<std::string, std::string>&
SCIRun::get_sci_environment()
{
  return (scirun_env);
}


#ifdef _WIN32
void getWin32RegistryValues(bfs::path& obj, bfs::path& src, bfs::path& appdata, bfs::path& thirdparty, std::string& packages)
{
#if 0 //DAN TODO
  // on an installed version of SCIRun, query these values from the registry, overwriting the compiled version
  // if not an installed version, return the compiled values unchanged
  HKEY software, company, version, scirun, pack, volatileEnvironment;

  // see http://msdn.microsoft.com/en-us/library/ms724872%28v=VS.85%29.aspx
  const int KEY_NAME_SIZE=255;
  const int VALUE_NAME_SIZE=16383;

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, KEY_READ, &software) == ERROR_SUCCESS)
  {
    if (RegOpenKeyEx(software, "SCI Institute", 0, KEY_READ, &company) == ERROR_SUCCESS)
    {
      if (RegOpenKeyEx(company, "SCIRun", 0, KEY_READ, &scirun) == ERROR_SUCCESS)
      {
        if (RegOpenKeyEx(scirun, SCIRUN_VERSION, 0, KEY_READ, &version) == ERROR_SUCCESS)
        {
          TCHAR data[VALUE_NAME_SIZE];
          data[VALUE_NAME_SIZE-1] = '\0';
          DWORD size = VALUE_NAME_SIZE;
          DWORD type;
          // RegGetValue will ensure that string is null-terminated, but only available for
          // WinXP 64-bit professional, Vista and up...
          LONG code = RegQueryValueEx(version, "InstallPath", 0, &type, (LPBYTE) data, &size);
          if (type == REG_SZ && code == ERROR_SUCCESS) {
            std::string installDir(data);
            // use Unix-style separators
            obj = bfs::path(installDir) / "bin";
            src = bfs::path(installDir) / "src";
            thirdparty = installDir;
            std::cout << "Loading SCIRun from: " << installDir.c_str() << "\n";
          }
          else
          {
            std::cerr << "Error or unexpected registry value." << std::endl;
            TCHAR* lpMsgBuf;
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                           0, GetLastError(),
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPTSTR) &lpMsgBuf, 0, 0);
            if (lpMsgBuf)
              std::cerr << lpMsgBuf << std::endl;

            LocalFree(lpMsgBuf);
          }


          if (RegOpenKeyEx(version, "Packages", 0, KEY_READ|KEY_ENUMERATE_SUB_KEYS, &pack) == ERROR_SUCCESS)
          {
            // see http://msdn.microsoft.com/en-us/library/ms724256(VS.85).aspx
            TCHAR  name[KEY_NAME_SIZE];           // buffer for subkey name
            DWORD  nameSize;                  // size of name string
            // MAX_PATH defined in WinDef.h
            TCHAR  className[MAX_PATH] = TEXT("");  // buffer for class name
            DWORD  classNameSize = MAX_PATH;  // size of class string
            DWORD  maxClassNameSize;          // longest class string
            DWORD  numSubKeys = 0;            // number of subkeys
            DWORD  maxSubKeySize;             // longest subkey size
            DWORD  numValues;              // number of values for key
            DWORD  maxValueName;           // longest value name
            DWORD  maxValueData;           // longest value data
            DWORD  securityDescriptorSize; // size of security descriptor
            FILETIME filetime;             // last write time

            DWORD index, code = ERROR_SUCCESS;
            // Force SCIRun package to be first.
            // The SCIRun package must always be available.
            packages = "SCIRun,";
            bool scirunPackageFound = false;

            // Get the class name and the value count.
            code = RegQueryInfoKey(pack, (LPTSTR) className, &classNameSize, 0, &numSubKeys, &maxSubKeySize,
                                   &maxClassNameSize, &numValues, &maxValueName, &maxValueData,
                                   &securityDescriptorSize, &filetime);
            // Enumerate the subkeys, until RegEnumKeyEx fails.
            if (code == ERROR_SUCCESS && numSubKeys > 0)
            {
              for (index = 0; index < numSubKeys; index++)
              {
                nameSize = KEY_NAME_SIZE;
                if (RegEnumKeyEx(pack, index, name, &nameSize, 0, 0, 0, &filetime) == ERROR_SUCCESS)
                {
                  if (! scirunPackageFound && std::string(name) == "SCIRun") {
                    scirunPackageFound = true;
                  }
                  else {
                    packages = packages + name + ",";
                  }
                }
              }
            }
            else
            {
              std::cerr << "Error reading SCIRun package data from registry." << std::endl;
              TCHAR* lpMsgBuf;
              FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                             0, GetLastError(),
                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                             (LPTSTR) &lpMsgBuf, 0, 0);
              if (lpMsgBuf)
                std::cerr << lpMsgBuf << std::endl;

              LocalFree(lpMsgBuf);
            }
            // lose trailing comma
            if (index > 0 && packages[packages.length()-1] == ',')
              packages[packages.length()-1] = 0;
            std::cout << "Packages (" << numSubKeys << "): " << packages << "\n";
            RegCloseKey(pack);
          }
          RegCloseKey(version);
        }
        RegCloseKey(scirun);
      }
      RegCloseKey(company);
    }
    RegCloseKey(software);
  }

  if (RegOpenKeyEx(HKEY_CURRENT_USER, "Volatile Environment", 0, KEY_READ, &volatileEnvironment) == ERROR_SUCCESS) {
    TCHAR data[MAX_PATH];
    DWORD size = MAX_PATH;
    DWORD type;
    int code = RegQueryValueEx(volatileEnvironment, "APPDATA", 0, &type, (LPBYTE) data, &size);
    if (type == REG_SZ && code == ERROR_SUCCESS) {
      boost_string_type userAppdata = boost::lexical_cast<boost_string_type>(data);
      // use Unix-style separators
      appdata = bfs::path(userAppdata) / "SCIRun";
    }
    RegCloseKey(volatileEnvironment);
  }
  else
  {
    std::cerr << "Error reading SCIRun application data path from registry." << std::endl;
    TCHAR* lpMsgBuf;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   0, GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPTSTR) &lpMsgBuf, 0, 0);
    if (lpMsgBuf)
      std::cerr << lpMsgBuf << std::endl;

    LocalFree(lpMsgBuf);
  }
#endif
}

#endif
// get_existing_env() will fill up the SCIRun::existing_env string set
// with all the currently set environment variable keys, but not their values

std::string replace_backslash_with_forward_slash(const bfs::path& path)
{
  return balgo::replace_all_copy(path.string(), "\\", "/");
}

std::string format_path(const bfs::path& path)
{
  // temporarily needed, until we have a GUI environment that works with native paths
#ifdef _WIN32
  return replace_backslash_with_forward_slash(path);
#else
  return path.string();
#endif
}

void set_env_to_zero_if_not_present(const std::string& key)
{
  if (!sci_getenv(key))
    sci_putenv(key, "0");
}

void putenv_if_not_present(const std::string& key, const std::string& value)
{
  if (!sci_getenv(key))
  {
    sci_putenv(key, value);
  }
}

/// @todo: add common state here
class SciEnvironmentBuilder
{
public:
  static void set_home(bfs::path& appdata)
  {
    // native windows doesn't have "HOME"
    if (!sci_getenv("HOME"))
    {
      // Use appropriate AppData directory (VISTA permissions) instead of OBJTOP (previous versions did this)
      sci_putenv("HOME", format_path(appdata));
    }
  }

  static void try_get_working_windows_directories(bfs::path& objdir, bfs::path& srcdir, bfs::path& appdata,
    bfs::path& thirdpartydir, std::string& packages)
  {
#ifdef _WIN32
    getWin32RegistryValues(objdir, srcdir, appdata, thirdpartydir, packages);
    if (! bfs::is_directory(appdata)) {
      try {
        bsys::error_code ec;
        if (! bfs::create_directory(appdata, ec) )
        {
          std::cerr << __FILE__ << ", " << __LINE__ << ": " << appdata.string() << " could not be created." << std::endl;
          std::cerr << __FILE__ << ", " << __LINE__ << ": " << "error " << ec.value() << ": " << ec.message() << std::endl;
        }
        if (! bfs::exists(appdata) )
          std::cerr << __FILE__ << ", " << __LINE__ << ": " << appdata.string() << " was not created." << std::endl;
      }
      catch (...) {
        std::cerr << __FILE__ << ", " << __LINE__ << ": create_directory failed in create_sci_environment(..)" << std::endl;
      }
    }
#endif
  }

  static void append_directory(std::string& objdir, const char* execname, const std::string& executable_name)
  {
    const char *path = sci_getenv("PATH");
    if (path &&
      objdir.find("/") == std::string::npos &&
      objdir.find("\\") == std::string::npos)
    {
      objdir = findFileInPath(execname, path);
      ASSERT(objdir.length());
      // findFileInPath appends directory separator to objdir
      objdir += executable_name;
    } else {
      auto cwd = bfs::current_path();
      objdir = (cwd / objdir).string();
    }
  }

  static void set_object_directory(const char* execname, bfs::path& objdir, std::string& executable_name)
  {
    if (!sci_getenv("SCIRUN_OBJDIR"))
    {
      if (!execname)
        sci_putenv("SCIRUN_OBJDIR", objdir.string());
      else
      {
        std::string objDirectory(execname);

        if (execname[0] != '/' && execname[1] != ':') // unix and windows compatible
        {
          append_directory(objDirectory, execname, executable_name);
        }

        std::string::size_type pos = objDirectory.find_last_of('/');
        if (pos == std::string::npos)
          pos = objDirectory.find_last_of('\\');

        std::string::size_type pos2 = objDirectory.find_last_of('.');
        std::string::size_type len = std::string::npos;
        if (pos2 == std::string::npos)
          len = objDirectory.size() - pos - 1;
        else
          len = pos2 - pos - 1;

        executable_name = objDirectory.substr(pos+1, len);
        objDirectory.erase(objDirectory.begin()+pos+1, objDirectory.end());

        sci_putenv("SCIRUN_OBJDIR", objDirectory);
      }
    }
  }

  static void set_obj_dir_non_windows(const bfs::path& testdir, bfs::path& objdir )
  {
    return;
#if SCIRUN4_CODE_TO_BE_CONVERTED_LATER //probably never, may not need this
#ifndef _WIN32
    std::string sciruntcl_package("sciruntcl");
    sciruntcl_package += SCIRUN_TCL_PACKAGE_VERSION;
    if ( bfs::exists( testdir / "lib" / sciruntcl_package ) )
    {
      objdir = testdir;
      sci_putenv("SCIRUN_OBJDIR", objdir.string());
    }
    else if ( bfs::exists( testdir / ".." / "lib" / sciruntcl_package ) )
    {
      objdir = testdir / "..";
      sci_putenv("SCIRUN_OBJDIR", objdir.string());
    }
    else if ( bfs::exists( testdir / ".." / ".." / "lib" / sciruntcl_package) )
    {
      objdir = testdir / ".." / "..";
      sci_putenv("SCIRUN_OBJDIR", objdir.string());
    }
#endif
#endif
  }

  static void test_and_set_various_source_dirs(const bfs::path& testdir, bfs::path& srcdir)
  {
    if ( bfs::exists( testdir / "src" ) )
    {
      srcdir = testdir / "src";
      sci_putenv("SCIRUN_SRCDIR", format_path(srcdir));
    }
    else if ( bfs::exists( testdir / ".." / "src" ) )
    {
      srcdir = testdir / ".." / "src";
      sci_putenv("SCIRUN_SRCDIR", format_path(srcdir));
    }
    else if ( bfs::exists( testdir / ".." / ".." / "src" ) )
    {
      srcdir = testdir / ".." / ".." / "src";
      sci_putenv("SCIRUN_SRCDIR", format_path(srcdir));
    }
  }

  static void override_defaults(char** env)
  {
    if (env) {
      char **environment = env;
      while (*environment) {
        const std::string str(*environment);
        const size_t pos = str.find("=");
        scirun_env[str.substr(0,pos)] = str.substr(pos+1, str.length());
        environment++;
      }
    }
  }

  static void set_example_nets_dir(const bfs::path& srcdir)
  {
    putenv_if_not_present("SCIRUN_EXAMPLE_NETS_DIR", format_path(get_example_nets_dir(srcdir.string())));
  }

  private:
    SciEnvironmentBuilder();
};

std::string SCIRun::get_example_nets_dir(const std::string& srcdir)
{
  bfs::path srcDirectory(srcdir);

  bfs::path nets = srcDirectory / "nets";
  if (bfs::exists(nets))
    return nets.string();

  bfs::path windowsInstall = srcDirectory / ".." / "example_nets" / "SCIRun";
  if (bfs::exists(windowsInstall))
    return bfs::system_complete(windowsInstall).string();

  return srcdir;
}

void
SCIRun::create_sci_environment(char **env, const char *execname)
{
  // set defaults
  scirun_env.clear();
  create_environment_defaults(scirun_env);

  SciEnvironmentBuilder::override_defaults(env);

  bfs::path srcdir = SCIRUN_SRCDIR;
  bfs::path objdir = SCIRUN_OBJDIR;

  if (execname)
  {
    bfs::path full_name(execname);
    bfs::path testdir = full_name.parent_path();
    SciEnvironmentBuilder::set_obj_dir_non_windows(testdir, objdir);
    SciEnvironmentBuilder::test_and_set_various_source_dirs(testdir, srcdir);
  }

  bfs::path thirdpartydir = objdir;
  const bfs::path tcllibrary = objdir / "lib" / "tcl8.6";
  const bfs::path tklibrary = objdir / "lib" / "tk8.6";

  sci_putenv("SCIRUN_THIRDPARTY_DIR", format_path(thirdpartydir));
  sci_putenv("TCL_LIBRARY", format_path(tcllibrary));
  sci_putenv("TK_LIBRARY", format_path(tklibrary));

#ifndef _WIN32
  setenv("TCL_LIBRARY", tcllibrary.c_str() , 1);
  setenv("TK_LIBRARY", tklibrary.c_str() , 1);
#endif

  bfs::path appdata;
  std::string packages = LOAD_PACKAGE;
  SciEnvironmentBuilder::try_get_working_windows_directories(objdir, srcdir, appdata, thirdpartydir, packages);

  std::string executable_name = "scirun";
  SciEnvironmentBuilder::set_object_directory(execname, objdir, executable_name);

  putenv_if_not_present("SCIRUN_SRCDIR", format_path(srcdir));

  SciEnvironmentBuilder::set_example_nets_dir(srcdir);

  putenv_if_not_present("SCIRUN_THIRDPARTY_DIR", format_path(thirdpartydir));

  putenv_if_not_present("SCIRUN_LOAD_PACKAGE", packages);

  std::string iwidgets = ITCL_WIDGETS;
  putenv_if_not_present("SCIRUN_ITCL_WIDGETS", iwidgets);

#if defined(SCIRUN_PACKAGE_SRC_PATH)
  std::string pkg_src_path = SCIRUN_PACKAGE_SRC_PATH;
  if (!pkg_src_path.empty())
    putenv_if_not_present("SCIRUN_PACKAGE_SRC_PATH", pkg_src_path);
#endif

#if defined(SCIRUN_PACKAGE_LIB_PATH)
  std::string pkg_lib_path = SCIRUN_PACKAGE_LIB_PATH;
  if (!pkg_lib_path.empty())
    putenv_if_not_present("SCIRUN_PACKAGE_LIB_PATH", pkg_lib_path);
#endif

  putenv_if_not_present("SCIRUN_TMP_DIR", "/tmp/");

  set_env_to_zero_if_not_present("SCIRUN_NET_SUBSTITUTE_DATADIR");
  set_env_to_zero_if_not_present("SCIRUN_NET_RELATIVE_FILENAMES");

  SciEnvironmentBuilder::set_home(appdata);

  sci_putenv("SCIRUN_ITCL_WIDGETS",
             MacroSubstitute(sci_getenv("SCIRUN_ITCL_WIDGETS")));

  sci_putenv("EXECUTABLE_NAME", executable_name);

  std::string rcfile = "." + executable_name + "rc";
  find_and_parse_rcfile(rcfile);
}

// parse_rcfile reads the file 'rcfile' into SCIRuns enviroment mechanism
// It uses sci_putenv to set variables in the environment.
// Returns true if the file was opened and parsed.  False otherwise.
bool
SCIRun::parse_rcfile( const char* rcfile )
{
  if ( !validFile( bfs::path(rcfile) )) return (false);

  std::ifstream file;

  try
  {
    file.open(rcfile);
  }
  catch(...)
  {
    return (false);
  }

  std::string line;

  while(!file.eof())
  {
    std::getline(file,line);

    std::string key;
    std::string value;

    size_t startloc = 0;
    while (startloc < line.size() && ((line[startloc] == ' ')||(line[startloc] == '\t'))) startloc++;
    size_t equalloc = line.find("=");
    if (equalloc != std::string::npos)
    {
      std::string key = line.substr(startloc,equalloc-startloc);
      size_t endloc = equalloc-startloc-1;
      while (endloc > 0 && ((key[endloc] == ' ')||(key[endloc] == '\t')) ) endloc--;
      key = key.substr(0,endloc+1);

      value = line.substr(equalloc+1);
      startloc = 0;
      while (startloc < value.size() && ((value[startloc] == ' ')||(value[startloc] == '\t'))) startloc++;
      value = value.substr(startloc);

      if (!value.empty())
      {
        endloc = value.size()-1;
        while (endloc > 0 && ((value[endloc] == ' ')||(value[endloc] == '\t')||
          (value[endloc] == '\n')||(value[endloc] == '\r')) ) endloc--;

        value = value.substr(0,endloc+1);

        if (key.size() > 0 && key[0] != '#')
        {
          sci_putenv(key,value);
        }
      }
    }


  }
  file.close();

  sci_putenv("SCIRUN_RC_PARSED","1");
  sci_putenv("SCIRUN_RCFILE",rcfile);

  return true;
}

// find_and_parse_rcfile will search for the rcfile file in
// default locations and read it into the environemnt if possible.
/// @todo: boostify this
void
SCIRun::find_and_parse_rcfile(const std::string &rcfile)
{
  bool foundrc=false;
  const std::string slash("/");

  // 1. check the local directory
  std::string filename(rcfile);
  foundrc = parse_rcfile(filename.c_str());

  // 2. check the BUILD_DIR
  if (!foundrc) {
    filename = SCIRUN_OBJDIR + slash + std::string(rcfile);
    foundrc = parse_rcfile(filename.c_str());
  }

  // 3. check the user's home directory
  const char *HOME = NULL;
  if (!foundrc && (HOME = sci_getenv("HOME"))) {
    filename = HOME + slash + std::string(rcfile);
    foundrc = parse_rcfile(filename.c_str());
  }

  // 4. check the source code directory
  if (!foundrc) {
    filename = SCIRUN_SRCDIR + slash + std::string(rcfile);
    foundrc = parse_rcfile(filename.c_str());
  }

  if (foundrc) sci_putenv("SCIRUN_RCFILE",filename);

  // Tell the user that we parsed the the rcfile...
//  std::cout << "Parsed " << rcfile << "... " << filename << "\n";
}


bool
SCIRun::update_rcfile(const std::string& key, const std::string& value)
{
  std::string filename = sci_getenv("SCIRUN_RCFILE");
  if (filename.size() == 0) return (false);

  std::ifstream file;

  try
  {
    file.open(filename.c_str());
  }
  catch(...)
  {
    return (false);
  }

  std::string line;
  std::list<std::string> lines;

  bool found = false;

  while(!file.eof())
  {
    std::getline(file,line);
    size_t startloc = 0;
    while (startloc < line.size() && ((line[startloc] == ' ')||(line[startloc] == '\t'))) startloc++;
    size_t equalloc = line.find("=");
    if (equalloc != std::string::npos)
    {
      std::string keyname = line.substr(startloc,equalloc-startloc);
      size_t endloc = equalloc-startloc-1;
      while (endloc > 0 && ((keyname[endloc] == ' ')||(keyname[endloc] == '\t')) ) endloc--;
      keyname = keyname.substr(0,endloc+1);
      if ((keyname == key) || (keyname == ("# "+key)))
      {
        line = key + " = " + value;
        found = true;
      }
    }
    line += "\n";
    lines.push_back(line);
  }
  file.close();

  if (found == false)
  {
    line = key + "=" + value + "\n";
    lines.push_back(line);
  }

  std::ofstream nfile;

  try
  {
    nfile.open(filename.c_str());
  }
  catch(...)
  {
    return (false);
  }

  std::list<std::string>::iterator it, it_end;
  it = lines.begin();
  it_end = lines.end();

  while(it != it_end)
  {
    nfile << (*it); ++it;
  }

  nfile.close();
  return (true);
}


void
SCIRun::copy_and_parse_scirunrc()
{
  try {
    std::string home   = sci_getenv("HOME");
    std::string srcdir = sci_getenv("SCIRUN_SRCDIR");

    if( home == "" || srcdir == "" ) {
      std::cout << "Warning: HOME ('" << home << "') or SCIRUN_SRCDIR ('" << srcdir << "') invalid... skipping copy...\n";
      return;
    }

    std::string srcRc = srcdir + "/scirunrc";

    std::string homerc = home + "/.scirunrc";
    if (validFile(homerc)) {
      const char* env_rcfile_version = sci_getenv("SCIRUN_RCFILE_VERSION");
      std::string backup_extension =(env_rcfile_version ? env_rcfile_version:"bak");
      std::string backuprc = homerc + "." + backup_extension;

      std::cout << "Backing up " << homerc << " to " << backuprc << std::endl;
      copyFile(homerc, backuprc);
    }

    std::cout << "Copying " << srcRc << " to " << homerc << "\n";
    if (copyFile(srcRc, homerc) == 0)
    {
      // If the scirunrc file was copied, then parse it.
      parse_rcfile(homerc.c_str());
    }
  }
  catch (...) {
    std::cerr << __FILE__ << ", " << __LINE__ << ": copy_and_parse_scirunrc() failed" << std::endl;
  }
}

// sci_getenv_p will lookup the value of the environment variable 'key' and
// returns false if the variable is equal to 'false', 'no', 'off', or '0'
// returns true otherwise.  Case insensitive.
bool
SCIRun::sci_getenv_p(const std::string &key)
{
  const char *value = sci_getenv(key);

  // If the environment variable does NOT EXIST OR is EMPTY then return FALSE
  if (!value || !(*value)) return false;

  std::string str(value);

  // string modified in place
  balgo::to_upper(str);
  // Only return false if value is zero (or equivalant)
  if (str == "FALSE" || str == "NO" || str == "OFF" || str == "0")
    return false;
  // Following C convention where any non-zero value is assumed true
  return true;
}

bool
SCIRun::replace_environment_variables(std::string& str)
{
  // scan for variable names

  std::string::size_type size = str.size();
  std::string::size_type old_size = str.size();
  std::string::size_type start_loc = 0;
  std::string::size_type dollar_loc = str.find('$',start_loc);

  while (dollar_loc != std::string::npos)
  {
    // found a possible variable
    if (dollar_loc+3 < size && str[dollar_loc+1] == '{')
    {
      std::string::size_type end_brac = str.find('}',dollar_loc+1);
      if (end_brac != std::string::npos)
      {
        std::string key = str.substr(dollar_loc+2,end_brac-dollar_loc-2);
        if (sci_getenv(key))
        {
          std::string value = sci_getenv(key);
          old_size = size;
          str = str.substr(0,dollar_loc)+value+str.substr(end_brac+1);
          size = str.size();
          start_loc = size - end_brac;
        }
      }
    }
    else
    {
      std::string::size_type end_loc = start_loc+1;

      while ( (end_loc < size) && ((str[end_loc] >= 'a' && str[end_loc] <= 'z')||
              (str[end_loc] >= 'A' && str[end_loc] <= 'Z') ||
              (str[end_loc] >= '0' && str[end_loc] <= '9') ||
              (str[end_loc] == '_' ))) end_loc++;

      if (end_loc > start_loc+1)
      {
        std::string key = str.substr(dollar_loc+1,end_loc-dollar_loc-1);
        if (sci_getenv(key))
        {
          std::string value = sci_getenv(key);
          old_size = size;
          str = str.substr(0,dollar_loc)+value+str.substr(end_loc);
          size = str.size();
          start_loc = size - end_loc - 1;
        }
      }
    }
    dollar_loc = str.find('$',start_loc+1);
    start_loc++;
  }
  return (true);
}
