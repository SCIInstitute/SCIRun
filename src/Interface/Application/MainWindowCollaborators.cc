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

#include <QtGui>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Core/Logging/Log.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>

#include "ui_ConnectionStyleWizardPage.h"
#include "ui_OtherSettingsWizardPage.h"

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Dataflow::Networks;

void TextEditAppender::log(const QString& message) const
{
  QMutexLocker locker(&mutex_);
  text_->append(message);
  text_->verticalScrollBar()->setValue(text_->verticalScrollBar()->maximum());
}

void TextEditAppender::error(const std::string& msg) const
{
  Log::get() << ERROR_LOG << msg << std::endl;
}

void TextEditAppender::warning(const std::string& msg) const
{
  Log::get() << WARN << msg << std::endl;
}

void TextEditAppender::remark(const std::string& msg) const
{
  Log::get() << NOTICE << msg << std::endl;
}

void TextEditAppender::status(const std::string& msg) const
{
  auto level = regressionMode_ ? INFO : DEBUG_LOG;
  Log::get() << level << msg << std::endl;
}

void TextEditAppender::log4(const std::string& message) const
{
  log(QString::fromStdString(message));
}

QString TreeViewModuleGetter::text() const
{
  return tree_.currentItem()->text(0);
}

bool TreeViewModuleGetter::isModule() const
{
  auto current = tree_.currentItem();
  return current && current->childCount() == 0 && current->parent() && !current->text(0).startsWith("clipboard") && current->textColor(0) != CLIPBOARD_COLOR;
}

QString TreeViewModuleGetter::clipboardXML() const
{
  return tree_.currentItem()->data(0, SCIRunMainWindow::clipboardKey).toString();
}

bool TreeViewModuleGetter::isClipboardXML() const
{
  auto current = tree_.currentItem();
  return current && current->childCount() == 0 && current->parent() && (current->text(0).startsWith("clipboard") || current->textColor(0) == CLIPBOARD_COLOR);
}

NotePosition ComboBoxDefaultNotePositionGetter::position() const
{
  return NotePosition(combo_.currentIndex() + 1);
}

CORE_SINGLETON_IMPLEMENTATION( WidgetDisablingService )

namespace
{
  class SetDisableFlag : public boost::static_visitor<>
  {
  public:
    explicit SetDisableFlag(bool flag) : flag_(flag) {}
    template <typename T>
    void operator()( T* widget ) const
    {
      //TODO: investigate this Mac Qt bug in more detail. A better workaround probably exists. (Or just wait until Qt 5)
      if (widget)
        widget->setDisabled(flag_);
    }
    bool flag_;
  };

  void setWidgetsDisableFlag(std::vector<InputWidget>& widgets, bool flag)
  {
    std::for_each(widgets.begin(), widgets.end(), [=](InputWidget& v) { boost::apply_visitor(SetDisableFlag(flag), v); });
  }
}

void WidgetDisablingService::addNetworkEditor(NetworkEditor* ne)
{
  ne_ = ne;
}

void WidgetDisablingService::addWidget(const InputWidget& w)
{
  inputWidgets_.push_back(w);
}

void WidgetDisablingService::removeWidget(const InputWidget& w)
{
  inputWidgets_.erase(std::remove(inputWidgets_.begin(), inputWidgets_.end(), w));
}

void WidgetDisablingService::disableInputWidgets()
{
  ne_->disableInputWidgets();
  setWidgetsDisableFlag(inputWidgets_, true);
  //qDebug() << "disabling widgets; service on?" << serviceEnabled_;
}

void WidgetDisablingService::enableInputWidgets()
{
  ne_->enableInputWidgets();
  setWidgetsDisableFlag(inputWidgets_, false);
  //qDebug() << "enabling widgets; service on?" << serviceEnabled_;
}

void WidgetDisablingService::temporarilyDisableService()
{
  //qDebug() << "temp disable service";
  serviceEnabled_ = false;
}

void WidgetDisablingService::temporarilyEnableService()
{
  //qDebug() << "temp enable service";
  serviceEnabled_ = true;
}

NewUserWizard::NewUserWizard(QWidget* parent) : QWizard(parent)
{
  setWindowTitle("SCIRun Initial Setup");
  setOption(NoCancelButton);

  addPage(createIntroPage());
  addPage(createPathSettingPage());
  addPage(createConnectionChoicePage());
  addPage(createOtherSettingsPage());
  addPage(createLicensePage());
  addPage(createDocPage());
}

NewUserWizard::~NewUserWizard()
{
  showPrefs();
}

void NewUserWizard::showPrefs()
{
  if (showPrefs_)
    SCIRunMainWindow::Instance()->actionPreferences_->trigger();
}

QWizardPage* NewUserWizard::createIntroPage()
{
  auto page = new QWizardPage;
  page->setTitle("Introduction");

  page->setSubTitle("This wizard will help you set up SCIRun for the first time and learn the basic SCIRun operations and hotkeys. All of these settings are available at any time in the Preferences window.");
  auto layout = new QVBoxLayout;
  auto pic = new QLabel;
  pic->setPixmap(QPixmap(":/general/Resources/scirunWizard.png"));
  layout->addWidget(pic);
  page->setLayout(layout);
  return page;
}

class PathSettingPage : public QWizardPage
{
public:
  explicit PathSettingPage(QLineEdit* pathWidget)
  {
    registerField("dataPath*", pathWidget);
  }
};

QWizardPage* NewUserWizard::createPathSettingPage()
{
  pathWidget_ = new QLineEdit("");
  pathWidget_->setReadOnly(true);
  auto page = new PathSettingPage(pathWidget_);
  page->setTitle("Configuring Paths");
  page->setSubTitle("Specify the location of SCIRun's data folder. This path is referenced in network files and modules using the code %SCIRUNDATADIR%.");
  auto downloadLabel = new QLabel("The data can be downloaded from <a href=\"http://www.sci.utah.edu/download/scirun/\">sci.utah.edu</a>");
  downloadLabel->setOpenExternalLinks(true);
  auto layout = new QVBoxLayout;
  layout->addWidget(downloadLabel);

  layout->addWidget(pathWidget_);
  auto button = new QPushButton("Set Path...");
  layout->addWidget(button);
  connect(button, SIGNAL(clicked()), SCIRunMainWindow::Instance(), SLOT(setDataDirectoryFromGUI()));
  connect(SCIRunMainWindow::Instance(), SIGNAL(dataDirectorySet(const QString&)), this, SLOT(updatePathLabel(const QString&)));

  page->setLayout(layout);

  return page;
}

QWizardPage* NewUserWizard::createLicensePage()
{
  auto page = new QWizardPage;
  page->setTitle("Applicable Licenses");
  QString licenseText(
    "<p><a href = \"https://raw.githubusercontent.com/SCIInstitute/SCIRun/master/src/LICENSE.txt\">SCIRun License</a>"
    "<p><a href = \"https://raw.githubusercontent.com/CIBC-Internal/teem/master/LICENSE.txt\">Teem License</a>"
//#if WITH_TETGEN
//    "<p><a href = \"http://wias-berlin.de/software/tetgen/1.5/FAQ-license.html\">Tetgen License</a>"
//#endif
    );
  auto layout = new QVBoxLayout;
  auto licenseLabel = new QLabel(licenseText);
  licenseLabel->setStyleSheet("QLabel { background-color : lightgray; color : blue; }");
  licenseLabel->setAlignment(Qt::AlignCenter);
  licenseLabel->setOpenExternalLinks(true);
  layout->addWidget(licenseLabel);
  page->setLayout(layout);
  return page;
}

QWizardPage* NewUserWizard::createDocPage()
{
  auto page = new QWizardPage;
  page->setTitle("Documentation");
  page->setSubTitle("For more information on SCIRun 5 functionality, documentation can be found at: ");
  auto layout = new QVBoxLayout;
  auto docLabel = new QLabel(
    "<p><a href = \"https://github.com/SCIInstitute/SCIRun/wiki\">New SCIRun Wiki</a>"
    "<p><a href = \"http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Documentation:SCIRun:Reference\">Old SCIRun Wiki</a>"
    "<p><a href = \"http://sciinstitute.github.io/scirun.pages/\">SCIRun Doc Home Page</a>"
    "<p><a href = \"mailto:scirun-users@sci.utah.edu\">SCIRun Users mailing list</a>"
  );
  docLabel->setStyleSheet("QLabel { background-color : lightgray; color : blue; }");
  docLabel->setAlignment(Qt::AlignCenter);
  docLabel->setOpenExternalLinks(true);
  layout->addWidget(docLabel);
  page->setLayout(layout);
  return page;
}

void NewUserWizard::updatePathLabel(const QString& dir)
{
  pathWidget_->setText(dir);
}

void NewUserWizard::setShowPrefs(int state)
{
  showPrefs_ = state != 0;
}

class ConnectionStyleWizardPage : public QWizardPage, public Ui::ConnectionStyleWizardPage
{
public:
  ConnectionStyleWizardPage()
  {
    setupUi(this);
    manhattanLabel_->setPixmap(QPixmap(":/general/Resources/manhattanPipe.png"));
    euclideanLabel_->setPixmap(QPixmap(":/general/Resources/euclideanPipe.png"));
    cubicLabel_->setPixmap(QPixmap(":/general/Resources/cubicPipe.png"));
    registerField("connectionChoice*", connectionComboBox_);
    connect(connectionComboBox_, SIGNAL(currentIndexChanged(int)), SCIRunMainWindow::Instance(), SLOT(setConnectionPipelineType(int)));
  }
};

QWizardPage* NewUserWizard::createConnectionChoicePage()
{
  return new ConnectionStyleWizardPage;
}

class OtherSettingsWizardPage : public QWizardPage, public Ui::OtherSettingsWizardPage
{
public:
  explicit OtherSettingsWizardPage(NewUserWizard* wiz)
  {
    setupUi(this);
    connect(saveBeforeExecuteCheckBox_, SIGNAL(stateChanged(int)), SCIRunMainWindow::Instance(), SLOT(setSaveBeforeExecute(int)));
    connect(loadPreferencesCheckBox_, SIGNAL(stateChanged(int)), wiz, SLOT(setShowPrefs(int)));
  }
};

QWizardPage* NewUserWizard::createOtherSettingsPage()
{
  return new OtherSettingsWizardPage(this);
}

void ToolkitInfo::setupAction(QAction* action, QObject* window) const
{
  QObject::connect(action, SIGNAL(triggered()), window, SLOT(toolkitDownload()));
  action->setProperty(ToolkitIconURL, iconUrl);
  action->setProperty(ToolkitURL, zipUrl);
  action->setProperty(ToolkitFilename, filename);
  action->setIcon(QPixmap(":/general/Resources/download.png"));
}

const char* ToolkitInfo::ToolkitIconURL{ "ToolkitIconURL" };
const char* ToolkitInfo::ToolkitURL{ "ToolkitURL" };
const char* ToolkitInfo::ToolkitFilename{ "ToolkitFilename" };

size_t NetworkStatusImpl::total() const
{
  return ned_->numModules();
}
size_t NetworkStatusImpl::waiting() const
{
  return countState(ModuleExecutionState::Value::Waiting);
}
size_t NetworkStatusImpl::executing() const
{
  return countState(ModuleExecutionState::Value::Executing);
}
size_t NetworkStatusImpl::errored() const
{
  return countState(ModuleExecutionState::Value::Errored);
}
size_t NetworkStatusImpl::nonReexecuted() const
{
  return -1; // not available yet
}
size_t NetworkStatusImpl::finished() const
{
  return countState(ModuleExecutionState::Value::Completed);
}
size_t NetworkStatusImpl::unexecuted() const
{
  return countState(ModuleExecutionState::Value::NotExecuted);
}

size_t NetworkStatusImpl::countState(ModuleExecutionState::Value val) const
{
  auto allStates = ned_->getNetworkEditorController()->moduleExecutionStates();
  return std::count(allStates.begin(), allStates.end(), val);
}

void NetworkEditorBuilder::connectAll(NetworkEditor* editor)
{
  // for any network editor
  QObject::connect(editor, SIGNAL(modified()), mainWindow_, SLOT(networkModified()));
  QObject::connect(mainWindow_, SIGNAL(defaultNotePositionChanged(NotePosition)), editor, SIGNAL(defaultNotePositionChanged(NotePosition)));

  // for active network editor
  QObject::connect(mainWindow_->actionSelectAll_, SIGNAL(triggered()), editor, SLOT(selectAll()));
  QObject::connect(mainWindow_->actionDelete_, SIGNAL(triggered()), editor, SLOT(del()));
  QObject::connect(mainWindow_->actionCleanUpNetwork_, SIGNAL(triggered()), editor, SLOT(cleanUpNetwork()));
  QObject::connect(editor, SIGNAL(zoomLevelChanged(int)), mainWindow_, SLOT(showZoomStatusMessage(int)));
  QObject::connect(mainWindow_->actionCut_, SIGNAL(triggered()), editor, SLOT(cut()));
  QObject::connect(mainWindow_->actionCopy_, SIGNAL(triggered()), editor, SLOT(copy()));
  QObject::connect(mainWindow_->actionPaste_, SIGNAL(triggered()), editor, SLOT(paste()));

  if (!editor->parentNetwork())
  {
    // root NetworkEditor only.
    QObject::connect(mainWindow_, SIGNAL(moduleItemDoubleClicked()), editor, SLOT(addModuleViaDoubleClickedTreeItem()));
    QObject::connect(mainWindow_->actionCenterNetworkViewer_, SIGNAL(triggered()), editor, SLOT(centerView()));
    QObject::connect(mainWindow_->actionPinAllModuleUIs_, SIGNAL(triggered()), editor, SLOT(pinAllModuleUIs()));
    QObject::connect(mainWindow_->actionRestoreAllModuleUIs_, SIGNAL(triggered()), editor, SLOT(restoreAllModuleUIs()));
    QObject::connect(mainWindow_->actionHideAllModuleUIs_, SIGNAL(triggered()), editor, SLOT(hideAllModuleUIs()));
    QObject::connect(mainWindow_->actionMakeSubnetwork_, SIGNAL(triggered()), editor, SLOT(makeSubnetwork()));
  }
  // children only
  // addDockWidget(Qt::RightDockWidgetArea, subnet);
}
