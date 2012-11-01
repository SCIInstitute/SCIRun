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
#include <iostream>
#include <Interface/Application/ModuleLogWindow.h>

//#include <Dataflow/Network/Module.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;

ModuleLogWindow::ModuleLogWindow(const QString& moduleName, QWidget* parent) : QDialog(parent)
{
  setupUi(this);
  setModal(false);
  setWindowTitle("Log for " + moduleName);
  setVisible(false);
  connect(buttonBox->button(QDialogButtonBox::Discard), SIGNAL(clicked()), logTextEdit_, SLOT(clear()));
}

void ModuleLogWindow::appendMessage(const QString& message, const QColor& color /* = Qt::black */)
{
  logTextEdit_->insertHtml(QString("<p style=\"color:") + color.name() + "\">" + message + "</p><br>");
}

void ModuleLogWindow::popupMessageBox(const QString& message)
{
  QMessageBox::critical(this->parentWidget(), windowTitle(), message, QMessageBox::Ok);
}

ModuleLogger::ModuleLogger(ModuleLogWindow* window)
{
  connect(this, SIGNAL(logSignal(const QString&, const QColor&)), window, SLOT(appendMessage(const QString&, const QColor&)));
  connect(this, SIGNAL(alert(const QColor&)), window, SIGNAL(messageReceived(const QColor&)));
  connect(this, SIGNAL(popup(const QString&)), window, SLOT(popupMessageBox(const QString&)));
}

void ModuleLogger::error(const std::string& msg)
{
  auto qmsg = QString::fromStdString(msg);
  logSignal("<b>ERROR: " + qmsg + "</b>", Qt::red);
  alert(Qt::red);
  popup(qmsg);
}

void ModuleLogger::warning(const std::string& msg)
{
  logSignal("WARNING: " + QString::fromStdString(msg), Qt::yellow);
  alert(Qt::yellow);
}

void ModuleLogger::remark(const std::string& msg)
{
  logSignal("REMARK: " + QString::fromStdString(msg), Qt::blue);
}

void ModuleLogger::status(const std::string& msg)
{
  logSignal(QString::fromStdString(msg), Qt::black);
}