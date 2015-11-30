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

#include <QApplication>
#include <QMessageBox>
#include <Interface/Application/GuiApplication.h>
#include <Interface/Application/GuiCommandFactory.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Core/Application/Application.h>
#include <boost/make_shared.hpp>

using namespace SCIRun::Gui;

int GuiApplication::run(int argc, const char* argv[])
{
  QApplication app(argc, const_cast<char**>(argv));

  try
  {
    SCIRun::Gui::SCIRunMainWindow* mainWin = SCIRun::Gui::SCIRunMainWindow::Instance();

    Core::Application::Instance().setCommandFactory(boost::make_shared<GuiGlobalCommandFactory>());
    mainWin->setController(Core::Application::Instance().controller());
    mainWin->initialize();

    app.exec();
    return mainWin->returnCode();
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(0, "Critical error", "Unhandled exception: " + QString(e.what()) + "\nExiting now.");
    std::cerr << "Unhandled exception: " << e.what() << std::endl;
    return 1;
  }
  catch (...)
  {
    QMessageBox::critical(0, "Critical error", "Unknown unhandled exception: exiting now.");
    std::cerr << "Unhandled exception: Unknown" << std::endl;
    return 1;
  }
}
