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
#include <Interface/qt_include.h>
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
#include <Dataflow/Serialization/Network/Importer/NetworkIO.h>
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

SCIRunMainWindow::SCIRunMainWindow()
{
  setupUi(this);
  builder_ = boost::make_shared<NetworkEditorBuilder>(this);
  dockManager_ = new DockManager(dockSpace_, this);

  {
    boost::shared_ptr<TextEditAppender> logger(new TextEditAppender(logTextBrowser_));
    GuiLog::Instance().addCustomSink(logger);
  }

  startup_ = true;

  QCoreApplication::setOrganizationName("SCI:CIBC Software");
  QCoreApplication::setApplicationName("SCIRun5");

  setAttribute(Qt::WA_DeleteOnClose);

  menubar_->setStyleSheet("QMenuBar::item::selected{background-color : rgb(66, 66, 69); } QMenuBar::item::!selected{ background-color : rgb(66, 66, 69); } ");

  dialogErrorControl_.reset(new DialogErrorControl(this));
  setupTagManagerWindow();

  setupPreferencesWindow();

  setupNetworkEditor();

  setTipsAndWhatsThis();

  connect(actionExecuteAll_, SIGNAL(triggered()), this, SLOT(executeAll()));
  connect(actionNew_, SIGNAL(triggered()), this, SLOT(newNetwork()));

  setActionIcons();

  createStandardToolbars();
  createExecuteToolbar();
  createAdvancedToolbar();
  createMacroToolbar();

  #ifdef __APPLE__
  connect(actionLaunchNewInstance_, SIGNAL(triggered()), this, SLOT(launchNewInstance()));
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
    connect(actionSearchBar_, SIGNAL(toggled(bool)), searchBar, SLOT(setVisible(bool)));
    connect(searchBar, SIGNAL(visibilityChanged(bool)), actionSearchBar_, SLOT(setChecked(bool)));
    searchBar->setVisible(false);
  }

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

  connect(actionRunMacro1_, SIGNAL(triggered()), this, SLOT(runMacro()));
  connect(actionRunMacro2_, SIGNAL(triggered()), this, SLOT(runMacro()));
  connect(actionRunMacro3_, SIGNAL(triggered()), this, SLOT(runMacro()));
  connect(actionRunMacro4_, SIGNAL(triggered()), this, SLOT(runMacro()));
  connect(actionRunMacro5_, SIGNAL(triggered()), this, SLOT(runMacro()));

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

	connect(actionRunNewModuleWizard_, SIGNAL(triggered()), this, SLOT(runNewModuleWizard()));
	actionRunNewModuleWizard_->setDisabled(true);

  connect(actionAbout_, SIGNAL(triggered()), this, SLOT(displayAcknowledgement()));
  connect(actionCreateToolkitFromDirectory_, SIGNAL(triggered()), this, SLOT(helpWithToolkitCreation()));

  connect(helpActionPythonAPI_, SIGNAL(triggered()), this, SLOT(loadPythonAPIDoc()));
  connect(helpActionModuleFuzzySearch_, SIGNAL(triggered()), this, SLOT(showModuleFuzzySearchHelp()));
  connect(helpActionSnippets_, SIGNAL(triggered()), this, SLOT(showSnippetHelp()));
  connect(helpActionClipboard_, SIGNAL(triggered()), this, SLOT(showClipboardHelp()));
	connect(helpActionTagLayer_, SIGNAL(triggered()), this, SLOT(showTagHelp()));
	connect(helpActionTriggeredScripts_, SIGNAL(triggered()), this, SLOT(showTriggerHelp()));
  connect(helpActionNewUserWizard_, SIGNAL(triggered()), this, SLOT(launchNewUserWizard()));
  connect(helpActionPythonWizard_, SIGNAL(triggered()), this, SLOT(launchPythonWizard()));

  connect(actionReset_Window_Layout, SIGNAL(triggered()), this, SLOT(resetWindowLayout()));
  connect(actionToggleFullScreenMode_, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));

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

	setupScriptedEventsWindow();
  setupProvenanceWindow();

  setupDevConsole();
  setupPythonConsole();

  connect(prefsWindow_->defaultNotePositionComboBox_, SIGNAL(activated(int)), this, SLOT(readDefaultNotePosition(int)));
  connect(prefsWindow_->defaultNoteSizeComboBox_, SIGNAL(activated(int)), this, SLOT(readDefaultNoteSize(int)));
  connect(prefsWindow_->cubicPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesCubicBezier()));
  connect(prefsWindow_->manhattanPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesManhattan()));
  connect(prefsWindow_->euclideanPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesEuclidean()));
  connect(prefsWindow_->maxCoresSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(maxCoreValueChanged(int)));
  //TODO: will be a user or network setting
  makePipesEuclidean();

  connect(moduleFilterLineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(filterModuleNamesInTreeView(const QString&)));

  connect(prefsWindow_->modulesSnapToCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(modulesSnapToChanged()));
  connect(prefsWindow_->modulesSnapToCheckBox_, SIGNAL(stateChanged(int)), networkEditor_, SIGNAL(snapToModules()));
  connect(prefsWindow_->portSizeEffectsCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(highlightPortsChanged()));
  connect(prefsWindow_->portSizeEffectsCheckBox_, SIGNAL(stateChanged(int)), networkEditor_, SIGNAL(highlightPorts(int)));
  connect(prefsWindow_->dockableModulesCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(adjustModuleDock(int)));

  makeFilterButtonMenu();

  connect(prefsWindow_->scirunDataPushButton_, SIGNAL(clicked()), this, SLOT(setDataDirectoryFromGUI()));
  //connect(prefsWindow_->addToPathButton_, SIGNAL(clicked()), this, SLOT(addToPathFromGUI()));
  connect(actionFilter_modules_, SIGNAL(triggered()), this, SLOT(setFocusOnFilterLine()));
  connect(actionAddModule_, SIGNAL(triggered()), this, SLOT(addModuleKeyboardAction()));
  actionAddModule_->setVisible(false);
  connect(actionSelectModule_, SIGNAL(triggered()), this, SLOT(selectModuleKeyboardAction()));
  actionSelectModule_->setVisible(false);
  connect(actionReportIssue_, SIGNAL(triggered()), this, SLOT(reportIssue()));
  connect(actionSelectMode_, SIGNAL(toggled(bool)), this, SLOT(setSelectMode(bool)));
  connect(actionDragMode_, SIGNAL(toggled(bool)), this, SLOT(setDragMode(bool)));
	connect(actionToggleTagLayer_, SIGNAL(toggled(bool)), this, SLOT(toggleTagLayer(bool)));
  connect(actionToggleMetadataLayer_, SIGNAL(toggled(bool)), this, SLOT(toggleMetadataLayer(bool)));
  connect(actionResetNetworkZoom_, SIGNAL(triggered()), this, SLOT(zoomNetwork()));
  connect(actionZoomIn_, SIGNAL(triggered()), this, SLOT(zoomNetwork()));
  connect(actionZoomOut_, SIGNAL(triggered()), this, SLOT(zoomNetwork()));
  connect(actionZoomBestFit_, SIGNAL(triggered()), this, SLOT(zoomNetwork()));
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

  connect(actionKeyboardShortcuts_, SIGNAL(triggered()), this, SLOT(showKeyboardShortcutsDialog()));

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
  connect(actionLoadToolkit_, SIGNAL(triggered()), this, SLOT(loadToolkit()));

  connect(networkEditor_, SIGNAL(networkExecuted()), networkProgressBar_.get(), SLOT(resetModulesDone()));
  connect(networkEditor_->moduleEventProxy().get(), SIGNAL(moduleExecuteEnd(double, const std::string&)), networkProgressBar_.get(), SLOT(incrementModulesDone(double, const std::string&)));
  connect(networkEditor_, SIGNAL(networkExecuted()), dialogErrorControl_.get(), SLOT(resetCounter()));
	connect(networkEditor_, SIGNAL(requestLoadNetwork(const QString&)), this, SLOT(checkAndLoadNetworkFile(const QString&)));
  connect(networkEditor_, SIGNAL(networkExecuted()), this, SLOT(changeExecuteActionIconToStop()));

  connect(prefsWindow_->actionTextIconCheckBox_, SIGNAL(clicked()), this, SLOT(adjustExecuteButtonAppearance()));
  prefsWindow_->actionTextIconCheckBox_->setCheckState(Qt::PartiallyChecked);
  adjustExecuteButtonAppearance();

  connect(networkEditor_, SIGNAL(newSubnetworkCopied(const QString&)), this, SLOT(updateClipboardHistory(const QString&)));
  connect(networkEditor_, SIGNAL(middleMouseClicked()), this, SLOT(switchMouseMode()));

  connect(openLogFolderButton_, SIGNAL(clicked()), this, SLOT(openLogFolder()));

  setupInputWidgets();

  logTextBrowser_->append("Hello! Welcome to SCIRun 5.");
  readSettings();

  setCurrentFile("");

  actionConfiguration_->setChecked(!configurationDockWidget_->isHidden());
  actionModule_Selector->setChecked(!moduleSelectorDockWidget_->isHidden());
  actionProvenance_->setChecked(!provenanceWindow_->isHidden());
  actionTriggeredEvents_->setChecked(!triggeredEventsWindow_->isHidden());
  actionTagManager_->setChecked(!tagManagerWindow_->isHidden());
  actionMiniview_->setChecked(!networkMiniViewDockWidget_->isHidden());

	moduleSelectorDockWidget_->setStyleSheet("QDockWidget {background: rgb(66,66,69); background-color: rgb(66,66,69) }"
		"QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }"
		"QHeaderView::section { background: rgb(66,66,69);} "
		);

  hideNonfunctioningWidgets();

  connect(moduleSelectorDockWidget_, SIGNAL(topLevelChanged(bool)), this, SLOT(updateDockWidgetProperties(bool)));

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
