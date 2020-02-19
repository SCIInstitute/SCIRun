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


#include <Interface/qt_include.h>
#include <iostream>
#include <Interface/Application/ModuleLogWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/DialogErrorControl.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;

ModuleLogWindow::ModuleLogWindow(const QString& moduleName, ModuleErrorDisplayer* displayer, boost::shared_ptr<SCIRun::Gui::DialogErrorControl> dialogErrorControl, QWidget* parent) : QDialog(parent), moduleName_(moduleName),
  displayer_(displayer),
	dialogErrorControl_(dialogErrorControl)
{
	setupUi(this);
	setModal(false);
	setWindowTitle("Log for " + moduleName);
  setVisible(false);
  buttonBox->button(QDialogButtonBox::Discard)->setText("Clear");
  connect(buttonBox->button(QDialogButtonBox::Discard), SIGNAL(clicked()), logTextEdit_, SLOT(clear()));
  logTextEdit_->setStyleSheet("background-color: lightgray;");
}

namespace
{
  template <class StringType>
  StringType formatWithColor(const StringType& message, const StringType& colorName)
  {
    //"<span style=\"color: red\">" << "[" << moduleName_ << "] " << msg << "</span>"
    return "<span style=\"color:" + colorName + "\">" + message + "</span><br>";
  }
}

void ModuleLogWindow::appendMessage(const QString& message, const QColor& color /* = Qt::black */)
{
  logTextEdit_->insertHtml(formatWithColor(message, color.name()));
}

void ModuleLogWindow::popupMessageBox(const QString& message)
{
  dialogErrorControl_->increaseCounter();
  auto errorText = "Error in " + moduleName_ + "\n" + message;
  if (dialogErrorControl_->showDialog())
  {
    QMessageBox* msgBox = new QMessageBox(parentWidget());
    msgBox->setIcon(QMessageBox::Critical);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setStandardButtons(QMessageBox::Ok);
    auto showButton = msgBox->addButton("Show Module", QMessageBox::ApplyRole);
    connect(showButton, SIGNAL(clicked()), this, SIGNAL(requestModuleVisible()));
    msgBox->setWindowTitle(windowTitle());
    msgBox->setText(errorText);
    msgBox->setModal(false);
    msgBox->show();
  }

  //TODO: need another limit on these?
  displayer_->displayError(errorText, [this]() { requestModuleVisible(); });
}

ModuleLogger::ModuleLogger(ModuleLogWindow* window) : moduleName_(window->name().toStdString())
{
  connect(this, SIGNAL(logSignal(const QString&, const QColor&)), window, SLOT(appendMessage(const QString&, const QColor&)));
  connect(this, SIGNAL(alert(const QColor&)), window, SIGNAL(messageReceived(const QColor&)));
  connect(this, SIGNAL(popup(const QString&)), window, SLOT(popupMessageBox(const QString&)));
}

ModuleLogger::~ModuleLogger()
{
}

void ModuleLogger::error(const std::string& msg) const
{
  const QColor red = Qt::red;
  auto qmsg = QString::fromStdString(msg);
  logSignal("<b>ERROR: " + qmsg + "</b>", red);
  alert(red);
  popup(qmsg);

  auto log = ModuleLog::Instance().get();
  if (log)
    log->error("[{0}] {1}", moduleName_, msg);

  errorReported_ = true;
}

void ModuleLogger::warning(const std::string& msg) const
{
  const QColor yellow = Qt::yellow;
  logSignal("WARNING: " + QString::fromStdString(msg), yellow);
  alert(yellow);

  auto log = ModuleLog::Instance().get();
  if (log)
    log->warn("[{0}] {1}", moduleName_, msg);
}

void ModuleLogger::remark(const std::string& msg) const
{
  const QColor blue = Qt::blue;
  logSignal("REMARK: " + QString::fromStdString(msg), blue);
  alert(blue);

  auto log = ModuleLog::Instance().get();
  if (log)
    log->info("[{0}] NOTICE: {1}", moduleName_, msg);
}

void ModuleLogger::status(const std::string& msg) const
{
  logSignal(QString::fromStdString(msg), Qt::black);

  auto log = ModuleLog::Instance().get();
  if (log)
    log->info("[{0}] {1}", moduleName_, msg);
}
