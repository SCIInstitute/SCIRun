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

#include <Core/ConsoleApplication/ConsoleCommands.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Core/Application/Application.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Core/Python/PythonInterpreter.h>

using namespace SCIRun::Core;
using namespace Commands;
using namespace Console;
using namespace SCIRun::Dataflow::Networks;
using namespace Algorithms;

LoadFileCommandConsole::LoadFileCommandConsole()
{
  addParameter(Name("FileNum"), 0);
}

bool LoadFileCommandConsole::execute()
{
  auto inputFiles = Application::Instance().parameters()->inputFiles();
  if (!inputFiles.empty())
  {
    auto filename = inputFiles[index_];

    /// @todo: real logger
    std::cout << "Attempting load of " << filename << std::endl;

    try
    {
      auto openedFile = XMLSerializer::load_xml<NetworkFile>(filename);

      if (openedFile)
      {
        Application::Instance().controller()->clear();
        Application::Instance().controller()->loadNetwork(openedFile);
        /// @todo: real logger
        std::cout << "File load done: " << filename << std::endl;
        return true;
      }
      /// @todo: real logger
      std::cout << "File load failed: " << filename << std::endl;
    }
    catch (...)
    {
      /// @todo: real logger
      std::cout << "File load failed: " << filename << std::endl;
    }
    return false;
  }

  return true;
}

bool SaveFileCommandConsole::execute()
{
  throw "todo";
}

bool ExecuteCurrentNetworkCommandConsole::execute()
{
  Application::Instance().controller()->executeAll(nullptr);
  return true;
}

QuitAfterExecuteCommandConsole::QuitAfterExecuteCommandConsole()
{
  addParameter(Name("RunningPython"), false);
}

bool QuitAfterExecuteCommandConsole::execute()
{
  std::cout << "Goodbye!" << std::endl;
  Application::Instance().controller()->connectNetworkExecutionFinished([](int code){ exit(code); });
  //SCIRunMainWindow::Instance()->setupQuitAfterExecute();
  return true;
}

QuitCommandConsole::QuitCommandConsole()
{
  addParameter(Name("RunningPython"), false);
}

bool QuitCommandConsole::execute()
{
  std::cout << "Exiting!" << std::endl;
  exit(0);
  return true;
}

bool PrintHelpCommand::execute()
{
  std::cout << Application::Instance().commandHelpString() << std::endl;
  return true;
}

bool PrintVersionCommand::execute()
{
  std::cout << Application::Instance().version() << std::endl;
  return true;
}

bool PrintModulesCommand::execute()
{
  std::cout << "MODULE LIST as of " << Application::Instance().version() << "\n" << Application::Instance().moduleList() << std::endl;
  return true;
}

bool InteractiveModeCommandConsole::execute()
{
  PythonInterpreter::Instance().run_string("import SCIRunPythonAPI; from SCIRunPythonAPI import *");
  std::string x;
  while (x.find("quit") == std::string::npos)
  {
    std::cout << "scirun5> ";
    std::getline(std::cin, x);
    //std::cout << "x is: " << x << std::endl;
    PythonInterpreter::Instance().run_string(x);
  }
  exit(0);
  return true;
}

bool RunPythonScriptCommandConsole::execute()
{
  auto script = Application::Instance().parameters()->pythonScriptFile();
  if (script)
  {
#ifdef BUILD_WITH_PYTHON
    std::cout << "RUNNING PYTHON SCRIPT: " << *script << std::endl;;

    Application::Instance().controller()->clear();
    PythonInterpreter::Instance().run_string("import SCIRunPythonAPI; from SCIRunPythonAPI import *");
    PythonInterpreter::Instance().run_file(script->string());

    //TODO: not sure what else to do here. Probably wait on a condition variable, or just loop forever
    if (!Application::Instance().parameters()->interactiveMode())
    {
      while (true)
      {
        std::cout << "Running Python script." << std::endl;
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
      }
    }
    std::cout << "Done running Python script." << std::endl;
    return true;
#else
    std::cout << "Python disabled, cannot run script " << *script << std::endl;
    return false;
#endif
  }
  return false;
}
