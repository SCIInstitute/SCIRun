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


#ifndef CORE_CONSOLEAPPLICATION_CONSOLECOMMANDS_H
#define CORE_CONSOLEAPPLICATION_CONSOLECOMMANDS_H

#include <Core/Command/Command.h>
#include <Core/Application/Application.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Core/ConsoleApplication/share.h>

namespace SCIRun {
namespace Core {
namespace Console {

  // Shared logic for the two commands that turn a file on disk into a loaded
  // network: resolve a filename, hand it to a format-specific parser, and
  // load the result into the controller. Load (.srn5) and Import (legacy
  // .srn/.net) differ only in how the filename is resolved, how the file is
  // parsed, and whether a parse failure should fail a regression test.
  class SCISHARE NetworkFileProcessCommandConsole : public Commands::FileCommand<Commands::ConsoleCommand>
  {
  public:
    bool execute() final;
  protected:
    virtual std::string resolveFilename() const;
    virtual Dataflow::Networks::NetworkFileHandle processFile(const std::string& filename) const = 0;
    virtual std::string actionVerb() const = 0;
    virtual bool failTestOnErrorInRegressionMode() const { return false; }
  };

  class SCISHARE LoadFileCommandConsole : public NetworkFileProcessCommandConsole
  {
  public:
    LoadFileCommandConsole();
  protected:
    std::string resolveFilename() const override;
    Dataflow::Networks::NetworkFileHandle processFile(const std::string& filename) const override;
    std::string actionVerb() const override { return "load"; }
  };

  class SCISHARE ImportFileCommandConsole : public NetworkFileProcessCommandConsole
  {
  protected:
    Dataflow::Networks::NetworkFileHandle processFile(const std::string& filename) const override;
    std::string actionVerb() const override { return "import"; }
    bool failTestOnErrorInRegressionMode() const override { return true; }
  };

  class SCISHARE SaveFileCommandConsole : public Commands::FileCommand<Commands::ConsoleCommand>, public Commands::SaveFileCommandHelper
  {
  public:
    bool execute() override;
  };

  class SCISHARE RunPythonScriptCommandConsole : public Core::Commands::ConsoleCommand
  {
  public:
    bool execute() override;
  };

  class SCISHARE ExecuteCurrentNetworkCommandConsole : public Core::Commands::ConsoleCommand
  {
  public:
    bool execute() override;
  };

  class SCISHARE QuitAfterExecuteCommandConsole : public Core::Commands::ConsoleCommand
  {
  public:
    QuitAfterExecuteCommandConsole();
    bool execute() override;
  };

  class SCISHARE QuitCommandConsole : public Core::Commands::ConsoleCommand
  {
  public:
    QuitCommandConsole();
    bool execute() override;
  };

  class SCISHARE PrintHelpCommand : public Core::Commands::ConsoleCommand
  {
  public:
    bool execute() override;
  };

  class SCISHARE InteractiveModeCommandConsole : public Core::Commands::ConsoleCommand
  {
  public:
    bool execute() override;
  };

  class SCISHARE PrintVersionCommand : public Core::Commands::ConsoleCommand
  {
  public:
    bool execute() override;
  };

  class SCISHARE PrintModulesCommand : public Core::Commands::ConsoleCommand
  {
  public:
    bool execute() override;
  };

   class SCISHARE SetupDataDirectoryCommand : public Core::Commands::ConsoleCommand
   {
   public:
     bool execute() override;
   };

}}}

#endif
