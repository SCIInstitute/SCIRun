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
#include <boost/algorithm/string.hpp>
#include <Core/Utils/Legacy/MemoryUtil.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/TagManagerWindow.h>
#include <Interface/Application/ShortcutsInterface.h>
#include <Interface/Application/TreeViewCollaborators.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/GuiCommands.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/NetworkExecutionProgressBar.h>
#include <Interface/Application/TriggeredEventsWindow.h>
#include <Interface/Application/MacroEditor.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h> //TODO
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Controller/ProvenanceManager.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Core/Application/Version.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>

#ifdef BUILD_WITH_PYTHON
#include <Interface/Application/PythonConsoleWidget.h>
#endif

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

  connect(actionFileBar_, &QAction::toggled, standardBar, &QWidget::setVisible);
  connect(standardBar, &QToolBar::visibilityChanged, actionFileBar_, &QAction::setChecked);

  connect(actionNetworkBar_, &QAction::toggled, networkBar, &QWidget::setVisible);
  connect(networkBar, &QToolBar::visibilityChanged, actionNetworkBar_, &QAction::setChecked);
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

  connect(actionAdvancedBar_, &QAction::toggled, advancedBar, &QToolBar::setVisible);
  connect(advancedBar, &QToolBar::visibilityChanged, actionAdvancedBar_, &QAction::setChecked);

  advancedBar->setVisible(false);
}

void SCIRunMainWindow::createMacroToolbar()
{
  auto macroBar = addToolBar("Macro");
  WidgetStyleMixin::toolbarStyle(macroBar);
  macroBar->setObjectName("MacroToolbar");

  macroBar->addAction(macroEditor_->toggleViewAction());
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

  connect(actionMacroBar_, &QAction::toggled, macroBar, &QToolBar::setVisible);
  connect(macroBar, &QToolBar::visibilityChanged, actionMacroBar_, &QAction::setChecked);

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

  networkProgressBar_.reset(new NetworkExecutionProgressBar(makeShared<NetworkStatusImpl>(networkEditor_), this));
  executeBar->addActions(networkProgressBar_->mainActions());
  executeBar->setStyleSheet("QToolBar { background-color: rgb(66,66,69); border: 1px solid black; color: black }"
    "QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }"
    );
  executeBar->setAutoFillBackground(true);
  connect(actionExecuteBar_, &QAction::toggled, executeBar, &QWidget::setVisible);
  connect(executeBar, &QToolBar::visibilityChanged, actionExecuteBar_, &QAction::setChecked);
}

void SCIRunMainWindow::postConstructionSignalHookup()
{
  for (auto& tree : {moduleSelectorTreeWidget_, userModuleSelectorTreeWidget_})
  {
    connect(tree, &QTreeWidget::itemDoubleClicked, this, &SCIRunMainWindow::filterDoubleClickedModuleSelectorItem);
  	tree->setContextMenuPolicy(Qt::CustomContextMenu);
  	connect(tree, &QTreeWidget::customContextMenuRequested, this, &SCIRunMainWindow::showModuleSelectorContextMenu);
  }

  WidgetDisablingService::Instance().addNetworkEditor(networkEditor_);
  connect(networkEditor_->getNetworkEditorController().get(), &NetworkEditorControllerGuiProxy::executionStarted, &WidgetDisablingService::Instance(), &WidgetDisablingService::disableInputWidgets);
  connect(networkEditor_->getNetworkEditorController().get(), &NetworkEditorControllerGuiProxy::executionFinished, &WidgetDisablingService::Instance(), &WidgetDisablingService::enableInputWidgets);
  connect(networkEditor_->getNetworkEditorController().get(), &NetworkEditorControllerGuiProxy::executionFinished, this, &SCIRunMainWindow::changeExecuteActionIconToPlay);
  connect(networkEditor_->getNetworkEditorController().get(), &NetworkEditorControllerGuiProxy::executionFinished, this, &SCIRunMainWindow::alertForNetworkCycles);

	connect(networkEditor_, &NetworkEditor::disableWidgetDisabling, &WidgetDisablingService::Instance(), &WidgetDisablingService::temporarilyDisableService);
  connect(networkEditor_, &NetworkEditor::reenableWidgetDisabling, &WidgetDisablingService::Instance(), &WidgetDisablingService::temporarilyEnableService);

  connect(networkEditor_->getNetworkEditorController().get(), &NetworkEditorControllerGuiProxy::moduleRemoved,
    networkEditor_, &NetworkEditor::removeModuleWidget);

  connect(networkEditor_->getNetworkEditorController().get(), &NetworkEditorControllerGuiProxy::moduleAdded,
    commandConverter_.get(), &GuiActionProvenanceConverter::moduleAdded);
  connect(networkEditor_->getNetworkEditorController().get(), &NetworkEditorControllerGuiProxy::moduleRemoved,
    commandConverter_.get(), &GuiActionProvenanceConverter::moduleRemoved);
  connect(networkEditor_->getNetworkEditorController().get(), &NetworkEditorControllerGuiProxy::connectionAdded,
    commandConverter_.get(), &GuiActionProvenanceConverter::connectionAdded);
  connect(networkEditor_->getNetworkEditorController().get(), &NetworkEditorControllerGuiProxy::connectionRemoved,
    commandConverter_.get(), &GuiActionProvenanceConverter::connectionRemoved);
  connect(networkEditor_, &NetworkEditor::moduleMoved,
    commandConverter_.get(), &GuiActionProvenanceConverter::moduleMoved);
  connect(provenanceWindow_, &ProvenanceWindow::modifyingNetwork, commandConverter_.get(), &GuiActionProvenanceConverter::networkBeingModifiedByProvenanceManager);
  connect(networkEditor_, &NetworkEditor::newModule, this, &SCIRunMainWindow::addModuleToWindowList);
  connect(networkEditor_->getNetworkEditorController().get(), &NetworkEditorControllerGuiProxy::moduleRemoved,
    this, &SCIRunMainWindow::removeModuleFromWindowList);

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
  std::vector<InputWidget> widgets {
    actionExecuteAll_,
    actionSave_,
    actionLoad_,
    actionSave_As_,
    actionNew_,
    actionDelete_,
    moduleSelectorTreeWidget_,
    userModuleSelectorTreeWidget_,
    actionRunScript_ };

  WidgetDisablingService::Instance().addWidgets(widgets.begin(), widgets.end());
  WidgetDisablingService::Instance().addWidgets(recentFileActions_.begin(), recentFileActions_.end());
}

void SCIRunMainWindow::setupNetworkEditor()
{
  moduleSelection_.reset(new TreeViewActiveModuleItem);

  {
    connect(moduleSelectorTreeWidget_, &QTreeWidget::itemPressed, [this]() { moduleSelection_->setActiveTree(moduleSelectorTreeWidget_); });
    connect(userModuleSelectorTreeWidget_, &QTreeWidget::itemPressed, [this]() { moduleSelection_->setActiveTree(userModuleSelectorTreeWidget_); });
  }

  //TODO: this logger will crash on Windows when the console is closed. See #1250. Need to figure out a better way to manage scope/lifetime of Qt widgets passed to global singletons...
  SharedPointer<TextEditAppender> moduleLog(new TextEditAppender(moduleLogTextBrowser_));
  ModuleLog::Instance().addCustomSink(moduleLog);
  GuiLog::Instance().setVerbose(LogSettings::Instance().verbose());

  defaultNotePositionGetter_.reset(new ComboBoxDefaultNotePositionGetter(prefsWindow_->defaultNotePositionComboBox_, prefsWindow_->defaultNoteSizeComboBox_));
  auto tagColorFunc = [this](int tag) { return tagManagerWindow_->tagColor(tag); };
  auto tagNameFunc = [this](int tag) { return tagManagerWindow_->tagName(tag); };
	auto preexecuteFunc = [this]() { preexecute(); };
  auto highResolutionExpandFactor = Core::Application::Instance().parameters()->developerParameters()->guiExpandFactor().value_or(1.0);
  {
    auto screen = QGuiApplication::screens()[0]->size();
    if (screen.height() > 1600 && screen.height() * screen.width() > 4096000) // 2560x1600
      highResolutionExpandFactor = NetworkBoundaries::highDPIExpandFactorDefault;
  }
  networkEditor_ = new NetworkEditor({ moduleSelection_, defaultNotePositionGetter_, preexecuteFunc,
    tagColorFunc, tagNameFunc, highResolutionExpandFactor, nullptr }, scrollAreaWidgetContents_);
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

  macroEditor_->toggleViewAction()->setIcon(QPixmap(":/general/Resources/script_play-512lime.png"));
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
  triggeredEventsWindow_->hide();

  macroEditor_ = new MacroEditor(this);
  connect(macroEditor_, &MacroEditor::macroButtonChanged, this, &SCIRunMainWindow::updateMacroButton);
  macroEditor_->hide();
}

void SCIRunMainWindow::setupProvenanceWindow()
{
  ProvenanceManagerHandle provenanceManager(new ProvenanceManager<NetworkFileHandle>(networkEditor_));
  provenanceWindow_ = new ProvenanceWindow(provenanceManager, this);

  connect(actionUndo_, &QAction::triggered, provenanceWindow_, &ProvenanceWindow::undo);
  connect(actionRedo_, &QAction::triggered, provenanceWindow_, &ProvenanceWindow::redo);
  actionUndo_->setEnabled(false);
  actionRedo_->setEnabled(false);
  connect(provenanceWindow_, &ProvenanceWindow::undoStateChanged, actionUndo_, &QAction::setEnabled);
  connect(provenanceWindow_, &ProvenanceWindow::redoStateChanged, actionRedo_, &QAction::setEnabled);
  connect(provenanceWindow_, &ProvenanceWindow::networkModified, networkEditor_, &NetworkEditor::updateViewport);

  commandConverter_.reset(new GuiActionProvenanceConverter(networkEditor_));

  connect(commandConverter_.get(), &GuiActionProvenanceConverter::provenanceItemCreated, provenanceWindow_, &ProvenanceWindow::addProvenanceItem);

	provenanceWindow_->hide();
}

void SCIRunMainWindow::setupDevConsole()
{
  actionDevConsole_->setEnabled(false);
  #if 0 // disable dev console for now
  devConsole_ = new DeveloperConsole(this);
  connect(actionDevConsole_, &QAction::toggled, devConsole_, &DeveloperConsole::setVisible);
  connect(devConsole_, &DeveloperConsole::visibilityChanged, actionDevConsole_, &QAction::setChecked);

  devConsole_->setVisible(false);
  devConsole_->setFloating(true);
  addDockWidget(Qt::TopDockWidgetArea, devConsole_);

  actionDevConsole_->setShortcut(QKeySequence("`"));
  connect(devConsole_, &DeveloperConsole::executorChosen, this, &SCIRunMainWindow::setExecutor);
  connect(devConsole_, &DeveloperConsole::globalPortCachingChanged, this, &SCIRunMainWindow::setGlobalPortCaching);
  #endif
}

void SCIRunMainWindow::setupPreferencesWindow()
{
  prefsWindow_ = new PreferencesWindow(networkEditor_, [this]() { writeSettings(); }, this);

  connect(actionPreferences_, &QAction::triggered, prefsWindow_, &PreferencesWindow::show);

  prefsWindow_->setVisible(false);
}

void SCIRunMainWindow::setupPythonConsole()
{
#ifdef BUILD_WITH_PYTHON
  pythonConsole_ = new PythonConsoleWidget(networkEditor_, this);
  pythonConsole_->toggleViewAction()->setIcon(QPixmap(":/general/Resources/terminal.png"));
  pythonConsole_->setVisible(false);
  pythonConsole_->setFloating(true);
	pythonConsole_->setObjectName("PythonConsole");
  addDockWidget(Qt::TopDockWidgetArea, pythonConsole_);
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
  auto savedSubnetworks = getSavedSubnetworksMenu();
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

  addFavoriteMenu(userModuleSelectorTreeWidget_);
  addRecentMenu(userModuleSelectorTreeWidget_);
  addFrequentMenu(userModuleSelectorTreeWidget_);
	addSnippetMenu(moduleSelectorTreeWidget_);
	addSavedSubnetworkMenu(userModuleSelectorTreeWidget_);
  fillSavedSubnetworkMenu();
	addClipboardHistoryMenu(userModuleSelectorTreeWidget_);
  fillTreeWidget(moduleSelectorTreeWidget_, moduleDescs, favoriteModuleNames_);
  sortFavorites();

  for (auto& tree : {moduleSelectorTreeWidget_, userModuleSelectorTreeWidget_})
  {
    GrabNameAndSetFlags visitor;
    visitTree(tree, visitor);
    tree->expandAll();
    tree->resizeColumnToContents(0);
    tree->resizeColumnToContents(1);
    tree->sortByColumn(0, Qt::AscendingOrder);

    connect(tree, &QTreeWidget::itemChanged, this, &SCIRunMainWindow::handleCheckedModuleEntry);

    tree->setStyleSheet(
      "QTreeWidget::indicator:unchecked {image: url(:/general/Resources/faveNo.png);}"
      "QTreeWidget::indicator:checked {image: url(:/general/Resources/faveYes.png);}");
  }
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
  connect(actionTagManager_, &QAction::toggled, tagManagerWindow_, &TagManagerWindow::setVisible);
  connect(tagManagerWindow_, &TagManagerWindow::visibilityChanged, actionTagManager_, &QAction::setChecked);
  tagManagerWindow_->setVisible(false);
  addDockWidget(Qt::TopDockWidgetArea, tagManagerWindow_);
}

#ifdef __APPLE__

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/sysctl.h>

std::string show_cpu_info()
{
  char buffer[1024];
  size_t size=sizeof(buffer);
  if (sysctlbyname("machdep.cpu.brand_string", &buffer, &size, NULL, 0) < 0) {
      perror("sysctl");
  }
  std::ostringstream ostr;
  ostr << buffer;
  return ostr.str();
}

bool isAppleSilicon()
{
  auto cpu = show_cpu_info();
  return cpu.find("Apple M") != std::string::npos;
}

int processIsTranslated()
{
   int ret = 0;
   size_t size = sizeof(ret);
   if (sysctlbyname("sysctl.proc_translated", &ret, &size, NULL, 0) == -1)
   {
      if (errno == ENOENT)
         return 0;
      return -1;
   }
   return ret;
}

bool nativeAppleSiliconProcess()
{
  return 0 == processIsTranslated();
}

#endif

void SCIRunMainWindow::setupVersionButton()
{
  auto qver = QString::fromStdString(VersionInfo::GIT_VERSION_TAG) + "+Qt";
  qver += qVersion();
  #ifdef __APPLE__
  auto appleChip = isAppleSilicon();
  qver += appleChip ? "(a64)" : "(x86)";
  if (appleChip)
    qver += nativeAppleSiliconProcess() ? "[native]" : "[translated]";
  #endif
  versionButton_ = new QPushButton("Version: " + qver);
  versionButton_->setFlat(true);
  versionButton_->setToolTip("Click to copy version tag to clipboard");
  versionButton_->setStyleSheet("QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }");
  connect(versionButton_, &QPushButton::clicked, this, &SCIRunMainWindow::copyVersionToClipboard);
  statusBar()->addPermanentWidget(versionButton_);
}
