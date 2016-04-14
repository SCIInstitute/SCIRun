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

#include <QtGui>
#include <Interface/Application/TriggeredEventsWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Core/Application/Preferences/Preferences.h>

using namespace SCIRun::Gui;
//using namespace SCIRun::Dataflow::Networks;
//using namespace SCIRun::Dataflow::Engine;

TriggeredEventsWindow::TriggeredEventsWindow(QWidget* parent /* = 0 */) : QDockWidget(parent)
{
  setupUi(this);
  connect(eventListWidget_, SIGNAL(itemSelectionChanged()), this, SLOT(updateScriptEditor()));
  connect(scriptPlainTextEdit_, SIGNAL(textChanged()), this, SLOT(updateScripts()));
}

const QMap<QString, QString>& TriggeredEventsWindow::getScripts() const
{
  return scripts_;
}

void TriggeredEventsWindow::setScripts(const QMap<QString, QString>& scripts)
{
  scripts_ = scripts;

  //TODO: hardcode the only enabled entry
  eventListWidget_->setCurrentItem(eventListWidget_->item(1));
  scriptPlainTextEdit_->setPlainText(scripts_[eventListWidget_->currentItem()->text()]);
}

void TriggeredEventsWindow::updateScriptEditor()
{
  auto scr = scripts_[eventListWidget_->currentItem()->text()];
  scriptPlainTextEdit_->setPlainText(scr);
}

void TriggeredEventsWindow::updateScripts()
{
  scripts_[eventListWidget_->currentItem()->text()] = scriptPlainTextEdit_->toPlainText();
  Core::Preferences::Instance().postModuleAddScript_temporarySolution.setValue(scriptPlainTextEdit_->toPlainText().toStdString());
}
