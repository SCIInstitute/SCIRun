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
  case GlobalCommands::ShowMainWindow:
    return boost::make_shared<ShowMainWindowGui>();
  case GlobalCommands::ShowSplashScreen:
    return boost::make_shared<ShowSplashScreenGui>();
  case GlobalCommands::PrintHelp:
    return boost::make_shared<PrintHelpCommand>();
  case GlobalCommands::PrintVersion:
    return boost::make_shared<PrintVersionCommand>();
  case GlobalCommands::PrintModules:
    return boost::make_shared<PrintModulesCommand>();
  case GlobalCommands::LoadNetworkFile:
    return boost::make_shared<LoadFileCommandGui>();
  case GlobalCommands::SaveNetworkFile:
    return boost::make_shared<NetworkSaveCommand>();
  case GlobalCommands::ImportNetworkFile:
    return boost::make_shared<FileImportCommand>();
  case GlobalCommands::RunPythonScript:
    return boost::make_shared<RunPythonScriptCommandGui>();
  case GlobalCommands::SetupDataDirectory:
    return boost::make_shared<SetupDataDirectoryCommandGui>();
  case GlobalCommands::ExecuteCurrentNetwork:
    return boost::make_shared<ExecuteCurrentNetworkCommandGui>();
  case GlobalCommands::DisableViewScenes:
    return boost::make_shared<DisableViewScenesCommandGui>();
  case GlobalCommands::InteractiveMode:
    return boost::make_shared<InteractiveModeCommandConsole>();
  case GlobalCommands::SetupQuitAfterExecute:
    return boost::make_shared<QuitAfterExecuteCommandGui>();
  case GlobalCommands::QuitCommand:
    return boost::make_shared<QuitCommandGui>();
  default:
    THROW_INVALID_ARGUMENT("Unknown global command type.");
  }
  return {};
}
