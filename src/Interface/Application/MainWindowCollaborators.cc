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
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Core/Logging/Log.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/GuiCommandFactory.h>
#include <Core/Application/Application.h>
#include <numeric>

#include "ui_ConnectionStyleWizardPage.h"
#include "ui_OtherSettingsWizardPage.h"
#include "ui_PythonWizardCodePage.h"

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Dataflow::Networks;

void TextEditAppender::log(const QString& message) const
{
  QMutexLocker locker(&mutex_);
  text_->append(message);
  text_->verticalScrollBar()->setValue(text_->verticalScrollBar()->maximum());
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
  return NotePosition(positionCombo_->currentIndex() + 1);
}

int ComboBoxDefaultNotePositionGetter::size() const
{
  return sizeCombo_->currentText().toInt();
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
    "<p><a href = \"http://sciinstitute.github.io/scirun.pages/\">SCIRun Doc Home Page</a>"
    "<p><a href = \"https://github.com/SCIInstitute/SCIRun/wiki\">New SCIRun Wiki</a>"
    "<p><a href = \"http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php5/CIBC:Documentation:SCIRun:Reference\">Old SCIRun Wiki</a>"
    "<p><a href = \"https://lists.sci.utah.edu/sympa/arc/scirun-users\">SCIRun Users mailing list</a>"
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
    registerField("connectionChoice", connectionComboBox_);
    SCIRunMainWindow::Instance()->setConnectionPipelineType(0);
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

PythonWizard::PythonWizard(std:: function<void(const QString&)> display, QWidget* parent) : displayPython_(display), QWizard(parent)
{
  setWindowTitle("SCIRun Python Help");
  setOption(NoBackButtonOnStartPage);

  resize(1000,450);
  setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);

  addPage(createIntroPage());
  addPage(createLatVolPage());
  addPage(createEditMeshBoundingBoxPage());
  addPage(createLatVolMeshBoxConnectionPage());
  addPage(createCalculateFieldDataPage());
  addPage(createNetworkEditingPage());
  addPage(createModuleStateEditingPage());
  addPage(createModuleInputPage());
  addPage(createSpecialSyntaxPage());
  addPage(createTopLevelScriptPage());
  addPage(createRunningPythonScriptsPage());


}

PythonWizard::~PythonWizard()
{
  showPrefs();
}

void PythonWizard::showPrefs()
{
  if (showPrefs_)
    SCIRunMainWindow::Instance()->actionPreferences_->trigger();
}

QWizardPage* PythonWizard::createIntroPage()
{
  auto page = new QWizardPage;
  page->setTitle("Welcome to Python Wizard");

  page->setSubTitle("This wizard will help you implement Python in SCIRun for the first time.");
  auto layout = new QVBoxLayout;
  auto pic = new QLabel;
  pic->setPixmap(QPixmap(":/general/Resources/scirunWizard.png"));
  layout->addWidget(pic);
  page->setLayout(layout);
  return page;
}

class PythonWizardCodePage : public QWizardPage, public Ui::PythonWizardCodePage
{
public:
  PythonWizardCodePage()
  {
    setupUi(this);
  }
};

QWizardPage* PythonWizard::createLatVolPage()
{
  auto page = new PythonWizardCodePage;
  page->infoText->setText("This will create a \"CreateLatVol\" module\n"
    "\n"
    "Then edit it's parameters");

  page->codeEdit->setPlainText("lat = scirun_add_module(\"CreateLatVol\")\n"
    "\n\n"
    "scirun_set_module_state(lat, \"XSize\", 20)\n"
    "scirun_set_module_state(lat, \"YSize\", 20)\n"
    "scirun_set_module_state(lat, \"ZSize\", 20)\n"
    "scirun_set_module_state(lat, \"ElementSizeNormalized\", 1)");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}

QWizardPage* PythonWizard::createEditMeshBoundingBoxPage()
{
  auto page = new PythonWizardCodePage;
  page->infoText->setText("This will create a \"EditMeshBoundingBox\" module to move the LatVol"
    "\n\n"
    "Then edit it's parameters");

  page->codeEdit->setPlainText("edit_box = scirun_add_module(\"EditMeshBoundingBox\")\n"
    "\n\n\n"
    "scirun_set_module_state(edit_box, \"OutputCenterX\", 0)\n"
    "scirun_set_module_state(edit_box, \"OutputCenterY\", 0)\n"
    "scirun_set_module_state(edit_box, \"OutputCenterZ\", 0)\n");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}

QWizardPage* PythonWizard::createLatVolMeshBoxConnectionPage()
{
  auto page = new PythonWizardCodePage;
  page->infoText->setText("Connect the LatVol and the EditMeshBoundingBox");

  page->codeEdit->setPlainText("scirun_connect_modules(lat, 0, edit_box, 0)");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}

QWizardPage* PythonWizard::createCalculateFieldDataPage()
{
  auto page = new PythonWizardCodePage;
  page->infoText->setText("Create a CalculateFieldData module to assign values to the LatVol");

  page->codeEdit->setPlainText("calc = scirun_add_module(\"CalculateFieldData\")");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}


QWizardPage* PythonWizard::createNetworkEditingPage()
{
  auto page = new QWizardPage;
  page->setTitle("Global Functions (Network Editing)");

  page->setSubTitle(R"( scirun_module_ids()
      Returns a list of all module ID strings in the current network, sorted by module creation time.)");
  auto layout = new QVBoxLayout;
  page->setLayout(layout);
  return page;
}

QWizardPage* PythonWizard::createModuleStateEditingPage()
{
  auto page = new QWizardPage;
  page->setTitle("Global Functions (Module State Editing)");

  page->setSubTitle(R"( scirun_get_module_state("ModuleID", "StateVariableName")
      Returns the value of the specified module state variable.

  scirun_set_module_state("ModuleID", "StateVariableName", value)
      Sets the specified module state variable's value

  scirun_dump_module_state("ModuleID")
      Returns a dictionary with the entire state of the specified module.

  scirun_get_module_transient_state("ModuleID", "StateVariableName")
      Returns the value of the specified module transient state variable.

  scirun_set_module_transient_state("ModuleID", "StateVariableName", value)
      Sets the specified module transient state variable's value. Used to pass data values (strings, matrices, fields) back to modules.)");
  auto layout = new QVBoxLayout;
  page->setLayout(layout);
  return page;
}

QWizardPage* PythonWizard::createModuleInputPage()
{
  auto page = new QWizardPage;
  page->setTitle("Global Functions (Module/DataType Input)");

  page->setSubTitle(R"( scirun_get_module_input_type("ModuleID", portIndex)
      Returns the type of the input data object on the specified port.

  scirun_get_module_input_object("ModuleID", "PortName")
      Returns a special PyDatatype wrapper object containing a copy of the data on the specified input port, by name.

  scirun_get_module_input_value("ModuleID", "PortName")
      Returns a Python object containing a copy of the data on the specified input port.

  scirun_get_module_input_object_by_index("ModuleID", portIndex)
      Returns a special Pydatatype wrapper object containing a copy of the data on the specified input port, by port index.

  scirun_get_module_input_value_by_index("ModuleID", portIndex)
      Returns a Python object containing a copy of the data on the specified input port, by index.)");
  auto layout = new QVBoxLayout;
  page->setLayout(layout);
  return page;
}

QWizardPage* PythonWizard::createSpecialSyntaxPage()
{
  auto page = new QWizardPage;
  page->setTitle("InterfaceWithPython (Special Syntax)");

  page->setSubTitle(R"(This module lets you set input/output variable names in the module UI. Once this is done (or by using the defaults), one can use assignment syntaxt to read input data and send output data.

  Examples:
        pythonOutput = "hello"    will send a string to the output port associated with   pythonOutput
        field = inputField1    will extract a field object from the input port associated with   inputField1
        outputString1 = "string concat" + inputString1    Input/Output can be combined on the same line.)");
  auto layout = new QVBoxLayout;
  page->setLayout(layout);
  return page;
}

QWizardPage* PythonWizard::createTopLevelScriptPage()
{
  auto page = new QWizardPage;
  page->setTitle("InterfaceWithPython (Top-Level Script)");

  page->setSubTitle(R"(In the InterfaceWithPython, there is a "Top-Level Script" tab which allows users to run matlab code in a broader scope on execution of the InterfaceWithPython Module.
This is helpful if there are variables or modules that are costly to compute or load, yet are used in more than one InterfaceWithPython Module.
The Matlab engine is one such example; to launch the matlab for this session, click the button below.)");
  auto layout = new QVBoxLayout;
  page->setLayout(layout);
  return page;
}

QWizardPage* PythonWizard::createRunningPythonScriptsPage()
{
  auto page = new QWizardPage;
  page->setTitle("Running Python Scripts");

  page->setSubTitle(R"(Python scripts can be used for many things in SCIRun, from building networks to batch executing data.

  To run a script saved to disk within SCIRun, there are a couple options:
    There is a run script option (looks like a magic wand), which is part of the advanced tool bar. This option will clear any network when a script is run, so it is great for running scripts that build and execute networks.

    Scripts can alsobe called as a command line input with the -s or -S flags. This option is similar to the run script tool, but it allows for passing script agruments after the script filename.
      For example, in OS X:  /Applications/SCIRun.app/Contents/MacOS/SCIRun -s *script_filename.py* *argl* .

    To run a script within SCIRun without clearing the network, open and exectute the script in the SCIRun python console:  exec(open('*path_to_script/filename.py*').read()) . This syntax can also be used in  SCIRun's interactive mode ( -i flag from the command line).)");
  auto layout = new QVBoxLayout;
  page->setLayout(layout);
  return page;
}

void PythonWizard::updatePathLabel(const QString& dir)
{
  pathWidget_->setText(dir);
}

void PythonWizard::setShowPrefs(int state)
{
  showPrefs_ = state != 0;
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
  if (!ned_)
  {
    return 0;
  }
  if (!ned_->getNetworkEditorController())
  {
    return 0;
  }
  auto allStates = ned_->getNetworkEditorController()->moduleExecutionStates();
  return std::count(allStates.begin(), allStates.end(), val);
}

void NetworkEditorBuilder::connectAll(NetworkEditor* editor)
{
  // for any network editor
  QObject::connect(editor, SIGNAL(modified()), mainWindow_, SLOT(networkModified()));
  QObject::connect(mainWindow_, SIGNAL(defaultNotePositionChanged(NotePosition)), editor, SIGNAL(defaultNotePositionChanged(NotePosition)));
  QObject::connect(mainWindow_, SIGNAL(defaultNoteSizeChanged(int)), editor, SIGNAL(defaultNoteSizeChanged(int)));

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

DockManager::DockManager(int& availableSize, QObject* parent) : QObject(parent),
  availableHeight_(availableSize),
  currentDialogs_(ModuleDialogGeneric::instances())
{

}

void DockManager::requestShow(ModuleDialogGeneric* dialog)
{
  //clear out old pointers
  collapseQueue_.erase(std::remove_if(collapseQueue_.begin(), collapseQueue_.end(),
    [this](ModuleDialogGeneric* d) { return currentDialogs_.find(d) == currentDialogs_.end(); }),
    collapseQueue_.end());

  // collapse oldest until they fit
  auto needToFit = availableHeight_ - dialog->size().height();
  while (usedSpace() > needToFit)
  {
    auto firstNonCollapsed = std::find_if(collapseQueue_.begin(), collapseQueue_.end(),
      [dialog](ModuleDialogGeneric* d) { return d != dialog && !d->isCollapsed(); });
    if (firstNonCollapsed != collapseQueue_.end())
      (*firstNonCollapsed)->collapse();
    else
      break;
  }

  // add latest
  collapseQueue_.push_back(dialog);
  dialog->expand();
}

int DockManager::usedSpace() const
{
  return std::accumulate(currentDialogs_.begin(), currentDialogs_.end(), 0,
    [](int height, ModuleDialogGeneric* d) { return height + (d->isCollapsed() ? 0 : d->size().height()); });
}

QString SCIRun::Gui::networkBackgroundImage()
{
  auto date = QDate::currentDate();
  if (12 == date.month() && 23 < date.day() && date.day() < 27)
    return ":/general/Resources/ski.png";
  if (11 == date.month() && 21 < date.day() && date.day() < 29 && 4==date.dayOfWeek())
    return ":/general/Resources/turkey.png";
  if (10 == date.month() && 31 == date.day())
    return ":/general/Resources/pumpkin.png";
  return standardNetworkBackgroundImage();
}

QString SCIRun::Gui::standardNetworkBackgroundImage()
{
  return ":/general/Resources/SCIgrid-small.png";
}

QString SCIRun::Gui::scirunStylesheet()
{
  return SCIRunMainWindow::Instance()->styleSheet();
}

QMainWindow* SCIRun::Gui::mainWindowWidget()
{
  return SCIRunMainWindow::Instance();
}

SCIRunGuiRunner::SCIRunGuiRunner(QApplication& app)
{
  auto mainWin = SCIRunMainWindow::Instance();

  Core::Application::Instance().setCommandFactory(boost::make_shared<GuiGlobalCommandFactory>());
  mainWin->setController(Core::Application::Instance().controller());
  mainWin->initialize();

  app.setStyleSheet("*{"
                    "color:white;"
                    "selection-background-color:blue;" // 336699 lighter blue
                    "background-color:rgb(66,66,69);"
                    "selection-color:yellow;}"
                    "QCheckBox,QLabel,QRadioButton{background-color:transparent}"
                    "QLineEdit{border:1px solid white}"
                    "QToolBar{"
                    "border:1px solid black;"
                    "color:black;}"
                    "QProgressBar{"
                    "border:0px solid black;"
                    "color:black;}");
  app.exec();
}

int SCIRunGuiRunner::returnCode()
{
  return SCIRunMainWindow::Instance()->returnCode();
}

FileDownloader::FileDownloader(QUrl imageUrl, QStatusBar* statusBar, QObject *parent) : QObject(parent), reply_(nullptr), statusBar_(statusBar)
{
  connect(&webCtrl_, SIGNAL(finished(QNetworkReply*)), this, SLOT(fileDownloaded(QNetworkReply*)));

  QNetworkRequest request(imageUrl);
  reply_ = webCtrl_.get(request);
  connect(reply_, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
}

void FileDownloader::fileDownloaded(QNetworkReply* reply)
{
  downloadedData_ = reply->readAll();
  reply->deleteLater();
  Q_EMIT downloaded();
}

void FileDownloader::downloadProgress(qint64 received, qint64 total) const
{
  if (statusBar_)
  {
    auto totalStr = total == -1 ? "?" : QString::number(total);
    statusBar_->showMessage(tr("File progress: %1 / %2").arg(received).arg(totalStr), 1000);
    if (received == total)
      statusBar_->showMessage("File downloaded.", 1000);
  }
}

ToolkitDownloader::ToolkitDownloader(QObject* infoObject, QStatusBar* statusBar, QWidget* parent) : QObject(parent), iconDownloader_(nullptr), zipDownloader_(nullptr), statusBar_(statusBar)
{
  if (infoObject)
  {
    iconUrl_ = infoObject->property(ToolkitInfo::ToolkitIconURL).toString();
    fileUrl_ = infoObject->property(ToolkitInfo::ToolkitURL).toString();
    filename_ = infoObject->property(ToolkitInfo::ToolkitFilename).toString();

    iconKey_ = ToolkitInfo::ToolkitIconURL + QString("--") + iconUrl_;

    downloadIcon();
  }
}

void ToolkitDownloader::downloadIcon()
{
  QSettings settings;
  if (!settings.contains(iconKey_))
  {
    iconDownloader_ = new FileDownloader(iconUrl_, nullptr, this);
    connect(iconDownloader_, SIGNAL(downloaded()), this, SLOT(showMessageBox()));
  }
  else
    showMessageBox();
}

void ToolkitDownloader::showMessageBox()
{
  QPixmap image;

  QSettings settings;

  if (settings.contains(iconKey_))
  {
    image.loadFromData(settings.value(iconKey_).toByteArray());
  }
  else
  {
    if (!iconDownloader_)
      return;

    image.loadFromData(iconDownloader_->downloadedData());
    settings.setValue(iconKey_, iconDownloader_->downloadedData());
  }

  QMessageBox toolkitInfo;
#ifdef WIN32
  toolkitInfo.setWindowTitle("Toolkit information");
#else
  toolkitInfo.setText("Toolkit information");
#endif
  toolkitInfo.setInformativeText("Click OK to download the latest version of this toolkit:\n\n" + fileUrl_);
  toolkitInfo.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  toolkitInfo.setIconPixmap(image);
  toolkitInfo.setDefaultButton(QMessageBox::Ok);
  toolkitInfo.show();
  auto choice = toolkitInfo.exec();

  if (choice == QMessageBox::Ok)
  {
    auto dir = QFileDialog::getExistingDirectory(qobject_cast<QWidget*>(parent()), "Select toolkit directory", ".");
    if (!dir.isEmpty())
    {
      toolkitDir_ = dir;
      zipDownloader_ = new FileDownloader(fileUrl_, statusBar_, this);
      connect(zipDownloader_, SIGNAL(downloaded()), this, SLOT(saveToolkit()));
    }
  }
}

void ToolkitDownloader::saveToolkit() const
{
  if (!zipDownloader_)
    return;

  auto fullFilename = toolkitDir_.filePath(filename_);
  QFile file(fullFilename);
  file.open(QIODevice::WriteOnly);
  file.write(zipDownloader_->downloadedData());
  file.close();
  statusBar_->showMessage("Toolkit file saved.", 1000);
}
