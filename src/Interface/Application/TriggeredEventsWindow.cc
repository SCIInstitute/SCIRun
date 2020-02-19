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
#include <Interface/Application/TriggeredEventsWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Interface/Modules/Base/CustomWidgets/CodeEditorWidgets.h>

using namespace SCIRun::Gui;

TriggeredEventsWindow::TriggeredEventsWindow(QWidget* parent /* = 0 */) : QDockWidget(parent),
  scriptPlainTextEdit_(new CodeEditor(this))
{
  setupUi(this);
  gridLayout_2->addWidget(scriptPlainTextEdit_, 2, 0);
  connect(eventListWidget_, SIGNAL(itemSelectionChanged()), this, SLOT(updateScriptEditor()));
  connect(scriptPlainTextEdit_, SIGNAL(textChanged()), this, SLOT(updateScripts()));
  connect(enabledCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(enableStateChanged(int)));
}

const QMap<QString, QString>& TriggeredEventsWindow::scripts() const
{
  return scripts_;
}

void TriggeredEventsWindow::setScripts(const QMap<QString, QString>& scripts)
{
  scripts_ = scripts;

  eventListWidget_->setCurrentItem(eventListWidget_->item(1));
  updateScriptEditor();
}

const QMap<QString, bool>& TriggeredEventsWindow::scriptEnabledFlags() const
{
  return scriptEnabledFlags_;
}

void TriggeredEventsWindow::setScriptEnabledFlags(const QMap<QString, bool>& scriptsEnabled)
{
  scriptEnabledFlags_ = scriptsEnabled;

  eventListWidget_->setCurrentItem(eventListWidget_->item(1));
  updateScriptEditor();
}

namespace
{
  const QString defaultScript = "# Insert Python API calls here.\n"
    "\n"
    "# Examples:\n"
    "\n"
    "# With the \"Post module add\" event, this snippet will change the initial filetype for a specific type of input module :\n"
    "# scirun_set_module_state(scirun_module_ids()[-1], 'FileTypeName', 'Matlab Matrix (*.mat)') if scirun_module_ids()[-1].startswith('ReadMatrix') else None\n"
    "\n"
    "# With the \"On network load\" event, this snippet will open the UIs for all the ViewScenes in the network :\n"
    "# [scirun_set_module_state(id, '__UI__', True) for id in scirun_module_ids() if id.startswith('ViewScene')]\n"
    "\n"
    "# With the \"Application start\" event, this snippet will print SCIRun's python library path, which one could then edit:\n"
    "import sys\n\nprint(sys.path)"
    ;
}

void TriggeredEventsWindow::updateScriptEditor()
{
  auto key = eventListWidget_->currentItem()->text();
  auto scr = scripts_[key];
  scriptPlainTextEdit_->setPlainText(!scr.isEmpty() ? scr : defaultScript);
  enabledCheckBox_->setChecked(scriptEnabledFlags_[key]);
  push();
}

void TriggeredEventsWindow::updateScripts()
{
  auto key = eventListWidget_->currentItem()->text();
  auto script = scriptPlainTextEdit_->toPlainText();
  scripts_[key] = script;

  //TODO: waiting on implementation of #41, see ModuleDialogGeneric.h comment
  push();
}

void TriggeredEventsWindow::enableStateChanged(int state)
{
  auto key = eventListWidget_->currentItem()->text();
  auto enabled = state == Qt::Checked;
  scriptEnabledFlags_[key] = enabled;

  //TODO: waiting on implementation of #41, see ModuleDialogGeneric.h comment
  push();
}

void TriggeredEventsWindow::push()
{
  auto& prefs = Core::Preferences::Instance();
  static std::map<QString, Core::TriggeredScriptInfo*> scriptInfos
  {
    { "Post module add", &prefs.postModuleAdd },
    { "On network load", &prefs.onNetworkLoad },
    { "Application start", &prefs.applicationStart }
  };

  for (int i = 0; i < eventListWidget_->count(); ++i)
  {
    auto key = eventListWidget_->item(i)->text();
    auto trig = scriptInfos.find(key);
    if (trig != scriptInfos.end())
    {
      trig->second->script.setValue(scripts_[key].toStdString());
      trig->second->enabled.setValue(scriptEnabledFlags_[key]);
    }
  }
}
