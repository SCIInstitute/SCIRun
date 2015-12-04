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
#include <Core/ConsoleApplication/ConsoleCommandFactory.h>
#include <Core/ConsoleApplication/ConsoleCommands.h>

using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::Console;

class NothingCommand : public ConsoleCommand
{
public:
  virtual bool execute() override { return true; }
};

CommandHandle ConsoleGlobalCommandFactory::create(GlobalCommands type) const
{
  switch (type)
  {
  case ShowMainWindow:
    return boost::make_shared<NothingCommand>();
  case ShowSplashScreen:
    return boost::make_shared<NothingCommand>();
  case PrintHelp:
    return boost::make_shared<PrintHelpCommand>();
  case PrintVersion:
    return boost::make_shared<PrintVersionCommand>();
  case LoadNetworkFile:
    return boost::make_shared<LoadFileCommandConsole>();
  case SaveNetworkFile:
    return boost::make_shared<SaveFileCommandConsole>();
  case RunPythonScript:
    return boost::make_shared<RunPythonScriptCommandConsole>();
  case ExecuteCurrentNetwork:
    return boost::make_shared<ExecuteCurrentNetworkCommandConsole>();
  case SetupQuitAfterExecute:
    return boost::make_shared<QuitAfterExecuteCommandConsole>();
  case QuitCommand:
    return boost::make_shared<QuitCommandConsole>();
  default:
    THROW_INVALID_ARGUMENT("Unknown global command type.");
  }
  return {};
}
