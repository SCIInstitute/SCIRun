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
#include <QtConcurrentRun>
#include <numeric>
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/GuiCommands.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Core/Logging/Log.h>
#include <boost/range/adaptors.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Dataflow::Networks;

bool LoadFileCommandGui::execute()
{
  auto inputFile = Application::Instance().parameters()->inputFile();
  return SCIRunMainWindow::Instance()->loadNetworkFile(QString::fromStdString(inputFile.get()));
}

bool ExecuteCurrentNetworkCommandGui::execute()
{
  SCIRunMainWindow::Instance()->executeAll();
  return true;
}

bool QuitAfterExecuteCommandGui::execute()
{
  SCIRunMainWindow::Instance()->setupQuitAfterExecute();
  return true;
}

bool QuitCommandGui::execute()
{
  SCIRunMainWindow::Instance()->quit();
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
  splash_ = new QSplashScreen(0, QPixmap(":/gear/splash-scirun.png").scaled(990, 490),  Qt::WindowStaysOnTopHint);
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
  QPointF findCenterOfNetwork(PointIter begin, PointIter end)
  {
    QPointF sum = std::accumulate(begin, end, QPointF(), [](const QPointF& acc, const typename PointIter::value_type& point) { return acc + QPointF(point.first, point.second); });
    size_t num = std::distance(begin, end);
    return sum / num;
  }
}

namespace std
{
template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& o, const std::pair<T1,T2>& p)
{
  return o << p.first << "," << p.second;
}
}

bool FileOpenCommand::execute()
{
  GuiLogger::Instance().log(QString("Attempting load of ") + filename_.c_str());

  try
  {
    auto openedFile = XMLSerializer::load_xml<NetworkFile>(filename_);

    if (openedFile)
    {
      auto load = boost::bind(&FileOpenCommand::loadImpl, this, openedFile);
      if (Core::Application::Instance().parameters()->isRegressionMode())
      {
        std::cout << "loading file sync" << std::endl;
        load();
      }
      else
      {
        std::cout << "loading file async" << std::endl;
        int numModules = static_cast<int>(openedFile->network.modules.size());
        QProgressDialog progress("Loading network " + QString::fromStdString(filename_), QString(), 0, numModules + 1, SCIRunMainWindow::Instance());
        progress.connect(networkEditor_->getNetworkEditorController().get(), SIGNAL(networkDoneLoading(int)), SLOT(setValue(int)));
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        progress.setValue(0);
        std::cout << "````````````````````QtConcurrent::run" << std::endl;
        QFuture<int> future = QtConcurrent::run(load);
        std::cout << "````````````````````future returned" << std::endl;
        progress.setValue(future.result());
        std::cout << "````````````````````future evaluated" << std::endl;
      }
      openedFile_ = openedFile;

      auto pointRange = openedFile_->modulePositions.modulePositions | boost::adaptors::map_values;
      QPointF center = findCenterOfNetwork(pointRange.begin(), pointRange.end());
      networkEditor_->centerOn(center);

      GuiLogger::Instance().log("File load done.");
      std::cout << "`````````````````````````FileOpenCommand returning" << std::endl;
      return true;
    }
    else
      GuiLogger::Instance().log("File load failed: null xml returned.");
  }
  catch (ExceptionBase& e)
  {
    GuiLogger::Instance().log("File load failed: exception in load_xml, " + QString(e.what()));
  }
  catch (std::exception& ex)
  {
    GuiLogger::Instance().log("File load failed: exception in load_xml, " + QString(ex.what()));
  }
  catch (...)
  {
    GuiLogger::Instance().log("File load failed: Unknown exception in load_xml.");
  }
  return false;
}

int FileOpenCommand::loadImpl(const NetworkFileHandle& file)
{
  networkEditor_->clear();
  networkEditor_->loadNetwork(file);
  return static_cast<int>(file->network.modules.size()) + 1;
}

bool RunPythonScriptCommandGui::execute()
{
  auto script = Application::Instance().parameters()->pythonScriptFile().get();
  SCIRunMainWindow::Instance()->runPythonScript(QString::fromStdString(script.string()));
  return true;
}

bool SetupDataDirectoryCommandGui::execute()
{
  auto dir = Application::Instance().parameters()->dataDirectory().get();
  LOG_DEBUG("Data dir set to: " << dir << std::endl);

  SCIRunMainWindow::Instance()->setDataDirectory(QString::fromStdString(dir.string()));

  return true;
}
