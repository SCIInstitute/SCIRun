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
#include <boost/algorithm/string.hpp>
#include <Core/Utils/Legacy/MemoryUtil.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/TagManagerWindow.h>
#include <Interface/Application/MacroEditor.h>
#include <Interface/Application/TreeViewCollaborators.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/GuiCommands.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/NetworkExecutionProgressBar.h>
#include <Interface/Modules/Base/DialogErrorControl.h>
#include <Interface/Application/TriggeredEventsWindow.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h> //TODO
#include <Dataflow/Engine/Controller/ProvenanceManager.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/Importer/NetworkIO.h>
#include <chrono>

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

SCIRunMainWindow::SCIRunMainWindow()
{
  setupUi(this);
  builder_ = makeShared<NetworkEditorBuilder>(this);
  //dockManager_ = new DockManager(dockSpace_, this);

  {
    SharedPointer<TextEditAppender> logger(new TextEditAppender(logTextBrowser_));
    GuiLog::Instance().addCustomSink(logger);
  }

  startup_ = true;

  QCoreApplication::setOrganizationName("SCI:CIBC Software");
  QCoreApplication::setApplicationName("SCIRun5");

  setAttribute(Qt::WA_DeleteOnClose);

  menubar_->setStyleSheet("QMenuBar::item::selected{background-color : rgb(66, 66, 69); } QMenuBar::item::!selected{ background-color : rgb(66, 66, 69); } ");

  setupTagManagerWindow();

  setupPreferencesWindow();

  setupNetworkEditor();

  setTipsAndWhatsThis();

  connect(actionExecuteAll_, &QAction::triggered, this, &SCIRunMainWindow::executeAll);
  connect(actionNew_, &QAction::triggered, this, &SCIRunMainWindow::newNetwork);

  createStandardToolbars();
  createExecuteToolbar();
  createAdvancedToolbar();

  #ifdef __APPLE__
  connect(actionLaunchNewInstance_, &QAction::triggered, this, &SCIRunMainWindow::launchNewInstance);
  #else
  menuFile_->removeAction(actionLaunchNewInstance_);
  #endif

  {
    auto searchAction = new QWidgetAction(this);
    searchAction->setDefaultWidget(new NetworkSearchWidget(networkEditor_));
    addToolBarBreak();
    auto searchBar = addToolBar("&Search");
    searchBar->setObjectName("SearchToolBar");
    WidgetStyleMixin::toolbarStyle(searchBar);
    searchBar->addAction(searchAction);
    connect(actionSearchBar_, &QAction::toggled, searchBar, &QToolBar::setVisible);
    connect(searchBar, &QToolBar::visibilityChanged, actionSearchBar_, &QAction::setChecked);
    searchBar->setVisible(false);
  }

  setContextMenuPolicy(Qt::NoContextMenu);

  scrollArea_->viewport()->setBackgroundRole(QPalette::Dark);
  scrollArea_->viewport()->setAutoFillBackground(true);
  scrollArea_->setStyleSheet(styleSheet());

  connect(actionSave_As_, &QAction::triggered, this, &SCIRunMainWindow::saveNetworkAs);
  connect(actionSave_, &QAction::triggered, this, &SCIRunMainWindow::saveNetwork);
  connect(actionLoad_, &QAction::triggered, this, &SCIRunMainWindow::loadNetwork);
  connect(actionImportNetwork_, &QAction::triggered, this, &SCIRunMainWindow::importLegacyNetwork);
  connect(actionQuit_, &QAction::triggered, this, &SCIRunMainWindow::close);
  connect(actionRunScript_, &QAction::triggered, this, &SCIRunMainWindow::runScript);

  connect(actionRunMacro1_, &QAction::triggered, this, &SCIRunMainWindow::runMacro);
  connect(actionRunMacro2_, &QAction::triggered, this, &SCIRunMainWindow::runMacro);
  connect(actionRunMacro3_, &QAction::triggered, this, &SCIRunMainWindow::runMacro);
  connect(actionRunMacro4_, &QAction::triggered, this, &SCIRunMainWindow::runMacro);
  connect(actionRunMacro5_, &QAction::triggered, this, &SCIRunMainWindow::runMacro);

  {
    QFile importerXML(":/general/Resources/LegacyModuleImporter.xml");

    if (importerXML.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      std::ostringstream ostr;
      QTextStream in(&importerXML);
      while (!in.atEnd())
      {
        QString line = in.readLine();
        ostr << line.toStdString();
      }
      std::istringstream file(ostr.str());
      LegacyNetworkIO::initializeStateConverter(file);
    }
  }

  actionQuit_->setShortcut(QKeySequence::Quit);

#ifdef __APPLE__
  actionToggleFullScreenMode_->setShortcut(QApplication::translate("SCIRunMainWindow", "Meta+Ctrl+F", 0));
#else
  actionToggleFullScreenMode_->setShortcut(Qt::Key_F11);
#endif

  actionDelete_->setShortcuts(QList<QKeySequence>() << QKeySequence::Delete << Qt::Key_Backspace);

	connect(actionRunNewModuleWizard_, &QAction::triggered, this, &SCIRunMainWindow::runNewModuleWizard);
	actionRunNewModuleWizard_->setDisabled(true);

  connect(actionAbout_, &QAction::triggered, this, &SCIRunMainWindow::displayAcknowledgement);
  connect(actionCreateToolkitFromDirectory_, &QAction::triggered, this, &SCIRunMainWindow::helpWithToolkitCreation);

  connect(helpActionPythonAPI_, &QAction::triggered, this, &SCIRunMainWindow::loadPythonAPIDoc);
  connect(helpActionModuleFuzzySearch_, &QAction::triggered, this, &SCIRunMainWindow::showModuleFuzzySearchHelp);
  connect(helpActionSnippets_, &QAction::triggered, this, &SCIRunMainWindow::showSnippetHelp);
  connect(helpActionClipboard_, &QAction::triggered, this, &SCIRunMainWindow::showClipboardHelp);
	connect(helpActionTagLayer_, &QAction::triggered, this, &SCIRunMainWindow::showTagHelp);
	connect(helpActionTriggeredScripts_, &QAction::triggered, this, &SCIRunMainWindow::showTriggerHelp);
  connect(helpActionNewUserWizard_, &QAction::triggered, this, &SCIRunMainWindow::launchNewUserWizard);
  connect(helpActionPythonWizard_, &QAction::triggered, this, &SCIRunMainWindow::launchPythonWizard);

  connect(actionReset_Window_Layout, &QAction::triggered, this, &SCIRunMainWindow::resetWindowLayout);
  connect(actionToggleFullScreenMode_, &QAction::triggered, this, &SCIRunMainWindow::toggleFullScreen);

#ifndef BUILD_WITH_PYTHON
  actionRunScript_->setEnabled(false);
#endif

  for (int i = 0; i < MaxRecentFiles; ++i)
  {
    recentFileActions_.push_back(new QAction(this));
    recentFileActions_[i]->setVisible(false);
    recentNetworksMenu_->addAction(recentFileActions_[i]);
    connect(recentFileActions_[i], &QAction::triggered, this, &SCIRunMainWindow::loadRecentNetwork);
  }

	setupScriptedEventsWindow();
  setupProvenanceWindow();
  createMacroToolbar();

  setActionIcons();

  setupDevConsole();
  setupPythonConsole();

  connect(prefsWindow_->defaultNotePositionComboBox_, qOverload<int>(&QComboBox::activated), this, &SCIRunMainWindow::readDefaultNotePosition);
  connect(prefsWindow_->defaultNoteSizeComboBox_, qOverload<int>(&QComboBox::activated), this, &SCIRunMainWindow::readDefaultNoteSize);
  connect(prefsWindow_->cubicPipesRadioButton_, &QPushButton::clicked, this, &SCIRunMainWindow::makePipesCubicBezier);
  connect(prefsWindow_->manhattanPipesRadioButton_, &QPushButton::clicked, this, &SCIRunMainWindow::makePipesManhattan);
  connect(prefsWindow_->euclideanPipesRadioButton_, &QPushButton::clicked, this, &SCIRunMainWindow::makePipesEuclidean);
  connect(prefsWindow_->maxCoresSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &SCIRunMainWindow::maxCoreValueChanged);
  //TODO: will be a user or network setting
  makePipesEuclidean();

  connect(moduleFilterLineEdit_, &QLineEdit::textChanged, this, &SCIRunMainWindow::filterModuleNamesInTreeView);

  connect(prefsWindow_->modulesSnapToCheckBox_, &QCheckBox::stateChanged, this, &SCIRunMainWindow::modulesSnapToChanged);
  connect(prefsWindow_->modulesSnapToCheckBox_, &QCheckBox::stateChanged, networkEditor_, &NetworkEditor::snapToModules);
  connect(prefsWindow_->portSizeEffectsCheckBox_, &QCheckBox::stateChanged, this, &SCIRunMainWindow::highlightPortsChanged);
  connect(prefsWindow_->portSizeEffectsCheckBox_, &QCheckBox::stateChanged, networkEditor_, &NetworkEditor::highlightPorts);
  connect(prefsWindow_->dockableModulesCheckBox_, &QCheckBox::stateChanged, this, &SCIRunMainWindow::adjustModuleDock);

  makeFilterButtonMenu();

  connect(prefsWindow_->scirunDataPushButton_, &QPushButton::clicked, this, &SCIRunMainWindow::setDataDirectoryFromGUI);
  connect(prefsWindow_->screenshotPathPushButton_, &QPushButton::clicked, this, &SCIRunMainWindow::setScreenshotDirectoryFromGUI);
  //connect(prefsWindow_->addToPathButton_, &QPushButton::clicked, this, &SCIRunMainWindow::addToPathFromGUI);
  connect(actionFilter_modules_, &QAction::triggered, this, &SCIRunMainWindow::setFocusOnFilterLine);
  connect(actionAddModule_, &QAction::triggered, this, &SCIRunMainWindow::addModuleKeyboardAction);
  actionAddModule_->setVisible(false);
  connect(actionSelectModule_, &QAction::triggered, this, &SCIRunMainWindow::selectModuleKeyboardAction);
  actionSelectModule_->setVisible(false);
  connect(actionReportIssue_, &QAction::triggered, this, &SCIRunMainWindow::reportIssue);
  connect(actionSelectMode_, &QAction::toggled, this, &SCIRunMainWindow::setSelectMode);
  connect(actionDragMode_, &QAction::toggled, this, &SCIRunMainWindow::setDragMode);
	connect(actionToggleTagLayer_, &QAction::toggled, this, &SCIRunMainWindow::toggleTagLayer);
  connect(actionToggleMetadataLayer_, &QAction::toggled, this, &SCIRunMainWindow::toggleMetadataLayer);
  connect(actionResetNetworkZoom_, &QAction::triggered, this, &SCIRunMainWindow::zoomNetwork);
  connect(actionZoomIn_, &QAction::triggered, this, &SCIRunMainWindow::zoomNetwork);
  connect(actionZoomOut_, &QAction::triggered, this, &SCIRunMainWindow::zoomNetwork);
  connect(actionZoomBestFit_, &QAction::triggered, this, &SCIRunMainWindow::zoomNetwork);
  connect(actionStateViewer_, &QAction::triggered, [this]() { networkEditor_->showStateViewer(); });

  auto dimFunc = [this](const char* type)
  {
    return [this, type]()
    {
      showStatusMessage(QString("Dimming connections of type ") + type, NetworkEditor::ConnectionHideTimeMS_);
      networkEditor_->hidePipesByType(type);
    };
  };
  connect(actionHideFieldPipes_, &QAction::triggered, dimFunc("Field"));
  connect(actionHideMatrixPipes_, &QAction::triggered, dimFunc("Matrix"));
  connect(actionHideStringPipes_, &QAction::triggered, dimFunc("String"));
  connect(actionHideGeometryPipes_, &QAction::triggered, dimFunc("Geometry"));
  connect(actionHideColorMapPipes_, &QAction::triggered, dimFunc("ColorMap"));

  actionCut_->setIcon(QPixmap(":/general/Resources/cut.png"));
  actionCopy_->setIcon(QPixmap(":/general/Resources/copy.png"));
  actionPaste_->setIcon(QPixmap(":/general/Resources/paste.png"));

  connect(actionKeyboardShortcuts_, &QAction::triggered, this, &SCIRunMainWindow::showKeyboardShortcutsDialog);

  //TODO: store in xml file, add to app resources
  {
    ToolkitInfo fwdInv{ "http://www.sci.utah.edu/images/software/forward-inverse/forward-inverse-mod.png",
#ifdef __APPLE__
      "https://codeload.github.com/SCIInstitute/FwdInvToolkit/zip/v1.4.3",
#else
      "http://sci.utah.edu/devbuilds/scirun5/toolkits/FwdInvToolkit_v1.4.3.zip",
#endif
      "FwdInvToolkit_stable.zip" };
    fwdInv.setupAction(actionForwardInverseStable_, this);
  }
  {
    ToolkitInfo fwdInvNightly{ "http://www.sci.utah.edu/images/software/forward-inverse/forward-inverse-mod.png",
      "https://codeload.github.com/SCIInstitute/FwdInvToolkit/zip/master",
      "FwdInvToolkit_nightly.zip" };
    fwdInvNightly.setupAction(actionForwardInverseNightly_, this);
  }
  {
    ToolkitInfo brainStim{ "http://www.sci.utah.edu/images/software/BrainStimulator/brain-stimulator-mod.png",
    #ifdef __APPLE__
      "https://codeload.github.com/SCIInstitute/BrainStimulator/zip/BrainStimulator_v1.3",
    #else
      "http://sci.utah.edu/devbuilds/scirun5/toolkits/BrainStimulator_v1.3.zip",
    #endif
      "BrainStimulator_stable.zip" };
    brainStim.setupAction(actionBrainStimulatorStable_, this);
  }
  {
    ToolkitInfo brainStimNightly{ "http://www.sci.utah.edu/images/software/BrainStimulator/brain-stimulator-mod.png",
      "https://codeload.github.com/SCIInstitute/BrainStimulator/zip/master",
      "BrainStimulator_nightly.zip" };
    brainStimNightly.setupAction(actionBrainStimulatorNightly_, this);
  }
  connect(actionLoadToolkit_, &QAction::triggered, this, &SCIRunMainWindow::loadToolkit);

  connect(networkEditor_, &NetworkEditor::networkExecuted, networkProgressBar_.get(), &NetworkExecutionProgressBar::resetModulesDone);
  connect(networkEditor_->moduleEventProxy().get(), &ModuleEventProxy::moduleExecuteEnd, networkProgressBar_.get(), &NetworkExecutionProgressBar::incrementModulesDone);
  connect(networkEditor_, &NetworkEditor::networkExecuted, []() { DialogErrorControl::instance().resetCounter(); });
	connect(networkEditor_, &NetworkEditor::requestLoadNetwork, this, &SCIRunMainWindow::checkAndLoadNetworkFile);
  connect(networkEditor_, &NetworkEditor::networkExecuted, this, &SCIRunMainWindow::changeExecuteActionIconToStop);

  connect(prefsWindow_->actionTextIconCheckBox_, &QPushButton::clicked, this, &SCIRunMainWindow::adjustExecuteButtonAppearance);
  prefsWindow_->actionTextIconCheckBox_->setCheckState(Qt::PartiallyChecked);
  adjustExecuteButtonAppearance();

  connect(networkEditor_, &NetworkEditor::newSubnetworkCopied, this, &SCIRunMainWindow::updateClipboardHistory);
  connect(networkEditor_, &NetworkEditor::middleMouseClicked, this, &SCIRunMainWindow::switchMouseMode);

  connect(openLogFolderButton_, &QPushButton::clicked, this, &SCIRunMainWindow::openLogFolder);

  setupDockToggleViewAction(moduleSelectorDockWidget_, "Ctrl+Shift+M");
  setupDockToggleViewAction(userModuleSelectorDockWidget_, "Ctrl+Shift+U");
  setupDockToggleViewAction(logDockWidget_, "Ctrl+Shift+L");
  setupDockToggleViewAction(networkMiniViewDockWidget_, "Ctrl+Shift+V");
  setupDockToggleViewAction(provenanceWindow_, "Ctrl+Shift+P");
  setupDockToggleViewAction(triggeredEventsWindow_, "Ctrl+Shift+E");
  setupDockToggleViewAction(tagManagerWindow_, "Ctrl+Shift+T");
  setupDockToggleViewAction(macroEditor_, "Ctrl+Shift+X");
  #ifdef BUILD_WITH_PYTHON
  setupDockToggleViewAction(pythonConsole_, "Ctrl+Shift+Y");
  #endif

  setupInputWidgets();

  logTextBrowser_->append("Hello! Welcome to SCIRun 5.");

  readSettings();

  setCurrentFile("");

  actionTagManager_->setChecked(!tagManagerWindow_->isHidden());

  connect(actionAutoRotateViewScene_, &QAction::triggered, prefsWindow_->autoRotateViewerOnMouseReleaseCheckbox_, &QCheckBox::toggle);
  connect(actionAutoRotateViewScene_, &QAction::triggered, [this]() { showStatusMessage(QString("ViewScene auto-rotate toggled."), 3000); });

  moduleSelectorDockWidget_->setStyleSheet("QDockWidget {background: rgb(66,66,69); background-color: rgb(66,66,69) }"
	  "QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }"
	  "QHeaderView::section { background: rgb(66,66,69);} "
	  );

  hideNonfunctioningWidgets();

  connect(moduleSelectorDockWidget_, &QDockWidget::topLevelChanged, this, &SCIRunMainWindow::updateDockWidgetProperties);

  setupVersionButton();

  WidgetStyleMixin::tabStyle(optionsTabWidget_);
}

const QString SCIRunMainWindow::saveFragmentData_("fragmentTree");

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
  commandConverter_.reset();
  networkEditor_->disconnect();
  networkEditor_->setNetworkEditorController(nullptr);
  networkEditor_->clear();
  Application::Instance().shutdown();
}

void SCIRunMainWindow::executeCommandLineRequests()
{
  Application::Instance().executeCommandLineRequests();
}

void SCIRunMainWindow::networkTimedOut()
{
	exitApplication(2);
}

void SCIRunMainWindow::setupDockToggleViewAction(QDockWidget* dock, const QString& shortcut)
{
  auto tva = dock->toggleViewAction();
  tva->setShortcut(QKeySequence(shortcut));
  menuWindow->addAction(tva);
}

QString SCIRunMainWindow::strippedName(const QString& fullFileName)
{
  QFileInfo info(fullFileName);
  return info.fileName();
}

namespace 
{
bool fileExistCheck(const std::string& filename)
{
  bool fileExists;
  //TODO: boost upgrade to 1.80 should remove the need for the try/catch--see issue #2407
  try
  {
    fileExists = boost::filesystem::exists(filename);
  }
  catch (...)
  {
    fileExists = false;
  }
  return fileExists;
}

bool superFileExistCheck(const std::string& filename)
{
  auto check = std::async([filename]() { return fileExistCheck(filename); });
  auto status = check.wait_for(std::chrono::seconds(1));
  if (status == std::future_status::ready)
    return check.get();
  return false;
}
}

void SCIRunMainWindow::updateRecentFileActions()
{
  QMutableStringListIterator i(recentFiles_);
  while (i.hasNext()) 
  {
    const auto file = i.next().toStdString();
    
    if (!superFileExistCheck(file))
    {
      logWarning("Network file {} not found, removing entry from recent list.", file);
      i.remove();
    }
  }

  for (int j = 0; j < MaxRecentFiles; ++j)
  {
    if (j < recentFiles_.count())
    {
      auto text = tr("&%1 %2")
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

bool SCIRunMainWindow::okToContinue()
{
  if (isWindowModified()
		&& !Application::Instance().parameters()->isRegressionMode()
		&& !quitAfterExecute_
		&& !skipSaveCheck_)
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

QString SCIRunMainWindow::mostRecentFile() const
{
  return !recentFiles_.empty() ? recentFiles_[0] : "";
}

template <class T>
QString idFromPointer(T* item)
{
  QString addressString;
  QTextStream addressStream(&addressString);
  addressStream << static_cast<const void*>(item);
  addressStream.flush();
  return addressString;
}

void SCIRunMainWindow::setupSubnetItem(QTreeWidgetItem* fave, bool addToMap, const QString& idFromMap)
{
  auto id = addToMap ? idFromPointer(fave) + "::" + fave->text(0) : idFromMap;
  fave->setData(0, Qt::UserRole, id);

  if (addToMap)
  {
    savedSubnetworksXml_[id] = fave->data(0, clipboardKey).toString();
    savedSubnetworksNames_[id] = fave->text(0);
    fave->setFlags(fave->flags() & ~Qt::ItemIsEditable);
  }
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

void SCIRunMainWindow::showStatusMessage(const QString& str)
{
	statusBar()->showMessage(str);
}

void SCIRunMainWindow::showStatusMessage(const QString& str, int timeInMsec)
{
	statusBar()->showMessage(str, timeInMsec);
}
