/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

//Uncomment line below to check for memory leaks (run in debug mode VS)
//#define LOOK_FOR_MEMORY_LEAKS

#include <Core/Application/Application.h>
#include <Interface/Application/GuiApplication.h>
#include <Core/ConsoleApplication/ConsoleApplication.h>

using namespace SCIRun::Core;
using namespace SCIRun::Gui;
using namespace SCIRun::Core::Console;

int mainImpl(int argc, const char* argv[])
{
#ifdef LOOK_FOR_MEMORY_LEAKS
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  //_CrtSetBreakAlloc(34006);
#endif

  Application::Instance().readCommandLine(argc, argv);

  //TODO: must read --headless flag here, or try pushing command queue building all the way up here

#ifndef BUILD_HEADLESS
  return GuiApplication::run(argc, argv);
#else
  return ConsoleApplication::run(argc, argv);
#endif
}

// If building on WIN32, use this entry point.
#ifdef WIN32

#include <windows.h>
#include <vector>
#include <boost/algorithm/string.hpp>

static std::string&& stripQuotes(std::string& s)
{
  if (s.front() == '"' && s.back() == '"')
  {
    s.erase(0, 1); // erase the first character
    s.erase(s.size() - 1); // erase the last character
  }
  return std::move(s);
}

const char* utf8_encode(const std::wstring &wstr)
{
  if (wstr.empty()) return "";
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
  char* strTo = new char[size_needed + 1];
  strTo[size_needed] = 0;
  WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), strTo, size_needed, NULL, NULL);
  return strTo;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef SCIRUN_SHOW_CONSOLE 
  AllocConsole();
  freopen("CONIN$", "r", stdin);
  freopen("CONOUT$", "w", stdout);
  freopen("CONOUT$", "w", stderr);
#endif

  //const int argc = __argc;
  const char *argv[50];
  //char *tempArgv[] = { GetCommandLine() };

  // The GetCommandLine() function returns argv as a single string. The split function splits it up into
  // the individual arguments.

  // TODO: another edge case is the Windows package. Full path of SCIRun.exe is passed with no quotes, so if the path
  // has spaces this function does not work--we need the entire exe path to be in argv[0]. Unit test coming soon.
  int argc;
  {
    std::cout << "NEW METHOD:\n\n";

    LPWSTR *szArglist;
    int nArgs;

    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (!szArglist)
    {
      wprintf(L"CommandLineToArgvW failed\n");
      return 0;
    }
    else
    {
      for (int i = 0; i < nArgs; i++)
      {
        printf("%d: %ws\n", i, szArglist[i]);
        argv[i] = utf8_encode(szArglist[i]);
        printf("%d: %ws\n", i, argv[i]);
      }
    }

    argc = nArgs;

    // Free memory allocated for CommandLineToArgvW arguments.

    LocalFree(szArglist);
  }

  /*

  std::cout << "PRINTING CMD LINE:\n" << tempArgv[0] << std::endl;

  std::vector<std::string> getArgv;
  boost::algorithm::split(getArgv, tempArgv[0], boost::is_any_of(" \0"), boost::algorithm::token_compress_on);
  
  // Put the individual arguments into the argv that will be passed.
  for (int i = 0; i < argc; i++)
  {
    argv[i] = stripQuotes(getArgv[i]).c_str();
    std::cout << i << " : " << getArgv[i] << std::endl;
    std::cout << i << " stripped: " << argv[i] << std::endl;
  }
  */
  return mainImpl(argc, argv);
}

#else // If not WIN32 use this main()/entry point.

int main(int argc, const char* argv[])
{
  return mainImpl(argc, argv);
}

#endif // End of main for non-Windows.

