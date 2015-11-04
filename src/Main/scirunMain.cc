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
#include <Core/Utils/Legacy/Environment.h>

#ifdef BUILD_WITH_PYTHON
#include <Core/Python/PythonInterpreter.h>
#endif

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Gui;
using namespace SCIRun::Core::Console;

int mainImpl(int argc, const char* argv[], char **environment)
{
#ifdef LOOK_FOR_MEMORY_LEAKS
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  //_CrtSetBreakAlloc(34006);
#endif

  //char** env = nullptr; //TODO: passed as third argument from main, needs testing.
  create_sci_environment(environment, argv[0]);

  Application::Instance().readCommandLine(argc, argv);

#ifdef BUILD_WITH_PYTHON
  SCIRun::Core::PythonInterpreter::Instance().initialize(true);
#endif

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

  const char *argv[100] = { 0 };
  int argc;
  {
    LPWSTR *szArglist;

    szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!szArglist)
    {
      std::cout << "CommandLineToArgvW failed" << std::endl;
      return 7;
    }
    else
    {
      for (int i = 0; i < argc; i++)
      {
        argv[i] = utf8_encode(szArglist[i]);
      }
    }

    // Free memory allocated for CommandLineToArgvW arguments.
    LocalFree(szArglist);
  }

  return mainImpl(argc, argv, nullptr);
}

#else // If not WIN32 use this main()/entry point.

int main(int argc, const char* argv[], char **environment)
{
  return mainImpl(argc, argv, environment);
}

#endif // End of main for non-Windows.
