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

#ifndef CORE_CONSOLEAPPLICATION_CONSOLECOMMANDS_H
#define CORE_CONSOLEAPPLICATION_CONSOLECOMMANDS_H

#include <Core/Command/Command.h>
#include <Core/ConsoleApplication/share.h>

namespace SCIRun {
namespace Core {
namespace Console {

  class SCISHARE LoadFileCommandConsole : public Core::Commands::ConsoleCommand
  {
  public:
    LoadFileCommandConsole();
    virtual bool execute() override;
  private:
    int index_ = 0;
  };

  class SCISHARE SaveFileCommandConsole : public Core::Commands::ConsoleCommand
  {
  public:
    //LoadFileCommandConsole();
    virtual bool execute() override;
  //private:
  //  int index_ = 0;
  };

  class SCISHARE RunPythonScriptCommandConsole : public Core::Commands::ConsoleCommand
  {
  public:
    virtual bool execute() override;
  };

  class SCISHARE ExecuteCurrentNetworkCommandConsole : public Core::Commands::ConsoleCommand
  {
  public:
    virtual bool execute() override;
  };

  class SCISHARE QuitAfterExecuteCommandConsole : public Core::Commands::ConsoleCommand
  {
  public:
    virtual bool execute() override;
  };

  class SCISHARE QuitCommandConsole : public Core::Commands::ConsoleCommand
  {
  public:
    virtual bool execute() override;
  };

  class SCISHARE PrintHelpCommand : public Core::Commands::ConsoleCommand
  {
  public:
    virtual bool execute() override;
  };

  class SCISHARE PrintVersionCommand : public Core::Commands::ConsoleCommand
  {
  public:
    virtual bool execute() override;
  };

  class SCISHARE PrintModulesCommand : public Core::Commands::ConsoleCommand
  {
  public:
    virtual bool execute() override;
  };

  /// @TODO
//   class SCISHARE SetupDataDirectoryCommand : public Core::Commands::ConsoleCommand
//   {
//   public:
//     virtual bool execute();
//   };

}}}

#endif
