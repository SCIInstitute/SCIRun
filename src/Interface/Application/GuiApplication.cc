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

#include <QApplication>
#include <QSplashScreen>
#include <QMessageBox>
#include <QTimer>
#include <Interface/Application/GuiApplication.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Core/Application/Application.h>

using namespace SCIRun::Gui;

int GuiApplication::run(int argc, const char* argv[])
{
  QApplication app(argc, const_cast<char**>(argv));

  try
  { 
    SCIRun::Gui::SCIRunMainWindow* mainWin = SCIRun::Gui::SCIRunMainWindow::Instance();

    QPixmap pixmap(":/gear/splash-scirun.png");
    QSplashScreen splash(0, pixmap,  Qt::WindowStaysOnTopHint);
    splash.show();

    QTimer splashTimer;
    splashTimer.setSingleShot( true );
    splashTimer.setInterval( 5000 );
    QObject::connect( &splashTimer, SIGNAL( timeout() ), &splash, SLOT( close() ));
    splashTimer.start(); 
    splash.showMessage("Welcome! Tip of the day: Press F1 and click anywhere in the interface for helpful hints.", Qt::AlignBottom, Qt::white);
    app.processEvents();
    
    mainWin->setController(Core::Application::Instance().controller());
    mainWin->raise();
    mainWin->show();
    mainWin->initialize();
    
    return app.exec();
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(0, "Critical error", "Unhandled exception: " + QString(e.what()) + "\nExiting now.");
    return -1;
  }
  catch (...)
  {
    QMessageBox::critical(0, "Critical error", "Unknown unhandled exception: exiting now.");
    return -1;
  }
}
