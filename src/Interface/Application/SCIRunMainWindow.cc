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
#include <algorithm>
#include <functional>
#include <boost/bind.hpp>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/DeveloperConsole.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/Preferences.h>
#include <Interface/Application/PythonConsoleWidget.h>
#include <Interface/Application/TreeViewCollaborators.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/GuiCommandFactory.h>
#include <Interface/Application/GuiCommands.h>
#include <Core/Logging/Logger.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/NetworkExecutionProgressBar.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h> //DOH! see TODO in setController
#include <Dataflow/Engine/Controller/ProvenanceManager.h>
#include <Core/Application/Application.h>

#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>

#include <Core/Command/CommandFactory.h>
#include <Core/Python/PythonInterpreter.h>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Commands;

SCIRunMainWindow* SCIRunMainWindow::instance_ = 0;

SCIRunMainWindow* SCIRunMainWindow::Instance()
{
  if (!instance_)
  {
    instance_ = new SCIRunMainWindow;
  }
  return instance_;
}

void SCIRunMainWindow::setController(SCIRun::Dataflow::Engine::NetworkEditorControllerHandle controller)
{
  boost::shared_ptr<NetworkEditorControllerGuiProxy> controllerProxy(new NetworkEditorControllerGuiProxy(controller));
  networkEditor_->setNetworkEditorController(controllerProxy);
  //TODO: need better way to wire this up
  controller->setModulePositionEditor(networkEditor_);
}

void SCIRunMainWindow::setupNetworkEditor()
{
  boost::shared_ptr<TreeViewModuleGetter> getter(new TreeViewModuleGetter(*moduleSelectorTreeWidget_));
  Core::Logging::LoggerHandle logger(new TextEditAppender(logTextBrowser_));
  GuiLogger::setInstance(logger);
  defaultNotePositionGetter_.reset(new ComboBoxDefaultNotePositionGetter(*defaultNotePositionComboBox_));
  networkEditor_ = new NetworkEditor(getter, defaultNotePositionGetter_, scrollAreaWidgetContents_);
  networkEditor_->setObjectName(QString::fromUtf8("networkEditor_"));
  networkEditor_->setContextMenuPolicy(Qt::ActionsContextMenu);
  networkEditor_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->verticalScrollBar()->setValue(0);
  networkEditor_->horizontalScrollBar()->setValue(0);
}

SCIRunMainWindow::SCIRunMainWindow()
{
	setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  
  setupNetworkEditor();

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

  connect(actionExecute_All_, SIGNAL(triggered()), networkEditor_, SLOT(executeAll()));
  connect(actionNew_, SIGNAL(triggered()), this, SLOT(newNetwork()));
  connect(networkEditor_, SIGNAL(modified()), this, SLOT(networkModified()));

  connect(defaultNotePositionComboBox_, SIGNAL(activated(int)), this, SLOT(readDefaultNotePosition(int)));
  connect(this, SIGNAL(defaultNotePositionChanged(NotePosition)), networkEditor_, SIGNAL(defaultNotePositionChanged(NotePosition)));
    
  gridLayout_5->addWidget(networkEditor_, 0, 0, 1, 1);
	
	QWidgetAction* moduleSearchAction = new QWidgetAction(this);
	moduleSearchAction->setDefaultWidget(new QLineEdit(this));

#if 0
  {
    //TODO!!!!
    moduleSearchAction->setVisible(true);

    QToolBar* f = addToolBar(tr("&Search"));

    QWidgetAction* showModuleLabel = new QWidgetAction(this);
    showModuleLabel->setDefaultWidget(new QLabel("Module Search:", this));
    showModuleLabel->setVisible(true);
    
    f->addAction(showModuleLabel);
    f->addAction(moduleSearchAction);
  }
#endif

  setActionIcons();

  QToolBar* standardBar = addToolBar("Standard");
  standardBar->addAction(actionNew_);
  standardBar->addAction(actionLoad_);
  standardBar->addAction(actionSave_);
  standardBar->addAction(actionRunScript_);
  standardBar->addAction(actionEnterWhatsThisMode_);

  QToolBar* executeBar = addToolBar(tr("&Execute"));
	executeBar->addAction(actionExecute_All_);
	
	networkProgressBar_.reset(new NetworkExecutionProgressBar(this));
  executeBar->addActions(networkProgressBar_->actions());
  connect(actionExecute_All_, SIGNAL(triggered()), networkProgressBar_.get(), SLOT(resetModulesDone()));
  connect(networkEditor_->moduleEventProxy().get(), SIGNAL(moduleExecuteEnd(const std::string&)), networkProgressBar_.get(), SLOT(incrementModulesDone()));
	
	scrollAreaWidgetContents_->addAction(actionExecute_All_);
  auto sep = new QAction(this);
  sep->setSeparator(true);
  scrollAreaWidgetContents_->addAction(sep);
	scrollAreaWidgetContents_->addActions(networkEditor_->getModuleSpecificActions());
  scrollAreaWidgetContents_->setContextMenuPolicy(Qt::ActionsContextMenu);

	scrollArea_->viewport()->setBackgroundRole(QPalette::Dark);
	scrollArea_->viewport()->setAutoFillBackground(true);	

	logTextBrowser_->setText("Hello! Welcome to the SCIRun5 Prototype.");

  GrabNameAndSetFlags visitor;
  visitTree(moduleSelectorTreeWidget_, visitor);
  //std::for_each(visitor.nameList_.begin(), visitor.nameList_.end(), boost::bind(&GuiLogger::log, boost::ref(GuiLogger::Instance()), _1));

  connect(actionSave_As_, SIGNAL(triggered()), this, SLOT(saveNetworkAs()));
  connect(actionSave_, SIGNAL(triggered()), this, SLOT(saveNetwork()));
  connect(actionLoad_, SIGNAL(triggered()), this, SLOT(loadNetwork()));
  connect(actionQuit_, SIGNAL(triggered()), this, SLOT(close()));
  connect(actionRunScript_, SIGNAL(triggered()), this, SLOT(runScript()));
  actionQuit_->setShortcut(QKeySequence::Quit);

#ifndef BUILD_WITH_PYTHON
  actionRunScript_->setEnabled(false);
#endif

  connect(cubicPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesCubicBezier()));
  connect(manhattanPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesManhattan()));
  connect(euclideanPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesEuclidean()));
  //TODO: will be a user or network setting
  makePipesEuclidean();
  
  for (int i = 0; i < MaxRecentFiles; ++i) 
  {
    recentFileActions_.push_back(new QAction(this));
    recentFileActions_[i]->setVisible(false);
    recentNetworksMenu_->addAction(recentFileActions_[i]);
    connect(recentFileActions_[i], SIGNAL(triggered()), this, SLOT(loadRecentNetwork()));
  }

  setupPreferencesWindow();
  readSettings();

  setCurrentFile("");

  moduleSelectorTreeWidget_->expandAll();
  moduleSelectorTreeWidget_->resizeColumnToContents(0);
  moduleSelectorTreeWidget_->resizeColumnToContents(1);
  connect(moduleSelectorTreeWidget_, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(filterDoubleClickedModuleSelectorItem(QTreeWidgetItem*)));
  connect(this, SIGNAL(moduleItemDoubleClicked()), networkEditor_, SLOT(addModuleViaDoubleClickedTreeItem()));
  connect(moduleFilterLineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(filterModuleNamesInTreeView(const QString&)));
  
  connect(chooseBackgroundColorButton_, SIGNAL(clicked()), this, SLOT(chooseBackgroundColor()));
  connect(resetBackgroundColorButton_, SIGNAL(clicked()), this, SLOT(resetBackgroundColor()));
  
  setupProvenanceWindow();
  setupDevConsole();
  setupPythonConsole();

  makeFilterButtonMenu();
  
  connect(networkEditor_, SIGNAL(sceneChanged(const QList<QRectF>&)), this, SLOT(updateMiniView()));
  connect(networkEditor_->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateMiniView()));
  connect(networkEditor_->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateMiniView()));
}

void SCIRunMainWindow::initialize()
{
  postConstructionSignalHookup();

  executeCommandLineRequests();
}

void SCIRunMainWindow::postConstructionSignalHookup()
{
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(executionStarted()), this, SLOT(disableInputWidgets()));
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(executionFinished(int)), this, SLOT(enableInputWidgets()));

  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(moduleRemoved(const SCIRun::Dataflow::Networks::ModuleId&)), 
    networkEditor_, SLOT(removeModuleWidget(const SCIRun::Dataflow::Networks::ModuleId&)));

  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(moduleAdded(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle)), 
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

  prefs_->setRegressionTestDataDir();
}

void SCIRunMainWindow::executeCommandLineRequests()
{
  SCIRun::Core::Application::Instance().executeCommandLineRequests(boost::make_shared<GuiGlobalCommandFactory>());
}

void SCIRunMainWindow::executeAll()
{
  networkEditor_->executeAll();
}

void SCIRunMainWindow::setupQuitAfterExecute()
{
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(executionFinished(int)), this, SLOT(exitApplication(int)));
  prefs_->setRegressionMode(true);
}

void SCIRunMainWindow::exitApplication(int code)
{
  close(); 
  /*qApp->*/exit(code);
}

void SCIRunMainWindow::quit()
{
  exitApplication(0);
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
  NetworkFileHandle file = networkEditor_->saveNetwork();

  XMLSerializer::save_xml(*file, fileName.toStdString(), "networkFile");
  setCurrentFile(fileName);

  statusBar()->showMessage(tr("File saved"), 2000);
  GuiLogger::Instance().log("File save done.");
  setWindowModified(false);
}

void SCIRunMainWindow::loadNetwork()
{
  if (okToContinue())
  {
    QString filename = QFileDialog::getOpenFileName(this, "Load Network...", latestNetworkDirectory_.path(), "*.srn5");
    loadNetworkFile(filename);
  }
}

void SCIRunMainWindow::loadNetworkFile(const QString& filename)
{
  if (!filename.isEmpty())
  {
    FileOpenCommand command(filename.toStdString(), networkEditor_);
    if (command.execute())
    {
      setCurrentFile(filename);
      statusBar()->showMessage(tr("File loaded"), 2000);
      networkProgressBar_->updateTotalModules(networkEditor_->numModules());
      provenanceWindow_->clear();
      provenanceWindow_->showFile(command.openedFile_);
    }
  }
}

bool SCIRunMainWindow::newNetwork()
{
  if (okToContinue())
  {
    networkEditor_->clear();
    setCurrentFile("");
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
  setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("SCIRun 5 Prototype")));
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
  if (isWindowModified() && !prefs_->isRegression())  //TODO: regressionMode
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

void SCIRunMainWindow::networkModified()
{
  setWindowModified(true);
  networkProgressBar_->updateTotalModules(networkEditor_->numModules());
}

void SCIRunMainWindow::setActionIcons() 
{
  actionNew_->setIcon(QPixmap(":/general/Resources/new.png"));
  actionLoad_->setIcon(QPixmap(":/general/Resources/load.png"));
  actionSave_->setIcon(QPixmap(":/general/Resources/save.png"));
  actionRunScript_->setIcon(QPixmap(":/general/Resources/script.png"));
  //actionSave_As_->setIcon(QApplication::style()->standardIcon(QStyle::SP_DriveCDIcon));  //TODO?
  actionExecute_All_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
  actionUndo_->setIcon(QIcon::fromTheme("edit-undo"));
  actionRedo_->setIcon(QIcon::fromTheme("edit-redo"));
  //actionCut_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
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
  startsWithAction->setChecked(true);
  filterActionGroup_->addAction(startsWithAction);
  filterMenu->addAction(startsWithAction);

  auto wildcardAction = new QAction("Use wildcards", filterButton_);
  wildcardAction->setCheckable(true);
  filterActionGroup_->addAction(wildcardAction);
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

void SCIRunMainWindow::readSettings()
{
  QSettings settings("SCI:CIBC Software", "SCIRun5");

  latestNetworkDirectory_ = settings.value("networkDirectory").toString();
  GuiLogger::Instance().log("Setting read: default network directory = " + latestNetworkDirectory_.path());

  recentFiles_ = settings.value("recentFiles").toStringList();
  updateRecentFileActions();
  GuiLogger::Instance().log("Setting read: recent network file list");

  QString regressionTestDataDir = settings.value("regressionTestDataDirectory").toString();
  GuiLogger::Instance().log("Setting read: regression test data directory = " + regressionTestDataDir);
  prefs_->setRegressionTestDataDir(regressionTestDataDir);

  //TODO: make a separate class for these keys, bad duplication.
  const QString colorKey = "backgroundColor";
  if (settings.contains(colorKey))
  {
    networkEditor_->setBackground(QColor(settings.value(colorKey).toString()));
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
}

void SCIRunMainWindow::writeSettings()
{
  QSettings settings("SCI:CIBC Software", "SCIRun5");

  settings.setValue("networkDirectory", latestNetworkDirectory_.path());
  settings.setValue("recentFiles", recentFiles_);
  settings.setValue("regressionTestDataDirectory", prefs_->regressionTestDataDir());
  settings.setValue("backgroundColor", networkEditor_->background().color().name());
  settings.setValue("defaultNotePositionIndex", defaultNotePositionComboBox_->currentIndex());
  settings.setValue("connectionPipeType", networkEditor_->connectionPipelineType());
}

void SCIRunMainWindow::disableInputWidgets()
{
  actionExecute_All_->setDisabled(true);
  actionSave_->setDisabled(true);
  actionLoad_->setDisabled(true);
  actionSave_As_->setDisabled(true);
  actionNew_->setDisabled(true);
  moduleSelectorTreeWidget_->setDisabled(true);
  networkEditor_->disableInputWidgets();
  scrollAreaWidgetContents_->setContextMenuPolicy(Qt::NoContextMenu);
  
  Q_FOREACH(QAction* action, recentNetworksMenu_->actions())
    action->setDisabled(true);

#ifdef BUILD_WITH_PYTHON
  pythonConsole_->setDisabled(true);
#endif
}

void SCIRunMainWindow::enableInputWidgets()
{
  actionExecute_All_->setEnabled(true);
  actionSave_->setEnabled(true);
  actionLoad_->setEnabled(true);
  actionSave_As_->setEnabled(true);
  actionNew_->setEnabled(true);
  moduleSelectorTreeWidget_->setEnabled(true);
  networkEditor_->enableInputWidgets();
  scrollAreaWidgetContents_->setContextMenuPolicy(Qt::ActionsContextMenu);

  Q_FOREACH(QAction* action, recentNetworksMenu_->actions())
    action->setEnabled(true);

#ifdef BUILD_WITH_PYTHON
  pythonConsole_->setDisabled(false);
#endif
}

void SCIRunMainWindow::chooseBackgroundColor()
{
  auto brush = networkEditor_->background();
  auto oldColor = brush.color();

  auto newColor = QColorDialog::getColor(oldColor, this, "Choose background color");
  if (newColor.isValid())
  {
    networkEditor_->setBackground(newColor);
    GuiLogger::Instance().log("Background color set to " + newColor.name());
  }
}

void SCIRunMainWindow::resetBackgroundColor()
{
  //TODO: standardize these defaults
  QColor defaultColor(Qt::darkGray);
  networkEditor_->setBackground(defaultColor);
  GuiLogger::Instance().log("Background color set to " + defaultColor.name());
}

void SCIRunMainWindow::setupProvenanceWindow()
{
  ProvenanceManagerHandle provenanceManager(new Dataflow::Engine::ProvenanceManager<SCIRun::Dataflow::Networks::NetworkFileHandle>(networkEditor_));
  provenanceWindow_ = new ProvenanceWindow(provenanceManager, this);
  connect(actionProvenance_, SIGNAL(toggled(bool)), provenanceWindow_, SLOT(setVisible(bool)));
  connect(provenanceWindow_, SIGNAL(visibilityChanged(bool)), actionProvenance_, SLOT(setChecked(bool)));

  provenanceWindow_->setVisible(false);
  provenanceWindow_->setFloating(true);
  addDockWidget(Qt::RightDockWidgetArea, provenanceWindow_);

  connect(actionUndo_, SIGNAL(triggered()), provenanceWindow_, SLOT(undo()));
  connect(actionRedo_, SIGNAL(triggered()), provenanceWindow_, SLOT(redo()));
  actionUndo_->setEnabled(false);
  actionRedo_->setEnabled(false);
  connect(provenanceWindow_, SIGNAL(undoStateChanged(bool)), actionUndo_, SLOT(setEnabled(bool)));
  connect(provenanceWindow_, SIGNAL(redoStateChanged(bool)), actionRedo_, SLOT(setEnabled(bool)));

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
}

void SCIRunMainWindow::setExecutor(int type)
{
  std::cout << "Executor of type " << type << " selected"  << std::endl;
  networkEditor_->getNetworkEditorController()->setExecutorType(type);
}

void SCIRunMainWindow::readDefaultNotePosition(int index)
{
  Q_EMIT defaultNotePositionChanged(defaultNotePositionGetter_->position()); //TODO: unit test.
}

void SCIRunMainWindow::setupPreferencesWindow()
{
  prefs_ = new PreferencesWindow(networkEditor_, this);

  connect(actionPreferences_, SIGNAL(triggered()), prefs_, SLOT(show()));
  //connect(prefs_, SIGNAL(visibilityChanged(bool)), actionPreferences_, SLOT(setChecked(bool)));
  prefs_->setVisible(false);
}

void SCIRunMainWindow::setupPythonConsole()
{
#ifdef BUILD_WITH_PYTHON
  pythonConsole_ = new PythonConsoleWidget(this);
  connect(actionPythonConsole_, SIGNAL(toggled(bool)), pythonConsole_, SLOT(setVisible(bool)));
  actionPythonConsole_->setIcon(QPixmap(":/general/Resources/terminal.png"));
  connect(pythonConsole_, SIGNAL(visibilityChanged(bool)), actionPythonConsole_, SLOT(setChecked(bool)));
  pythonConsole_->setVisible(false);
  pythonConsole_->setFloating(true);
  addDockWidget(Qt::TopDockWidgetArea, pythonConsole_);
#else
  actionPythonConsole_->setEnabled(false);
#endif
}

void SCIRunMainWindow::runPythonScript(const QString& scriptFileName)
{
#ifdef BUILD_WITH_PYTHON
  GuiLogger::Instance().log("RUNNING PYTHON SCRIPT: " + scriptFileName);
  SCIRun::Core::PythonInterpreter::Instance().run_file(scriptFileName.toStdString());
  statusBar()->showMessage(tr("Script is running."), 2000);
#else
  GuiLogger::Instance().log("Python not included in this build, cannot run " + scriptFileName);
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