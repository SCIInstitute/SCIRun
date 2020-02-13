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


#ifndef INTERFACE_APPLICATION_MAINWINDOWCOLLABORATORS_H
#define INTERFACE_APPLICATION_MAINWINDOWCOLLABORATORS_H

#ifndef Q_MOC_RUN
#include <Core/Logging/LoggerInterface.h>
#include <Core/Logging/Log.h>
#include <Core/Utils/Singleton.h>
#include <set>
#include <deque>
#include <Interface/Application/NetworkEditor.h>  //TODO
#include <Interface/Application/NetworkExecutionProgressBar.h>
#endif
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDir>
#include <QLineEdit>
#include <QMutex>
#include <QWizard>

class QTextEdit;
class QTreeWidget;
class QComboBox;
class QStatusBar;

namespace SCIRun {
namespace Gui {

  class SCIRunMainWindow;
  class ModuleDialogGeneric;

  class TextEditAppender : public Core::Logging::LogAppenderStrategy
  {
  public:
    explicit TextEditAppender(QTextEdit* text) : text_(text) {}
    void log(const QString& message) const;
    virtual void log4(const std::string& message) const override;
  private:
    QTextEdit* text_;
    mutable QMutex mutex_;
  };

  class TreeViewModuleGetter : public CurrentModuleSelection
  {
  public:
    explicit TreeViewModuleGetter(QTreeWidget& tree) : tree_(tree) {}
    QString text() const override;
    QString clipboardXML() const override;
    bool isModule() const override;
    bool isClipboardXML() const override;
  private:
    QTreeWidget& tree_;
  };

  class ComboBoxDefaultNotePositionGetter : public DefaultNotePositionGetter
  {
  public:
    ComboBoxDefaultNotePositionGetter(QComboBox* positionCombo, QComboBox* sizeCombo) : positionCombo_(positionCombo), sizeCombo_(sizeCombo) {}
    NotePosition position() const override;
    int size() const override;
  private:
    QComboBox* positionCombo_;
    QComboBox* sizeCombo_;
  };

  typedef boost::variant<QAction*, QWidget*> InputWidget;

  // During network execution, this class manages disabling parts of the GUI
  class WidgetDisablingService : public QObject, boost::noncopyable
  {
    Q_OBJECT

    CORE_SINGLETON( WidgetDisablingService );

  private:
    WidgetDisablingService() {}
  public Q_SLOTS:
    void disableInputWidgets();
    void enableInputWidgets();
    void temporarilyDisableService();
    void temporarilyEnableService();
  public:
    void addNetworkEditor(NetworkEditor* ne);
    void addWidget(const InputWidget& w);
    void removeWidget(const InputWidget& w);

    template <class Iter>
    void addWidgets(Iter begin, Iter end)
    {
      std::copy(begin, end, std::back_inserter(inputWidgets_));
    }
  private:
    NetworkEditor* ne_ {nullptr};
    std::vector<InputWidget> inputWidgets_;
    bool serviceEnabled_ {true};
  };

  inline void addWidgetToExecutionDisableList(const InputWidget& w)
  {
    WidgetDisablingService::Instance().addWidget(w);
  }
  inline void removeWidgetFromExecutionDisableList(const InputWidget& w)
  {
    WidgetDisablingService::Instance().removeWidget(w);
  }


  class FileDownloader : public QObject
  {
    Q_OBJECT

  public:
    explicit FileDownloader(QUrl imageUrl, QStatusBar* statusBar, QObject *parent = nullptr);
    QByteArray downloadedData() const { return downloadedData_; }

  Q_SIGNALS:
    void downloaded();

  private Q_SLOTS:
    void fileDownloaded(QNetworkReply* reply);
    void downloadProgress(qint64 received, qint64 total) const;
  private:
    QNetworkAccessManager webCtrl_;
    QNetworkReply* reply_;
    QByteArray downloadedData_;
    QStatusBar* statusBar_;
  };

  class ToolkitDownloader : public QObject
  {
    Q_OBJECT
  public:
    explicit ToolkitDownloader(QObject* infoObject, QStatusBar* statusBar, QWidget* parent = nullptr);
  private Q_SLOTS:
    void showMessageBox();
    void saveToolkit() const;

  private:
    void downloadIcon(); //TODO: cache somehow
    FileDownloader* iconDownloader_;
    FileDownloader* zipDownloader_;
    QString iconUrl_, iconKey_, fileUrl_, filename_;
    QDir toolkitDir_;
    QStatusBar* statusBar_;
  };

  class NewUserWizard : public QWizard
  {
    Q_OBJECT
  public:
    explicit NewUserWizard(QWidget* parent);
    ~NewUserWizard();
  public Q_SLOTS:
    void setShowPrefs(int state);
  private Q_SLOTS:
    void updatePathLabel(const QString& dir);
    void showPrefs();
  private:
    QWizardPage* createIntroPage();
    QWizardPage* createPathSettingPage();
    QWizardPage* createLicensePage();
    QWizardPage* createConnectionChoicePage();
    QWizardPage* createDocPage();
    QWizardPage* createOtherSettingsPage();
    QLineEdit* pathWidget_;
    bool showPrefs_{ false };
  };

  class PythonWizard : public QWizard
  {
    Q_OBJECT
  public:
    enum {  Page_Home, Page_Create_Intro, Page_Save, Page_LatVol, Page_MeshBox, Page_Connection,
            Page_CalcData, Page_Iso, Page_ShowField, Page_ViewScene, Page_Execute,
            Page_Load_Intro, Page_Load,
            Page_Wand_Intro, Page_Wand,
            Page_Int_Intro, Page_Base, Page_IntPy, Page_SetPy};
    explicit PythonWizard(std:: function<void(const QString&)> display, QWidget* parent);
    ~PythonWizard();
  public Q_SLOTS:
    void setShowPrefs(int state);
  private Q_SLOTS:
    void updatePathLabel(const QString& dir);
    void showPrefs();
    void customClicked(int which);
    void switchPage(QAbstractButton* button);


  private:
    QWizardPage* createIntroPage();
    QWizardPage* createCreateIntroPage();
    QWizardPage* createSaveNetworkPage();
    QWizardPage* createLatVolPage();
    QWizardPage* createEditMeshBoundingBoxPage();
    QWizardPage* createConnectionPage();
    QWizardPage* createCalculateFieldDataPage();
    QWizardPage* createExtractIsosurfacePage();
    QWizardPage* createShowFieldPage();
    QWizardPage* createViewScenePage();
    QWizardPage* createExecutePage();

    QWizardPage* createLoadingNetworkIntroPage();
    QWizardPage* createLoadNetworkPage();

    QWizardPage* createWandIntroPage();
    QWizardPage* createWandPage();

    QWizardPage* createInterfaceIntroPage();
    QWizardPage* createBaseNetworkPage();
    QWizardPage* createAddIntPyPage();
    QWizardPage* createSetPythonPage();

    QLineEdit* pathWidget_;
    bool showPrefs_{ false };
    std::function<void(const QString&)> displayPython_;
  };

  struct ToolkitInfo
  {
    static const char* ToolkitIconURL;
    static const char* ToolkitURL;
    static const char* ToolkitFilename;

    QString iconUrl, zipUrl, filename;

    void setupAction(QAction* action, QObject* window) const;
  };

  class NetworkStatusImpl : public NetworkStatus
  {
  public:
    explicit NetworkStatusImpl(NetworkEditor* ned) : ned_(ned) {}
    size_t total() const override;
    size_t waiting() const override;
    size_t executing() const override;
    size_t errored() const override;
    size_t nonReexecuted() const override;
    size_t finished() const override;
    size_t unexecuted() const override;
  private:
    NetworkEditor* ned_;
    size_t countState(Dataflow::Networks::ModuleExecutionState::Value val) const;
  };

  class NetworkEditorBuilder
  {
  public:
    explicit NetworkEditorBuilder(SCIRunMainWindow* mainWindow) : mainWindow_(mainWindow) {}
    void connectAll(NetworkEditor* editor);
  private:
    SCIRunMainWindow* mainWindow_;
  };

  class DockManager : public QObject
  {
    Q_OBJECT
  public:
    explicit DockManager(int& availableSize, QObject* parent);
  public Q_SLOTS:
    void requestShow(ModuleDialogGeneric* dialog);
  private:
    int& availableHeight_;
    const std::set<ModuleDialogGeneric*>& currentDialogs_;
    std::deque<ModuleDialogGeneric*> collapseQueue_;
    int usedSpace() const;
  };

  QString networkBackgroundImage();
  QString standardNetworkBackgroundImage();

  //TODO: global function replacements for SCIRunMainWindow access. extract into new file/namespace
  QString scirunStylesheet();
  QMainWindow* mainWindowWidget();

  class SCIRunGuiRunner
  {
  public:
    explicit SCIRunGuiRunner(QApplication& app);
    int returnCode();
    static void reportIssue();
  };
}
}
#endif
