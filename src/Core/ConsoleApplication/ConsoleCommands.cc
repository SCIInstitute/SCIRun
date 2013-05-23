/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
#include <Core/Application/Application.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>

using namespace SCIRun::Core;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::Console;
//using namespace SCIRun::Dataflow::Networks;

bool LoadFileCommandConsole::execute()
{
  auto inputFile = Application::Instance().parameters()->inputFile();
  std::cout << "LoadFileCommandConsole::execute()" << std::endl;
  //SCIRunMainWindow::Instance()->loadNetworkFile(QString::fromStdString(inputFile.get()));
  return true;
}

bool ExecuteCurrentNetworkCommandConsole::execute()
{
  std::cout << "ExecuteCurrentNetworkCommandConsole::execute()" << std::endl;
//  SCIRunMainWindow::Instance()->executeAll();
  return true;
}

bool QuitAfterExecuteCommandConsole::execute()
{
  std::cout << "QuitAfterExecuteCommandConsole::execute()" << std::endl;
  //SCIRunMainWindow::Instance()->setupQuitAfterExecute();
  return true;
}

bool QuitCommandConsole::execute()
{
  std::cout << "QuitCommandConsole::execute()" << std::endl;
  //SCIRunMainWindow::Instance()->quit();
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

bool FileOpenCommandConsole::execute()
{
  //GuiLogger::Instance().log(QString("Attempting load of ") + filename_.c_str());

  //try
  //{
  //  openedFile_ = XMLSerializer::load_xml<NetworkFile>(filename_);

  //  if (openedFile_)
  //  {
  //    networkEditor_->clear();
  //    networkEditor_->loadNetwork(openedFile_);
  //    GuiLogger::Instance().log("File load done.");
  //    return true;
  //  }
  //  else
  //    GuiLogger::Instance().log("File load failed.");
  //}
  //catch (...)
  //{
  //  GuiLogger::Instance().log("File load failed.");
  //}
  //return false;
  std::cout << "FileOpenCommandConsole::execute()" << std::endl;
  return true;
}

bool RunPythonScriptCommandConsole::execute()
{
  auto script = Application::Instance().parameters()->pythonScriptFile().get();
  //SCIRunMainWindow::Instance()->runPythonScript(QString::fromStdString(script.string()));
  std::cout << "RunPythonScriptCommandConsole::execute()" << std::endl;
  return true;
}