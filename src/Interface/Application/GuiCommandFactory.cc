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

#include <boost/make_shared.hpp>
#include <Core/Utils/Exception.h>
#include <Interface/Application/GuiCommandFactory.h>
#include <Interface/Application/GuiCommands.h>
#include <Core/ConsoleApplication/ConsoleCommands.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::Console;

CommandHandle GuiGlobalCommandFactory::create(GlobalCommands type) const
{
  switch (type)
  {
  case ShowMainWindow:
    return boost::make_shared<ShowMainWindowGui>();
  case ShowSplashScreen:
    return boost::make_shared<ShowSplashScreenGui>();
  case PrintHelp:
    return boost::make_shared<PrintHelpCommand>();
  case PrintVersion:
    return boost::make_shared<PrintVersionCommand>();
  case PrintModules:
    return boost::make_shared<PrintModulesCommand>();
  case LoadNetworkFile:
    return boost::make_shared<LoadFileCommandGui>();
  case SaveNetworkFile:
    return boost::make_shared<NetworkSaveCommand>();
  case ImportNetworkFile:
    return boost::make_shared<FileImportCommand>();
  case RunPythonScript:
    return boost::make_shared<RunPythonScriptCommandGui>();
  case SetupDataDirectory:
    return boost::make_shared<SetupDataDirectoryCommandGui>();
  case ExecuteCurrentNetwork:
    return boost::make_shared<ExecuteCurrentNetworkCommandGui>();
  case SetupQuitAfterExecute:
    return boost::make_shared<QuitAfterExecuteCommandGui>();
  case QuitCommand:
    return boost::make_shared<QuitCommandGui>();
  default:
    THROW_INVALID_ARGUMENT("Unknown global command type.");
  }
  return {};
}
