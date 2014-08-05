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

#ifndef INTERFACE_APPLICATION_GUICOMMANDS_H
#define INTERFACE_APPLICATION_GUICOMMANDS_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Command/Command.h>

class QSplashScreen;
class QTimer;

namespace SCIRun {
namespace Gui {

  class NetworkEditor;

  class LoadFileCommandGui : public Core::Commands::GuiCommand
  {
  public:
    virtual bool execute();
  };

  class RunPythonScriptCommandGui : public Core::Commands::GuiCommand
  {
  public:
    virtual bool execute();
  };

  class ExecuteCurrentNetworkCommandGui : public Core::Commands::GuiCommand
  {
  public:
    virtual bool execute();
  };

  class QuitAfterExecuteCommandGui : public Core::Commands::GuiCommand
  {
  public:
    virtual bool execute();
  };

  class QuitCommandGui : public Core::Commands::GuiCommand
  {
  public:
    virtual bool execute();
  };

  class ShowMainWindowGui : public Core::Commands::GuiCommand
  {
  public:
    virtual bool execute();
  };

  class SetupDataDirectoryCommandGui : public Core::Commands::GuiCommand
  {
  public:
    virtual bool execute();
  };
  
  class ShowSplashScreenGui : public Core::Commands::GuiCommand
  {
  public:
    ShowSplashScreenGui();
    virtual bool execute();
  private:
    static void initSplashScreen();
    static QSplashScreen* splash_;
    static QTimer* splashTimer_;
  };
  
  class FileOpenCommand : public Core::Commands::GuiCommand
  {
  public:
    FileOpenCommand(const std::string& filename, NetworkEditor* networkEditor) : filename_(filename), networkEditor_(networkEditor) {}
    virtual bool execute();

    Dataflow::Networks::NetworkFileHandle openedFile_;
  private:
    std::string filename_;
    NetworkEditor* networkEditor_;
  };


}
}
#endif
