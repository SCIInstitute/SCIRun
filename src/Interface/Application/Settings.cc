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
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/TagManagerWindow.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/TriggeredEventsWindow.h>
#include <Interface/Application/MacroEditor.h>
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

  QMap<QString, QString> toStrMap(const QMap<QString, QVariant>& m)
  {
    QMap<QString, QString> ss;
    for (const auto& sv : m.toStdMap())
    {
      ss[sv.first] = sv.second.toString();
    }
    return ss;
  }

  QList<QStringList> toStrList(const QList<QVariant>& lv)
  {
    QList<QStringList> ls;
    for (const auto& v : lv)
    {
      ls.push_back(v.toStringList());
    }
    return ls;
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

  template <typename T>
  QList<QVariant> fromTypedList(const QList<T>& list)
  {
    QList<QVariant> lv;
    for (const auto& t : list)
    {
      lv.push_back(t);
    }
    return lv;
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

namespace SCIRun
{
  namespace Gui
  {
    class SettingsValueInterface
    {
    public:
      virtual ~SettingsValueInterface() {}
      virtual void read() = 0;
      virtual void postRead() = 0;
      virtual void write() = 0;
    };
  }
}

template <typename T>
using ReadConverter = std::function<T(const QVariant&)>;

template <typename T>
struct StdStringifier
{
  std::string operator()(const T& t) const
  {
    return std::to_string(t);
  }
};

template <>
struct StdStringifier<bool>
{
  std::string operator()(const bool& t) const
  {
    return t ? "true" : "false";
  }
};

template <>
struct StdStringifier<QString>
{
  std::string operator()(const QString& t) const
  {
    return t.toStdString();
  }
};

template <>
struct StdStringifier<QStringList>
{
  std::string operator()(const QStringList& t) const
  {
    return t.join(", ").toStdString();
  }
};

template <typename T, typename Stringifier = StdStringifier<T>>
class SettingsValue : public SettingsValueInterface
{
public:
  SettingsValue(const QString& name, ReadConverter<T> readConverter,
    std::function<void(const T&)> postRead, std::function<T()> retriever) :
    name_(name), readConverter_(readConverter),
    postRead_(postRead), retriever_(retriever) {}

  void read() override
  {
    QSettings settings;
    if (settings.contains(name_))
    {
      value_ = readConverter_(settings.value(name_));
      guiLogDebug("Setting read: {} = {}", name_.toStdString(), stringify_(*value_));
    }
  }

  void postRead() override
  {
    if (value_)
      postRead_(*value_);
  }

  void write() override
  {
    QSettings settings;
    settings.setValue(name_, retriever_());
  }
private:
  boost::optional<T> value_;
  const QString name_;
  ReadConverter<T> readConverter_;
  std::function<void(const T&)> postRead_;
  std::function<T()> retriever_;
  Stringifier stringify_;
};

template <typename T, typename FuncT1, typename FuncT2>
SettingsValueInterfacePtr makeSetting(const QString& name, ReadConverter<T> readConverter,
  FuncT1 postRead, FuncT2 retriever)
{
  return SettingsValueInterfacePtr(new SettingsValue<T>(name, readConverter, postRead, retriever));
}

namespace
{
  ReadConverter<int> toInt = [](const QVariant& qv) { return qv.toInt(); };
  ReadConverter<bool> toBool = [](const QVariant& qv) { return qv.toBool(); };
  ReadConverter<QString> toString = [](const QVariant& qv) { return qv.toString(); };
  ReadConverter<QStringList> toStringList = [](const QVariant& qv) { return qv.toStringList(); };
}

#define prefs Preferences::Instance()

void SCIRunMainWindow::readSettings()
{
  QSettings settings;

  settingsValues_ =
  {
    makeSetting("connectionPipeType", toInt,
      [this](int p) { setConnectionPipelineType(p); },
      [this]() { return networkEditor_->connectionPipelineType(); }),
    makeSetting("defaultNotePositionIndex", toInt,
      [this](int pos) { prefsWindow_->defaultNotePositionComboBox_->setCurrentIndex(pos); },
      [this]() { return prefsWindow_->defaultNotePositionComboBox_->currentIndex(); }),
    makeSetting(qname(prefs.modulesSnapToGrid), toBool,
      [this](bool b) { prefs.modulesSnapToGrid.setValue(b); prefsWindow_->modulesSnapToCheckBox_->setChecked(b); },
      []() { return prefs.modulesSnapToGrid.val(); }),
    makeSetting(qname(prefs.highlightPorts), toBool,
      [this](bool b) { prefs.highlightPorts.setValue(b); prefsWindow_->portSizeEffectsCheckBox_->setChecked(b); },
      []() { return prefs.highlightPorts.val(); }),
    makeSetting(qname(prefs.modulesAreDockable), toBool,
      [this](bool b) { prefs.modulesAreDockable.setValueWithSignal(b); prefsWindow_->dockableModulesCheckBox_->setChecked(b); },
      []() { return prefs.modulesAreDockable.val(); }),
    makeSetting(qname(prefs.autoNotes), toBool,
      [this](bool b) { prefs.autoNotes.setValue(b); prefsWindow_->autoModuleNoteCheckbox_->setChecked(b); },
      []() { return prefs.autoNotes.val(); }),
    makeSetting("disableModuleErrorDialogs", toBool,
      [this](bool b) { prefsWindow_->setDisableModuleErrorDialogs(b); },
      [this]() { return prefsWindow_->disableModuleErrorDialogs(); }),
    makeSetting("showModuleErrorInlineMessages", toBool,
      [this](bool b) { prefsWindow_->setModuleErrorInlineMessages(b); },
      []() { return prefs.showModuleErrorInlineMessages.val(); }),
    makeSetting(qname(prefs.highDPIAdjustment), toBool,
      [this](bool b) { prefsWindow_->setHighDPIAdjustment(b);
                      if (b) networkEditor_->setHighResolutionExpandFactor(); },
      []() { return prefs.highDPIAdjustment.val(); }),
    makeSetting("saveBeforeExecute", toBool,
      [this](bool b) { prefsWindow_->setSaveBeforeExecute(b); },
      [this]() { return prefsWindow_->saveBeforeExecute(); }),
    makeSetting("newViewSceneMouseControls", toBool,
      [](bool b) { prefs.useNewViewSceneMouseControls.setValue(b); },
      []() { return prefs.useNewViewSceneMouseControls.val(); }),
    makeSetting("invertMouseZoom", toBool,
      [](bool b) { prefs.invertMouseZoom.setValue(b); },
      []() { return prefs.invertMouseZoom.val(); }),
    makeSetting("widgetSelectionCorrection", toBool,
      [](bool b) { prefs.widgetSelectionCorrection.setValue(b); },
      []() { return prefs.widgetSelectionCorrection.val(); }),
    makeSetting("autoRotateViewerOnMouseRelease", toBool,
      [](bool b) { prefs.autoRotateViewerOnMouseRelease.setValue(b); },
      []() { return prefs.autoRotateViewerOnMouseRelease.val(); }),
    makeSetting("forceGridBackground", toBool,
      [this](bool b) { prefs.forceGridBackground.setValueWithSignal(b); prefsWindow_->forceGridBackgroundCheckBox_->setChecked(b);},
      []() { return prefs.forceGridBackground.val(); }),
    makeSetting("undoMaxItems", toInt,
      [this](int p) { provenanceWindow_->setMaxItems(p); },
      [this]() { return provenanceWindow_->maxItems(); }),
    makeSetting("maxCores", toInt,
      [this](int p) { prefsWindow_->maxCoresSpinBox_->setValue(p); },
      [this]() { return prefsWindow_->maxCoresSpinBox_->value(); }),
    makeSetting("dataDirectory", toString,
      [this](const QString& s) { setDataDirectory(s); },
      []() { return QString::fromStdString(prefs.dataDirectory().string()); }),
    makeSetting("favoriteModules", toStringList,
      [this](const QStringList& qsl) { favoriteModuleNames_ = qsl; },
      [this]() { return favoriteModuleNames_; }),
    makeSetting("dataPath", toStringList,
      [this](const QStringList& qsl) { setDataPath(qsl.join(";")); },
      []() { return convertPathList(prefs.dataPath()); }),
    makeSetting("tagNames", toStringList,
      [this](const QStringList& qsl) { tagManagerWindow_->setTagNames(qsl.toVector()); },
      [this]() { return tagManagerWindow_->getTagNames(); }),
    makeSetting("tagColors", toStringList,
      [this](const QStringList& qsl) { tagManagerWindow_->setTagColors(qsl.toVector()); },
      [this]() { return tagManagerWindow_->getTagColors(); })
  };

  for (auto& setting : settingsValues_)
  {
    setting->read();
    setting->postRead();
  }

  //TODO: centralize all these values in Preferences singleton, together with keys as names.
  //TODO: extract QSettings logic into "PreferencesIO" class
  //TODO: set up signal/slot for each prefs variable to make it easy to track changes from arbitrary widgets

  latestNetworkDirectory_.setPath(settings.value("networkDirectory").toString());
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

  const QString triggeredScripts = "triggeredScripts";
  if (settings.contains(triggeredScripts))
  {
    auto scriptsMap = settings.value(triggeredScripts).toMap();
    triggeredEventsWindow_->setScripts(toStrMap(scriptsMap));
  }

  const QString triggeredScriptEnableFlags = "triggeredScriptEnableFlags";
  if (settings.contains(triggeredScriptEnableFlags))
  {
    auto scriptsMap = settings.value(triggeredScriptEnableFlags).toMap();
    guiLogDebug("Setting read: triggeredScriptEnableFlags = {} [size]", scriptsMap.size());
    triggeredEventsWindow_->setScriptEnabledFlags(toBoolMap(scriptsMap));
  }

  const QString macros = "macros";
  if (settings.contains(macros))
  {
    auto macrosList = settings.value(macros).toList();
    macroEditor_->setScripts(toStrList(macrosList));
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

  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
}

void SCIRunMainWindow::writeSettings()
{
  QSettings settings;

  for (auto& setting : settingsValues_)
  {
    setting->write();
  }

  //TODO: centralize all these values in Preferences singleton, together with keys as names

  settings.setValue("networkDirectory", latestNetworkDirectory_.path());
  settings.setValue("recentFiles", recentFiles_);
  settings.setValue("triggeredScripts", fromStrMap(triggeredEventsWindow_->scripts()));
  settings.setValue("triggeredScriptEnableFlags", fromBoolMap(triggeredEventsWindow_->scriptEnabledFlags()));
  settings.setValue("macros", fromTypedList<QStringList>(macroEditor_->scripts()));
  settings.setValue("savedSubnetworksNames", savedSubnetworksNames_);
  settings.setValue("savedSubnetworksXml", savedSubnetworksXml_);
  settings.setValue("toolkitFiles", toolkitFiles_);

  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
}
