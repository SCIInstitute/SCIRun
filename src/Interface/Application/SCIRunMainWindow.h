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


#ifndef INTERFACE_APPLICATION_SCIRUN_MAIN_WINDOW_H
#define INTERFACE_APPLICATION_SCIRUN_MAIN_WINDOW_H

#include <QDialog>
#include <QDir>
#include <Dataflow/Network/NetworkFwd.h>
#include <Interface/Application/PositionProvider.h>
#include "ui_SCIRunMainWindow.h"

class QToolButton;

namespace SCIRun {
  namespace Dataflow {
    namespace Networks {
      struct ModuleId;
      struct ToolkitFile;
    }
    namespace Engine {
      class NetworkEditorController;
    }}}

namespace SCIRun {
namespace Gui {

class NetworkEditor;
class ProvenanceWindow;
class PreferencesWindow;
class ShortcutsInterface;
class TagManagerWindow;
class PythonConsoleWidget;
class FileDownloader;
class TriggeredEventsWindow;
class MacroEditor;
class NetworkEditorBuilder;
class SettingsValueInterface;
using SettingsValueInterfacePtr = std::shared_ptr<SettingsValueInterface>;

class MainWindowCommands
{
public:
  virtual ~MainWindowCommands() {}
  //TODO: #643
};

class SCIRunMainWindow : public QMainWindow, public Ui::SCIRunMainWindow
{
	Q_OBJECT
public:
	static SCIRunMainWindow* Instance();
  void setController(boost::shared_ptr<SCIRun::Dataflow::Engine::NetworkEditorController> controller);
  void initialize();

  //command access: extract an interface
  void addNetworkActionsToBar(QToolBar* toolbar) const;
  void addToDataDirectory(const QString& dir);
  void addToolkit(const QString& filename, const QString& directory, const SCIRun::Dataflow::Networks::ToolkitFile& toolkit);
  bool loadNetworkFile(const QString& filename, bool isTemporary = false);
  bool importLegacyNetworkFile(const QString& filename);
  void quit();
  void runPythonScript(const QString& scriptFileName);
  void saveNetworkFile(const QString& fileName);
  void setCurrentFile(const QString& fileName);
  void setDataDirectory(const QString& dir);
  void setDataPath(const QString& dirs);
  void setupQuitAfterExecute();

  //TODO: extract another interface for command objects
  NetworkEditor* networkEditor() { return networkEditor_; }

  bool isInFavorites(const QString& module) const;
  const QMap<QString,QMap<QString,QString>>& styleSheetDetails() const { return styleSheetDetails_; }

  void preexecute();
  void skipSaveCheck() { skipSaveCheck_ = true; }

  ~SCIRunMainWindow();
  int returnCode() const { return returnCode_; }

  QString mostRecentFile() const;
  static const int clipboardKey = 125;
public Q_SLOTS:
  void executeAll();
  void showZoomStatusMessage(int zoomLevel);
  void setDataDirectoryFromGUI();
  void setConnectionPipelineType(int type);
  void setSaveBeforeExecute(int state);
  void reportIssue();
protected:
  virtual void closeEvent(QCloseEvent* event) override;
  virtual void keyPressEvent(QKeyEvent *event) override;
  virtual void keyReleaseEvent(QKeyEvent *event) override;
  virtual void showEvent(QShowEvent* event) override;
  virtual void hideEvent(QHideEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
private:
  static SCIRunMainWindow* instance_;
  SCIRunMainWindow();
  NetworkEditor* networkEditor_;
  ProvenanceWindow* provenanceWindow_;
  TagManagerWindow* tagManagerWindow_ {nullptr};
  PreferencesWindow* prefsWindow_;
  PythonConsoleWidget* pythonConsole_;
  ShortcutsInterface* shortcuts_ { nullptr };
  QActionGroup* filterActionGroup_;
  QAction* actionEnterWhatsThisMode_;
  QStringList favoriteModuleNames_;
  QMap<QString, QVariant> savedSubnetworksXml_;
  QMap<QString, QVariant> savedSubnetworksNames_;
  QStringList toolkitFiles_, importedToolkits_;
  QMap<QString, QString> toolkitDirectories_;
  QMap<QString, Dataflow::Networks::ToolkitFile> toolkitNetworks_;
  QMap<QString, QMenu*> toolkitMenus_;
  QToolButton* executeButton_;
  QByteArray windowState_;
  QPushButton* versionButton_;
  TriggeredEventsWindow* triggeredEventsWindow_;
  MacroEditor* macroEditor_;

  void createStandardToolbars();
  void createExecuteToolbar();
  void createMacroToolbar();
  void createAdvancedToolbar();
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
  void setupScriptedEventsWindow();
  void setupDevConsole();
  void setupPreferencesWindow();
  void setupTagManagerWindow();
  void setupPythonConsole();
  void fillModuleSelector();
  void fillSavedSubnetworkMenu();
  void setupInputWidgets();
  void setupVersionButton();
  void printStyleSheet() const;
  void hideNonfunctioningWidgets();
  void setupSubnetItem(QTreeWidgetItem* fave, bool addToMap, const QString& idFromMap);
  void showStatusMessage(const QString& str);
  void showStatusMessage(const QString& str, int timeInMsec);
  void addFragmentsToMenu(const QMap<QString, QVariant>& names, const QMap<QString, QVariant>& xmls);

  void addFavoriteMenu(QTreeWidget* tree);
  QTreeWidgetItem* getTreeMenu(QTreeWidget* tree, const QString& text);
  QTreeWidgetItem* getFavoriteMenu(QTreeWidget* tree);
  QTreeWidgetItem* getClipboardHistoryMenu(QTreeWidget* tree);
  QTreeWidgetItem* getSavedSubnetworksMenu(QTreeWidget* tree);
  void addSnippet(const QString& code, QTreeWidgetItem* snips);
  void readCustomSnippets(QTreeWidgetItem* snips);
  void addSnippetMenu(QTreeWidget* tree);
  void addSavedSubnetworkMenu(QTreeWidget* tree);
  void addClipboardHistoryMenu(QTreeWidget* tree);
  QTreeWidgetItem* addFavoriteItem(QTreeWidgetItem* faves, QTreeWidgetItem* module);
  void fillTreeWidget(QTreeWidget* tree, const Dataflow::Networks::ModuleDescriptionMap& moduleMap, const QStringList& favoriteModuleNames);
  void sortFavorites(QTreeWidget* tree);

  enum { MaxRecentFiles = 5 }; //TODO: could be a user setting
  std::vector<QAction*> recentFileActions_;
  QStringList recentFiles_;
  QString currentFile_;
  QDir latestNetworkDirectory_;
  int returnCode_ { 0 };
  QMap<QString,QMap<QString,QString>> styleSheetDetails_;
  QMap<QString, QAction*> currentModuleActions_;
  QMap<QString, QMenu*> currentSubnetActions_;
  boost::shared_ptr<class DialogErrorControl> dialogErrorControl_;
  boost::shared_ptr<class NetworkExecutionProgressBar> networkProgressBar_;
  boost::shared_ptr<class GuiActionProvenanceConverter> commandConverter_;
  boost::shared_ptr<class DefaultNotePositionGetter> defaultNotePositionGetter_;
  bool quitAfterExecute_ { false };
  bool skipSaveCheck_ = false;
  bool startup_;
  boost::shared_ptr<NetworkEditorBuilder> builder_;
  int dockSpace_{0};
  class DockManager* dockManager_;
  static const QString saveFragmentData_;
  std::vector<SettingsValueInterfacePtr> settingsValues_;

Q_SIGNALS:
  void moduleItemDoubleClicked();
  void defaultNotePositionChanged(NotePosition position);
  void defaultNoteSizeChanged(int size);
  void dataDirectorySet(const QString& dir);
private Q_SLOTS:
  void addModuleKeyboardAction();
  void addModuleToWindowList(const QString& id, bool hasUI);
  void addToPathFromGUI();
  void adjustExecuteButtonAppearance();
  void adjustModuleDock(int state);
  void alertForNetworkCycles(int code);
  void changeExecuteActionIconToPlay();
  void changeExecuteActionIconToStop();
  void checkAndLoadNetworkFile(const QString& filename);
  void clearFragmentList();
  void copyVersionToClipboard();
  void displayAcknowledgement();
  void exitApplication(int code);
  void exportFragmentList();
  void filterDoubleClickedModuleSelectorItem(QTreeWidgetItem* item);
  void filterModuleNamesInTreeView(const QString& start);
  void handleCheckedModuleEntry(QTreeWidgetItem* item, int column);
  void helpWithToolkitCreation();
  void highlightPortsChanged();
  void importFragmentList();
  void importLegacyNetwork();
  void launchNewInstance();
  void launchNewUserWizard();
  void launchPythonWizard();
  void loadNetwork();
  void loadPythonAPIDoc();
  void loadRecentNetwork();
  void loadToolkit();
  void loadToolkitsFromFile(const QString& filename);
  void makePipesCubicBezier();
  void makePipesEuclidean();
  void makePipesManhattan();
  void maxCoreValueChanged(int value);
  void modulesSnapToChanged();
  void networkModified();
  void networkTimedOut();
  bool newNetwork();
  void openLogFolder();
  void openToolkitFolder();
  void openToolkitNetwork();
  void readDefaultNotePosition(int index);
  void readDefaultNoteSize(int index);
  void removeModuleFromWindowList(const SCIRun::Dataflow::Networks::ModuleId& modId);
  void removeSavedSubnetwork();
  void removeToolkit();
  void renameSavedSubnetwork();
  void resetWindowLayout();
  void runNewModuleWizard();
  void runScript();
  void runMacro();
  void updateMacroButton(int index, const QString& name);
  void saveNetwork();
  void saveNetworkAs();
  void selectModuleKeyboardAction();
  void setDragMode(bool toggle);
  void setExecutor(int type);
  void setFocusOnFilterLine();
  void setGlobalPortCaching(bool enable);
  void setSelectMode(bool toggle);
  void showClipboardHelp();
  void showKeyboardShortcutsDialog();
  void showModuleSelectorContextMenu(const QPoint& p);
  void showSnippetHelp();
  void showTagHelp();
  void showTriggerHelp();
  void showModuleFuzzySearchHelp();
  void switchMouseMode();
  void toggleFullScreen();
  void toggleMetadataLayer(bool toggle);
  void toggleTagLayer(bool toggle);
  void toolkitDownload();
  void updateClipboardHistory(const QString& xml);
  void updateDockWidgetProperties(bool isFloating);
  void zoomNetwork();
};

}
}
#endif
