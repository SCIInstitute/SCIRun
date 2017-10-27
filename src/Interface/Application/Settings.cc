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
#include <Interface/Application/ProvenanceWindow.h>
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

  // QStringList valueListAsString(const QList<QVariant>& qvs)
  // {
  //   QStringList qsl;
  //   for (const auto& qv : qvs)
  //   {
  //     qsl.append(qv.toString());
  //   }
  //   return qsl;
  // }

  QMap<QString, QString> toStrMap(const QMap<QString, QVariant>& m)
  {
    QMap<QString, QString> ss;
    for (const auto& sv : m.toStdMap())
    {
      ss[sv.first] = sv.second.toString();
    }
    return ss;
  }

  template <typename T>
  QMap<QString, QVariant> fromTypedMap(const QMap<QString, T>& m)
  {
    QMap<QString, QVariant> sv;
    for (const auto& ss : m.toStdMap())
    {
      sv[ss.first] = ss.second;
    }
    return sv;
  }

  QMap<QString, QVariant> fromStrMap(const QMap<QString, QString>& m)
  {
    return fromTypedMap(m);
  }

  QMap<QString, bool> toBoolMap(const QMap<QString, QVariant>& m)
  {
    QMap<QString, bool> ss;
    for (const auto& sv : m.toStdMap())
    {
      ss[sv.first] = sv.second.toBool();
    }
    return ss;
  }

  QMap<QString, QVariant> fromBoolMap(const QMap<QString, bool>& m)
  {
    return fromTypedMap(m);
  }
}

void SCIRunMainWindow::readSettings()
{
  auto& prefs = Preferences::Instance();
  QSettings settings;

  //TODO: centralize all these values in Preferences singleton, together with keys as names.
  //TODO: extract QSettings logic into "PreferencesIO" class
  //TODO: set up signal/slot for each prefs variable to make it easy to track changes from arbitrary widgets

  latestNetworkDirectory_ = settings.value("networkDirectory").toString();
  guiLogDebug("Setting read: default network directory = {}", latestNetworkDirectory_.path().toStdString());
  recentFiles_ = settings.value("recentFiles").toStringList();
  updateRecentFileActions();
  guiLogDebug("Setting read: recent network file list: \n\t{}",
    QStringList(
      QList<QString>::fromStdList(
        std::list<QString>(
          recentFiles_.begin(),
          recentFiles_.begin() + std::min(recentFiles_.size(), 5))))
    .join("\n\t")
    .toStdString());

  //TODO: make a separate class for these keys, bad duplication.
  const QString colorKey = qname(prefs.networkBackgroundColor);
  if (settings.contains(colorKey))
  {
    auto value = settings.value(colorKey).toString();
    prefs.networkBackgroundColor.setValue(value.toStdString());
    networkEditor_->setBackground(QColor(value));
    guiLogDebug("Setting read: background color = {}", networkEditor_->background().color().name().toStdString());
  }

  const QString notePositionKey = "defaultNotePositionIndex";
  if (settings.contains(notePositionKey))
  {
    int notePositionIndex = settings.value(notePositionKey).toInt();
    prefsWindow_->defaultNotePositionComboBox_->setCurrentIndex(notePositionIndex);
    guiLogDebug("Setting read: default note position = {}", notePositionIndex);
  }

  const QString pipeTypeKey = "connectionPipeType";
  if (settings.contains(pipeTypeKey))
  {
    int pipeType = settings.value(pipeTypeKey).toInt();
    setConnectionPipelineType(pipeType);
    guiLogDebug("Setting read: connection pipe style = {}", pipeType);
  }

  const auto snapTo = qname(prefs.modulesSnapToGrid);
  if (settings.contains(snapTo))
  {
    auto value = settings.value(snapTo).toBool();
    prefs.modulesSnapToGrid.setValue(value);
    prefsWindow_->modulesSnapToCheckBox_->setChecked(value);
    guiLogDebug("Setting read: modules snap to grid = {}", prefs.modulesSnapToGrid);
  }

  const QString portHighlight = qname(prefs.highlightPorts);
  if (settings.contains(portHighlight))
  {
    auto value = settings.value(portHighlight).toBool();
    prefs.highlightPorts.setValue(value);
    prefsWindow_->portSizeEffectsCheckBox_->setChecked(value);
    guiLogDebug("Setting read: highlight ports on hover = {}", prefs.highlightPorts);
  }

  const QString dockable = qname(prefs.modulesAreDockable);
  if (settings.contains(dockable))
  {
    auto value = settings.value(dockable).toBool();
    prefs.modulesAreDockable.setValueWithSignal(value);
    prefsWindow_->dockableModulesCheckBox_->setChecked(value);
    guiLogDebug("Setting read: modules are dockable = {}", prefs.modulesAreDockable);
  }

  const QString autoNotes = qname(prefs.autoNotes);
  if (settings.contains(autoNotes))
  {
    auto value = settings.value(autoNotes).toBool();
    prefs.autoNotes.setValue(value);
    prefsWindow_->autoModuleNoteCheckbox_->setChecked(value);
    guiLogDebug("Setting read: automatic module notes = {}", prefs.autoNotes);
  }

  const QString disableModuleErrorDialogsKey = "disableModuleErrorDialogs";
  if (settings.contains(disableModuleErrorDialogsKey))
  {
    bool disableModuleErrorDialogs = settings.value(disableModuleErrorDialogsKey).toBool();
    guiLogDebug("Setting read: disable module error dialogs = {}", disableModuleErrorDialogs);
    prefsWindow_->setDisableModuleErrorDialogs(disableModuleErrorDialogs);
  }

  const QString showModuleErrorInlineMessagesKey = "showModuleErrorInlineMessages";
  if (settings.contains(showModuleErrorInlineMessagesKey))
  {
    bool val = settings.value(showModuleErrorInlineMessagesKey).toBool();
    guiLogDebug("Setting read: show module inline error = {}", val);
    prefsWindow_->setModuleErrorInlineMessages(val);
  }

  {
    const QString highDPIAdjustment = "highDPIAdjustment";
    if (settings.contains(highDPIAdjustment))
    {
      bool val = settings.value(highDPIAdjustment).toBool();
      guiLogDebug("Setting read: high DPI adjustment = {}", val);
      prefsWindow_->setHighDPIAdjustment(val);
      if (val)
        networkEditor_->setHighResolutionExpandFactor();
    }
  }

  const QString saveBeforeExecute = "saveBeforeExecute";
  if (settings.contains(saveBeforeExecute))
  {
    bool mode = settings.value(saveBeforeExecute).toBool();
    guiLogDebug("Setting read: save before execute = {}", mode);
    prefsWindow_->setSaveBeforeExecute(mode);
  }

  const QString newViewSceneMouseControls = "newViewSceneMouseControls";
  if (settings.contains(newViewSceneMouseControls))
  {
    bool mode = settings.value(newViewSceneMouseControls).toBool();
    guiLogDebug("Setting read: newViewSceneMouseControls = {}", mode);
    Core::Preferences::Instance().useNewViewSceneMouseControls.setValue(mode);
  }

  const QString invertMouseZoom = "invertMouseZoom";
  if (settings.contains(invertMouseZoom))
  {
    bool mode = settings.value(invertMouseZoom).toBool();
    guiLogDebug("Setting read: invertMouseZoom = {}", mode);
    Core::Preferences::Instance().invertMouseZoom.setValue(mode);
  }

  const QString favoriteModules = "favoriteModules";
  if (settings.contains(favoriteModules))
  {
    auto faves = settings.value(favoriteModules).toStringList();
    guiLogDebug("Setting read: favoriteModules = {}", faves.join(", ").toStdString());
    favoriteModuleNames_ = faves;
  }

  const QString dataDirectory = "dataDirectory";
  if (settings.contains(dataDirectory))
  {
    auto dataDir = settings.value(dataDirectory).toString();
    guiLogDebug("Setting read: dataDirectory = {}", dataDir.toStdString());
    setDataDirectory(dataDir);
  }

  const QString dataPath = "dataPath";
  if (settings.contains(dataPath))
  {
    auto path = settings.value(dataPath).toStringList().join(";");
    guiLogDebug("Setting read: dataPath = {}", path.toStdString());
    setDataPath(path);
  }

  const QString tagNamesKey = "tagNames";
  if (settings.contains(tagNamesKey))
  {
    auto tagNames = settings.value(tagNamesKey).toStringList();
    guiLogDebug("Setting read: tagNames = {}", tagNames.join(";").toStdString());
    tagManagerWindow_->setTagNames(tagNames.toVector());
  }

  const QString tagColorsKey = "tagColors";
  if (settings.contains(tagColorsKey))
  {
    auto tagColors = settings.value(tagColorsKey).toStringList();
    guiLogDebug("Setting read: tagColors = {}", tagColors.join(";").toStdString());
    tagManagerWindow_->setTagColors(tagColors.toVector());
  }
  else
    tagManagerWindow_->setTagColors(QVector<QString>());

  const QString triggeredScripts = "triggeredScripts";
  if (settings.contains(triggeredScripts))
  {
    auto scriptsMap = settings.value(triggeredScripts).toMap();
    // guiLogDebug("Setting read: triggeredScripts = {}",
    //  + QStringList(scriptsMap.keys()).join(";") + " -> " + valueListAsString(scriptsMap.values()).join(";"));
    triggeredEventsWindow_->setScripts(toStrMap(scriptsMap));
  }

  const QString triggeredScriptEnableFlags = "triggeredScriptEnableFlags";
  if (settings.contains(triggeredScriptEnableFlags))
  {
    auto scriptsMap = settings.value(triggeredScriptEnableFlags).toMap();
    guiLogDebug("Setting read: triggeredScriptEnableFlags = {} [size]", scriptsMap.size());
    triggeredEventsWindow_->setScriptEnabledFlags(toBoolMap(scriptsMap));
  }

  const QString savedSubnetworksNames = "savedSubnetworksNames";
  if (settings.contains(savedSubnetworksNames))
  {
    auto subnetMap = settings.value(savedSubnetworksNames).toMap();
    guiLogDebug("Setting read: savedSubnetworksNames = {} [size]", subnetMap.size());
    savedSubnetworksNames_ = subnetMap;
  }

  const QString savedSubnetworksXml = "savedSubnetworksXml";
  if (settings.contains(savedSubnetworksXml))
  {
    auto subnetMap = settings.value(savedSubnetworksXml).toMap();
    guiLogDebug("Setting read: savedSubnetworksXml = {} [size]", subnetMap.size());
    savedSubnetworksXml_ = subnetMap;
  }

  const QString toolkitFiles = "toolkitFiles";
  if (settings.contains(toolkitFiles))
  {
    auto toolkits = settings.value(toolkitFiles).toStringList();
    guiLogDebug("Setting read: toolkitFiles = {} [size]", toolkits.size());
    toolkitFiles_ = toolkits;
  }

  const QString undoMaxItems = "undoMaxItems";
  if (settings.contains(undoMaxItems))
  {
    auto max = settings.value(undoMaxItems).toInt();
    guiLogDebug("Setting read: undoMaxItems = {}", max);
    provenanceWindow_->setMaxItems(max);
  }

  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
}

void SCIRunMainWindow::writeSettings()
{
  QSettings settings;
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
  settings.setValue(qname(prefs.highDPIAdjustment), prefs.highDPIAdjustment.val());
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
  settings.setValue("triggeredScriptEnableFlags", fromBoolMap(triggeredEventsWindow_->getScriptEnabledFlags()));
  settings.setValue("savedSubnetworksNames", savedSubnetworksNames_);
  settings.setValue("savedSubnetworksXml", savedSubnetworksXml_);
  settings.setValue("toolkitFiles", toolkitFiles_);
  settings.setValue("undoMaxItems", provenanceWindow_->maxItems());

  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
}
