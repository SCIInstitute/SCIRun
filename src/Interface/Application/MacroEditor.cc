/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
#include <Interface/Application/MacroEditor.h>
#include <Interface/Application/NetworkEditor.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Interface/Modules/Base/CustomWidgets/CodeEditorWidgets.h>

using namespace SCIRun::Gui;

MacroEditor::MacroEditor(QWidget* parent /* = 0 */) : QDockWidget(parent),
  scriptPlainTextEdit_(new CodeEditor(this))
{
  setupUi(this);
  gridLayout_2->addWidget(scriptPlainTextEdit_, 1, 1);
  connect(macroListWidget_, SIGNAL(itemSelectionChanged()), this, SLOT(updateScriptEditor()));
  connect(scriptPlainTextEdit_, SIGNAL(textChanged()), this, SLOT(updateScripts()));
  connect(addPushButton_, SIGNAL(clicked()), this, SLOT(addMacro()));
  connect(removePushButton_, SIGNAL(clicked()), this, SLOT(removeMacro()));

  auto assignMenu = new QMenu(this);
  for (int i = 1; i <= 5; ++i)
  {
    setupAssignToAction(assignMenu->addAction(QString::number(i)), i);
  }

  assignToButtonPushButton_->setMenu(assignMenu);
}

static const char* macroIndex = "macroIndex";

void MacroEditor::setupAssignToAction(QAction* action, int i)
{
  action->setProperty(macroIndex, i);
  connect(action, SIGNAL(triggered()), this, SLOT(assignToButton()));
}

void MacroEditor::assignToButton()
{
  auto index = sender()->property(macroIndex).toInt();
  qDebug() << "assigning to macro " << index;
  auto selected = macroListWidget_->selectedItems();
  if (!selected.isEmpty())
  {
    selected[0]->setText(tr("[%0] %1").arg(index).arg(selected[0]->text()));
  }
}

const QMap<QString, QString>& MacroEditor::scripts() const
{
  return macros_;
}

void MacroEditor::setScripts(const QMap<QString, QString>& macros)
{
  macros_ = macros;

  for (const auto& macroPair : macros_.toStdMap())
  {
    auto item = new QListWidgetItem(macroPair.first, macroListWidget_);
  }
  if (!macros_.isEmpty())
  {
    macroListWidget_->setCurrentItem(macroListWidget_->item(0));
    updateScriptEditor();
  }
}

namespace
{
  const QString defaultScript = "# Insert Python API calls here.\n"
    "mods = scirun_module_ids()\n"
    "view = scirun_add_module('ViewScene')\n"
    "cnt=0\n"
    "for mod in mods:\n"
    "\tprint(mod)\n"
    "\tif 'Show' in mod:\n"
    "\t\tprint('connecting ',cnt)\n"
    "\t\tscirun_connect_modules(mod,0,view,cnt)\n"
    "\t\tcnt+=1\n"
    ;
}

void MacroEditor::addMacro()
{
  bool ok;
  QString text = QInputDialog::getText(this, "Add Macro", "Macro name:", QLineEdit::Normal, "", &ok);
  if (ok && !text.isEmpty())
  {
    auto item = new QListWidgetItem(text, macroListWidget_);
    macros_[text] = defaultScript;
  }
}

void MacroEditor::removeMacro()
{
  for (auto& item : macroListWidget_->selectedItems())
  {
    qDebug() << "removing " << item->text();
    macros_.remove(item->text());
    macroListWidget_->blockSignals(true);
    delete item;
    macroListWidget_->blockSignals(false);
    updateScriptEditor();
  }
}

void MacroEditor::updateScriptEditor()
{
  auto item = macroListWidget_->currentItem();
  if (item)
  {
    auto key = item->text();
    auto scr = macros_[key];
    scriptPlainTextEdit_->setPlainText(!scr.isEmpty() ? scr : defaultScript);
  }
  else
  {
    scriptPlainTextEdit_->setPlainText("");
  }
}

void MacroEditor::updateScripts()
{
  if (macroListWidget_->count() > 0)
  {
    auto key = macroListWidget_->currentItem()->text();
    auto script = scriptPlainTextEdit_->toPlainText();
    macros_[key] = script;
  }
}
