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
struct Stringifier
{
  std::string operator()(const T& t) const
  {
    return std::to_string(t);
  }
};

template <>
struct Stringifier<bool>
{
  std::string operator()(const bool& t) const
  {
    return t ? "true" : "false";
  }
};

template <>
struct Stringifier<QString>
{
  std::string operator()(const QString& t) const
  {
    return t.toStdString();
  }
};

template <>
struct Stringifier<QStringList>
{
  std::string operator()(const QStringList& t) const
  {
    return t.join(", ").toStdString();
  }
};

template <typename T>
struct Stringifier<QList<T>>
{
  std::string operator()(const QList<T>& t) const
  {
    return "<list>";
  }
};

template <>
struct Stringifier<QByteArray>
{
  std::string operator()(const QByteArray& t) const
  {
    return "<byte array>";
  }
};

template <>
struct Stringifier<QMap<QString, QVariant>>
{
  std::string operator()(const QMap<QString, QVariant>& t) const
  {
    return "map [#items = " + std::to_string(t.size()) + "]";
  }
};

template <typename T>
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
  Stringifier<T> stringify_;
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
  ReadConverter<QMap<QString, QVariant>> toMap = [](const QVariant& qv) { return qv.toMap(); };
  ReadConverter<QList<QVariant>> toList = [](const QVariant& qv) { return qv.toList(); };
  ReadConverter<QByteArray> toByteArray = [](const QVariant& qv) { return qv.toByteArray(); };
}

#define prefs Preferences::Instance()

void SCIRunMainWindow::readSettings()
{
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
      [this](bool b) { prefsWindow_->setWidgetSelectionCorrection(b); },
      []() { return prefs.widgetSelectionCorrection.val(); }),
    makeSetting("autoRotateViewerOnMouseRelease", toBool,
      [this](bool b) { prefsWindow_->setAutoRotateViewerOnMouseRelease(b); },
      []() { return prefs.autoRotateViewerOnMouseRelease.val(); }),
    makeSetting("moduleExecuteDownstreamOnly", toBool,
      [this](bool b) { prefsWindow_->setModuleExecuteDownstreamOnly(b); },
      []() { return prefs.moduleExecuteDownstreamOnly.val(); }),
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
    makeSetting("networkDirectory", toString,
      [this](const QString& s) { latestNetworkDirectory_.setPath(s); },
      [this]() { return latestNetworkDirectory_.path(); }),
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
      [this]() { return tagManagerWindow_->getTagColors(); }),
    makeSetting("recentFiles", toStringList,
      [this](const QStringList& qsl) { recentFiles_ = qsl; updateRecentFileActions(); },
      [this]() { return recentFiles_; }),
    makeSetting("toolkitFiles", toStringList,
      [this](const QStringList& qsl) { toolkitFiles_ = qsl; },
      [this]() { return toolkitFiles_; }),
    makeSetting("triggeredScripts", toMap,
      [this](const QMap<QString, QVariant>& qmap) { triggeredEventsWindow_->setScripts(toStrMap(qmap)); },
      [this]() { return fromStrMap(triggeredEventsWindow_->scripts()); }),
    makeSetting("triggeredScriptEnableFlags", toMap,
      [this](const QMap<QString, QVariant>& qmap) { triggeredEventsWindow_->setScriptEnabledFlags(toBoolMap(qmap)); },
      [this]() { return fromBoolMap(triggeredEventsWindow_->scriptEnabledFlags()); }),
    makeSetting("savedSubnetworksNames", toMap,
      [this](const QMap<QString, QVariant>& qmap) { savedSubnetworksNames_ = qmap; },
      [this]() { return savedSubnetworksNames_; }),
    makeSetting("savedSubnetworksXml", toMap,
      [this](const QMap<QString, QVariant>& qmap) { savedSubnetworksXml_ = qmap; },
      [this]() { return savedSubnetworksXml_; }),
    makeSetting("macros", toList,
      [this](const QList<QVariant>& ql) { macroEditor_->setScripts(toStrList(ql)); },
      [this]() { return fromTypedList<QStringList>(macroEditor_->scripts()); }),
    makeSetting("geometry", toByteArray,
      [this](const QByteArray& ba) { restoreGeometry(ba); },
      [this]() { return saveGeometry(); }),
    makeSetting("windowState", toByteArray,
      [this](const QByteArray& ba) { restoreState(ba); },
      [this]() { return saveState(); })
  };

  for (auto& setting : settingsValues_)
  {
    setting->read();
    setting->postRead();
  }

  //TODO: extract QSettings logic into "PreferencesIO" class
  //TODO: set up signal/slot for each prefs variable to make it easy to track changes from arbitrary widgets
}

void SCIRunMainWindow::writeSettings()
{
  for (auto& setting : settingsValues_)
  {
    setting->write();
  }
}
