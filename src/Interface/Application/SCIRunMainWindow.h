/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

class QToolButton;

namespace SCIRun {
  namespace Dataflow {
    namespace Networks {
      struct ModuleId;
    }
    namespace Engine {
      class NetworkEditorController;
    }}}

namespace SCIRun {
namespace Gui {

class NetworkEditor;
class ProvenanceWindow;
class DeveloperConsole;
class PreferencesWindow;
class ShortcutsInterface;
class TagManagerWindow;
class PythonConsoleWidget;
class FileDownloader;

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
  bool importLegacyNetworkFile(const QString& filename);
  void setupQuitAfterExecute();
  void quit();
  void runPythonScript(const QString& scriptFileName);
  void setDataDirectory(const QString& dir);
  void setDataPath(const QString& dirs);
  void addToDataDirectory(const QString& dir);
  void setCurrentFile(const QString& fileName);

  bool newInterface() const;
  bool isInFavorites(const QString& module) const;
  const QMap<QString,QMap<QString,QString>>& styleSheetDetails() const { return styleSheetDetails_; }

  ~SCIRunMainWindow();
  int returnCode() const { return returnCode_; }
public Q_SLOTS:
  void executeAll();
  void showZoomStatusMessage(int zoomLevel);
protected:
  virtual void closeEvent(QCloseEvent* event) override;
  virtual void keyPressEvent(QKeyEvent *event) override;
  virtual void keyReleaseEvent(QKeyEvent *event) override;
  virtual void showEvent(QShowEvent* event) override;
  virtual void hideEvent(QHideEvent* event) override;
private:
  static SCIRunMainWindow* instance_;
  SCIRunMainWindow();
  NetworkEditor* networkEditor_;
  ProvenanceWindow* provenanceWindow_;
  TagManagerWindow* tagManagerWindow_;
  DeveloperConsole* devConsole_;
  PreferencesWindow* prefsWindow_;
  PythonConsoleWidget* pythonConsole_;
  ShortcutsInterface* shortcuts_;
  QActionGroup* filterActionGroup_;
  QAction* actionEnterWhatsThisMode_;
  QStringList favoriteModuleNames_;
  QToolButton* executeButton_;
  QByteArray windowState_;
  void postConstructionSignalHookup();
  void executeCommandLineRequests();
  void setTipsAndWhatsThis();
  bool okToContinue();
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
  void setupTagManagerWindow();
  void setupPythonConsole();
  void fillModuleSelector();
  void setupInputWidgets();
  void printStyleSheet() const;
  void hideNonfunctioningWidgets();
  void showStatusMessage(const QString& str);
  void showStatusMessage(const QString& str, int timeInMsec);

  enum { MaxRecentFiles = 5 }; //TODO: could be a user setting
  std::vector<QAction*> recentFileActions_;
  QStringList recentFiles_;
  QString currentFile_;
  QDir latestNetworkDirectory_;
  bool firstTimePythonShown_;
  int returnCode_;
  QMap<QString,QMap<QString,QString>> styleSheetDetails_;
  QMap<QString, QAction*> currentModuleActions_;
  boost::shared_ptr<class DialogErrorControl> dialogErrorControl_;
  boost::shared_ptr<class NetworkExecutionProgressBar> networkProgressBar_;
  boost::shared_ptr<class GuiActionProvenanceConverter> commandConverter_;
  boost::shared_ptr<class DefaultNotePositionGetter> defaultNotePositionGetter_;
  bool quitAfterExecute_;
  bool runningPythonScript_ = false;

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
  void importLegacyNetwork();
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
  void alertForNetworkCycles(int code);
  void updateDockWidgetProperties(bool isFloating);
  void setDataDirectoryFromGUI();
  void toolkitDownload();
  void addToPathFromGUI();
  void displayAcknowledgement();
  void setFocusOnFilterLine();
  void addModuleKeyboardAction();
  void showKeyboardShortcutsDialog();
  void selectModuleKeyboardAction();
  void modulesSnapToChanged();
  void highlightPortsChanged();
  void openLogFolder();
  void runNewModuleWizard();
  void resetWindowLayout();
  void zoomNetwork();
  void networkTimedOut();
  void changeExecuteActionIconToStop();
  void changeExecuteActionIconToPlay();
  void adjustExecuteButtonAppearance();
  void addModuleToWindowList(const QString& id, bool hasUI);
  void removeModuleFromWindowList(const SCIRun::Dataflow::Networks::ModuleId& modId);
  void setDragMode(bool toggle);
  void setSelectMode(bool toggle);
  void toggleTagLayer(bool toggle);
  void toggleMetadataLayer(bool toggle);
  void adjustModuleDock(int state);
  void exitApplication(int code);
};

}
}
#endif
