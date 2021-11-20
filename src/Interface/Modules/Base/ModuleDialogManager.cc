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


#include <QtGui>
#include <QAction>
#include <QAbstractButton>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Interface/Modules/Base/ModuleDialogManager.h>
#include <Interface/Modules/Base/ModuleLogWindow.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;


ModuleLogWindow* ModuleDialogManager::setupLogging(ModuleErrorDisplayer* displayer, QAction* showLogAction, QWidget* parent)
{
  logWindow_ = new ModuleLogWindow(QString::fromStdString(module_->id().id_), displayer, parent);
  if (showLogAction)
  {
    QObject::connect(showLogAction, &QAction::triggered, logWindow_, &QDialog::show);
    QObject::connect(showLogAction, &QAction::triggered, logWindow_, &QDialog::raise);
  }
  else
  {
    qDebug() << "showLogAction null";
  }
  
  module_->setLogger(makeShared<ModuleLogger>(logWindow_));
  return logWindow_;
}

void ModuleDialogManager::connectDisplayLogButton(QAbstractButton* button)
{
  QObject::connect(button, &QAbstractButton::clicked, logWindow_, &QDialog::show);
  QObject::connect(button, &QAbstractButton::clicked, logWindow_, &QDialog::raise);
}

void ModuleDialogManager::closeOptions()
{
  if (options_)
  {
    options_->close();
  }
}

void ModuleDialogManager::destroyLog()
{
  delete logWindow_;
  logWindow_ = nullptr;
}

void ModuleDialogManager::destroyOptions()
{
  delete options_;
  options_ = nullptr;
}

void ModuleDialogManager::createOptions()
{
  options_ = ModuleDialogGeneric::factory()->makeDialog(module_->id().id_, module_->get_state());
}