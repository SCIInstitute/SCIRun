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
#include <Core/Logging/Logger.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/NetworkExecutionProgressBar.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Application/Application.h>


#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;

namespace
{
  struct GrabNameAndSetFlags
  {
    QStringList nameList_;
    void operator()(QTreeWidgetItem* item)
    {
      nameList_ << item->text(0) + "," + QString::number(item->childCount());
      if (item->childCount() != 0)
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
  };

  template <class Func>
  void visitItem(QTreeWidgetItem* item, Func& itemFunc)
  {
    itemFunc(item);
    for (int i = 0; i < item->childCount(); ++i)
      visitItem(item->child(i), itemFunc);
  }

  template <class Func>
  void visitTree(QTreeWidget* tree, Func& itemFunc) 
  {
    for (int i = 0; i < tree->topLevelItemCount(); ++i)
      visitItem(tree->topLevelItem(i), itemFunc);
  }

  class TextEditAppender : public Core::Logging::LoggerInterface
  {
  public:
    explicit TextEditAppender(QTextEdit* text) : text_(text) {}

    void log(const QString& message) const 
    {
      text_->append(message);
    }

    virtual void error(const std::string& msg) const
    {
      log("Error: " + QString::fromStdString(msg));
    }

    virtual void warning(const std::string& msg) const
    {
      log("Warning: " + QString::fromStdString(msg));
    }

    virtual void remark(const std::string& msg) const
    {
      log("Remark: " + QString::fromStdString(msg));
    }

    virtual void status(const std::string& msg) const
    {
      log(QString::fromStdString(msg));
    }
  private:
    QTextEdit* text_;
  };

  class TreeViewModuleGetter : public CurrentModuleSelection
  {
  public:
    explicit TreeViewModuleGetter(QTreeWidget& tree) : tree_(tree) {}
    virtual QString text() const
    {
      return tree_.currentItem()->text(0);
    }
    virtual bool isModule() const
    {
      return tree_.currentItem()->childCount() == 0;
    }
  private:
    QTreeWidget& tree_;
  };
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

void SCIRunMainWindow::setController(boost::shared_ptr<SCIRun::Dataflow::Engine::NetworkEditorController> controller)
{
  boost::shared_ptr<NetworkEditorControllerGuiProxy> controllerProxy(new NetworkEditorControllerGuiProxy(controller));
  networkEditor_->setNetworkEditorController(controllerProxy);
}


SCIRunMainWindow::SCIRunMainWindow()
{
	setupUi(this);

  boost::shared_ptr<TreeViewModuleGetter> getter(new TreeViewModuleGetter(*moduleSelectorTreeWidget_));
  Core::Logging::LoggerHandle logger(new TextEditAppender(logTextBrowser_));
  GuiLogger::setInstance(logger);
  networkEditor_ = new NetworkEditor(getter, scrollAreaWidgetContents_);
  networkEditor_->setObjectName(QString::fromUtf8("networkEditor_"));
  networkEditor_->setContextMenuPolicy(Qt::ActionsContextMenu);
  networkEditor_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->setModuleDumpAction(actionDump_positions);
  networkEditor_->verticalScrollBar()->setValue(0);
  networkEditor_->horizontalScrollBar()->setValue(0);

  actionExecute_All_->setStatusTip(tr("Execute all modules"));
  actionSave_->setStatusTip(tr("Save network"));
  actionLoad_->setStatusTip(tr("Load network"));
  connect(actionExecute_All_, SIGNAL(triggered()), networkEditor_, SLOT(executeAll()));
  connect(actionClear_Network_, SIGNAL(triggered()), this, SLOT(clearNetwork()));
  connect(networkEditor_, SIGNAL(modified()), this, SLOT(networkModified()));

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
  standardBar->addAction(actionLoad_);
  standardBar->addAction(actionSave_);

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

  connect(cubicPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesCubicBezier()));
  connect(manhattanPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesManhattan()));
  connect(euclideanPipesRadioButton_, SIGNAL(clicked()), this, SLOT(makePipesEuclidean()));
  //TODO: will be a user or network setting
  makePipesEuclidean();
  
  setCurrentFile("");

  moduleSelectorTreeWidget_->expandAll();

  connect(moduleFilterLineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(filterModuleNamesInTreeView(const QString&)));
  makeFilterButtonMenu();
  activateWindow();
}

void SCIRunMainWindow::doInitialStuff()
{
  auto inputFile = SCIRun::Core::Application::Instance().parameters()->inputFile();
  if (inputFile)
  {
    loadNetworkFile(QString::fromStdString(inputFile.get()));
    if (SCIRun::Core::Application::Instance().parameters()->executeNetwork())
    {
      // -e
      networkEditor_->executeAll();
    }
    else if (SCIRun::Core::Application::Instance().parameters()->executeNetworkAndQuit())
    {
      // -E
      //TODO: exit code should be from network execution for regression testing.
      //TODO: don't like passing the callback all the way down...better way to do it?--yes, when network done event is available, just have to add a quit() subscriber.
      // for exit code: qApp->exit(code); 
      networkEditor_->executeAll([this](int code) {close(); qApp->exit(code);});
    }
  }
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
  QString filename = QFileDialog::getSaveFileName(this, "Save Network...", ".", "*.srn5");
  if (!filename.isEmpty())
    saveNetworkFile(filename);
}

void SCIRunMainWindow::saveNetworkFile(const QString& fileName)
{
  NetworkXMLHandle data = networkEditor_->saveNetwork();

  ModulePositionsHandle positions = networkEditor_->dumpModulePositions();

  NetworkFile file;
  file.network = *data;
  file.modulePositions = *positions;

  XMLSerializer::save_xml(file, fileName.toStdString(), "networkFile");
  setCurrentFile(fileName);

  statusBar()->showMessage(tr("File saved"), 2000);
  GuiLogger::Instance().log("File save done.");
  setWindowModified(false);
}

class FileOpenCommand
{
public:
  FileOpenCommand(const std::string& filename, NetworkEditor* networkEditor) : filename_(filename), networkEditor_(networkEditor) {}
  void execute()
  {
    networkEditor_->clear();
    GuiLogger::Instance().log(QString("Attempting load of ") + filename_.c_str());

    try
    {
      boost::shared_ptr<NetworkFile> xml = XMLSerializer::load_xml<NetworkFile>(filename_);

      if (xml)
      {
        networkEditor_->loadNetwork(xml->network);
        networkEditor_->moveModules(xml->modulePositions);
      }
      else
        GuiLogger::Instance().log("File load failed.");

      GuiLogger::Instance().log("File load done.");
    }
    catch (...)
    {
      GuiLogger::Instance().log("File load failed.");
    }
  }
private:
  std::string filename_;
  NetworkEditor* networkEditor_;
};

void SCIRunMainWindow::loadNetwork()
{
  if (okToContinue())
  {
    QString filename = QFileDialog::getOpenFileName(this, "Load Network...", ".", "*.srn5");
    loadNetworkFile(filename);
  }
}

void SCIRunMainWindow::loadNetworkFile(const QString& filename)
{
  if (!filename.isEmpty())
  {
    FileOpenCommand command(filename.toStdString(), networkEditor_);
    command.execute();

    setCurrentFile(filename);
    statusBar()->showMessage(tr("File loaded"), 2000);
    networkProgressBar_->updateTotalModules(networkEditor_->numModules());
  }
}

bool SCIRunMainWindow::clearNetwork()
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
    //recentFiles.removeAll(curFile);
    //recentFiles.prepend(curFile);
    //updateRecentFileActions();
  }
  setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("SCIRun 5 Prototype")));
}

QString SCIRunMainWindow::strippedName(const QString& fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}

void SCIRunMainWindow::closeEvent(QCloseEvent* event)
{
  if (okToContinue())
  {
    //writeSettings();
    event->accept();
  }
  else
    event->ignore();
}

bool SCIRunMainWindow::okToContinue()
{
  if (isWindowModified())
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

void SCIRunMainWindow::ToggleRenderer()
{
}

void SCIRunMainWindow::setActionIcons() 
{
  actionLoad_->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon));
  actionSave_->setIcon(QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon));
  //actionSave_As_->setIcon(QApplication::style()->standardIcon(QStyle::SP_DriveCDIcon));  //TODO?
  actionExecute_All_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
}

struct HideItemsNotMatchingString
{
  explicit HideItemsNotMatchingString(bool useRegex, const QString& pattern) : match_("*" + pattern + "*", Qt::CaseInsensitive, QRegExp::Wildcard), start_(pattern), useRegex_(useRegex) {}
  QRegExp match_;
  QString start_;
  bool useRegex_;

  void operator()(QTreeWidgetItem* item)
  {
    if (item)
    {
      if (0 == item->childCount())
      {
        item->setHidden(shouldHide(item));
      }
      else
      {
        bool shouldHideCategory = true;
        for (int i = 0; i < item->childCount(); ++i)
        {
          auto child = item->child(i);
          if (!child->isHidden())
          {
            shouldHideCategory = false;
            break;
          }
        }
        item->setHidden(shouldHideCategory);
      }
    }
  }

  bool shouldHide(QTreeWidgetItem* item) 
  {
    auto text = item->text(0);
    if (useRegex_)
      return !match_.exactMatch(text);
    return !text.startsWith(start_, Qt::CaseInsensitive);
  }
};

struct ShowAll
{
  void operator()(QTreeWidgetItem* item)
  {
    item->setHidden(false);
  }
};

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
  std::cout << "Bezier selected" << std::endl;
}

void SCIRunMainWindow::makePipesEuclidean()
{
  std::cout << "euclidean selected" << std::endl;
}

void SCIRunMainWindow::makePipesManhattan()
{
  //TODO
}