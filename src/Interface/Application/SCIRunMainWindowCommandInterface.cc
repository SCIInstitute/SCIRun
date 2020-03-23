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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <Core/Utils/Legacy/MemoryUtil.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/ShortcutsInterface.h>
#include <Interface/Application/TreeViewCollaborators.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/GuiCommands.h>
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

void SCIRunMainWindow::initialize()
{
  postConstructionSignalHookup();

  fillModuleSelector();

  executeCommandLineRequests();
}

void SCIRunMainWindow::setController(NetworkEditorControllerHandle controller)
{
  auto controllerProxy(boost::make_shared<NetworkEditorControllerGuiProxy>(controller, networkEditor_));
  networkEditor_->setNetworkEditorController(controllerProxy);
  //TODO: need better way to wire this up
  controller->setSerializationManager(networkEditor_);
}

void SCIRunMainWindow::preexecute()
{
	if (Preferences::Instance().saveBeforeExecute && !Application::Instance().parameters()->isRegressionMode())
	{
		saveNetwork();
	}
}

void SCIRunMainWindow::setupQuitAfterExecute()
{
  connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(executionFinished(int)), this, SLOT(exitApplication(int)));
  quitAfterExecute_ = true;
}

void SCIRunMainWindow::saveNetworkFile(const QString& fileName)
{
  writeSettings();
  NetworkSaveCommand save;
  save.set(Variables::Filename, fileName.toStdString());
  save.execute();
}

bool SCIRunMainWindow::loadNetworkFile(const QString& filename, bool isTemporary)
{
  if (!filename.isEmpty())
  {
    RENDERER_LOG("Opening network file: {}", filename.toStdString());
    FileOpenCommand command;
    command.set(Variables::Filename, filename.toStdString());
    command.set(Name("temporaryFile"), isTemporary);
    if (command.execute())
    {
      networkProgressBar_->updateTotalModules(networkEditor_->numModules());
      if (!isTemporary)
      {
        setCurrentFile(filename);
        statusBar()->showMessage(tr("File loaded: ") + filename, 2000);
        provenanceWindow_->clear();
        provenanceWindow_->showFile(command.file_);
      }
      else
      {
        setCurrentFile("");
        setWindowModified(true);
        showStatusMessage("Toolkit network loaded. ", 2000);
      }
			networkEditor_->viewport()->update();
      return true;
    }
    else
    {
      if (Application::Instance().parameters()->isRegressionMode())
        exit(7);
      //TODO: set error code to non-0 so regression tests fail!
      // probably want to control this with a --regression flag.
    }
  }
  return false;
}

bool SCIRunMainWindow::importLegacyNetworkFile(const QString& filename)
{
	bool success = false;
  if (!filename.isEmpty())
  {
    FileImportCommand command;
    command.set(Variables::Filename, filename.toStdString());
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
    auto isoString = boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::universal_time());
    auto logFileName = QString::fromStdString(Core::Logging::LogSettings::Instance().logDirectory().string())
      + "/" + (strippedName(filename) + "_importLog_" + QString::fromStdString(isoString) + ".log");
		QFile logFile(logFileName);
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

void SCIRunMainWindow::setCurrentFile(const QString& fileName)
{
  currentFile_ = fileName;
  setCurrentFileName(currentFile_.toStdString());
  setWindowModified(false);
  auto shownName = tr("Untitled");
  if (!currentFile_.isEmpty())
  {
    shownName = strippedName(currentFile_);
    latestNetworkDirectory_ = QFileInfo(currentFile_).dir();
    recentFiles_.removeAll(currentFile_);
    while (recentFiles_.size() > MaxRecentFiles - 1)
      recentFiles_.removeLast();
    recentFiles_.prepend(currentFile_);
    updateRecentFileActions();
  }
  setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("SCIRun")));
}

void SCIRunMainWindow::runPythonScript(const QString& scriptFileName)
{
#ifdef BUILD_WITH_PYTHON
  NetworkEditor::InEditingContext iec(networkEditor_);
  GuiLogger::logInfoQ("RUNNING PYTHON SCRIPT: " + scriptFileName);
  PythonInterpreter::Instance().importSCIRunLibrary();
  PythonInterpreter::Instance().run_file(scriptFileName.toStdString());
  statusBar()->showMessage(tr("Script is running."), 2000);
#else
  GuiLogger::logInfoQ("Python not included in this build, cannot run " + scriptFileName);
#endif
}

bool SCIRunMainWindow::isInFavorites(const QString& module) const
{
	return favoriteModuleNames_.contains(module);
}

void SCIRunMainWindow::setDataDirectory(const QString& dir)
{
  if (!dir.isEmpty())
  {
    prefsWindow_->scirunDataLineEdit_->setText(dir);
    prefsWindow_->scirunDataLineEdit_->setToolTip(dir);

    RemembersFileDialogDirectory::setStartingDir(dir);
    Preferences::Instance().setDataDirectory(dir.toStdString());
    Q_EMIT dataDirectorySet(dir);
  }
}

void SCIRunMainWindow::setDataPath(const QString& dirs)
{
	if (!dirs.isEmpty())
	{
    //prefsWindow_->scirunDataPathTextEdit_->setPlainText(dirs);
    //prefsWindow_->scirunDataPathTextEdit_->setToolTip(dirs);

		Preferences::Instance().setDataPath(dirs.toStdString());
	}
}

void SCIRunMainWindow::addToDataDirectory(const QString& dir)
{
	if (!dir.isEmpty())
	{
    // auto text = prefsWindow_->scirunDataPathTextEdit_->toPlainText();
		// if (!text.isEmpty())
		// 	text += ";\n";
		// text += dir;
    // prefsWindow_->scirunDataPathTextEdit_->setPlainText(text);
    // prefsWindow_->scirunDataPathTextEdit_->setToolTip(prefsWindow_->scirunDataPathTextEdit_->toPlainText());

		RemembersFileDialogDirectory::setStartingDir(dir);
		Preferences::Instance().addToDataPath(dir.toStdString());
	}
}

void SCIRunMainWindow::addToolkit(const QString& filename, const QString& directory, const ToolkitFile& toolkit)
{
  auto menu = menuToolkits_->addMenu(filename);
  auto networks = menu->addMenu("Networks");
  toolkitDirectories_[filename] = directory;
  toolkitNetworks_[filename] = toolkit;
  auto fullpath = directory + QDir::separator() + filename + ".toolkit";
  toolkitMenus_[fullpath] = menu;
  std::map<std::string, std::map<std::string, NetworkFile>> toolkitMenuData;
  for (const auto& toolkitPair : toolkit.networks)
  {
    std::vector<std::string> elements;
    for (const auto& p : boost::filesystem::path(toolkitPair.first))
      elements.emplace_back(p.filename().string());

    if (elements.size() == 2)
    {
      toolkitMenuData[elements[0]][elements[1]] = toolkitPair.second;
    }
    else
    {
      qDebug() << "Cannot handle toolkit folders of depth > 1";
    }
  }

  for (const auto& t1 : toolkitMenuData)
  {
    auto t1menu = networks->addMenu(QString::fromStdString(t1.first));
    for (const auto& t2 : t1.second)
    {
      auto networkAction = t1menu->addAction(QString::fromStdString(t2.first));
      std::ostringstream net;
      XMLSerializer::save_xml(t2.second, net, "networkFile");
      networkAction->setProperty("network", QString::fromStdString(net.str()));
      connect(networkAction, SIGNAL(triggered()), this, SLOT(openToolkitNetwork()));
    }
  }

  auto folder = menu->addAction("Open Toolkit Directory");
  folder->setProperty("path", directory);
  connect(folder, SIGNAL(triggered()), this, SLOT(openToolkitFolder()));

  auto remove = menu->addAction("Remove Toolkit...");
  remove->setProperty("filename", filename);
  remove->setProperty("fullpath", fullpath);
  connect(remove, SIGNAL(triggered()), this, SLOT(removeToolkit()));

  if (!startup_)
  {
    QMessageBox::information(this, "Toolkit loaded", "Toolkit " + filename +
      " successfully imported. A new submenu is available under Toolkits for loading networks.\n\n"
      + "Remember to update your data folder under Preferences->Paths.");
    actionPreferences_->trigger();
  }
}
