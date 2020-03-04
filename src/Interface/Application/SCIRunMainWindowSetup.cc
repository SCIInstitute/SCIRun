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


#include <es-log/trace-log.h>
#include <QtGui>
#include <functional>
#include <boost/bind.hpp>
#include <boost/assign.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <Core/Utils/Legacy/MemoryUtil.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/DeveloperConsole.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/TagManagerWindow.h>
#include <Interface/Application/ShortcutsInterface.h>
#include <Interface/Application/TreeViewCollaborators.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/GuiCommands.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/NetworkExecutionProgressBar.h>
#include <Interface/Application/DialogErrorControl.h>
#include <Interface/Application/TriggeredEventsWindow.h>
#include <Interface/Application/MacroEditor.h>
#include <Interface/Modules/Base/RemembersFileDialogDirectory.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h> //TODO
#include <Interface/Application/ModuleWizard/ModuleWizard.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Controller/ProvenanceManager.h>
#include <Dataflow/Network/SimpleSourceSink.h>  //TODO: encapsulate!!!
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Core/Thread/Parallel.h>
#include <Core/Application/Version.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Core/Utils/CurrentFileName.h>

#ifdef BUILD_WITH_PYTHON
#include <Interface/Application/PythonConsoleWidget.h>
#include <Core/Python/PythonInterpreter.h>
#endif

#include <Dataflow/Serialization/Network/XMLSerializer.h>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;

void SCIRunMainWindow::createStandardToolbars()
{
  auto standardBar = addToolBar("File");
  WidgetStyleMixin::toolbarStyle(standardBar);
  standardBar->setObjectName("FileToolBar");
  standardBar->addAction(actionNew_);
  standardBar->addAction(actionLoad_);
  standardBar->addAction(actionSave_);
  standardBar->addAction(actionEnterWhatsThisMode_);
  standardBar->addAction(actionDragMode_);
  standardBar->addAction(actionSelectMode_);

  auto networkBar = addToolBar("Network");
  addNetworkActionsToBar(networkBar);

  actionZoomBestFit_->setDisabled(true);

  connect(actionFileBar_, SIGNAL(toggled(bool)), standardBar, SLOT(setVisible(bool)));
  connect(standardBar, SIGNAL(visibilityChanged(bool)), actionFileBar_, SLOT(setChecked(bool)));

  connect(actionNetworkBar_, SIGNAL(toggled(bool)), networkBar, SLOT(setVisible(bool)));
  connect(networkBar, SIGNAL(visibilityChanged(bool)), actionNetworkBar_, SLOT(setChecked(bool)));
}

void SCIRunMainWindow::addNetworkActionsToBar(QToolBar* toolbar) const
{
  WidgetStyleMixin::toolbarStyle(toolbar);
  toolbar->setObjectName("NetworkToolBar");

  toolbar->addAction(actionPinAllModuleUIs_);
  toolbar->addAction(actionRestoreAllModuleUIs_);
  toolbar->addAction(actionHideAllModuleUIs_);
  toolbar->addSeparator();
  toolbar->addAction(actionCenterNetworkViewer_);
  toolbar->addAction(actionZoomIn_);
  toolbar->addAction(actionZoomOut_);
  toolbar->addAction(actionResetNetworkZoom_);
}

void SCIRunMainWindow::createAdvancedToolbar()
{
  auto advancedBar = addToolBar("Advanced");
  WidgetStyleMixin::toolbarStyle(advancedBar);
  advancedBar->setObjectName("AdvancedToolBar");

  advancedBar->addAction(actionRunScript_);
  advancedBar->addAction(actionToggleMetadataLayer_);
  advancedBar->addAction(actionToggleTagLayer_);
  //TODO: turn back on after IBBM
  //advancedBar->addAction(actionMakeSubnetwork_);
  advancedBar->addActions(networkProgressBar_->advancedActions());

  connect(actionAdvancedBar_, SIGNAL(toggled(bool)), advancedBar, SLOT(setVisible(bool)));
  connect(advancedBar, SIGNAL(visibilityChanged(bool)), actionAdvancedBar_, SLOT(setChecked(bool)));

  advancedBar->setVisible(false);
}

void SCIRunMainWindow::createMacroToolbar()
{
  auto macroBar = addToolBar("Macro");
  WidgetStyleMixin::toolbarStyle(macroBar);
  macroBar->setObjectName("MacroToolbar");

  macroBar->addAction(actionMacroEditor_);
  macroBar->addAction(actionRunMacro1_);
  macroBar->addAction(actionRunMacro2_);
  macroBar->addAction(actionRunMacro3_);
  macroBar->addAction(actionRunMacro4_);
  macroBar->addAction(actionRunMacro5_);
  actionRunMacro1_->setProperty(MacroEditor::Index, 1);
  actionRunMacro2_->setProperty(MacroEditor::Index, 2);
  actionRunMacro3_->setProperty(MacroEditor::Index, 3);
  actionRunMacro4_->setProperty(MacroEditor::Index, 4);
  actionRunMacro5_->setProperty(MacroEditor::Index, 5);

  connect(actionMacroBar_, SIGNAL(toggled(bool)), macroBar, SLOT(setVisible(bool)));
  connect(macroBar, SIGNAL(visibilityChanged(bool)), actionMacroBar_, SLOT(setChecked(bool)));

  macroBar->setVisible(false);
}

void SCIRunMainWindow::createExecuteToolbar()
{
  auto executeBar = addToolBar(tr("&Execute"));
  executeBar->setObjectName("ExecuteToolBar");

  executeButton_ = new QToolButton;
  executeButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  executeButton_->addAction(actionExecuteAll_);
  executeButton_->setDefaultAction(actionExecuteAll_);
  executeBar->addWidget(executeButton_);

  networkProgressBar_.reset(new NetworkExecutionProgressBar(boost::make_shared<NetworkStatusImpl>(networkEditor_), this));
  executeBar->addActions(networkProgressBar_->mainActions());
  executeBar->setStyleSheet("QToolBar { background-color: rgb(66,66,69); border: 1px solid black; color: black }"
    "QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }"
    );
  executeBar->setAutoFillBackground(true);
  connect(actionExecuteBar_, SIGNAL(toggled(bool)), executeBar, SLOT(setVisible(bool)));
  connect(executeBar, SIGNAL(visibilityChanged(bool)), actionExecuteBar_, SLOT(setChecked(bool)));
}

void SCIRunMainWindow::postConstructionSignalHookup()
{
  connect(moduleSelectorTreeWidget_, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(filterDoubleClickedModuleSelectorItem(QTreeWidgetItem*)));
	moduleSelectorTreeWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(moduleSelectorTreeWidget_, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showModuleSelectorContextMenu(const QPoint&)));

  WidgetDisablingService::Instance().addNetworkEditor(networkEditor_);
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(executionStarted()), &WidgetDisablingService::Instance(), SLOT(disableInputWidgets()));
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(executionFinished(int)), &WidgetDisablingService::Instance(), SLOT(enableInputWidgets()));
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(executionFinished(int)), this, SLOT(changeExecuteActionIconToPlay()));
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(executionFinished(int)), this, SLOT(alertForNetworkCycles(int)));

	connect(networkEditor_, SIGNAL(disableWidgetDisabling()), &WidgetDisablingService::Instance(), SLOT(temporarilyDisableService()));
  connect(networkEditor_, SIGNAL(reenableWidgetDisabling()), &WidgetDisablingService::Instance(), SLOT(temporarilyEnableService()));

  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(moduleRemoved(const SCIRun::Dataflow::Networks::ModuleId&)),
    networkEditor_, SLOT(removeModuleWidget(const SCIRun::Dataflow::Networks::ModuleId&)));

  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(moduleAdded(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle, const SCIRun::Dataflow::Engine::ModuleCounter&)),
    commandConverter_.get(), SLOT(moduleAdded(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle)));
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(moduleRemoved(const SCIRun::Dataflow::Networks::ModuleId&)),
    commandConverter_.get(), SLOT(moduleRemoved(const SCIRun::Dataflow::Networks::ModuleId&)));
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)),
    commandConverter_.get(), SLOT(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)));
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(connectionRemoved(const SCIRun::Dataflow::Networks::ConnectionId&)),
    commandConverter_.get(), SLOT(connectionRemoved(const SCIRun::Dataflow::Networks::ConnectionId&)));
  connect(networkEditor_, SIGNAL(moduleMoved(const SCIRun::Dataflow::Networks::ModuleId&, double, double)),
    commandConverter_.get(), SLOT(moduleMoved(const SCIRun::Dataflow::Networks::ModuleId&, double, double)));
  connect(provenanceWindow_, SIGNAL(modifyingNetwork(bool)), commandConverter_.get(), SLOT(networkBeingModifiedByProvenanceManager(bool)));
  connect(networkEditor_, SIGNAL(newModule(const QString&, bool)), this, SLOT(addModuleToWindowList(const QString&, bool)));
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(moduleRemoved(const SCIRun::Dataflow::Networks::ModuleId&)),
    this, SLOT(removeModuleFromWindowList(const SCIRun::Dataflow::Networks::ModuleId&)));

  for (const auto& t : toolkitFiles_)
  {
    loadToolkitsFromFile(t);
  }

  startup_ = false;
}

void SCIRunMainWindow::setTipsAndWhatsThis()
{
  actionExecuteAll_->setStatusTip(tr("Execute all modules"));
  actionExecuteAll_->setWhatsThis(tr("Click this option to execute all modules in the current network editor."));
  actionNew_->setStatusTip(tr("New network"));
  actionNew_->setWhatsThis(tr("Click this option to start editing a blank network file."));
  actionSave_->setStatusTip(tr("Save network"));
  actionSave_->setWhatsThis(tr("Click this option to save the current network to disk."));
  actionLoad_->setStatusTip(tr("Load network"));
  actionLoad_->setWhatsThis(tr("Click this option to load a new network file from disk."));
  actionEnterWhatsThisMode_ = QWhatsThis::createAction(this);
  actionEnterWhatsThisMode_->setStatusTip(tr("Enter What's This? Mode"));
  actionEnterWhatsThisMode_->setShortcuts(QList<QKeySequence>() << tr("Ctrl+H") << tr("F1"));
  actionHideAllModuleUIs_->setWhatsThis("Hides all module UI windows.");
  actionRestoreAllModuleUIs_->setWhatsThis("Restores all module UI windows.");
  actionPinAllModuleUIs_->setWhatsThis("Pins all module UI windows to right side of main window.");
  //todo: zoom actions, etc
}

void SCIRunMainWindow::setupInputWidgets()
{
  // will be slicker in C++11
  using namespace boost::assign;
  std::vector<InputWidget> widgets;
  widgets += actionExecuteAll_,
    actionSave_,
    actionLoad_,
    actionSave_As_,
    actionNew_,
    actionDelete_,
    moduleSelectorTreeWidget_,
    actionRunScript_;

  WidgetDisablingService::Instance().addWidgets(widgets.begin(), widgets.end());
  WidgetDisablingService::Instance().addWidgets(recentFileActions_.begin(), recentFileActions_.end());
}

void SCIRunMainWindow::setupNetworkEditor()
{
  boost::shared_ptr<TreeViewModuleGetter> getter(new TreeViewModuleGetter(*moduleSelectorTreeWidget_));

  //TODO: this logger will crash on Windows when the console is closed. See #1250. Need to figure out a better way to manage scope/lifetime of Qt widgets passed to global singletons...
  boost::shared_ptr<TextEditAppender> moduleLog(new TextEditAppender(moduleLogTextBrowser_));
  ModuleLog::Instance().addCustomSink(moduleLog);
  GuiLog::Instance().setVerbose(LogSettings::Instance().verbose());

  defaultNotePositionGetter_.reset(new ComboBoxDefaultNotePositionGetter(prefsWindow_->defaultNotePositionComboBox_, prefsWindow_->defaultNoteSizeComboBox_));
  auto tagColorFunc = [this](int tag) { return tagManagerWindow_->tagColor(tag); };
  auto tagNameFunc = [this](int tag) { return tagManagerWindow_->tagName(tag); };
	auto preexecuteFunc = [this]() { preexecute(); };
  auto highResolutionExpandFactor = Core::Application::Instance().parameters()->developerParameters()->guiExpandFactor().get_value_or(1.0);
  {
    auto screen = QGuiApplication::screens()[0]->size();
    if (screen.height() * screen.width() > 4096000) // 2560x1600
      highResolutionExpandFactor = NetworkBoundaries::highDPIExpandFactorDefault;
  }
  networkEditor_ = new NetworkEditor({ getter, defaultNotePositionGetter_, dialogErrorControl_, preexecuteFunc,
    tagColorFunc, tagNameFunc, highResolutionExpandFactor, dockManager_ }, scrollAreaWidgetContents_);
  gridLayout_5->addWidget(networkEditor_, 0, 0, 1, 1);

  builder_->connectAll(networkEditor_);
  NetworkEditor::setConnectorFunc([this](NetworkEditor* ed) { builder_->connectAll(ed); });
  NetworkEditor::setMiniview(networkMiniviewGraphicsView_);

  networkMiniviewGraphicsView_->setScene(networkEditor_->scene());
  networkMiniviewGraphicsView_->scale(.1, .1);
  networkMiniviewGraphicsView_->setBackgroundBrush(QPixmap(":/general/Resources/SCIgrid-large.png"));
}

void SCIRunMainWindow::setActionIcons()
{
  actionNew_->setIcon(QPixmap(":/general/Resources/new/general/new.png"));
  actionLoad_->setIcon(QPixmap(":/general/Resources/new/general/folder.png"));
  actionSave_->setIcon(QPixmap(":/general/Resources/new/general/save.png"));
  actionRunScript_->setIcon(QPixmap(":/general/Resources/new/general/wand.png"));
  actionExecuteAll_->setIcon(QPixmap(":/general/Resources/new/general/run.png"));
  actionUndo_->setIcon(QPixmap(":/general/Resources/undo.png"));
  actionRedo_->setIcon(QPixmap(":/general/Resources/redo.png"));

  actionMacroEditor_->setIcon(QPixmap(":/general/Resources/script_play-512lime.png"));
  actionRunMacro1_->setIcon(QPixmap(":/general/Resources/flash1.png"));
  actionRunMacro2_->setIcon(QPixmap(":/general/Resources/flash2.png"));
  actionRunMacro3_->setIcon(QPixmap(":/general/Resources/flash3.png"));
  actionRunMacro4_->setIcon(QPixmap(":/general/Resources/flash4.png"));
  actionRunMacro5_->setIcon(QPixmap(":/general/Resources/flash5.png"));

  actionHideAllModuleUIs_->setIcon(QPixmap(":/general/Resources/new/general/hideAll.png"));
  actionPinAllModuleUIs_->setIcon(QPixmap(":/general/Resources/new/general/rightAll.png"));
  actionRestoreAllModuleUIs_->setIcon(QPixmap(":/general/Resources/new/general/showAll.png"));

  actionCenterNetworkViewer_->setIcon(QPixmap(":/general/Resources/align_center.png"));
  actionResetNetworkZoom_->setIcon(QPixmap(":/general/Resources/zoom_reset.png"));
  actionZoomIn_->setIcon(QPixmap(":/general/Resources/zoom_in.png"));
  actionZoomOut_->setIcon(QPixmap(":/general/Resources/zoom_out.png"));
  actionZoomBestFit_->setIcon(QPixmap(":/general/Resources/zoom_fit.png"));
  actionDragMode_->setIcon(QPixmap(":/general/Resources/cursor_hand_icon.png"));
  actionSelectMode_->setIcon(QPixmap(":/general/Resources/select.png"));

  actionToggleMetadataLayer_->setIcon(QPixmap(":/general/Resources/metadataLayer.png"));
  actionToggleTagLayer_->setIcon(QPixmap(":/general/Resources/tagLayer.png"));
  actionMakeSubnetwork_->setIcon(QPixmap(":/general/Resources/subnet3.png"));
  //IBBM disable
  actionMakeSubnetwork_->setDisabled(true);
}

void SCIRunMainWindow::makeFilterButtonMenu()
{
  auto updateFilterStatus = [this]() { filterModuleNamesInTreeView(moduleFilterLineEdit_->text()); };

  auto filterMenu = new QMenu(filterButton_);
  filterActionGroup_ = new QActionGroup(filterMenu);
  auto startsWithAction = new QAction("Starts with", filterButton_);
  connect(startsWithAction, &QAction::triggered, updateFilterStatus);
  startsWithAction->setCheckable(true);
  filterActionGroup_->addAction(startsWithAction);
  filterMenu->addAction(startsWithAction);

  auto wildcardAction = new QAction("Use wildcards", filterButton_);
  connect(wildcardAction, &QAction::triggered, updateFilterStatus);
  wildcardAction->setCheckable(true);
  filterActionGroup_->addAction(wildcardAction);
  filterMenu->addAction(wildcardAction);

  auto fuzzySearchAction = new QAction("Use fuzzy search", filterButton_);
  connect(fuzzySearchAction, &QAction::triggered, updateFilterStatus);
  fuzzySearchAction->setCheckable(true);
  filterActionGroup_->addAction(fuzzySearchAction);
  fuzzySearchAction->setChecked(true);
  filterMenu->addAction(fuzzySearchAction);

  auto filterUIAction = new QAction("Filter UI only", filterButton_);
  connect(filterUIAction, &QAction::triggered, updateFilterStatus);
  filterUIAction->setCheckable(true);
  filterActionGroup_->addAction(filterUIAction);
  filterUIAction->setChecked(false);
  filterMenu->addAction(filterUIAction);

  filterButton_->setMenu(filterMenu);
}

void SCIRunMainWindow::setupScriptedEventsWindow()
{
  triggeredEventsWindow_ = new TriggeredEventsWindow(this);
  connect(actionTriggeredEvents_, SIGNAL(toggled(bool)), triggeredEventsWindow_, SLOT(setVisible(bool)));
  connect(triggeredEventsWindow_, SIGNAL(visibilityChanged(bool)), actionTriggeredEvents_, SLOT(setChecked(bool)));
  triggeredEventsWindow_->hide();

  macroEditor_ = new MacroEditor(this);
  connect(actionMacroEditor_, SIGNAL(toggled(bool)), macroEditor_, SLOT(setVisible(bool)));
  connect(macroEditor_, SIGNAL(visibilityChanged(bool)), actionMacroEditor_, SLOT(setChecked(bool)));
  connect(macroEditor_, SIGNAL(macroButtonChanged(int, const QString&)), this, SLOT(updateMacroButton(int, const QString&)));
  macroEditor_->hide();
}

void SCIRunMainWindow::setupProvenanceWindow()
{
  ProvenanceManagerHandle provenanceManager(new ProvenanceManager<NetworkFileHandle>(networkEditor_));
  provenanceWindow_ = new ProvenanceWindow(provenanceManager, this);
  connect(actionProvenance_, SIGNAL(toggled(bool)), provenanceWindow_, SLOT(setVisible(bool)));
  connect(provenanceWindow_, SIGNAL(visibilityChanged(bool)), actionProvenance_, SLOT(setChecked(bool)));

  connect(actionUndo_, SIGNAL(triggered()), provenanceWindow_, SLOT(undo()));
  connect(actionRedo_, SIGNAL(triggered()), provenanceWindow_, SLOT(redo()));
  actionUndo_->setEnabled(false);
  actionRedo_->setEnabled(false);
  connect(provenanceWindow_, SIGNAL(undoStateChanged(bool)), actionUndo_, SLOT(setEnabled(bool)));
  connect(provenanceWindow_, SIGNAL(redoStateChanged(bool)), actionRedo_, SLOT(setEnabled(bool)));
  connect(provenanceWindow_, SIGNAL(networkModified()), networkEditor_, SLOT(updateViewport()));

  commandConverter_.reset(new GuiActionProvenanceConverter(networkEditor_));

  connect(commandConverter_.get(), SIGNAL(provenanceItemCreated(SCIRun::Dataflow::Engine::ProvenanceItemHandle)), provenanceWindow_, SLOT(addProvenanceItem(SCIRun::Dataflow::Engine::ProvenanceItemHandle)));

	provenanceWindow_->hide();
}

void SCIRunMainWindow::setupDevConsole()
{
  actionDevConsole_->setEnabled(false);
  #if 0 // disable dev console for now
  devConsole_ = new DeveloperConsole(this);
  connect(actionDevConsole_, SIGNAL(toggled(bool)), devConsole_, SLOT(setVisible(bool)));
  connect(devConsole_, SIGNAL(visibilityChanged(bool)), actionDevConsole_, SLOT(setChecked(bool)));

  devConsole_->setVisible(false);
  devConsole_->setFloating(true);
  addDockWidget(Qt::TopDockWidgetArea, devConsole_);

  actionDevConsole_->setShortcut(QKeySequence("`"));
  connect(devConsole_, SIGNAL(executorChosen(int)), this, SLOT(setExecutor(int)));
  connect(devConsole_, SIGNAL(globalPortCachingChanged(bool)), this, SLOT(setGlobalPortCaching(bool)));
  #endif
}

void SCIRunMainWindow::setupPreferencesWindow()
{
  prefsWindow_ = new PreferencesWindow(networkEditor_, [this]() { writeSettings(); }, this);

  connect(actionPreferences_, SIGNAL(triggered()), prefsWindow_, SLOT(show()));

  prefsWindow_->setVisible(false);
}

void SCIRunMainWindow::setupPythonConsole()
{
#ifdef BUILD_WITH_PYTHON
  pythonConsole_ = new PythonConsoleWidget(networkEditor_, this);
  connect(actionPythonConsole_, SIGNAL(toggled(bool)), pythonConsole_, SLOT(setVisible(bool)));
  actionPythonConsole_->setIcon(QPixmap(":/general/Resources/terminal.png"));
  connect(pythonConsole_, SIGNAL(visibilityChanged(bool)), actionPythonConsole_, SLOT(setChecked(bool)));
  pythonConsole_->setVisible(false);
  pythonConsole_->setFloating(true);
	pythonConsole_->setObjectName("PythonConsole");
  addDockWidget(Qt::TopDockWidgetArea, pythonConsole_);
#else
  actionPythonConsole_->setEnabled(false);
#endif
}

void SCIRunMainWindow::fillSavedSubnetworkMenu()
{
  if (savedSubnetworksNames_.size() != savedSubnetworksXml_.size())
  {
    qDebug() << "invalid subnet saved settings: sizes don't match" << savedSubnetworksNames_.size() << "," << savedSubnetworksXml_.size() << ',' << savedSubnetworksNames_.keys().size() << savedSubnetworksNames_.keys();
    return;
  }
  addFragmentsToMenu(savedSubnetworksNames_, savedSubnetworksXml_);
}

void SCIRunMainWindow::addFragmentsToMenu(const QMap<QString, QVariant>& names, const QMap<QString, QVariant>& xmls)
{
  auto savedSubnetworks = getSavedSubnetworksMenu(moduleSelectorTreeWidget_);
  auto keys = names.keys(); // don't inline this into the zip call! temporary containers don't work with zip.
  for (auto&& tup : zip(names, xmls, keys))
  {
    auto subnet = new QTreeWidgetItem();
    QVariant name, xml;
    QString key;
    boost::tie(name, xml, key) = tup;
    subnet->setText(0, name.toString());
    subnet->setData(0, clipboardKey, xml.toString());
		subnet->setForeground(0, CLIPBOARD_COLOR);
		savedSubnetworks->addChild(subnet);
		setupSubnetItem(subnet, false, key);
  }
}

void SCIRunMainWindow::fillModuleSelector()
{
  moduleSelectorTreeWidget_->clear();

  auto moduleDescs = networkEditor_->getNetworkEditorController()->getAllAvailableModuleDescriptions();

  addFavoriteMenu(moduleSelectorTreeWidget_);
	addSnippetMenu(moduleSelectorTreeWidget_);
	addSavedSubnetworkMenu(moduleSelectorTreeWidget_);
  fillSavedSubnetworkMenu();
	addClipboardHistoryMenu(moduleSelectorTreeWidget_);
  fillTreeWidget(moduleSelectorTreeWidget_, moduleDescs, favoriteModuleNames_);
  sortFavorites(moduleSelectorTreeWidget_);

  GrabNameAndSetFlags visitor;
  visitTree(moduleSelectorTreeWidget_, visitor);

  moduleSelectorTreeWidget_->expandAll();
  moduleSelectorTreeWidget_->resizeColumnToContents(0);
  moduleSelectorTreeWidget_->resizeColumnToContents(1);
  moduleSelectorTreeWidget_->sortByColumn(0, Qt::AscendingOrder);

  connect(moduleSelectorTreeWidget_, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(handleCheckedModuleEntry(QTreeWidgetItem*, int)));

  moduleSelectorTreeWidget_->setStyleSheet(
    "QTreeWidget::indicator:unchecked {image: url(:/general/Resources/faveNo.png);}"
    "QTreeWidget::indicator:checked {image: url(:/general/Resources/faveYes.png);}");
}

void SCIRunMainWindow::hideNonfunctioningWidgets()
{
	//TODO: make issues to implement these, as I don't want to forget they are there.
  QList<QAction*> nonfunctioningActions;
  nonfunctioningActions <<
    actionInsert_;
  QList<QMenu*> nonfunctioningMenus;
  QList<QWidget*> nonfunctioningWidgets;

  Q_FOREACH(QAction* a, nonfunctioningActions)
    a->setVisible(false);
  Q_FOREACH(QMenu* m, nonfunctioningMenus)
    m->menuAction()->setVisible(false);
  Q_FOREACH(QWidget* w, nonfunctioningWidgets)
    w->setVisible(false);
}

void SCIRunMainWindow::setupTagManagerWindow()
{
  tagManagerWindow_ = new TagManagerWindow(this);
  connect(actionTagManager_, SIGNAL(toggled(bool)), tagManagerWindow_, SLOT(setVisible(bool)));
  connect(tagManagerWindow_, SIGNAL(visibilityChanged(bool)), actionTagManager_, SLOT(setChecked(bool)));
  tagManagerWindow_->setVisible(false);
  addDockWidget(Qt::TopDockWidgetArea, tagManagerWindow_);
}

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define QT5_VERSION_STRING "+Qt" TOSTRING(QT5_VERSION)

void SCIRunMainWindow::setupVersionButton()
{
  auto qVersion = QString::fromStdString(VersionInfo::GIT_VERSION_TAG);
  qVersion += QT5_VERSION_STRING;
  versionButton_ = new QPushButton("Version: " + qVersion);
  versionButton_->setFlat(true);
  versionButton_->setToolTip("Click to copy version tag to clipboard");
  versionButton_->setStyleSheet("QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }");
  connect(versionButton_, SIGNAL(clicked()), this, SLOT(copyVersionToClipboard()));
  statusBar()->addPermanentWidget(versionButton_);
}
