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


#include <QApplication>
#include <QMessageBox>
#include <Interface/Application/GuiApplication.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Core/Logging/Log.h>
#include <boost/make_shared.hpp>
#include <iostream>

using namespace SCIRun::Gui;

class SCIRunGuiApplication : public QApplication
{
public:
  SCIRunGuiApplication(int& argc, char** argv) :
    QApplication(argc, argv) {}

  bool notify(QObject* receiver, QEvent* event) override
  {
    try
    {
      return QApplication::notify(receiver, event);
    }
    catch (const std::exception& e)
    {
      QMessageBox::critical(0, "Critical error", "Unhandled exception: " + QString(e.what()) + "\nPlease report an issue describing what caused this message.");
      SCIRun::logCritical("Unhandled exception: {}", e.what());
      SCIRunGuiRunner::reportIssue();
      return false;
    }
    catch (...)
    {
      QMessageBox::critical(0, "Critical error", "Unknown unhandled exception: please report an issue describing what caused this message.");
      SCIRun::logCritical("Unhandled exception: Unknown type");
      SCIRunGuiRunner::reportIssue();
      return false;
    }
  }
};


int GuiApplication::run(int argc, const char* argv[])
{
  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  SCIRunGuiApplication app(argc, const_cast<char**>(argv));

  try
  {
    SCIRunGuiRunner gui(app);
    return gui.returnCode();
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(0, "Critical error", "Unhandled exception: " + QString(e.what()) + "\nExiting now.");
    logCritical("Unhandled exception: {}", e.what());
    return 1;
  }
  catch (...)
  {
    QMessageBox::critical(0, "Critical error", "Unknown unhandled exception: exiting now.");
    logCritical("Unhandled exception: Unknown type");
    return 1;
  }
}
