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
  void visitTree(QStringList& list, QTreeWidgetItem* item)
  {
    list << item->text(0) + "," + QString::number(item->childCount());
    if (item->childCount() != 0)
      item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    for (int i = 0; i < item->childCount(); ++i)
      visitTree(list, item->child(i));
  }

  QStringList visitTree(QTreeWidget* tree) 
  {
    QStringList list;
    for (int i = 0; i < tree->topLevelItemCount(); ++i)
      visitTree(list, tree->topLevelItem(i));
    return list;
  }

  class TextEditAppender : public Core::Logging::LoggerInterface
  {
  public:
    explicit TextEditAppender(QTextEdit* text) : text_(text) {}

    void log(const QString& message) const 
    {
      text_->append(message);
    }

    virtual void error(const std::string& msg)
    {
      log("Error: " + QString::fromStdString(msg));
    }

    virtual void warning(const std::string& msg)
    {
      log("Warning: " + QString::fromStdString(msg));
    }

    virtual void remark(const std::string& msg)
    {
      log("Remark: " + QString::fromStdString(msg));
    }

    virtual void status(const std::string& msg)
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
	
	QToolBar* executeBar = addToolBar(tr("&Execute"));
	executeBar->addAction(actionExecute_All_);
	
	QWidgetAction* globalProgress = new QWidgetAction(this);
	globalProgress->setDefaultWidget(new QProgressBar(this));
	globalProgress->setVisible(true);
	executeBar->addAction(globalProgress);
	
	QWidgetAction* moduleCounter = new QWidgetAction(this);
	moduleCounter->setDefaultWidget(new QLabel("0/0", this));
	moduleCounter->setVisible(true);
	executeBar->addAction(moduleCounter);
	
	scrollAreaWidgetContents_->addAction(actionExecute_All_);
	scrollAreaWidgetContents_->setContextMenuPolicy(Qt::ActionsContextMenu);
	scrollArea_->viewport()->setBackgroundRole(QPalette::Dark);
	scrollArea_->viewport()->setAutoFillBackground(true);	
  networkEditor_->addActions(scrollAreaWidgetContents_);

	logTextBrowser_->setText("Hello! Welcome to the SCIRun5 Prototype.");

  QStringList result = visitTree(moduleSelectorTreeWidget_);
  std::for_each(result.begin(), result.end(), boost::bind(&GuiLogger::log, boost::ref(GuiLogger::Instance()), _1));

  connect(actionSave_As_, SIGNAL(triggered()), this, SLOT(saveNetworkAs()));
  connect(actionSave_, SIGNAL(triggered()), this, SLOT(saveNetwork()));
  connect(actionLoad_, SIGNAL(triggered()), this, SLOT(loadNetwork()));

  connect(actionQuit_, SIGNAL(triggered()), this, SLOT(close()));
  setCurrentFile("");

  moduleSelectorTreeWidget_->expandAll();
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
      networkEditor_->executeAll([this] {close(); qApp->quit();});
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
  //updateStatusBar();
}

void SCIRunMainWindow::ToggleRenderer()
{
}
