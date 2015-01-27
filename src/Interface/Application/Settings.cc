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
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/Connection.h>
#include <Core/Application/Preferences/Preferences.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;

namespace
{
  QString qname(const AlgorithmParameter& ap)
  {
    return QString::fromStdString(ap.name().name());
  }

  QStringList convertPathList(const std::vector<boost::filesystem::path>& paths)
  {
    QStringList strs;
    for (const auto& path : paths)
      strs << QString::fromStdString(path.string());
    return strs;
  }
}

void SCIRunMainWindow::readSettings()
{
  Preferences& prefs = Preferences::Instance();
  QSettings settings("SCI:CIBC Software", "SCIRun5");

  //TODO: centralize all these values in Preferences singleton, together with keys as names.
  //TODO: extract QSettings logic into "PreferencesIO" class
  //TODO: set up signal/slot for each prefs variable to make it easy to track changes from arbitrary widgets

  latestNetworkDirectory_ = settings.value("networkDirectory").toString();
  GuiLogger::Instance().log("Setting read: default network directory = " + latestNetworkDirectory_.path());

  recentFiles_ = settings.value("recentFiles").toStringList();
  updateRecentFileActions();
  GuiLogger::Instance().log("Setting read: recent network file list");

  QString regressionTestDataDir = settings.value("regressionTestDataDirectory").toString();
  GuiLogger::Instance().log("Setting read: regression test data directory = " + regressionTestDataDir);
  prefs_->setRegressionTestDataDir(regressionTestDataDir);

  //TODO: make a separate class for these keys, bad duplication.
  const QString colorKey = qname(prefs.networkBackgroundColor);
  if (settings.contains(colorKey))
  {
    auto value = settings.value(colorKey).toString();
    prefs.networkBackgroundColor.setValue(value.toStdString());
    networkEditor_->setBackground(QColor(value));
    GuiLogger::Instance().log("Setting read: background color = " + networkEditor_->background().color().name());
  }

  const QString notePositionKey = "defaultNotePositionIndex";
  if (settings.contains(notePositionKey))
  {
    int notePositionIndex = settings.value(notePositionKey).toInt();
    defaultNotePositionComboBox_->setCurrentIndex(notePositionIndex);
    GuiLogger::Instance().log("Setting read: default note position = " + QString::number(notePositionIndex));
  }

  const QString pipeTypeKey = "connectionPipeType";
  if (settings.contains(pipeTypeKey))
  {
    int pipeType = settings.value(pipeTypeKey).toInt();
    networkEditor_->setConnectionPipelineType(pipeType);
    GuiLogger::Instance().log("Setting read: connection pipe style = " + QString::number(pipeType));
    switch (pipeType)
    {
    case MANHATTAN:
      manhattanPipesRadioButton_->setChecked(true);
      break;
    case CUBIC:
      cubicPipesRadioButton_->setChecked(true);
      break;
    case EUCLIDEAN:
      euclideanPipesRadioButton_->setChecked(true);
      break;
    }
  }

  const QString snapTo = qname(prefs.modulesSnapToGrid);
  if (settings.contains(snapTo))
  {
    auto value = settings.value(snapTo).toBool();
    prefs.modulesSnapToGrid.setValue(value);
    modulesSnapToCheckBox_->setChecked(value);
    GuiLogger::Instance().log("Setting read: modules snap to grid = " + QString(prefs.modulesSnapToGrid ? "true" : "false"));
  }

  const QString dockable = qname(prefs.modulesAreDockable);
  if (settings.contains(dockable))
  {
    auto value = settings.value(dockable).toBool();
    prefs.modulesAreDockable.setValue(value);
    dockableModulesCheckBox_->setChecked(value);
    GuiLogger::Instance().log("Setting read: modules are dockable = " + QString::number(prefs.modulesAreDockable));
  }

  const QString disableModuleErrorDialogsKey = "disableModuleErrorDialogs";
  if (settings.contains(disableModuleErrorDialogsKey))
  {
    bool disableModuleErrorDialogs = settings.value(disableModuleErrorDialogsKey).toBool();
    GuiLogger::Instance().log("Setting read: disable module error dialogs = " + QString::number(disableModuleErrorDialogs));
    prefs_->setDisableModuleErrorDialogs(disableModuleErrorDialogs);
  }

  const QString saveBeforeExecute = "saveBeforeExecute";
  if (settings.contains(saveBeforeExecute))
  {
    bool mode = settings.value(saveBeforeExecute).toBool();
    GuiLogger::Instance().log("Setting read: save before execute = " + QString::number(mode));
    prefs_->setSaveBeforeExecute(mode);
  }

  const QString newViewSceneMouseControls = "newViewSceneMouseControls";
  if (settings.contains(newViewSceneMouseControls))
  {
    bool mode = settings.value(newViewSceneMouseControls).toBool();
    GuiLogger::Instance().log("Setting read: newViewSceneMouseControls = " + QString::number(mode));
    Core::Preferences::Instance().useNewViewSceneMouseControls.setValue(mode);
  }

  const QString favoriteModules = "favoriteModules";
  if (settings.contains(favoriteModules))
  {
    auto faves = settings.value(favoriteModules).toStringList();
    GuiLogger::Instance().log("Setting read: favoriteModules = " + faves.join(", "));
    favoriteModuleNames_ = faves;
  }

  const QString dataDirectory = "dataDirectory";
  if (settings.contains(dataDirectory))
  {
    auto dataDir = settings.value(dataDirectory).toString();
    GuiLogger::Instance().log("Setting read: dataDirectory = " + dataDir);
    setDataDirectory(dataDir);
  }

  const QString dataPath = "dataPath";
  if (settings.contains(dataPath))
  {
    auto path = settings.value(dataPath).toStringList().join(";");
    GuiLogger::Instance().log("Setting read: dataPath = " + path);
    setDataPath(path);
  }

  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
}

void SCIRunMainWindow::writeSettings()
{
  QSettings settings("SCI:CIBC Software", "SCIRun5");
  Preferences& prefs = Preferences::Instance();

  //TODO: centralize all these values in Preferences singleton, together with keys as names

  settings.setValue("networkDirectory", latestNetworkDirectory_.path());
  settings.setValue("recentFiles", recentFiles_);
  settings.setValue("regressionTestDataDirectory", prefs_->regressionTestDataDir());
  settings.setValue(qname(prefs.networkBackgroundColor), QString::fromStdString(prefs.networkBackgroundColor));
  settings.setValue(qname(prefs.modulesSnapToGrid), prefs.modulesSnapToGrid.val());
  settings.setValue(qname(prefs.modulesAreDockable), prefs.modulesAreDockable.val());
  settings.setValue("defaultNotePositionIndex", defaultNotePositionComboBox_->currentIndex());
  settings.setValue("connectionPipeType", networkEditor_->connectionPipelineType());
  settings.setValue("disableModuleErrorDialogs", prefs_->disableModuleErrorDialogs());
  settings.setValue("saveBeforeExecute", prefs_->saveBeforeExecute());
  settings.setValue("newViewSceneMouseControls", prefs.useNewViewSceneMouseControls.val());
  settings.setValue("favoriteModules", favoriteModuleNames_);
  settings.setValue("dataDirectory", QString::fromStdString(prefs.dataDirectory().string()));
  settings.setValue("dataPath", convertPathList(prefs.dataPath()));

  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
}
