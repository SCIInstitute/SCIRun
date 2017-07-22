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

///@file soloader.cpp
///@author Chris Moulding
///@date Sept 1998

#include <Core/Utils/Legacy/Assert.h>
#include <Core/Utils/Legacy/soloader.h>
#include <Core/Utils/Legacy/Environment.h>
#include <Core/Thread/Mutex.h>

#include <iostream>
#include <string>

using namespace SCIRun::Core::Thread;

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

// Create a lock around this one to be sure that we do not call this one twice
static Mutex dl_lock_("dlopen/dlsym/dlclose lock");
static std::map<std::string,LIBRARY_HANDLE> scirun_open_libs_;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
void* GetLibrarySymbolAddress(const std::string libname, const std::string symbolname, std::string& errormsg)
{
  errormsg = "";
  LIBRARY_HANDLE LibraryHandle = 0;
  void* proc = 0;

#ifdef _WIN32
  dl_lock_.lock();
  LibraryHandle = LoadLibrary(libname.c_str());

  if (LibraryHandle == 0)
  {
    // If an error occured retrieve this one before we unlock
    char* lpMsgBuf;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPTSTR) &lpMsgBuf, 0, NULL);
    if (lpMsgBuf)  errormsg = std::string(lpMsgBuf);
  }
  dl_lock_.unlock();

#else
  ASSERT(SCIRun::sci_getenv("SCIRUN_OBJDIR"));
  std::string name = std::string(SCIRun::sci_getenv("SCIRUN_OBJDIR")) + "/lib/" + libname;

  dl_lock_.lock();
  // If an error occured retrieve this one before we unlock
  LibraryHandle = dlopen(name.c_str(), RTLD_NOW|RTLD_GLOBAL);
  if (LibraryHandle == 0) { char *msg = dlerror();  if(msg) errormsg = std::string(msg); }

  dl_lock_.unlock();

  if( LibraryHandle == 0 )
  {
    // dlopen of absolute path failed...  Perhaps they have a DYLD_LIBRARY_PATH var set...
    // If so, if we try again without the path, then maybe it will succeed...
    dl_lock_.lock();
    LibraryHandle = dlopen(libname.c_str(), RTLD_NOW|RTLD_GLOBAL);
    if (LibraryHandle == 0) { char *msg = dlerror();  if(msg) errormsg = std::string(msg); }
    dl_lock_.unlock();
  }
#endif

  if (LibraryHandle == 0)
  {
    return 0;
  }

#ifdef _WIN32
  dl_lock_.lock();
  proc = GetProcAddress(LibraryHandle,symbolname.c_str());
  if (proc == 0)
  {
    // If an error occured retrieve this one before we unlock
    char* lpMsgBuf;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPTSTR) &lpMsgBuf, 0, NULL);
    if (lpMsgBuf) errormsg = std::string(lpMsgBuf);
  }
  dl_lock_.unlock();
  return (void*) proc;

#elif defined __APPLE__ && !defined APPLE_LEOPARD
  // Add a leading underscore to the symbolname for call to mach lib functions
  // If you don't check against the underscored symbol name NSIsSymbolNameDefined
  // will never return true.

  dl_lock_.lock();
  std::string usymbolname = std::string("-") + symbolname;
  if( NSIsSymbolNameDefined(usymbolname.c_str()))
  {
    proc =  dlsym(LibraryHandle,symbolname.c_str());
    if (proc == 0) { char *msg = dlerror();  if(msg) errormsg = std::string(msg); }
  }
  dl_lock_.unlock();

#else
  dl_lock_.lock();
  dlerror(); // clear existing error.
  proc = dlsym(LibraryHandle,symbolname.c_str());
  if (proc == 0) { char *msg = dlerror();  if(msg) errormsg = std::string(msg); }
  dl_lock_.unlock();
#endif

  return proc;
}

LIBRARY_HANDLE
findLib(std::string lib, std::string& errormsg)
{
  LIBRARY_HANDLE handle=0;
  const char *env = SCIRun::sci_getenv("SCIRUN_PACKAGE_LIB_PATH");
  std::string tempPaths(env?env:"");
  // try to find the library in the specified path
  while (tempPaths!="")
  {
    std::string dir;
#ifdef _WIN32
    // make sure we don't throw away the drive letter
    const unsigned int firstColon = tempPaths.find(':',2);
#else
    const unsigned int firstColon = tempPaths.find(':');
#endif
    if(firstColon < tempPaths.size())
    {
      dir=tempPaths.substr(0,firstColon);
      tempPaths=tempPaths.substr(firstColon+1);
    }
    else
    {
      dir=tempPaths;
      tempPaths="";
    }

    handle = GetLibraryHandle(dir + "/" + lib,errormsg);
    if (handle) return (handle);
  }

  const char *objdir = SCIRun::sci_getenv("SCIRUN_OBJDIR");
  if (objdir)
  {
    handle = GetLibraryHandle(std::string(objdir)+"/"+lib,errormsg);
    if (handle) return (handle);

    handle = GetLibraryHandle(std::string(objdir)+"/lib/"+lib,errormsg);
    if (handle) return (handle);

    handle = GetLibraryHandle(std::string(objdir)+"/lib/Debug/"+lib, errormsg);
    if (handle) return (handle);

    handle = GetLibraryHandle(std::string(objdir)+"/lib/Release/"+lib,errormsg);
    if (handle) return (handle);

    handle = GetLibraryHandle(std::string(objdir)+"../lib/"+lib,errormsg);
    if (handle) return (handle);

    handle = GetLibraryHandle(std::string(objdir)+"../lib/Debug/"+lib, errormsg);
    if (handle) return (handle);

    handle = GetLibraryHandle(std::string(objdir)+"../lib/Release/"+lib,errormsg);
    if (handle) return (handle);

  }


  // if not yet found, try to find it in the rpath
  // or the LD_LIBRARY_PATH (last resort)
  handle = GetLibraryHandle(lib,errormsg);
  return (handle);
}
#endif

void* GetHandleSymbolAddress(LIBRARY_HANDLE handle, const std::string& symbolname, std::string& errormsg)
{
  errormsg = "";
  void* proc = 0;
  Guard g(dl_lock_.get());
#ifdef _WIN32
  proc = GetProcAddress(handle,symbolname.c_str());
  if (proc == 0)
  {
    // If an error occured retrieve this one before we unlock
    char* lpMsgBuf;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPTSTR) &lpMsgBuf, 0, NULL);
    if (lpMsgBuf) errormsg = std::string(lpMsgBuf);
    LocalFree(lpMsgBuf);
  }

#elif defined __APPLE__ && !defined APPLE_LEOPARD
  // Add a leading underscore to the symbolname for call to mach lib functions
  // If you don't check against the underscored symbol name NSIsSymbolNameDefined
  // will never return true.

  std::string usymbolname = "_" + symbolname;

  if( NSIsSymbolNameDefined(usymbolname.c_str()) )
  {
    proc =  dlsym(handle,symbolname.c_str());
    if (proc == 0) { char *msg = dlerror();  if(msg) errormsg = std::string(msg); }
  }

#else
  dlerror(); // clear existing error.
  proc = dlsym(handle,symbolname.c_str());
  if (proc == 0) { char *msg = dlerror();  if(msg) errormsg = std::string(msg); }
#endif
  return (proc);
}

LIBRARY_HANDLE GetLibraryHandle(const std::string& libname,std::string& errormsg)
{
  LIBRARY_HANDLE lh;
  Guard g(dl_lock_.get());
#ifdef _WIN32
  if (libname == "")
  {
    lh = 0;
  }
  else
  {
    lh = LoadLibrary(libname.c_str());
    if (lh == 0) {
      // If an error occured retrieve this one before we unlock
      char* lpMsgBuf;
      FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                     NULL, GetLastError(),
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPTSTR) &lpMsgBuf, 0, NULL);
      if (lpMsgBuf) errormsg = std::string(lpMsgBuf);
      LocalFree(lpMsgBuf);
    }
  }
#else
  std::string name;
  if (libname != "")
  {
    if ( libname[0] == '/' )
      name = libname;
    else
    {
      #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      ASSERT(SCIRun::sci_getenv("SCIRUN_OBJDIR"));
      name = std::string(SCIRun::sci_getenv("SCIRUN_OBJDIR"))+"/lib/"+libname;
      #endif
    }
  }

  if (libname != "")
  {
    if (scirun_open_libs_.find(libname) != scirun_open_libs_.end())
    {
      lh = scirun_open_libs_[libname];
    }
    else
    {
      lh = dlopen(name.c_str(), RTLD_NOW|RTLD_GLOBAL);
      scirun_open_libs_[libname] = lh;
    }
  }
  else
  {
    lh = dlopen(NULL, RTLD_NOW|RTLD_GLOBAL);
  }

  if (lh == 0) { char *msg = dlerror();  if(msg) errormsg = std::string(msg); }

#endif
  return (lh);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
void CloseLibrary(LIBRARY_HANDLE LibraryHandle)
{
#ifdef _WIN32
  dl_lock_.lock();
  FreeLibrary(LibraryHandle);
  dl_lock_.unlock();
#else
  dl_lock_.lock();
  dlclose(LibraryHandle);
  std::map<std::string,LIBRARY_HANDLE>::iterator it, it_end;
  it= scirun_open_libs_.begin();
  it_end = scirun_open_libs_.end();
  while (it != it_end)
  {
    if ((*it).second == LibraryHandle) scirun_open_libs_.erase(it);
    ++it;
  }
  dl_lock_.unlock();
#endif
}

#endif

LIBRARY_HANDLE FindLibInPath(const std::string& lib, const std::string& path, std::string& errormsg)
{
  LIBRARY_HANDLE handle;
  std::string tempPaths = path;
  std::string dir;

  // try to find the library in the specified path
  while (tempPaths!="")
  {
    const size_t firstColon = tempPaths.find(':');
    if (firstColon < tempPaths.size())
    {
      dir = tempPaths.substr(0,firstColon);
      tempPaths = tempPaths.substr(firstColon+1);
    }
    else
    {
      dir = tempPaths;
      tempPaths = "";
    }

    handle = GetLibraryHandle(dir+"/"+lib,errormsg);
    if (handle)
      return (handle);
  }

  // if not yet found, try to find it in the rpath
  // or the LD_LIBRARY_PATH (last resort)
  handle = GetLibraryHandle(lib,errormsg);

  return (handle);
}
