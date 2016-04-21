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
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/TagManagerWindow.h>
#include <Interface/Application/TriggeredEventsWindow.h>
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

  QStringList valueListAsString(const QList<QVariant>& qvs)
  {
    QStringList qsl;
    for (const auto& qv : qvs)
    {
      qsl.append(qv.toString());
    }
    return qsl;
  }

  QMap<QString, QString> toStrMap(const QMap<QString, QVariant>& m)
  {
    QMap<QString, QString> ss;
    for (const auto& sv : m.toStdMap())
    {
      ss[sv.first] = sv.second.toString();
    }
    return ss;
  }

  QMap<QString, QVariant> fromStrMap(const QMap<QString, QString>& m)
  {
    QMap<QString, QVariant> sv;
    for (const auto& ss : m.toStdMap())
    {
      sv[ss.first] = ss.second;
    }
    return sv;
  }
}

void SCIRunMainWindow::readSettings()
{
  auto& prefs = Preferences::Instance();
  QSettings settings("SCI:CIBC Software", "SCIRun5");

  //TODO: centralize all these values in Preferences singleton, together with keys as names.
  //TODO: extract QSettings logic into "PreferencesIO" class
  //TODO: set up signal/slot for each prefs variable to make it easy to track changes from arbitrary widgets

  latestNetworkDirectory_ = settings.value("networkDirectory").toString();
  GuiLogger::Instance().logInfo("Setting read: default network directory = " + latestNetworkDirectory_.path());

  recentFiles_ = settings.value("recentFiles").toStringList();
  updateRecentFileActions();
  GuiLogger::Instance().logInfo("Setting read: recent network file list");

  //TODO: make a separate class for these keys, bad duplication.
  const QString colorKey = qname(prefs.networkBackgroundColor);
  if (settings.contains(colorKey))
  {
    auto value = settings.value(colorKey).toString();
    prefs.networkBackgroundColor.setValue(value.toStdString());
    networkEditor_->setBackground(QColor(value));
    GuiLogger::Instance().logInfo("Setting read: background color = " + networkEditor_->background().color().name());
  }

  const QString notePositionKey = "defaultNotePositionIndex";
  if (settings.contains(notePositionKey))
  {
    int notePositionIndex = settings.value(notePositionKey).toInt();
    prefsWindow_->defaultNotePositionComboBox_->setCurrentIndex(notePositionIndex);
    GuiLogger::Instance().logInfo("Setting read: default note position = " + QString::number(notePositionIndex));
  }

  const QString pipeTypeKey = "connectionPipeType";
  if (settings.contains(pipeTypeKey))
  {
    int pipeType = settings.value(pipeTypeKey).toInt();
    networkEditor_->setConnectionPipelineType(pipeType);
    GuiLogger::Instance().logInfo("Setting read: connection pipe style = " + QString::number(pipeType));
    switch (pipeType)
    {
    case MANHATTAN:
      prefsWindow_->manhattanPipesRadioButton_->setChecked(true);
      break;
    case CUBIC:
      prefsWindow_->cubicPipesRadioButton_->setChecked(true);
      break;
    case EUCLIDEAN:
      prefsWindow_->euclideanPipesRadioButton_->setChecked(true);
      break;
    }
  }

  const auto snapTo = qname(prefs.modulesSnapToGrid);
  if (settings.contains(snapTo))
  {
    auto value = settings.value(snapTo).toBool();
    prefs.modulesSnapToGrid.setValue(value);
    prefsWindow_->modulesSnapToCheckBox_->setChecked(value);
    GuiLogger::Instance().logInfo("Setting read: modules snap to grid = " + QString::number(prefs.modulesSnapToGrid));
  }

  const QString portHighlight = qname(prefs.highlightPorts);
  if (settings.contains(portHighlight))
  {
    auto value = settings.value(portHighlight).toBool();
    prefs.highlightPorts.setValue(value);
    prefsWindow_->portSizeEffectsCheckBox_->setChecked(value);
    GuiLogger::Instance().logInfo("Setting read: highlight ports on hover = " + QString::number(prefs.highlightPorts));
  }

  const QString dockable = qname(prefs.modulesAreDockable);
  if (settings.contains(dockable))
  {
    auto value = settings.value(dockable).toBool();
    prefs.modulesAreDockable.setValue(value);
    prefsWindow_->dockableModulesCheckBox_->setChecked(value);
    GuiLogger::Instance().logInfo("Setting read: modules are dockable = " + QString::number(prefs.modulesAreDockable));
  }

  const QString autoNotes = qname(prefs.autoNotes);
  if (settings.contains(autoNotes))
  {
    auto value = settings.value(autoNotes).toBool();
    prefs.autoNotes.setValue(value);
    prefsWindow_->autoModuleNoteCheckbox_->setChecked(value);
    GuiLogger::Instance().logInfo("Setting read: automatic module notes = " + QString::number(prefs.autoNotes));
  }

  const QString disableModuleErrorDialogsKey = "disableModuleErrorDialogs";
  if (settings.contains(disableModuleErrorDialogsKey))
  {
    bool disableModuleErrorDialogs = settings.value(disableModuleErrorDialogsKey).toBool();
    GuiLogger::Instance().logInfo("Setting read: disable module error dialogs = " + QString::number(disableModuleErrorDialogs));
    prefsWindow_->setDisableModuleErrorDialogs(disableModuleErrorDialogs);
  }

  const QString showModuleErrorInlineMessagesKey = "showModuleErrorInlineMessages";
  if (settings.contains(showModuleErrorInlineMessagesKey))
  {
    bool val = settings.value(showModuleErrorInlineMessagesKey).toBool();
    GuiLogger::Instance().logInfo("Setting read: show module inline error = " + QString::number(val));
    prefsWindow_->setModuleErrorInlineMessages(val);
  }

  const QString saveBeforeExecute = "saveBeforeExecute";
  if (settings.contains(saveBeforeExecute))
  {
    bool mode = settings.value(saveBeforeExecute).toBool();
    GuiLogger::Instance().logInfo("Setting read: save before execute = " + QString::number(mode));
    prefsWindow_->setSaveBeforeExecute(mode);
  }

  const QString newViewSceneMouseControls = "newViewSceneMouseControls";
  if (settings.contains(newViewSceneMouseControls))
  {
    bool mode = settings.value(newViewSceneMouseControls).toBool();
    GuiLogger::Instance().logInfo("Setting read: newViewSceneMouseControls = " + QString::number(mode));
    Core::Preferences::Instance().useNewViewSceneMouseControls.setValue(mode);
  }
  
  const QString invertMouseZoom = "invertMouseZoom";
  if (settings.contains(invertMouseZoom))
  {
    bool mode = settings.value(invertMouseZoom).toBool();
    GuiLogger::Instance().logInfo("Setting read: invertMouseZoom = " + QString::number(mode));
    Core::Preferences::Instance().invertMouseZoom.setValue(mode);
  }

  const QString favoriteModules = "favoriteModules";
  if (settings.contains(favoriteModules))
  {
    auto faves = settings.value(favoriteModules).toStringList();
    GuiLogger::Instance().logInfo("Setting read: favoriteModules = " + faves.join(", "));
    favoriteModuleNames_ = faves;
  }

  const QString dataDirectory = "dataDirectory";
  if (settings.contains(dataDirectory))
  {
    auto dataDir = settings.value(dataDirectory).toString();
    GuiLogger::Instance().logInfo("Setting read: dataDirectory = " + dataDir);
    setDataDirectory(dataDir);
  }

  const QString dataPath = "dataPath";
  if (settings.contains(dataPath))
  {
    auto path = settings.value(dataPath).toStringList().join(";");
    GuiLogger::Instance().logInfo("Setting read: dataPath = " + path);
    setDataPath(path);
  }

  const QString tagNamesKey = "tagNames";
  if (settings.contains(tagNamesKey))
  {
    auto tagNames = settings.value(tagNamesKey).toStringList();
    GuiLogger::Instance().logInfo("Setting read: tagNames = " + tagNames.join(";"));
    tagManagerWindow_->setTagNames(tagNames.toVector());
  }

  const QString tagColorsKey = "tagColors";
  if (settings.contains(tagColorsKey))
  {
    auto tagColors = settings.value(tagColorsKey).toStringList();
    GuiLogger::Instance().logInfo("Setting read: tagColors = " + tagColors.join(";"));
    tagManagerWindow_->setTagColors(tagColors.toVector());
  }
  else
    tagManagerWindow_->setTagColors(QVector<QString>());

  const QString triggeredScripts = "triggeredScripts";
  if (settings.contains(triggeredScripts))
  {
    auto scriptsMap = settings.value(triggeredScripts).toMap();
    GuiLogger::Instance().logInfo("Setting read: triggeredScripts = " + QStringList(scriptsMap.keys()).join(";") + " -> " + valueListAsString(scriptsMap.values()).join(";"));
    triggeredEventsWindow_->setScripts(toStrMap(scriptsMap));
  }

  const QString savedSubnetworks = "savedSubnetworks";
  if (settings.contains(savedSubnetworks))
  {
    auto subnetMap = settings.value(savedSubnetworks).toMap();
    GuiLogger::Instance().logInfo("Setting read: savedSubnetworks = " + QStringList(subnetMap.keys()).join(";"));
    savedSubnetworks_ = subnetMap;
  }

  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
}

void SCIRunMainWindow::writeSettings()
{
  QSettings settings("SCI:CIBC Software", "SCIRun5");
  auto& prefs = Preferences::Instance();

  //TODO: centralize all these values in Preferences singleton, together with keys as names

  settings.setValue("networkDirectory", latestNetworkDirectory_.path());
  settings.setValue("recentFiles", recentFiles_);
  settings.setValue(qname(prefs.networkBackgroundColor), QString::fromStdString(prefs.networkBackgroundColor));
  settings.setValue(qname(prefs.modulesSnapToGrid), prefs.modulesSnapToGrid.val());
  settings.setValue(qname(prefs.modulesAreDockable), prefs.modulesAreDockable.val());
  settings.setValue(qname(prefs.autoNotes), prefs.autoNotes.val());
  settings.setValue(qname(prefs.highlightPorts), prefs.highlightPorts.val());
  settings.setValue(qname(prefs.showModuleErrorInlineMessages), prefs.showModuleErrorInlineMessages.val());
  settings.setValue("defaultNotePositionIndex", prefsWindow_->defaultNotePositionComboBox_->currentIndex());
  settings.setValue("connectionPipeType", networkEditor_->connectionPipelineType());
  settings.setValue("disableModuleErrorDialogs", prefsWindow_->disableModuleErrorDialogs());
  settings.setValue("saveBeforeExecute", prefsWindow_->saveBeforeExecute());
  settings.setValue("newViewSceneMouseControls", prefs.useNewViewSceneMouseControls.val());
  settings.setValue("invertMouseZoom", prefs.invertMouseZoom.val());
  settings.setValue("favoriteModules", favoriteModuleNames_);
  settings.setValue("dataDirectory", QString::fromStdString(prefs.dataDirectory().string()));
  settings.setValue("dataPath", convertPathList(prefs.dataPath()));
  settings.setValue("tagNames", tagManagerWindow_->getTagNames());
  settings.setValue("tagColors", tagManagerWindow_->getTagColors());
  settings.setValue("triggeredScripts", fromStrMap(triggeredEventsWindow_->getScripts()));
  settings.setValue("savedSubnetworks", savedSubnetworks_);

  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
}
