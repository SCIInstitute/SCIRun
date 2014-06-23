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
#include <boost/assign.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <Core/Utils/Legacy/MemoryUtil.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/DeveloperConsole.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/PythonConsoleWidget.h>
#include <Interface/Application/TreeViewCollaborators.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/GuiCommandFactory.h>
#include <Interface/Application/GuiCommands.h>
#include <Core/Logging/LoggerInterface.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/NetworkExecutionProgressBar.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h> //DOH! see TODO in setController
#include <Dataflow/Engine/Controller/ProvenanceManager.h>
#include <Core/Application/Application.h>
#include <Core/Application/Preferences.h>
#include <Core/Logging/Log.h>

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
using namespace SCIRun::Core::Logging;

SCIRunMainWindow::SCIRunMainWindow() : firstTimePythonShown_(true)
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

  connect(actionExecute_All_, SIGNAL(triggered()), this, SLOT(executeAll()));
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

  //TODO???????
  setContextMenuPolicy(Qt::NoContextMenu);
  //scrollAreaWidgetContents_->setContextMenuPolicy(Qt::ActionsContextMenu);

	scrollArea_->viewport()->setBackgroundRole(QPalette::Dark);
	scrollArea_->viewport()->setAutoFillBackground(true);	

	logTextBrowser_->setText("Hello! Welcome to the SCIRun5 Prototype.");

 

  connect(actionSave_As_, SIGNAL(triggered()), this, SLOT(saveNetworkAs()));
  connect(actionSave_, SIGNAL(triggered()), this, SLOT(saveNetwork()));
  connect(actionLoad_, SIGNAL(triggered()), this, SLOT(loadNetwork()));
  connect(actionQuit_, SIGNAL(triggered()), this, SLOT(close()));
  connect(actionRunScript_, SIGNAL(triggered()), this, SLOT(runScript()));
  connect(actionSelectAll_, SIGNAL(triggered()), networkEditor_, SLOT(selectAll()));
  actionQuit_->setShortcut(QKeySequence::Quit);
  connect(actionDelete_, SIGNAL(triggered()), networkEditor_, SLOT(del()));
  actionDelete_->setShortcut(QKeySequence::Delete);

  connect(actionAbout_, SIGNAL(triggered()), this, SLOT(displayAcknowledgement()));

#ifndef BUILD_WITH_PYTHON
  actionRunScript_->setEnabled(false);
#endif

  connect(cubicPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesCubicBezier()));
  connect(manhattanPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesManhattan()));
  connect(euclideanPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesEuclidean()));
  //TODO: will be a user or network setting
  makePipesEuclidean();

  connect(largeModuleSizeRadioButton_, SIGNAL(clicked()), this, SLOT(makeModulesLargeSize()));
  connect(smallModuleSizeRadioButton_, SIGNAL(clicked()), this, SLOT(makeModulesSmallSize()));
  
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

  setupInputWidgets();
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

void SCIRunMainWindow::setupInputWidgets()
{
  using namespace boost::assign;
  inputWidgets_ += actionExecute_All_,
    actionSave_,
    actionLoad_,
    actionSave_As_,
    actionNew_,
    actionDelete_,
    moduleSelectorTreeWidget_,
    actionRunScript_;
  std::copy(recentFileActions_.begin(), recentFileActions_.end(), std::back_inserter(inputWidgets_));

#ifdef BUILD_WITH_PYTHON
  inputWidgets_ += pythonConsole_;
#endif
}

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
  //networkEditor_->setContextMenuPolicy(Qt::ActionsContextMenu);
  networkEditor_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->verticalScrollBar()->setValue(0);
  networkEditor_->horizontalScrollBar()->setValue(0);
}

void SCIRunMainWindow::executeCommandLineRequests()
{
  SCIRun::Core::Application::Instance().executeCommandLineRequests(boost::make_shared<GuiGlobalCommandFactory>());
}

void SCIRunMainWindow::executeAll()
{
  if (Core::Preferences::Instance().saveBeforeExecute)
  {
    saveNetwork();
  }

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
  std::string fileNameWithExtension = fileName.toStdString();
  if (!boost::algorithm::ends_with(fileNameWithExtension, ".srn5"))
    fileNameWithExtension += ".srn5";

  NetworkFileHandle file = networkEditor_->saveNetwork();

  XMLSerializer::save_xml(*file, fileNameWithExtension, "networkFile");
  setCurrentFile(QString::fromStdString(fileNameWithExtension));

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
    else
    {
      if (Core::Application::Instance().parameters()->isRegressionMode())
        exit(7);
      //TODO: set error code to non-0 so regression tests fail!
      // probably want to control this with a --regression flag.
    }
  }
}

bool SCIRunMainWindow::newNetwork()
{
  if (okToContinue())
  {
    networkEditor_->clear();
    provenanceWindow_->clear();
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

//TODO: hook up to modules' state_changed_sig_t via GlobalStateManager
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

  const QString disableModuleErrorDialogsKey = "disableModuleErrorDialogs";
  if (settings.contains(disableModuleErrorDialogsKey))
  {
    bool disableModuleErrorDialogs = settings.value(disableModuleErrorDialogsKey).toBool();
    GuiLogger::Instance().log("Setting read: disable module error dialogs = " + QString::number(disableModuleErrorDialogs));
    prefs_->setDisableModuleErrorDialogs(disableModuleErrorDialogs);
  }

  const QString saveBeforeExecute = "saveBeforeExecute";
  if (settings.contains(saveBeforeExecute))
  {
    bool mode = settings.value(saveBeforeExecute).toBool();
    GuiLogger::Instance().log("Setting read: save before execute = " + QString::number(mode));
    prefs_->setSaveBeforeExecute(mode);
  }

  const QString newViewSceneMouseControls = "newViewSceneMouseControls";
  if (settings.contains(newViewSceneMouseControls))
  {
    bool mode = settings.value(newViewSceneMouseControls).toBool();
    GuiLogger::Instance().log("Setting read: newViewSceneMouseControls = " + QString::number(mode));
    Core::Preferences::Instance().useNewViewSceneMouseControls = mode;
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
  settings.setValue("disableModuleErrorDialogs", prefs_->disableModuleErrorDialogs());
  settings.setValue("saveBeforeExecute", prefs_->saveBeforeExecute());
  settings.setValue("newViewSceneMouseControls", Core::Preferences::Instance().useNewViewSceneMouseControls);
}

namespace
{
  template <bool Flag>
  class SetDisableFlag : public boost::static_visitor<>
  {
  public:
    template <typename T>
    void operator()( T* widget ) const
    {
      widget->setDisabled(Flag);
    }
  };

  //TODO: VS2010 compiler can't handle this function; check 2012 and clang
  template <bool Flag>
  void setWidgetsDisableFlag(std::vector<InputWidget>& widgets)
  {
    std::for_each(widgets.begin(), widgets.end(), [](InputWidget& v) { boost::apply_visitor(SetDisableFlag<Flag>(), v); });
  }
}

void SCIRunMainWindow::disableInputWidgets()
{
  networkEditor_->disableInputWidgets();
  std::for_each(inputWidgets_.begin(), inputWidgets_.end(), [](InputWidget& v) { boost::apply_visitor(SetDisableFlag<true>(), v); });
}

void SCIRunMainWindow::enableInputWidgets()
{
  networkEditor_->enableInputWidgets();
  std::for_each(inputWidgets_.begin(), inputWidgets_.end(), [](InputWidget& v) { boost::apply_visitor(SetDisableFlag<false>(), v); });
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
  Log::get() << DEBUG_LOG << "Executor of type " << type << " selected"  << std::endl;
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
  connect(actionPythonConsole_, SIGNAL(toggled(bool)), this, SLOT(showPythonWarning(bool)));
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
  std::cout << "Modules are large" << std::endl;
}

void SCIRunMainWindow::makeModulesSmallSize()
{
  std::cout << "TODO: Modules are small" << std::endl;
}

namespace {

  void addFavoriteMenu(QTreeWidget* tree)
  {
    auto faves = new QTreeWidgetItem();
    faves->setText(0, "Favorites");

    tree->addTopLevelItem(faves);
  }

void fillTreeWidget(QTreeWidget* tree, const ModuleDescriptionMap& moduleMap)
{
  BOOST_FOREACH(const ModuleDescriptionMap::value_type& package, moduleMap)
  {
    const std::string& packageName = package.first;
    auto packageItem = new QTreeWidgetItem();
    packageItem->setText(0, QString::fromStdString(packageName));
    tree->addTopLevelItem(packageItem);
    size_t totalModules = 0;
    BOOST_FOREACH(const ModuleDescriptionMap::value_type::second_type::value_type& category, package.second)
    {
      const std::string& categoryName = category.first;
      auto categoryItem = new QTreeWidgetItem();
      categoryItem->setText(0, QString::fromStdString(categoryName));
      packageItem->addChild(categoryItem);
      BOOST_FOREACH(const ModuleDescriptionMap::value_type::second_type::value_type::second_type::value_type& module, category.second)
      {
        const std::string& moduleName = module.first;
        auto m = new QTreeWidgetItem();
        m->setText(0, QString::fromStdString(moduleName));
        m->setText(1, QString::fromStdString(module.second.moduleStatus_));
        m->setText(2, QString::fromStdString(module.second.moduleInfo_));
        categoryItem->addChild(m);
        totalModules++;
      }
      categoryItem->setText(1, "Category Module Count = " + QString::number(category.second.size()));
    }
    packageItem->setText(1, "Package Module Count = " + QString::number(totalModules));
  }
}
}

void SCIRunMainWindow::fillModuleSelector()
{
  moduleSelectorTreeWidget_->clear();

  auto moduleDescs = networkEditor_->getNetworkEditorController()->getAllAvailableModuleDescriptions();

  addFavoriteMenu(moduleSelectorTreeWidget_);
  fillTreeWidget(moduleSelectorTreeWidget_, moduleDescs);

  GrabNameAndSetFlags visitor;
  visitTree(moduleSelectorTreeWidget_, visitor);

  moduleSelectorTreeWidget_->expandAll();
  moduleSelectorTreeWidget_->resizeColumnToContents(0);
  moduleSelectorTreeWidget_->resizeColumnToContents(1);
  moduleSelectorTreeWidget_->sortByColumn(0, Qt::AscendingOrder);
}

void SCIRunMainWindow::displayAcknowledgement()
{
  QMessageBox::information(this, "NIH/NIGMS Center for Integrative Biomedical Computing Acknowledgment", 
    "CIBC software and the data sets provided on this web site are Open Source software projects that are principally funded through the SCI Institute's NIH/NCRR CIBC. For us to secure the funding that allows us to continue providing this software, we must have evidence of its utility. Thus we ask users of our software and data to acknowledge us in their publications and inform us of these publications. Please use the following acknowledgment and send us references to any publications, presentations, or successful funding applications that make use of the NIH/NCRR CIBC software or data sets we provide. <p> <i>This project was supported by the National Institute of General Medical Sciences of the National Institutes of Health under grant number P41GM103545.</i>");
}