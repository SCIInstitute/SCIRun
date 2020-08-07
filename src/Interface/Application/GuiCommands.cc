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


#include <Interface/qt_include.h>
#include <numeric>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/GuiCommands.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/NetworkEditor.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/Importer/NetworkIO.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Interface/Application/Utility.h>
#include <Core/Logging/Log.h>
#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string.hpp>
#include <Core/Utils/CurrentFileName.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;
using namespace Commands;
using namespace SCIRun::Dataflow::Networks;
using namespace Algorithms;

LoadFileCommandGui::LoadFileCommandGui()
{
  addParameter(Name("FileNum"), 0);
}

bool LoadFileCommandGui::execute()
{
  std::string inputFile;
  auto inputFilesFromCommandLine = Application::Instance().parameters()->inputFiles();

  if (!inputFilesFromCommandLine.empty())
    inputFile = inputFilesFromCommandLine[0];
  else
  {
    inputFile = get(Variables::Filename).toFilename().string();
    if (mostRecentFileCode() == inputFile)
      inputFile = SCIRunMainWindow::Instance()->mostRecentFile().toStdString();
  }

  return SCIRunMainWindow::Instance()->loadNetworkFile(QString::fromStdString(inputFile));
}

bool ExecuteCurrentNetworkCommandGui::execute()
{
  SCIRunMainWindow::Instance()->executeAll();
  return true;
}

static const AlgorithmParameterName RunningPython("RunningPython");

QuitAfterExecuteCommandGui::QuitAfterExecuteCommandGui()
{
  addParameter(RunningPython, false);
}

bool QuitAfterExecuteCommandGui::execute()
{
  if (get(RunningPython).toBool())
    SCIRunMainWindow::Instance()->skipSaveCheck();
  SCIRunMainWindow::Instance()->setupQuitAfterExecute();
  return true;
}

QuitCommandGui::QuitCommandGui()
{
  addParameter(RunningPython, false);
}

bool QuitCommandGui::execute()
{
  if (get(RunningPython).toBool())
    SCIRunMainWindow::Instance()->skipSaveCheck();
  SCIRunMainWindow::Instance()->quit();
  exit(0);
  return true;
}

bool ShowMainWindowGui::execute()
{
  auto mainWin = SCIRunMainWindow::Instance();
  mainWin->activateWindow();

  mainWin->raise();
  mainWin->show();
  return true;
}

ShowSplashScreenGui::ShowSplashScreenGui()
{
  initSplashScreen();
}

bool ShowSplashScreenGui::execute()
{
  splash_->show();

  splashTimer_->start();
  splash_->showMessage("Welcome! Tip: Press F1 and click anywhere in the interface for helpful hints.", Qt::AlignBottom, Qt::white);
  qApp->processEvents();

  return true;
}

void ShowSplashScreenGui::initSplashScreen()
{
  splash_ = new QSplashScreen(QPixmap(":/general/Resources/scirun_5_0_alpha.png"), Qt::WindowStaysOnTopHint);
  splashTimer_ = new QTimer;
  splashTimer_->setSingleShot( true );
  splashTimer_->setInterval( 5000 );
  QObject::connect( splashTimer_, SIGNAL( timeout() ), splash_, SLOT( close() ));
}

QSplashScreen* ShowSplashScreenGui::splash_ = 0;
QTimer* ShowSplashScreenGui::splashTimer_ = 0;

namespace
{
  template <class PointIter>
  QPointF centroidOfPointRange(PointIter begin, PointIter end)
  {
    QPointF sum = std::accumulate(begin, end, QPointF(), [](const QPointF& acc, const typename PointIter::value_type& point) { return acc + QPointF(point.first, point.second); });
    size_t num = std::distance(begin, end);
    return sum / num;
  }

  QPointF findCenterOfNetworkFile(const NetworkFile& file)
  {
    return findCenterOfNetwork(file.modulePositions);
  }
}

QPointF SCIRun::Gui::findCenterOfNetwork(const ModulePositions& positions)
{
  auto pointRange = positions.modulePositions | boost::adaptors::map_values;
  return centroidOfPointRange(pointRange.begin(), pointRange.end());
}

namespace std
{
  template <typename T1, typename T2>
  std::ostream& operator<<(std::ostream& o, const std::pair<T1,T2>& p)
  {
    return o << p.first << "," << p.second;
  }
}

bool NetworkFileProcessCommand::execute()
{
  auto filename = get(Variables::Filename).toFilename().string();
  auto tempFile = get(Name("temporaryFile")).toBool();
  GuiLogger::logInfoQ("Attempting load of " + QString::fromStdString(filename));

  try
  {
    auto file = processXmlFile(filename);

    if (file)
    {
      auto load = boost::bind(&NetworkFileProcessCommand::guiProcess, this, file);
      if (Core::Application::Instance().parameters()->isRegressionMode())
      {
        load();
      }
      else
      {
        int numModules = static_cast<int>(file->network.modules.size());
        QProgressDialog progress("Loading network " + (tempFile ? "" : QString::fromStdString(filename)), QString(), 0, numModules + 1, SCIRunMainWindow::Instance());
        progress.connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(networkDoneLoading(int)), SLOT(setValue(int)));
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        progress.setValue(0);

        //TODO: trying to load in a separate thread exposed problems with the signal/slots related to wiring up the GUI elements,
        // so I'll come back to this idea when there's time to refactor that part of the code (NEC::loadNetwork)
        //QFuture<int> future = QtConcurrent::run(load);
        //progress.setValue(future.result());
        networkEditor_->setVisibility(false);
        progress.setValue(load());
        networkEditor_->setVisibility(true);
      }
      file_ = file;

      auto center = findCenterOfNetworkFile(*file);
      networkEditor_->centerOn(center);

      if (!tempFile)
      {
        GuiLogger::logInfoStd("File load done (" + filename + ").");
        SCIRun::Core::setCurrentFileName(filename);
      }
      return true;
    }
    GuiLogger::logErrorStd("File load failed (" + filename + "): null xml returned.");
  }
  catch (ExceptionBase& e)
  {
    std::string message(e.what());
    GuiLogger::logErrorStd("File load failed (" + filename + "): SCIRun exception in load_xml, " + message);

    auto quiet = get(Core::Algorithms::AlgorithmParameterName("QuietMode")).toBool();

    if (!quiet)
    {
      if (message.find("InterfaceWithTetGen") != std::string::npos)
      {
        QMessageBox::warning(SCIRunMainWindow::Instance(), "TetGen module not found",
          "TetGen module not found, please rebuild with TetGen enabled or find a TetGen-enabled build.");
      }
      else if (message.find("Undefined module") != std::string::npos)
      {
        QMessageBox::warning(SCIRunMainWindow::Instance(), "Legacy module not found",
          QString("If you just ran the network converter command, the error is: ") + e.what());
      }
    }
  }
  catch (std::exception& ex)
  {
    GuiLogger::logErrorStd("File load failed (" + filename + "): std::exception in load_xml, " + ex.what());
  }
  catch (...)
  {
    GuiLogger::logErrorStd("File load failed (" + filename + "): Unknown exception in load_xml.");
  }
  return false;
}

int NetworkFileProcessCommand::guiProcess(const NetworkFileHandle& file)
{
  networkEditor_->clear();
  networkEditor_->loadNetwork(file);
  return static_cast<int>(file->network.modules.size()) + 1;
}

FileOpenCommand::FileOpenCommand()
{
  addParameter(Name("temporaryFile"), false);
}

NetworkFileHandle FileOpenCommand::processXmlFile(const std::string& filename)
{
  return XMLSerializer::load_xml<NetworkFile>(filename);
}

FileImportCommand::FileImportCommand()
{
  addParameter(Name("temporaryFile"), false);
}

NetworkFileHandle FileImportCommand::processXmlFile(const std::string& filename)
{
  auto dtdpath = Core::Application::Instance().executablePath();
  const auto& modFactory = Core::Application::Instance().controller()->moduleFactory();
  LegacyNetworkIO lnio(dtdpath.string(), modFactory, logContents_);
  return lnio.load_net(filename);
}

bool RunPythonScriptCommandGui::execute()
{
  auto& app = Application::Instance();
  if (app.parameters()->quitAfterOneScriptedExecution())
  {
    SCIRunMainWindow::Instance()->skipSaveCheck();
    SCIRunMainWindow::Instance()->setupQuitAfterExecute();
    app.controller()->stopExecutionContextLoopWhenExecutionFinishes();
  }

  auto script = app.parameters()->pythonScriptFile().get();
  SCIRunMainWindow::Instance()->runPythonScript(QString::fromStdString(script.string()));
  return true;
}

bool SetupDataDirectoryCommandGui::execute()
{
  auto dir = Application::Instance().parameters()->dataDirectory().get();
  LOG_DEBUG("Data dir set to: {}", dir.string());

  SCIRunMainWindow::Instance()->setDataDirectory(QString::fromStdString(dir.string()));

  return true;
}

bool NetworkSaveCommand::execute()
{
  auto filename = get(Variables::Filename).toFilename().string();
  auto fileNameWithExtension = saveImpl(filename);
  if (!fileNameWithExtension.empty())
  {
    SCIRunMainWindow::Instance()->setCurrentFile(QString::fromStdString(fileNameWithExtension));
    SCIRunMainWindow::Instance()->statusBar()->showMessage("File saved: " + QString::fromStdString(filename), 2000);
    GuiLogger::logInfoQ("File save done: " + QString::fromStdString(filename));
    SCIRunMainWindow::Instance()->setWindowModified(false);
    setCurrentFileName(filename);

    return true;
  }
  return false;
}

NetworkFileProcessCommand::NetworkFileProcessCommand() : networkEditor_(SCIRunMainWindow::Instance()->networkEditor())
{
}

bool DisableViewScenesCommandGui::execute()
{
  SCIRunMainWindow::Instance()->networkEditor()->disableViewScenes();
  //TODO: hook up enableViewScenes to execution finished
  return true;
}

bool ToolkitUnpackerCommand::execute()
{
  ToolkitFile toolkit;
  auto filename = get(Variables::Filename).toFilename();
  std::ifstream istr(filename.string());
  toolkit.load(istr);

  auto add = !toolkit.networks.empty();
  if (add)
    SCIRunMainWindow::Instance()->addToolkit(QString::fromStdString(filename.leaf().stem().string()),
      QString::fromStdString(filename.parent_path().string()), toolkit);

  return add;
}
