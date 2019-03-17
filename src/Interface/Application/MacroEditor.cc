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
#include <Interface/Application/SCIRunMainWindow.h>

#ifdef BUILD_WITH_PYTHON
#include <Core/Python/PythonInterpreter.h>
#endif

using namespace SCIRun::Gui;
using namespace SCIRun::Core;

MacroEditor::MacroEditor(QWidget* parent /* = 0 */) : QDockWidget(parent),
  scriptPlainTextEdit_(new CodeEditor(this))
{
  setupUi(this);
  gridLayout_2->addWidget(scriptPlainTextEdit_, 1, 1);
  connect(macroListWidget_, SIGNAL(itemSelectionChanged()), this, SLOT(updateScriptEditor()));
  connect(scriptPlainTextEdit_, SIGNAL(textChanged()), this, SLOT(updateScripts()));
  connect(addPushButton_, SIGNAL(clicked()), this, SLOT(addMacro()));
  connect(removePushButton_, SIGNAL(clicked()), this, SLOT(removeMacro()));
  connect(runNowPushButton_, SIGNAL(clicked()), this, SLOT(runSelectedMacro()));

  auto assignMenu = new QMenu(this);
  for (int i = 1; i <= 5; ++i)
  {
    setupAssignToAction(assignMenu->addAction(QString::number(i)), i);
  }

  assignToButtonPushButton_->setMenu(assignMenu);
}

const char* MacroEditor::Index = "macroIndex";
static int macroIndexInt = Qt::UserRole;

void MacroEditor::setupAssignToAction(QAction* action, int i)
{
  action->setProperty(Index, i);
  connect(action, SIGNAL(triggered()), this, SLOT(assignToButton()));
}

void MacroEditor::assignToButton()
{
  auto index = sender()->property(Index).toInt();
  auto selected = macroListWidget_->selectedItems();
  auto row = macroListWidget_->currentRow();
  if (!selected.isEmpty())
  {
    for (int i = 0; i < macroListWidget_->count(); ++i)
    {
      auto other = macroListWidget_->item(i);
      if (other->data(macroIndexInt).toInt() == index)
      {
        other->setToolTip("");
        other->setData(macroIndexInt, 0);
        macros_[i][MacroListItem::ButtonNumber] = "";
      }
    }

    auto item = selected[0];
    item->setToolTip(tr("Assigned to macro button %0").arg(index));
    item->setData(macroIndexInt, index);
    macros_[row][MacroListItem::ButtonNumber] = QString::number(index);
  }
}

const MacroNameValueList& MacroEditor::scripts() const
{
  return macros_;
}

void MacroEditor::setScripts(const MacroNameValueList& macros)
{
  macros_ = macros;

  for (const auto& macroList : macros_)
  {
    auto item = new QListWidgetItem(macroList[MacroListItem::Name], macroListWidget_);
    if (macroList.size() > MacroListItem::ButtonNumber)
    {
      auto button = macroList[MacroListItem::ButtonNumber];
      if (!button.isEmpty())
      {
        auto index = button.toInt();
        item->setToolTip(tr("Assigned to macro button %0").arg(index));
        item->setData(macroIndexInt, index);
      }
    }
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
    "# This useful example will connect all available ShowField modules to a new ViewScene.\n"
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
  auto name = QInputDialog::getText(this, "Add Macro", "Macro name:", QLineEdit::Normal, "", &ok);
  if (ok && !name.isEmpty())
  {
    new QListWidgetItem(name, macroListWidget_);
    macros_.push_back(QStringList() << name << defaultScript << "");
  }
}

void MacroEditor::removeMacro()
{
  auto row = macroListWidget_->currentRow();
  for (auto& item : macroListWidget_->selectedItems())
  {
    auto name = item->text();
    macros_.removeAt(row);

    macroListWidget_->blockSignals(true);
    delete item;
    macroListWidget_->blockSignals(false);
    updateScriptEditor();
  }
}

void MacroEditor::updateScriptEditor()
{
  auto item = macroListWidget_->currentItem();
  auto row = macroListWidget_->currentRow();
  if (item)
  {
    auto key = item->text();
    auto scr = macros_[row][MacroListItem::Script];
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
    auto row = macroListWidget_->currentRow();
    auto key = macroListWidget_->currentItem()->text();
    auto script = scriptPlainTextEdit_->toPlainText();
    macros_[row][MacroListItem::Script] = script;
  }
}

void MacroEditor::runSelectedMacro()
{
#ifdef BUILD_WITH_PYTHON
  auto row = macroListWidget_->currentRow();
  if (row >= 0 && row < macros_.size())
  {
    auto scr = macros_[row][MacroListItem::Script];

    NetworkEditor::InEditingContext iec(SCIRunMainWindow::Instance()->networkEditor());
    PythonInterpreter::Instance().run_script(scr.toStdString());
  }
#endif
}

QString MacroEditor::macroForButton(int i) const
{
  return "";
}
