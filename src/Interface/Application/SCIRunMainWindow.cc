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
#include <algorithm>
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
#include <Interface/Application/ModuleProxyWidget.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/NetworkExecutionProgressBar.h>
#include <Interface/Application/DialogErrorControl.h>
#include <Interface/Modules/Base/RemembersFileDialogDirectory.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h> //TODO
#include <Interface/Application/ModuleWizard/ModuleWizard.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h> //DOH! see TODO in setController
#include <Dataflow/Engine/Controller/ProvenanceManager.h>
#include <Dataflow/Network/SimpleSourceSink.h>  //TODO: encapsulate!!!
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Core/Application/Version.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Core/Command/CommandFactory.h>

#ifdef BUILD_WITH_PYTHON
#include <Interface/Application/PythonConsoleWidget.h>
#include <Core/Python/PythonInterpreter.h>
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

static const char* ToolkitIconURL = "ToolkitIconURL";
static const char* ToolkitURL = "ToolkitURL";
static const char* ToolkitFilename = "ToolkitFilename";

SCIRunMainWindow::SCIRunMainWindow() : shortcuts_(nullptr), firstTimePythonShown_(true), returnCode_(0), quitAfterExecute_(false)
{
	setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	if (newInterface())
		setStyleSheet(
		"background-color: rgb(66,66,69);"
		"color: white;"
		"selection-color: yellow;"
		"selection-background-color: blue;"//336699 lighter blue
		"QToolBar {        background-color: rgb(66,66,69); border: 1px solid black; color: black;     }"
		"QProgressBar {        background-color: rgb(66,66,69); border: 0px solid black; color: black  ;   }"
		"QDockWidget {background: rgb(66,66,69); background-color: rgb(66,66,69); }"
		"QPushButton {"
		"  border: 2px solid #8f8f91;"
		"  border - radius: 6px;"
		"  background - color: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 : 1,"
		"  stop : 0 #f6f7fa, stop: 1 #dadbde);"
		"  min - width: 80px;"
		"}"
		"QPushButton:pressed{"
		"background - color: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 : 1,"
		"stop : 0 #dadbde, stop: 1 #f6f7fa);"
		"}"
		"QPushButton:flat{"
		"          border: none; /* no border for a flat push button */"
		"}"
		"QPushButton:default {"
		"border - color: navy; /* make the default button prominent */"
		"}"
		);
	menubar_->setStyleSheet("QMenuBar::item::selected{background-color : rgb(66, 66, 69); } QMenuBar::item::!selected{ background-color : rgb(66, 66, 69); } ");
	dialogErrorControl_.reset(new DialogErrorControl(this));
  setupTagManagerWindow();
  tagManagerWindow_->hide();
  setupPreferencesWindow();
  setupNetworkEditor();

  setTipsAndWhatsThis();

  connect(actionExecute_All_, SIGNAL(triggered()), this, SLOT(executeAll()));
  connect(actionNew_, SIGNAL(triggered()), this, SLOT(newNetwork()));
  connect(networkEditor_, SIGNAL(modified()), this, SLOT(networkModified()));

  gridLayout_5->addWidget(networkEditor_, 0, 0, 1, 1);

  QWidgetAction* moduleSearchAction = new QWidgetAction(this);
  moduleSearchAction->setDefaultWidget(new QLineEdit(this));

#if 0
  {
    //TODO!!!!
    moduleSearchAction->setVisible(true);

    QToolBar* f = addToolBar(tr("&Search"));
    f->setObjectName("SearchToolBar");

    QWidgetAction* showModuleLabel = new QWidgetAction(this);
    showModuleLabel->setDefaultWidget(new QLabel("Module Search:", this));
    showModuleLabel->setVisible(true);

    f->addAction(showModuleLabel);
    f->addAction(moduleSearchAction);
  }
#endif

  setActionIcons();

  QToolBar* standardBar = addToolBar("Standard");
	WidgetStyleMixin::toolbarStyle(standardBar);
  standardBar->setObjectName("StandardToolBar");
  standardBar->addAction(actionNew_);
  standardBar->addAction(actionLoad_);
  standardBar->addAction(actionSave_);
  standardBar->addAction(actionRunScript_);
  standardBar->addAction(actionEnterWhatsThisMode_);
  standardBar->addSeparator();
  standardBar->addAction(actionPinAllModuleUIs_);
  standardBar->addAction(actionRestoreAllModuleUIs_);
  standardBar->addAction(actionHideAllModuleUIs_);
  standardBar->addSeparator();
  standardBar->addAction(actionCenterNetworkViewer_);
  standardBar->addAction(actionZoomIn_);
  standardBar->addAction(actionZoomOut_);
  //standardBar->addAction(actionZoomBestFit_);
  actionZoomBestFit_->setDisabled(true);
  standardBar->addAction(actionResetNetworkZoom_);
  standardBar->addAction(actionDragMode_);
  standardBar->addAction(actionSelectMode_);
  standardBar->addAction(actionToggleMetadataLayer_);
  standardBar->addAction(actionToggleTagLayer_);
  //setUnifiedTitleAndToolBarOnMac(true);

  QToolBar* executeBar = addToolBar(tr("&Execute"));
  executeBar->setObjectName("ExecuteToolBar");

	executeButton_ = new QToolButton;
	executeButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	executeButton_->addAction(actionExecute_All_);
	executeButton_->setDefaultAction(actionExecute_All_);
	executeBar->addWidget(executeButton_);

  networkProgressBar_.reset(new NetworkExecutionProgressBar(this));
  executeBar->addActions(networkProgressBar_->actions());
  executeBar->setStyleSheet("QToolBar { background-color: rgb(66,66,69); border: 1px solid black; color: black }"
		"QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }"
		);
  executeBar->setAutoFillBackground(true);

  networkEditor_->addAction(actionExecute_All_);
  auto sep = new QAction(this);
  sep->setSeparator(true);
  networkEditor_->addAction(sep);
  networkEditor_->addAction(actionCut_);
  networkEditor_->addAction(actionCopy_);
  networkEditor_->addAction(actionPaste_);
	sep = new QAction(this);
  sep->setSeparator(true);
	networkEditor_->addAction(sep);
	networkEditor_->addAction(actionResetNetworkZoom_);
	networkEditor_->addAction(actionDragMode_);
  networkEditor_->addAction(actionSelectMode_);
  networkEditor_->addAction(actionToggleMetadataLayer_);
  networkEditor_->addAction(actionToggleTagLayer_);

  setContextMenuPolicy(Qt::NoContextMenu);

  scrollArea_->viewport()->setBackgroundRole(QPalette::Dark);
  scrollArea_->viewport()->setAutoFillBackground(true);
  scrollArea_->setStyleSheet(styleSheet());

  connect(actionSave_As_, SIGNAL(triggered()), this, SLOT(saveNetworkAs()));
  connect(actionSave_, SIGNAL(triggered()), this, SLOT(saveNetwork()));
  connect(actionLoad_, SIGNAL(triggered()), this, SLOT(loadNetwork()));
  connect(actionImportNetwork_, SIGNAL(triggered()), this, SLOT(importLegacyNetwork()));
  connect(actionQuit_, SIGNAL(triggered()), this, SLOT(close()));
  connect(actionRunScript_, SIGNAL(triggered()), this, SLOT(runScript()));
  connect(actionSelectAll_, SIGNAL(triggered()), networkEditor_, SLOT(selectAll()));
  actionQuit_->setShortcut(QKeySequence::Quit);
  connect(actionDelete_, SIGNAL(triggered()), networkEditor_, SLOT(del()));
  actionDelete_->setShortcuts(QList<QKeySequence>() << QKeySequence::Delete << Qt::Key_Backspace);
	connect(actionCleanUpNetwork_, SIGNAL(triggered()), networkEditor_, SLOT(cleanUpNetwork()));
	connect(actionRunNewModuleWizard_, SIGNAL(triggered()), this, SLOT(runNewModuleWizard()));
	actionRunNewModuleWizard_->setDisabled(true);

  connect(actionAbout_, SIGNAL(triggered()), this, SLOT(displayAcknowledgement()));
  connect(actionPinAllModuleUIs_, SIGNAL(triggered()), networkEditor_, SLOT(pinAllModuleUIs()));
  connect(actionRestoreAllModuleUIs_, SIGNAL(triggered()), networkEditor_, SLOT(restoreAllModuleUIs()));
  connect(actionHideAllModuleUIs_, SIGNAL(triggered()), networkEditor_, SLOT(hideAllModuleUIs()));

  connect(actionReset_Window_Layout, SIGNAL(triggered()), this, SLOT(resetWindowLayout()));

#ifndef BUILD_WITH_PYTHON
  actionRunScript_->setEnabled(false);
#endif

  for (int i = 0; i < MaxRecentFiles; ++i)
  {
    recentFileActions_.push_back(new QAction(this));
    recentFileActions_[i]->setVisible(false);
    recentNetworksMenu_->addAction(recentFileActions_[i]);
    connect(recentFileActions_[i], SIGNAL(triggered()), this, SLOT(loadRecentNetwork()));
  }

  setupProvenanceWindow();
  provenanceWindow_->hide();
  setupDevConsole();
  setupPythonConsole();

  connect(prefsWindow_->defaultNotePositionComboBox_, SIGNAL(activated(int)), this, SLOT(readDefaultNotePosition(int)));
  connect(this, SIGNAL(defaultNotePositionChanged(NotePosition)), networkEditor_, SIGNAL(defaultNotePositionChanged(NotePosition)));

  connect(prefsWindow_->largeModuleSizeToolButton_, SIGNAL(clicked()), this, SLOT(makeModulesLargeSize()));
  connect(prefsWindow_->smallModuleSizeToolButton_, SIGNAL(clicked()), this, SLOT(makeModulesSmallSize()));

  connect(prefsWindow_->cubicPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesCubicBezier()));
  connect(prefsWindow_->manhattanPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesManhattan()));
  connect(prefsWindow_->euclideanPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesEuclidean()));
  //TODO: will be a user or network setting
  makePipesEuclidean();

  connect(this, SIGNAL(moduleItemDoubleClicked()), networkEditor_, SLOT(addModuleViaDoubleClickedTreeItem()));
  connect(moduleFilterLineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(filterModuleNamesInTreeView(const QString&)));

#if 0 //TODO: decide on modifiable background color
  connect(chooseBackgroundColorButton_, SIGNAL(clicked()), this, SLOT(chooseBackgroundColor()));
  connect(resetBackgroundColorButton_, SIGNAL(clicked()), this, SLOT(resetBackgroundColor()));
#endif
  prefsWindow_->chooseBackgroundColorButton_->setHidden(true);
  prefsWindow_->resetBackgroundColorButton_->setHidden(true);
  prefsWindow_->backgroundColorLabel_->setHidden(true);

  connect(prefsWindow_->modulesSnapToCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(modulesSnapToChanged()));
  connect(prefsWindow_->modulesSnapToCheckBox_, SIGNAL(stateChanged(int)), networkEditor_, SIGNAL(snapToModules()));

  connect(prefsWindow_->portSizeEffectsCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(highlightPortsChanged()));
  connect(prefsWindow_->portSizeEffectsCheckBox_, SIGNAL(stateChanged(int)), networkEditor_, SIGNAL(highlightPorts(int)));

  connect(prefsWindow_->dockableModulesCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(adjustModuleDock(int)));

  makeFilterButtonMenu();

  if (newInterface())
    networkEditor_->setBackgroundBrush(QPixmap(":/general/Resources/SCIgrid-small.png"));

  connect(prefsWindow_->scirunDataPushButton_, SIGNAL(clicked()), this, SLOT(setDataDirectoryFromGUI()));
  connect(prefsWindow_->addToPathButton_, SIGNAL(clicked()), this, SLOT(addToPathFromGUI()));
  connect(actionFilter_modules_, SIGNAL(triggered()), this, SLOT(setFocusOnFilterLine()));
  connect(actionAddModule_, SIGNAL(triggered()), this, SLOT(addModuleKeyboardAction()));
  actionAddModule_->setVisible(false);
  connect(actionSelectModule_, SIGNAL(triggered()), this, SLOT(selectModuleKeyboardAction()));
  actionSelectModule_->setVisible(false);

  connect(actionSelectMode_, SIGNAL(toggled(bool)), this, SLOT(setSelectMode(bool)));
  connect(actionDragMode_, SIGNAL(toggled(bool)), this, SLOT(setDragMode(bool)));

	connect(actionToggleTagLayer_, SIGNAL(toggled(bool)), this, SLOT(toggleTagLayer(bool)));
  connect(actionToggleMetadataLayer_, SIGNAL(toggled(bool)), this, SLOT(toggleMetadataLayer(bool)));

  connect(actionResetNetworkZoom_, SIGNAL(triggered()), this, SLOT(zoomNetwork()));
  connect(actionZoomIn_, SIGNAL(triggered()), this, SLOT(zoomNetwork()));
  connect(actionZoomOut_, SIGNAL(triggered()), this, SLOT(zoomNetwork()));
  connect(actionZoomBestFit_, SIGNAL(triggered()), this, SLOT(zoomNetwork()));
  connect(networkEditor_, SIGNAL(zoomLevelChanged(int)), this, SLOT(showZoomStatusMessage(int)));
  connect(actionCenterNetworkViewer_, SIGNAL(triggered()), networkEditor_, SLOT(centerView()));

	connect(actionCut_, SIGNAL(triggered()), networkEditor_, SLOT(cut()));
	connect(actionCopy_, SIGNAL(triggered()), networkEditor_, SLOT(copy()));
	connect(actionPaste_, SIGNAL(triggered()), networkEditor_, SLOT(paste()));
  actionCut_->setIcon(QPixmap(":/general/Resources/cut.png"));
  actionCopy_->setIcon(QPixmap(":/general/Resources/copy.png"));
  actionPaste_->setIcon(QPixmap(":/general/Resources/paste.png"));

  connect(actionKeyboardShortcuts_, SIGNAL(triggered()), this, SLOT(showKeyboardShortcutsDialog()));

  //TODO: store in xml file, add to app resources
	connect(actionForwardInverse_, SIGNAL(triggered()), this, SLOT(toolkitDownload()));
  actionForwardInverse_->setProperty(ToolkitIconURL, QString("http://www.sci.utah.edu/images/software/forward-inverse/forward-inverse-mod.png"));
  actionForwardInverse_->setProperty(ToolkitURL, QString("http://sci.utah.edu/devbuilds/scirun5/toolkits/FwdInvToolkit_v1.zip"));
  actionForwardInverse_->setProperty(ToolkitFilename, QString("FwdInvToolkit_v1.zip"));
  actionForwardInverse_->setIcon(QPixmap(":/general/Resources/download.png"));

	connect(actionBrainStimulator_, SIGNAL(triggered()), this, SLOT(toolkitDownload()));
  actionBrainStimulator_->setProperty(ToolkitIconURL, QString("http://www.sci.utah.edu/images/software/BrainStimulator/brain-stimulator-mod.png"));
  actionBrainStimulator_->setProperty(ToolkitURL, QString("http://sci.utah.edu/devbuilds/scirun5/toolkits/BrainStimulator_v1.2.zip"));
  actionBrainStimulator_->setProperty(ToolkitFilename, QString("BrainStimulator_v1.2.zip"));
  actionBrainStimulator_->setIcon(QPixmap(":/general/Resources/download.png"));

  connect(networkEditor_, SIGNAL(networkExecuted()), networkProgressBar_.get(), SLOT(resetModulesDone()));
  connect(networkEditor_->moduleEventProxy().get(), SIGNAL(moduleExecuteEnd(const std::string&)), networkProgressBar_.get(), SLOT(incrementModulesDone()));

  connect(networkEditor_, SIGNAL(networkExecuted()), dialogErrorControl_.get(), SLOT(resetCounter()));

  connect(networkEditor_, SIGNAL(networkExecuted()), this, SLOT(changeExecuteActionIconToStop()));
  connect(prefsWindow_->actionTextIconCheckBox_, SIGNAL(clicked()), this, SLOT(adjustExecuteButtonAppearance()));
  prefsWindow_->actionTextIconCheckBox_->setCheckState(Qt::PartiallyChecked);
  adjustExecuteButtonAppearance();

  connect(openLogFolderButton_, SIGNAL(clicked()), this, SLOT(openLogFolder()));

  setupInputWidgets();

  logTextBrowser_->append("Hello! Welcome to SCIRun 5.");
  readSettings();

  setCurrentFile("");

  actionConfiguration_->setChecked(!configurationDockWidget_->isHidden());
  actionModule_Selector->setChecked(!moduleSelectorDockWidget_->isHidden());
  actionProvenance_->setChecked(!provenanceWindow_->isHidden());
  actionTagManager_->setChecked(!tagManagerWindow_->isHidden());

	moduleSelectorDockWidget_->setStyleSheet("QDockWidget {background: rgb(66,66,69); background-color: rgb(66,66,69) }"
		"QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }"
		"QHeaderView::section { background: rgb(66,66,69);} "
		);

  hideNonfunctioningWidgets();

  connect(moduleSelectorDockWidget_, SIGNAL(topLevelChanged(bool)), this, SLOT(updateDockWidgetProperties(bool)));

  statusBar()->addPermanentWidget(new QLabel("Version: " + QString::fromStdString(VersionInfo::GIT_VERSION_TAG)));

	WidgetStyleMixin::tabStyle(optionsTabWidget_);
}

void SCIRunMainWindow::initialize()
{
  postConstructionSignalHookup();

  fillModuleSelector();

  executeCommandLineRequests();
}

void SCIRunMainWindow::postConstructionSignalHookup()
{
  connect(moduleSelectorTreeWidget_, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(filterDoubleClickedModuleSelectorItem(QTreeWidgetItem*)));

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
}

void SCIRunMainWindow::setTipsAndWhatsThis()
{
  actionExecute_All_->setStatusTip(tr("Execute all modules"));
  actionExecute_All_->setWhatsThis(tr("Click this option to execute all modules in the current network editor."));
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
  widgets += actionExecute_All_,
    actionSave_,
    actionLoad_,
    actionSave_As_,
    actionNew_,
    actionDelete_,
    moduleSelectorTreeWidget_,
    actionRunScript_;
#ifdef BUILD_WITH_PYTHON
  widgets += pythonConsole_;
#endif

  WidgetDisablingService::Instance().addWidgets(widgets.begin(), widgets.end());
  WidgetDisablingService::Instance().addWidgets(recentFileActions_.begin(), recentFileActions_.end());
}

SCIRunMainWindow* SCIRunMainWindow::instance_ = nullptr;

SCIRunMainWindow* SCIRunMainWindow::Instance()
{
  if (!instance_)
  {
    instance_ = new SCIRunMainWindow;
  }
  return instance_;
}

SCIRunMainWindow::~SCIRunMainWindow()
{
  GuiLogger::setInstance(nullptr);
  Log::get().clearAppenders();
  Log::get("Modules").clearAppenders();
  networkEditor_->disconnect();
  networkEditor_->setNetworkEditorController(nullptr);
  networkEditor_->clear();
  Application::Instance().shutdown();
}

void SCIRunMainWindow::setController(SCIRun::Dataflow::Engine::NetworkEditorControllerHandle controller)
{
  boost::shared_ptr<NetworkEditorControllerGuiProxy> controllerProxy(new NetworkEditorControllerGuiProxy(controller));
  networkEditor_->setNetworkEditorController(controllerProxy);
  //TODO: need better way to wire this up
  controller->setSerializationManager(networkEditor_);
}

void SCIRunMainWindow::setupNetworkEditor()
{
  boost::shared_ptr<TreeViewModuleGetter> getter(new TreeViewModuleGetter(*moduleSelectorTreeWidget_));
	const bool regression = Application::Instance().parameters()->isRegressionMode();
  boost::shared_ptr<TextEditAppender> logger(new TextEditAppender(logTextBrowser_, regression));
  GuiLogger::setInstance(logger);
  Log::get().addCustomAppender(logger);
  //TODO: this logger will crash on Windows when the console is closed. See #1250. Need to figure out a better way to manage scope/lifetime of Qt widgets passed to global singletons...
  //boost::shared_ptr<TextEditAppender> moduleLog(new TextEditAppender(moduleLogTextBrowser_));
  //Log::get("Modules").addCustomAppender(moduleLog);
  defaultNotePositionGetter_.reset(new ComboBoxDefaultNotePositionGetter(*prefsWindow_->defaultNotePositionComboBox_));
  auto tagColorFunc = [this](int tag) { return tagManagerWindow_->tagColor(tag); };
  networkEditor_ = new NetworkEditor(getter, defaultNotePositionGetter_, dialogErrorControl_, tagColorFunc, scrollAreaWidgetContents_);
  networkEditor_->setObjectName(QString::fromUtf8("networkEditor_"));
  //networkEditor_->setContextMenuPolicy(Qt::ActionsContextMenu);
  networkEditor_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->verticalScrollBar()->setValue(0);
  networkEditor_->horizontalScrollBar()->setValue(0);
}

void SCIRunMainWindow::executeCommandLineRequests()
{
  SCIRun::Core::Application::Instance().executeCommandLineRequests();
}

void SCIRunMainWindow::executeAll()
{
  if (Core::Preferences::Instance().saveBeforeExecute && !Application::Instance().parameters()->isRegressionMode())
  {
    saveNetwork();
  }

	if (Application::Instance().parameters()->isRegressionMode())
	{
		auto timeout = Application::Instance().parameters()->regressionTimeoutSeconds();
		QTimer::singleShot(1000 * *timeout, this, SLOT(networkTimedOut()));
	}

  networkEditor_->executeAll();
}

void SCIRunMainWindow::setupQuitAfterExecute()
{
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(executionFinished(int)), this, SLOT(exitApplication(int)));
  quitAfterExecute_ = true;
}

void SCIRunMainWindow::exitApplication(int code)
{
  close();
  returnCode_ = code;
  qApp->exit(code);
}

void SCIRunMainWindow::quit()
{
  exitApplication(0);
}

void SCIRunMainWindow::networkTimedOut()
{
	exitApplication(2);
}

void SCIRunMainWindow::saveNetwork()
{
  if (currentFile_.isEmpty())
    saveNetworkAs();
  else
    saveNetworkFile(currentFile_);
}

void SCIRunMainWindow::saveNetworkAs()
{
  QString filename = QFileDialog::getSaveFileName(this, "Save Network...", latestNetworkDirectory_.path(), "*.srn5");
  if (!filename.isEmpty())
    saveNetworkFile(filename);
}

void SCIRunMainWindow::saveNetworkFile(const QString& fileName)
{
  NetworkSaveCommand save;
  save.set(Variables::Filename, fileName.toStdString());
  save.execute();
}

void SCIRunMainWindow::loadNetwork()
{
  if (okToContinue())
  {
    QString filename = QFileDialog::getOpenFileName(this, "Load Network...", latestNetworkDirectory_.path(), "*.srn5");
    loadNetworkFile(filename);
  }
}

bool SCIRunMainWindow::loadNetworkFile(const QString& filename)
{
  if (!filename.isEmpty())
  {
    FileOpenCommand command(filename.toStdString(), networkEditor_);
    if (command.execute())
    {
      setCurrentFile(filename);
      statusBar()->showMessage(tr("File loaded: ") + filename, 2000);
      networkProgressBar_->updateTotalModules(networkEditor_->numModules());
      provenanceWindow_->clear();
      provenanceWindow_->showFile(command.file_);
			networkEditor_->viewport()->update();
      return true;
    }
    else
    {
      if (Core::Application::Instance().parameters()->isRegressionMode())
        exit(7);
      //TODO: set error code to non-0 so regression tests fail!
      // probably want to control this with a --regression flag.
    }
  }
  return false;
}

void SCIRunMainWindow::importLegacyNetwork()
{
  if (okToContinue())
  {
    QString filename = QFileDialog::getOpenFileName(this, "Import Old Network...", latestNetworkDirectory_.path(), "*.srn");
    importLegacyNetworkFile(filename);
  }
}

bool SCIRunMainWindow::importLegacyNetworkFile(const QString& filename)
{
	bool success = false;
  if (!filename.isEmpty())
  {
    FileImportCommand command(filename.toStdString(), networkEditor_);
    if (command.execute())
    {
      statusBar()->showMessage(tr("File imported: ") + filename, 2000);
      networkProgressBar_->updateTotalModules(networkEditor_->numModules());
      networkEditor_->viewport()->update();
      success = true;
    }
    else
    {
      statusBar()->showMessage(tr("File import failed: ") + filename, 2000);
    }
		auto log = QString::fromStdString(command.logContents());
		auto logFileName = latestNetworkDirectory_.path() + "/" + ("importLog_" + strippedName(filename) + ".log");
		QFile logFile(logFileName); //todo: add timestamp
    if (logFile.open(QFile::WriteOnly | QFile::Text))
		{
			QTextStream stream(&logFile);
			stream << log;
			QMessageBox::information(this, "SRN File Import", "SRN File Import log file can be found here: " + logFileName
				+ "\n\nAdditionally, check the log directory for a list of missing modules (look for file missingModules.log)");
    }
		else
		{
			QMessageBox::information(this, "SRN File Import", "Failed to write SRN File Import log file: " + logFileName);
		}
  }
  return success;
}

bool SCIRunMainWindow::newNetwork()
{
  if (okToContinue())
  {
    networkEditor_->clear();
    provenanceWindow_->clear();
    setCurrentFile("");
		networkEditor_->viewport()->update();
    return true;
  }
  return false;
}

void SCIRunMainWindow::setCurrentFile(const QString& fileName)
{
  currentFile_ = fileName;
  setWindowModified(false);
  QString shownName = tr("Untitled");
  if (!currentFile_.isEmpty())
  {
    shownName = strippedName(currentFile_);
    latestNetworkDirectory_ = QFileInfo(currentFile_).dir();
    recentFiles_.removeAll(currentFile_);
    recentFiles_.prepend(currentFile_);
    updateRecentFileActions();
  }
  setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("SCIRun")));
}

QString SCIRunMainWindow::strippedName(const QString& fullFileName)
{
  QFileInfo info(fullFileName);
  return info.fileName();
}

void SCIRunMainWindow::updateRecentFileActions()
{
  QMutableStringListIterator i(recentFiles_);
  while (i.hasNext()) {
    if (!QFile::exists(i.next()))
      i.remove();
  }

  for (int j = 0; j < MaxRecentFiles; ++j)
  {
    if (j < recentFiles_.count())
    {
      QString text = tr("&%1 %2")
        .arg(j + 1)
        .arg(strippedName(recentFiles_[j]));

      recentFileActions_[j]->setText(text);
      recentFileActions_[j]->setData(recentFiles_[j]);
      recentFileActions_[j]->setVisible(true);
    }
    else
    {
      recentFileActions_[j]->setVisible(false);
    }
  }
}

void SCIRunMainWindow::loadRecentNetwork()
{
  if (okToContinue())
  {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
      loadNetworkFile(action->data().toString());
  }
}

void SCIRunMainWindow::closeEvent(QCloseEvent* event)
{
  windowState_ = saveState();
  if (okToContinue())
  {
    writeSettings();
    event->accept();
  }
  else
    event->ignore();
}

bool SCIRunMainWindow::okToContinue()
{
  if (isWindowModified() && !Application::Instance().parameters()->isRegressionMode() && !quitAfterExecute_ && !runningPythonScript_)
  {
    int r = QMessageBox::warning(this, tr("SCIRun 5"), tr("The document has been modified.\n" "Do you want to save your changes?"),
      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (QMessageBox::Yes == r)
    {
      saveNetwork();
      return true;
    }
    else if (QMessageBox::Cancel == r)
      return false;
  }
  return true;
}

//TODO: hook up to modules' state_changed_sig_t via GlobalStateManager
void SCIRunMainWindow::networkModified()
{
  setWindowModified(true);
  networkProgressBar_->updateTotalModules(networkEditor_->numModules());
}

void SCIRunMainWindow::setActionIcons()
{
  actionNew_->setIcon(QPixmap(":/general/Resources/new/general/new.png"));
  actionLoad_->setIcon(QPixmap(":/general/Resources/new/general/folder.png"));
  actionSave_->setIcon(QPixmap(":/general/Resources/new/general/save.png"));
  actionRunScript_->setIcon(QPixmap(":/general/Resources/new/general/wand.png"));
  //actionSave_As_->setIcon(QApplication::style()->standardIcon(QStyle::SP_DriveCDIcon));  //TODO?
  actionExecute_All_->setIcon(QPixmap(":/general/Resources/new/general/run.png"));
  actionUndo_->setIcon(QPixmap(":/general/Resources/undo.png"));
  actionRedo_->setIcon(QPixmap(":/general/Resources/redo.png"));
  //actionCut_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
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
}

void SCIRunMainWindow::filterModuleNamesInTreeView(const QString& start)
{
  ShowAll show;
  visitTree(moduleSelectorTreeWidget_, show);

  bool regexSelected = filterActionGroup_->checkedAction()->text().contains("wildcards");

  HideItemsNotMatchingString func(regexSelected, start);

  //note: goofy double call, first to hide the leaves, then hide the categories.
  visitTree(moduleSelectorTreeWidget_, func);
  visitTree(moduleSelectorTreeWidget_, func);
}

void SCIRunMainWindow::makeFilterButtonMenu()
{
  auto filterMenu = new QMenu(filterButton_);
  filterActionGroup_ = new QActionGroup(filterMenu);
  auto startsWithAction = new QAction("Starts with", filterButton_);
  startsWithAction->setCheckable(true);
  filterActionGroup_->addAction(startsWithAction);
  filterMenu->addAction(startsWithAction);

  auto wildcardAction = new QAction("Use wildcards", filterButton_);
  wildcardAction->setCheckable(true);
  filterActionGroup_->addAction(wildcardAction);
  wildcardAction->setChecked(true);
  filterMenu->addAction(wildcardAction);

  filterButton_->setMenu(filterMenu);
}

void SCIRunMainWindow::makePipesCubicBezier()
{
  networkEditor_->setConnectionPipelineType(CUBIC);
}

void SCIRunMainWindow::makePipesEuclidean()
{
  networkEditor_->setConnectionPipelineType(EUCLIDEAN);
}

void SCIRunMainWindow::makePipesManhattan()
{
  networkEditor_->setConnectionPipelineType(MANHATTAN);
}

void SCIRunMainWindow::chooseBackgroundColor()
{
  auto brush = networkEditor_->background();
  auto oldColor = brush.color();

  auto newColor = QColorDialog::getColor(oldColor, this, "Choose background color");
  if (newColor.isValid())
  {
    networkEditor_->setBackground(newColor);
    GuiLogger::Instance().logInfo("Background color set to " + newColor.name());
  }
}

void SCIRunMainWindow::setDragMode(bool toggle)
{
  if (toggle)
  {
    networkEditor_->setMouseAsDragMode();
    statusBar()->showMessage("Mouse in drag mode", 2000);
  }
  if (actionDragMode_->isChecked())
  {
    actionSelectMode_->setChecked(false);
  }
  else
  {
    actionSelectMode_->setChecked(true);
  }
}

void SCIRunMainWindow::setSelectMode(bool toggle)
{
  if (toggle)
  {
    networkEditor_->setMouseAsSelectMode();
    statusBar()->showMessage("Mouse in select mode", 2000);
  }
  if (actionSelectMode_->isChecked())
  {
    actionDragMode_->setChecked(false);
  }
  else
  {
    actionDragMode_->setChecked(true);
  }
}

void SCIRunMainWindow::zoomNetwork()
{
  auto action = qobject_cast<QAction*>(sender());
  if (action)
  {
    const QString name = action->text();
    if (name == "Zoom In")
    {
      networkEditor_->zoomIn();
    }
    else if (name == "Zoom Out")
    {
      networkEditor_->zoomOut();
    }
    else if (name == "Reset Network Zoom")
    {
      networkEditor_->zoomReset();
    }
    else if (name == "Zoom Best Fit")
    {
      networkEditor_->zoomBestFit();
    }
  }
  else
  {
    qDebug() << "Sender was null or not an action";
  }
}

void SCIRunMainWindow::showZoomStatusMessage(int zoomLevel)
{
  statusBar()->showMessage(tr("Zoom: %1%").arg(zoomLevel), 2000);
}

void SCIRunMainWindow::resetBackgroundColor()
{
  //TODO: standardize these defaults
  QColor defaultColor(Qt::darkGray);
  networkEditor_->setBackground(defaultColor);
  GuiLogger::Instance().logInfo("Background color set to " + defaultColor.name());
}

void SCIRunMainWindow::setupProvenanceWindow()
{
  ProvenanceManagerHandle provenanceManager(new Dataflow::Engine::ProvenanceManager<SCIRun::Dataflow::Networks::NetworkFileHandle>(networkEditor_));
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
}

void SCIRunMainWindow::filterDoubleClickedModuleSelectorItem(QTreeWidgetItem* item)
{
  if (item && item->childCount() == 0)
    Q_EMIT moduleItemDoubleClicked();
}

void SCIRunMainWindow::setupDevConsole()
{
  devConsole_ = new DeveloperConsole(this);
  connect(actionDevConsole_, SIGNAL(toggled(bool)), devConsole_, SLOT(setVisible(bool)));
  connect(devConsole_, SIGNAL(visibilityChanged(bool)), actionDevConsole_, SLOT(setChecked(bool)));
  devConsole_->setVisible(false);
  devConsole_->setFloating(true);
  addDockWidget(Qt::TopDockWidgetArea, devConsole_);
  actionDevConsole_->setShortcut(QKeySequence("`"));
  connect(devConsole_, SIGNAL(executorChosen(int)), this, SLOT(setExecutor(int)));
  connect(devConsole_, SIGNAL(globalPortCachingChanged(bool)), this, SLOT(setGlobalPortCaching(bool)));
}

void SCIRunMainWindow::setExecutor(int type)
{
  LOG_DEBUG("Executor of type " << type << " selected"  << std::endl);
  networkEditor_->getNetworkEditorController()->setExecutorType(type);
}

void SCIRunMainWindow::setGlobalPortCaching(bool enable)
{
  LOG_DEBUG("Global port caching flag set to " << (enable ? "true" : "false") << std::endl);
  //TODO: encapsulate better
  SimpleSink::setGlobalPortCachingFlag(enable);
}

void SCIRunMainWindow::readDefaultNotePosition(int index)
{
  Q_EMIT defaultNotePositionChanged(defaultNotePositionGetter_->position()); //TODO: unit test.
}

void SCIRunMainWindow::setupPreferencesWindow()
{
  prefsWindow_ = new PreferencesWindow(networkEditor_, this);

  connect(actionPreferences_, SIGNAL(triggered()), prefsWindow_, SLOT(show()));
  //connect(prefs_, SIGNAL(visibilityChanged(bool)), actionPreferences_, SLOT(setChecked(bool)));
  prefsWindow_->setVisible(false);
}

void SCIRunMainWindow::setupPythonConsole()
{
#ifdef BUILD_WITH_PYTHON
  pythonConsole_ = new PythonConsoleWidget(this);
  connect(actionPythonConsole_, SIGNAL(toggled(bool)), pythonConsole_, SLOT(setVisible(bool)));
  connect(actionPythonConsole_, SIGNAL(toggled(bool)), this, SLOT(showPythonWarning(bool)));
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

void SCIRunMainWindow::runPythonScript(const QString& scriptFileName)
{
#ifdef BUILD_WITH_PYTHON
  runningPythonScript_ = true;
  GuiLogger::Instance().logInfo("RUNNING PYTHON SCRIPT: " + scriptFileName);
  SCIRun::Core::PythonInterpreter::Instance().run_string("import SCIRunPythonAPI; from SCIRunPythonAPI import *");
  SCIRun::Core::PythonInterpreter::Instance().run_file(scriptFileName.toStdString());
  statusBar()->showMessage(tr("Script is running."), 2000);
#else
  GuiLogger::Instance().logInfo("Python not included in this build, cannot run " + scriptFileName);
#endif
}

void SCIRunMainWindow::runScript()
{
  if (okToContinue())
  {
    QString filename = QFileDialog::getOpenFileName(this, "Load Script...", latestNetworkDirectory_.path(), "*.py");
    runPythonScript(filename);
  }
}

void SCIRunMainWindow::updateMiniView()
{
  //networkEditorMiniViewLabel_->setText("+" + networkEditorMiniViewLabel_->text());
  QPixmap network = networkEditor_->sceneGrab();
  networkEditorMiniViewLabel_->setPixmap(network.scaled(networkEditorMiniViewLabel_->size(),
    Qt::KeepAspectRatio,
    Qt::SmoothTransformation));
}

void SCIRunMainWindow::showPythonWarning(bool visible)
{
  if (visible && firstTimePythonShown_)
  {
    firstTimePythonShown_ = false;
    QMessageBox::warning(this, "Warning: Known Python interface issue",
      "Attention Python interface user: this feature is not fully implemented. The main issue is that changes made to the current network from the GUI, such as adding/removing modules, are not reflected in the Python console's state. Thus strange bugs can be created by switching between Python-edit mode and standard-GUI-edit mode. Please use the Python console to test your commands, then compose a script that you can run separately without needing the GUI. This issue will be resolved in the next milestone. Thank you!");
  }
}

void SCIRunMainWindow::makeModulesLargeSize()
{
  networkEditor_->setModuleMini(false);
}

void SCIRunMainWindow::makeModulesSmallSize()
{
  networkEditor_->setModuleMini(true);
}

namespace {

  QColor favesColor()
  {
    return SCIRunMainWindow::Instance()->newInterface() ? Qt::yellow : Qt::darkYellow;
  }
  QColor packageColor()
  {
    return SCIRunMainWindow::Instance()->newInterface() ? Qt::yellow : Qt::darkYellow;
  }
  QColor categoryColor()
  {
    return SCIRunMainWindow::Instance()->newInterface() ? Qt::green : Qt::darkGreen;
  }

  const QString bullet = "* ";
  const QString favoritesText = bullet + "Favorites";

  void addFavoriteMenu(QTreeWidget* tree)
  {
    auto faves = new QTreeWidgetItem();
    faves->setText(0, favoritesText);
    faves->setForeground(0, favesColor());

    tree->addTopLevelItem(faves);
  }

  QTreeWidgetItem* getFavoriteMenu(QTreeWidget* tree)
  {
    for (int i = 0; i < tree->topLevelItemCount(); ++i)
    {
      auto top = tree->topLevelItem(i);
      if (top->text(0) == favoritesText)
      {
        return top;
      }
    }
    return nullptr;
  }

  void addSnippet(const QString& code, QTreeWidgetItem* snips)
  {
    auto snipItem = new QTreeWidgetItem();
    snipItem->setText(0, code);
    snips->addChild(snipItem);
  }

  void readCustomSnippets(QTreeWidgetItem* snips)
  {
    QFile inputFile("snippets.txt");
    if (inputFile.open(QIODevice::ReadOnly))
    {
      GuiLogger::Instance().logInfo("Snippet file opened: " + inputFile.fileName());
      QTextStream in(&inputFile);
      while (!in.atEnd())
      {
        QString line = in.readLine();
        addSnippet(line, snips);
        GuiLogger::Instance().logInfo("Snippet read: " + line);
      }
      inputFile.close();
    }
  }

  void addSnippetMenu(QTreeWidget* tree)
	{
		auto snips = new QTreeWidgetItem();
    snips->setText(0, bullet + "Snippets");
		snips->setForeground(0, favesColor());

		//hard-code a few popular ones.

    addSnippet("[ReadField->ShowField->ViewScene]", snips);
    addSnippet("[CreateLatVol->ShowField->ViewScene]", snips);
    addSnippet("[ReadField->ReportFieldInfo]", snips);
    addSnippet("[CreateStandardColorMap->RescaleColorMap->ShowField->ViewScene]", snips);
    addSnippet("[GetFieldBoundary->FairMesh->ShowField]", snips);
    //TODO coming later, with grammar
		//addSnippet("[CreateLatVol->(CreateStandardColorMap->RescaleColorMap->ShowField)->ViewScene]", snips);

	  readCustomSnippets(snips);

	  tree->addTopLevelItem(snips);
	}

  void addFavoriteItem(QTreeWidgetItem* faves, QTreeWidgetItem* module)
  {
    LOG_DEBUG("Adding item to favorites: " << module->text(0).toStdString() << std::endl);
    auto copy = new QTreeWidgetItem(*module);
    copy->setData(0, Qt::CheckStateRole, QVariant());
    faves->addChild(copy);
  }

  void fillTreeWidget(QTreeWidget* tree, const ModuleDescriptionMap& moduleMap, const QStringList& favoriteModuleNames)
  {
    QTreeWidgetItem* faves = getFavoriteMenu(tree);
		for (const auto& package : moduleMap)
    {
      const auto& packageName = package.first;
      auto packageItem = new QTreeWidgetItem();
      packageItem->setText(0, QString::fromStdString(packageName));
      packageItem->setForeground(0, packageColor());
      tree->addTopLevelItem(packageItem);
      size_t totalModules = 0;
      for (const auto& category : package.second)
      {
        const auto& categoryName = category.first;
        auto categoryItem = new QTreeWidgetItem();
        categoryItem->setText(0, QString::fromStdString(categoryName));
        categoryItem->setForeground(0, categoryColor());
        packageItem->addChild(categoryItem);
				for (const auto& module : category.second)
        {
          const auto& moduleName = module.first;
          auto moduleItem = new QTreeWidgetItem();
          auto name = QString::fromStdString(moduleName);
          moduleItem->setText(0, name);
          if (favoriteModuleNames.contains(name))
          {
            moduleItem->setCheckState(0, Qt::Checked);
            addFavoriteItem(faves, moduleItem);
          }
          else
          {
            moduleItem->setCheckState(0, Qt::Unchecked);
          }
          moduleItem->setText(1, QString::fromStdString(module.second.moduleStatus_));
          moduleItem->setForeground(1, Qt::lightGray);
          moduleItem->setText(2, QString::fromStdString(module.second.moduleInfo_));
          moduleItem->setForeground(2, Qt::lightGray);
          categoryItem->addChild(moduleItem);
          totalModules++;
        }
        categoryItem->setText(1, "Category Module Count = " + QString::number(category.second.size()));
        categoryItem->setForeground(1, Qt::magenta);
      }
      packageItem->setText(1, "Package Module Count = " + QString::number(totalModules));
      packageItem->setForeground(1, Qt::magenta);
    }
  }

  void sortFavorites(QTreeWidget* tree)
  {
    QTreeWidgetItem* faves = getFavoriteMenu(tree);
    faves->sortChildren(0, Qt::AscendingOrder);
  }

}

void SCIRunMainWindow::fillModuleSelector()
{
  moduleSelectorTreeWidget_->clear();

  auto moduleDescs = networkEditor_->getNetworkEditorController()->getAllAvailableModuleDescriptions();

  addFavoriteMenu(moduleSelectorTreeWidget_);
	addSnippetMenu(moduleSelectorTreeWidget_);
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

void SCIRunMainWindow::handleCheckedModuleEntry(QTreeWidgetItem* item, int column)
{
  if (item && 0 == column)
  {
    moduleSelectorTreeWidget_->setCurrentItem(item);

    QTreeWidgetItem* faves = getFavoriteMenu(moduleSelectorTreeWidget_);

    if (item->checkState(0) == Qt::Checked)
    {
      if (faves)
      {
        addFavoriteItem(faves, item);
        faves->sortChildren(0, Qt::AscendingOrder);
        favoriteModuleNames_ << item->text(0);
      }
    }
    else
    {
      if (faves)
      {
        favoriteModuleNames_.removeAll(item->text(0));
        for (int i = 0; i < faves->childCount(); ++i)
        {
          auto child = faves->child(i);
          if (child->text(0) == item->text(0))
            faves->removeChild(child);
        }
      }
    }
  }
}

bool SCIRunMainWindow::isInFavorites(const QString& module) const
{
	return favoriteModuleNames_.contains(module);
}

void SCIRunMainWindow::displayAcknowledgement()
{
  QMessageBox::information(this, "NIH/NIGMS Center for Integrative Biomedical Computing Acknowledgment",
    "CIBC software and the data sets provided on this web site are Open Source software projects that are principally funded through the SCI Institute's NIH/NCRR CIBC. For us to secure the funding that allows us to continue providing this software, we must have evidence of its utility. Thus we ask users of our software and data to acknowledge us in their publications and inform us of these publications. Please use the following acknowledgment and send us references to any publications, presentations, or successful funding applications that make use of the NIH/NCRR CIBC software or data sets we provide. <p> <i>This project was supported by the National Institute of General Medical Sciences of the National Institutes of Health under grant number P41GM103545.</i>");
}

void SCIRunMainWindow::setDataDirectory(const QString& dir)
{
  if (!dir.isEmpty())
  {
    prefsWindow_->scirunDataLineEdit_->setText(dir);
    prefsWindow_->scirunDataLineEdit_->setToolTip(dir);

    RemembersFileDialogDirectory::setStartingDir(dir);
    Core::Preferences::Instance().setDataDirectory(dir.toStdString());
  }
}

void SCIRunMainWindow::setDataPath(const QString& dirs)
{
	if (!dirs.isEmpty())
	{
    prefsWindow_->scirunDataPathTextEdit_->setPlainText(dirs);
    prefsWindow_->scirunDataPathTextEdit_->setToolTip(dirs);

		Core::Preferences::Instance().setDataPath(dirs.toStdString());
	}
}

void SCIRunMainWindow::addToDataDirectory(const QString& dir)
{
	if (!dir.isEmpty())
	{
    auto text = prefsWindow_->scirunDataPathTextEdit_->toPlainText();
		if (!text.isEmpty())
			text += ";\n";
		text += dir;
    prefsWindow_->scirunDataPathTextEdit_->setPlainText(text);
    prefsWindow_->scirunDataPathTextEdit_->setToolTip(prefsWindow_->scirunDataPathTextEdit_->toPlainText());

		RemembersFileDialogDirectory::setStartingDir(dir);
		Core::Preferences::Instance().addToDataPath(dir.toStdString());
	}
}

void SCIRunMainWindow::setDataDirectoryFromGUI()
{
  QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Data Directory"), ".");
  setDataDirectory(dir);
}

void SCIRunMainWindow::addToPathFromGUI()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Add Directory to Data Path"), ".");
	addToDataDirectory(dir);
}

bool SCIRunMainWindow::newInterface() const
{
  return Core::Application::Instance().parameters()->entireCommandLine().find("--originalGUI") == std::string::npos;
}

void SCIRunMainWindow::printStyleSheet() const
{
  std::cout << "Printing style sheet details map" << std::endl;
  for (auto styleMapIt = styleSheetDetails_.constBegin(); styleMapIt != styleSheetDetails_.constEnd(); ++styleMapIt)
  {
    std::cout << "Style for: " << styleMapIt.key().toStdString() << std::endl;
    for (auto styleIt = styleMapIt.value().constBegin(); styleIt != styleMapIt.value().constEnd(); ++styleIt)
    {
      std::cout << "key: " << styleIt.key().toStdString() << " value: " << styleIt.value().toStdString() << std::endl;
    }
  }
}

void SCIRunMainWindow::setFocusOnFilterLine()
{
  moduleFilterLineEdit_->setFocus(Qt::ShortcutFocusReason);
  statusBar()->showMessage(tr("Module filter activated"), 2000);
}

//disable these
void SCIRunMainWindow::addModuleKeyboardAction()
{
  //TODO
  auto item = moduleSelectorTreeWidget_->currentItem();
  if (item && item->childCount() == 0)
    std::cout << "Current module: " << item->text(0).toStdString() << std::endl;
}

void SCIRunMainWindow::selectModuleKeyboardAction()
{
  moduleSelectorTreeWidget_->setFocus(Qt::ShortcutFocusReason);
  statusBar()->showMessage(tr("Module selection activated"), 2000);
}

void SCIRunMainWindow::modulesSnapToChanged()
{
  bool snapTo = prefsWindow_->modulesSnapToCheckBox_->isChecked();
  Preferences::Instance().modulesSnapToGrid.setValue(snapTo);
}

void SCIRunMainWindow::highlightPortsChanged()
{
  bool val = prefsWindow_->portSizeEffectsCheckBox_->isChecked();
  Preferences::Instance().highlightPorts.setValue(val);
}

void SCIRunMainWindow::resetWindowLayout()
{
  configurationDockWidget_->hide();
  devConsole_->hide();
  provenanceWindow_->hide();
  moduleSelectorDockWidget_->show();
  moduleSelectorDockWidget_->setFloating(false);
  addDockWidget(Qt::LeftDockWidgetArea, moduleSelectorDockWidget_);

  qDebug() << "TODO: toolbars";
}

void SCIRunMainWindow::hideNonfunctioningWidgets()
{
	//TODO: make issues to implement these, as I don't want to forget they are there.
  QList<QAction*> nonfunctioningActions;
  nonfunctioningActions <<
    actionInsert_;
  QList<QMenu*> nonfunctioningMenus;
  nonfunctioningMenus <<
    menuSubnets_;
  QList<QWidget*> nonfunctioningWidgets;
  nonfunctioningWidgets <<
    prefsWindow_->scirunNetsLabel_ <<
    prefsWindow_->scirunNetsLineEdit_ <<
    prefsWindow_->scirunNetsPushButton_ <<
    prefsWindow_->userDataLabel_ <<
    prefsWindow_->userDataLineEdit_ <<
    prefsWindow_->userDataPushButton_ <<
    prefsWindow_->dataSetGroupBox_ <<
    networkEditorMiniViewLabel_ <<
    miniviewTextLabel_ <<
    prefsWindow_->scirunDataPathTextEdit_ <<
    prefsWindow_->addToPathButton_;

  Q_FOREACH(QAction* a, nonfunctioningActions)
    a->setVisible(false);
  Q_FOREACH(QMenu* m, nonfunctioningMenus)
    m->menuAction()->setVisible(false);
  Q_FOREACH(QWidget* w, nonfunctioningWidgets)
    w->setVisible(false);
}

void SCIRunMainWindow::adjustModuleDock(int state)
{
  bool dockable = prefsWindow_->dockableModulesCheckBox_->isChecked();
  actionPinAllModuleUIs_->setEnabled(dockable);
  Preferences::Instance().modulesAreDockable.setValue(dockable);
}

void SCIRunMainWindow::showEvent(QShowEvent* event)
{
  restoreState(windowState_);
  QMainWindow::showEvent(event);
}

void SCIRunMainWindow::hideEvent(QHideEvent * event)
{
  windowState_ = saveState();
  QMainWindow::hideEvent(event);
}

void SCIRunMainWindow::updateDockWidgetProperties(bool isFloating)
{
  auto dock = qobject_cast<QDockWidget*>(sender());
  if (dock && isFloating)
  {
    dock->setWindowFlags(Qt::Window);
    dock->show();
  }
}

#ifdef __APPLE__
static const Qt::Key MetadataShiftKey = Qt::Key_Meta;
#else
static const Qt::Key MetadataShiftKey = Qt::Key_CapsLock;
#endif

void SCIRunMainWindow::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Shift)
	{
		showStatusMessage("Network zoom active");
	}
  else if (event->key() == MetadataShiftKey)
  {
    networkEditor_->metadataLayer(true);
		showStatusMessage("Metadata layer active");
  }
  else if (event->key() == Qt::Key_Alt)
  {
		if (!actionToggleTagLayer_->isChecked())
		{
	 		networkEditor_->tagLayer(true, NoTag);
			showStatusMessage("Tag layer active: none");
		}
  }
	else if (event->key() == Qt::Key_A)
	{
		if (!actionToggleTagLayer_->isChecked())
		{
    	if (networkEditor_->tagLayerActive())
    	{
      	networkEditor_->tagLayer(true, AllTags);
				showStatusMessage("Tag layer active: All");
    	}
		}
	}
  else if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9)
  {
		if (!actionToggleTagLayer_->isChecked())
		{
    	if (networkEditor_->tagLayerActive())
    	{
      	auto key = event->key() - Qt::Key_0;
      	networkEditor_->tagLayer(true, key);
				showStatusMessage("Tag layer active: " + QString::number(key));
    	}
		}
  }

  QMainWindow::keyPressEvent(event);
}

void SCIRunMainWindow::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Shift)
	{
		showStatusMessage("Network zoom inactive", 1000);
	}
  else if (event->key() == MetadataShiftKey)
  {
    networkEditor_->metadataLayer(false);
		showStatusMessage("Metadata layer inactive", 1000);
  }
  else if (event->key() == Qt::Key_Alt)
  {
		if (!actionToggleTagLayer_->isChecked())
		{
    	networkEditor_->tagLayer(false, -1);
			showStatusMessage("Tag layer inactive", 1000);
		}
  }

  QMainWindow::keyPressEvent(event);
}

void SCIRunMainWindow::changeExecuteActionIconToStop()
{
  actionExecute_All_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaStop));
	actionExecute_All_->setText("Halt Execution");
}

void SCIRunMainWindow::changeExecuteActionIconToPlay()
{
  actionExecute_All_->setIcon(QPixmap(":/general/Resources/new/general/run.png"));
	actionExecute_All_->setText("Execute All");
}

void SCIRunMainWindow::openLogFolder()
{
  auto logPath = QString::fromStdString(Application::Instance().logDirectory().string());
  QDesktopServices::openUrl(QUrl::fromLocalFile(logPath));
}

void SCIRunMainWindow::adjustExecuteButtonAppearance()
{
  switch (prefsWindow_->actionTextIconCheckBox_->checkState())
  {
  case 0:
    prefsWindow_->actionTextIconCheckBox_->setText("Execute Button Text");
		executeButton_->setToolButtonStyle(Qt::ToolButtonTextOnly);
    break;
  case 1:
    prefsWindow_->actionTextIconCheckBox_->setText("Execute Button Icon");
		executeButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    break;
  case 2:
    prefsWindow_->actionTextIconCheckBox_->setText("Execute Button Text+Icon");
		executeButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    break;
  }
}

void SCIRunMainWindow::alertForNetworkCycles(int code)
{
  if (code == -1)
  {
    QMessageBox::warning(this, "Network graph has a cycle", "Your network contains a cycle. The execution scheduler cannot handle cycles at this time. Please ensure all cycles are broken before executing.");
    networkEditor_->resetNetworkDueToCycle();
  }
}

void SCIRunMainWindow::addModuleToWindowList(const QString& modId, bool hasUI)
{
  if (menuCurrent_->actions().isEmpty())
    menuCurrent_->setEnabled(true);
  auto modAction = new QAction(this);
  modAction->setText(modId);
  modAction->setEnabled(hasUI);
  connect(modAction, SIGNAL(triggered()), networkEditor_, SLOT(moduleWindowAction()));
  currentModuleActions_.insert(modId, modAction);
  menuCurrent_->addAction(modAction);
}

void SCIRunMainWindow::removeModuleFromWindowList(const ModuleId& modId)
{
  auto name = QString::fromStdString(modId.id_);
  auto action = currentModuleActions_[name];
  menuCurrent_->removeAction(action);
  currentModuleActions_.remove(name);
  if (menuCurrent_->actions().isEmpty())
    menuCurrent_->setEnabled(false);
}

void SCIRunMainWindow::setupTagManagerWindow()
{
  tagManagerWindow_ = new TagManagerWindow(this);
  connect(actionTagManager_, SIGNAL(toggled(bool)), tagManagerWindow_, SLOT(setVisible(bool)));
  connect(tagManagerWindow_, SIGNAL(visibilityChanged(bool)), actionTagManager_, SLOT(setChecked(bool)));
}

void SCIRunMainWindow::toggleTagLayer(bool toggle)
{
	networkEditor_->tagLayer(toggle, AllTags);
	if (toggle)
		showStatusMessage("Tag layer active: all");
	else
		showStatusMessage("Tag layer inactive", 1000);
}

void SCIRunMainWindow::showStatusMessage(const QString& str)
{
	statusBar()->showMessage(str);
}

void SCIRunMainWindow::showStatusMessage(const QString& str, int timeInMsec)
{
	statusBar()->showMessage(str, timeInMsec);
}

void SCIRunMainWindow::toggleMetadataLayer(bool toggle)
{
	networkEditor_->metadataLayer(toggle);
	//TODO: extract methods
	if (toggle)
		showStatusMessage("Metadata layer active");
	else
		showStatusMessage("Metadata layer inactive", 1000);
}

void SCIRunMainWindow::showKeyboardShortcutsDialog()
{
  if (!shortcuts_)
  {
    shortcuts_ = new ShortcutsInterface(this);
  }
  shortcuts_->show();
}

void SCIRunMainWindow::runNewModuleWizard()
{
	qDebug() << "new module wizard coming soon";
	ClassWizard* wizard = new ClassWizard(this);
	wizard->show();
}

FileDownloader::FileDownloader(QUrl imageUrl, QStatusBar* statusBar, QObject *parent) : QObject(parent), reply_(nullptr), statusBar_(statusBar)
{
 	connect(&webCtrl_, SIGNAL(finished(QNetworkReply*)), this, SLOT(fileDownloaded(QNetworkReply*)));

 	QNetworkRequest request(imageUrl);
	reply_ = webCtrl_.get(request);
  connect(reply_, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
}

void FileDownloader::fileDownloaded(QNetworkReply* reply)
{
  downloadedData_ = reply->readAll();
	reply->deleteLater();
  Q_EMIT downloaded();
}

void FileDownloader::downloadProgress(qint64 received, qint64 total)
{
  if (statusBar_)
    statusBar_->showMessage(tr("File progress: %1 / %2").arg(received).arg(total), 1000);
}

void SCIRunMainWindow::toolkitDownload()
{
	QAction* action = qobject_cast<QAction*>(sender());

	static std::vector<ToolkitDownloader*> downloaders;
  downloaders.push_back(new ToolkitDownloader(action, statusBar(), this));
}

ToolkitDownloader::ToolkitDownloader(QObject* infoObject, QStatusBar* statusBar, QWidget* parent) : QObject(parent), iconDownloader_(nullptr), zipDownloader_(nullptr), statusBar_(statusBar)
{
  if (infoObject)
  {
    iconUrl_ = infoObject->property(ToolkitIconURL).toString();
    //qDebug() << "Toolkit info: \nIcon: " << iconUrl_;
    fileUrl_ = infoObject->property(ToolkitURL).toString();
    //qDebug() << "File url: " << fileUrl_;
    filename_ = infoObject->property(ToolkitFilename).toString();
    //qDebug() << "Filename: " << filename_;

    downloadIcon();
  }
}

void ToolkitDownloader::downloadIcon()
{
  iconDownloader_ = new FileDownloader(iconUrl_, nullptr, this);
  connect(iconDownloader_, SIGNAL(downloaded()), this, SLOT(showMessageBox()));
}

void ToolkitDownloader::showMessageBox()
{
  if (!iconDownloader_)
    return;

  QPixmap image;
  image.loadFromData(iconDownloader_->downloadedData());

  QMessageBox toolkitInfo;
#ifdef WIN32
  toolkitInfo.setWindowTitle("Toolkit information");
#else
  toolkitInfo.setText("Toolkit information");
#endif
  toolkitInfo.setInformativeText("Click OK to download the latest version of this toolkit.");
  toolkitInfo.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  toolkitInfo.setIconPixmap(image);
  toolkitInfo.setDefaultButton(QMessageBox::Ok);
  toolkitInfo.show();
  auto choice = toolkitInfo.exec();

  if (choice == QMessageBox::Ok)
  {
    auto dir = QFileDialog::getExistingDirectory(qobject_cast<QWidget*>(parent()), "Select toolkit directory", ".");
    if (!dir.isEmpty())
    {
      //qDebug() << "directory selected " << dir;
      toolkitDir_ = dir;
      zipDownloader_ = new FileDownloader(fileUrl_, statusBar_, this);
      connect(zipDownloader_, SIGNAL(downloaded()), this, SLOT(saveToolkit()));
    }
  }
}

void ToolkitDownloader::saveToolkit()
{
  if (!zipDownloader_)
    return;

  QString fullFilename = toolkitDir_.filePath(filename_);
  //qDebug() << "saving to " << fullFilename;
  QFile file(fullFilename);
  file.open(QIODevice::WriteOnly);
  file.write(zipDownloader_->downloadedData());
  file.close();
  //qDebug() << "save done";
}
