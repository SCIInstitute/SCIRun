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

#include <QtGui>
#include <Core/Application/Application.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/GuiCommands.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/NetworkEditor.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Dataflow::Networks;

bool LoadFileCommandGui::execute()
{
  auto inputFile = Application::Instance().parameters()->inputFile();
  SCIRunMainWindow::Instance()->loadNetworkFile(QString::fromStdString(inputFile.get()));
  return true;
}

bool ExecuteCurrentNetworkCommandGui::execute()
{
  SCIRunMainWindow::Instance()->executeAll();
  return true;
}

bool QuitAfterExecuteCommandGui::execute()
{
  SCIRunMainWindow::Instance()->setupQuitAfterExecute();
  return true;
}

bool QuitCommandGui::execute()
{
  SCIRunMainWindow::Instance()->quit();
  return true;
}

bool ShowMainWindowGui::execute()
{
  auto mainWin = SCIRunMainWindow::Instance();
  mainWin->activateWindow();
  
  mainWin->raise();
  mainWin->show();
  return true;
}

ShowSplashScreenGui::ShowSplashScreenGui() 
{
  initSplashScreen();
}

bool ShowSplashScreenGui::execute()
{
  splash_->show();

  splashTimer_->start();
  splash_->showMessage("Welcome! Tip: Press F1 and click anywhere in the interface for helpful hints.", Qt::AlignBottom, Qt::white);
  qApp->processEvents();

  return true;
}

void ShowSplashScreenGui::initSplashScreen()
{
  splash_ = new QSplashScreen(0, QPixmap(":/gear/splash-scirun.png"),  Qt::WindowStaysOnTopHint);
  splashTimer_ = new QTimer;
  splashTimer_->setSingleShot( true );
  splashTimer_->setInterval( 5000 );
  QObject::connect( splashTimer_, SIGNAL( timeout() ), splash_, SLOT( close() ));
}

QSplashScreen* ShowSplashScreenGui::splash_ = 0;
QTimer* ShowSplashScreenGui::splashTimer_ = 0;

bool FileOpenCommand::execute()
{
  GuiLogger::Instance().log(QString("Attempting load of ") + filename_.c_str());

  try
  {
    openedFile_ = XMLSerializer::load_xml<NetworkFile>(filename_);

    if (openedFile_)
    {
      networkEditor_->clear();
      networkEditor_->loadNetwork(openedFile_);
      GuiLogger::Instance().log("File load done.");
      return true;
    }
    else
      GuiLogger::Instance().log("File load failed.");
  }
  catch (...)
  {
    GuiLogger::Instance().log("File load failed.");
  }
  return false;
}

bool RunPythonScriptCommandGui::execute()
{
  auto script = Application::Instance().parameters()->pythonScriptFile().get();
  SCIRunMainWindow::Instance()->runPythonScript(QString::fromStdString(script.string()));
  return true;
}