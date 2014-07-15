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

#ifndef INTERFACE_APPLICATION_SCIRUN_MAIN_WINDOW_H
#define INTERFACE_APPLICATION_SCIRUN_MAIN_WINDOW_H

#include <QDialog>
#include <QDir>
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include <Interface/Application/Note.h>
#include "ui_SCIRunMainWindow.h"

namespace SCIRun {
  namespace Dataflow {
    namespace Engine {
      class NetworkEditorController;
    }}}

namespace SCIRun {
namespace Gui {

class NetworkEditor;
class ProvenanceWindow;
class DeveloperConsole;
class PreferencesWindow;
class PythonConsoleWidget;

typedef boost::variant<QAction*, QWidget*> InputWidget;

class SCIRunMainWindow : public QMainWindow, public Ui::SCIRunMainWindow
{
	Q_OBJECT
public:
	static SCIRunMainWindow* Instance();
  void setController(boost::shared_ptr<SCIRun::Dataflow::Engine::NetworkEditorController> controller);
  void initialize();

  //command access: extract an interface
  void saveNetworkFile(const QString& fileName);
  void loadNetworkFile(const QString& filename);
  void setupQuitAfterExecute();
  void quit();
  void runPythonScript(const QString& scriptFileName);

public Q_SLOTS:
  void executeAll();
protected:
  virtual void closeEvent(QCloseEvent* event);
private:
  static SCIRunMainWindow* instance_;
  SCIRunMainWindow();
  NetworkEditor* networkEditor_;
  ProvenanceWindow* provenanceWindow_;
  DeveloperConsole* devConsole_;
  PreferencesWindow* prefs_;
  PythonConsoleWidget* pythonConsole_;
  QActionGroup* filterActionGroup_;
  QAction* actionEnterWhatsThisMode_;
  QStringList favoriteModuleNames_;
  
private:
  void postConstructionSignalHookup();
  void executeCommandLineRequests();
  bool okToContinue();
  void setCurrentFile(const QString& fileName);
  void updateRecentFileActions();
  QString strippedName(const QString& fullFileName);
  void setActionIcons();
  void makeFilterButtonMenu();
  void writeSettings();
  void readSettings();
  void setupNetworkEditor();
  void setupProvenanceWindow();
  void setupDevConsole();
  void setupPreferencesWindow();
  void setupPythonConsole();
  void fillModuleSelector();
  void setupInputWidgets();
  enum { MaxRecentFiles = 5 }; //TODO: could be a user setting
  std::vector<QAction*> recentFileActions_;
  QStringList recentFiles_;
  QString currentFile_;
  QDir latestNetworkDirectory_;
  bool firstTimePythonShown_;
  boost::shared_ptr<class DialogErrorControl> dialogErrorControl_; 
  boost::shared_ptr<class NetworkExecutionProgressBar> networkProgressBar_;
  boost::shared_ptr<class GuiActionProvenanceConverter> commandConverter_;
  boost::shared_ptr<class DefaultNotePositionGetter> defaultNotePositionGetter_;
  std::vector<InputWidget> inputWidgets_;
Q_SIGNALS:
  void moduleItemDoubleClicked();
  void defaultNotePositionChanged(NotePosition position);
private Q_SLOTS:
  void saveNetworkAs();
  void saveNetwork();
  void loadNetwork();
  void loadRecentNetwork();
  bool newNetwork();
  void runScript();
  void networkModified();
  void filterModuleNamesInTreeView(const QString& start);
  void makePipesEuclidean();
  void makePipesCubicBezier();
  void makePipesManhattan();
  void disableInputWidgets(); 
  void enableInputWidgets();
  void chooseBackgroundColor();
  void resetBackgroundColor();
  void filterDoubleClickedModuleSelectorItem(QTreeWidgetItem* item);
  void handleCheckedModuleEntry(QTreeWidgetItem* item, int column);
  void setExecutor(int type);
  void setGlobalPortCaching(bool enable);
  void readDefaultNotePosition(int index);
  void updateMiniView();
  void showPythonWarning(bool visible);
  void makeModulesLargeSize();
  void makeModulesSmallSize();
  void displayAcknowledgement();
  void exitApplication(int code);
};

}
}
#endif
