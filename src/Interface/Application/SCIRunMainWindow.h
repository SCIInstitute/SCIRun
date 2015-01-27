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

class SCIRunMainWindow : public QMainWindow, public Ui::SCIRunMainWindow
{
	Q_OBJECT
public:
	static SCIRunMainWindow* Instance();
  void setController(boost::shared_ptr<SCIRun::Dataflow::Engine::NetworkEditorController> controller);
  void initialize();

  //command access: extract an interface
  void saveNetworkFile(const QString& fileName);
  bool loadNetworkFile(const QString& filename);
  void setupQuitAfterExecute();
  void quit();
  void runPythonScript(const QString& scriptFileName);
  void setDataDirectory(const QString& dir);
  void setDataPath(const QString& dirs);
  void addToDataDirectory(const QString& dir);

  bool newInterface() const;
  const QMap<QString,QMap<QString,QString>>& styleSheetDetails() const { return styleSheetDetails_; }

  ~SCIRunMainWindow();
public Q_SLOTS:
  void executeAll();
  void showZoomStatusMessage(int zoomLevel);
protected:
  virtual void closeEvent(QCloseEvent* event) override;
  virtual void keyPressEvent(QKeyEvent *event) override;
  virtual void keyReleaseEvent(QKeyEvent *event) override;
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
  void setTipsAndWhatsThis();
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
  void parseStyleXML();
  void printStyleSheet() const;
  void hideNonfunctioningWidgets();

  enum { MaxRecentFiles = 5 }; //TODO: could be a user setting
  std::vector<QAction*> recentFileActions_;
  QStringList recentFiles_;
  QString currentFile_;
  QDir latestNetworkDirectory_;
  bool firstTimePythonShown_;
  QMap<QString,QMap<QString,QString>> styleSheetDetails_;
  boost::shared_ptr<class DialogErrorControl> dialogErrorControl_;
  boost::shared_ptr<class NetworkExecutionProgressBar> networkProgressBar_;
  boost::shared_ptr<class GuiActionProvenanceConverter> commandConverter_;
  boost::shared_ptr<class DefaultNotePositionGetter> defaultNotePositionGetter_;
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
  void setDataDirectoryFromGUI();
  void addToPathFromGUI();
  void displayAcknowledgement();
  void setFocusOnFilterLine();
  void addModuleKeyboardAction();
  void selectModuleKeyboardAction();
  void modulesSnapToChanged();
  void resetWindowLayout();
  void zoomNetwork();
  void setDragMode(bool toggle);
  void setSelectMode(bool toggle);
  void adjustModuleDock(int state);
  void exitApplication(int code);
};

}
}
#endif
