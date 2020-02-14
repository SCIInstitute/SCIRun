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


#ifndef INTERFACE_APPLICATION_GUICOMMANDS_H
#define INTERFACE_APPLICATION_GUICOMMANDS_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Command/Command.h>
#include <Core/Application/Application.h>
#include <QString>

class QSplashScreen;
class QTimer;

namespace SCIRun {
namespace Gui {

  class NetworkEditor;
  class SCIRunMainWindow;

  class LoadFileCommandGui : public Core::Commands::FileCommand<Core::Commands::GuiCommand>
  {
  public:
    LoadFileCommandGui();
    virtual bool execute() override;
  //private:
  //  int index_ = 0;
  };

  class RunPythonScriptCommandGui : public Core::Commands::GuiCommand
  {
  public:
    virtual bool execute() override;
  };

  class ExecuteCurrentNetworkCommandGui : public Core::Commands::GuiCommand
  {
  public:
    virtual bool execute() override;
  };

  class QuitAfterExecuteCommandGui : public Core::Commands::GuiCommand
  {
  public:
    QuitAfterExecuteCommandGui();
    virtual bool execute() override;
  };

  class QuitCommandGui : public Core::Commands::GuiCommand
  {
  public:
    QuitCommandGui();
    virtual bool execute() override;
  };

  class ShowMainWindowGui : public Core::Commands::GuiCommand
  {
  public:
    virtual bool execute() override;
  };

  class SetupDataDirectoryCommandGui : public Core::Commands::GuiCommand
  {
  public:
    virtual bool execute() override;
  };

  class ShowSplashScreenGui : public Core::Commands::GuiCommand
  {
  public:
    ShowSplashScreenGui();
    virtual bool execute() override;
  private:
    static void initSplashScreen();
    static QSplashScreen* splash_;
    static QTimer* splashTimer_;
  };

  class NetworkFileProcessCommand : public Core::Commands::FileCommand<Core::Commands::GuiCommand>
  {
  public:
    NetworkFileProcessCommand();
    bool execute() override;

    Dataflow::Networks::NetworkFileHandle file_;
  protected:
    virtual Dataflow::Networks::NetworkFileHandle processXmlFile(const std::string& filename) = 0;
    int guiProcess(const Dataflow::Networks::NetworkFileHandle& file);
    NetworkEditor* networkEditor_;
  };

  class FileOpenCommand : public NetworkFileProcessCommand
  {
  public:
    FileOpenCommand();
  protected:
    Dataflow::Networks::NetworkFileHandle processXmlFile(const std::string& filename) override;
  };

  class FileImportCommand : public NetworkFileProcessCommand
  {
  public:
    FileImportCommand();
    std::string logContents() const { return logContents_.str(); }
  protected:
    Dataflow::Networks::NetworkFileHandle processXmlFile(const std::string& filename) override;
    std::ostringstream logContents_;
  };

  class NetworkSaveCommand : public Core::Commands::FileCommand<Core::Commands::GuiCommand>, public Core::Commands::SaveFileCommandHelper
  {
  public:
    bool execute() override;
  };

  class DisableViewScenesCommandGui : public Core::Commands::GuiCommand
  {
  public:
    bool execute() override;
  };

  class ToolkitUnpackerCommand : public Core::Commands::FileCommand<Core::Commands::GuiCommand>
  {
  public:
    bool execute() override;
  };
}
}
#endif
