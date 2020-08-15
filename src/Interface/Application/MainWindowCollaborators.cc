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
#include "ui_PythonWizardPage.h"
#include "ui_PythonWizardCodePage.h"
#include "ui_IntWithPythonPage.h"

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
  return current && current->childCount() == 0 && current->parent() && !current->text(0).startsWith("clipboard") && current->foreground(0) != CLIPBOARD_COLOR;
}

QString TreeViewModuleGetter::clipboardXML() const
{
  return tree_.currentItem()->data(0, SCIRunMainWindow::clipboardKey).toString();
}

bool TreeViewModuleGetter::isClipboardXML() const
{
  auto current = tree_.currentItem();
  return current && current->childCount() == 0 && current->parent() && (current->text(0).startsWith("clipboard") || current->foreground(0) == CLIPBOARD_COLOR);
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
    "<p><a href = \"http://sciinstitute.github.io/SCIRun/\">SCIRun Doc Home Page</a>"
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


PythonWizard::PythonWizard(std:: function<void(const QString&)> display, QWidget* parent) : QWizard(parent), displayPython_(display)
{
  setWindowTitle("SCIRun Python Help");
  setOption(NoBackButtonOnStartPage);
  setOption(HaveCustomButton1, true);
  setButtonText(CustomButton1, "Back To Home");


  resize(1000,450);
  setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);

  setPage(Page_Home, createIntroPage());
  setPage(Page_Create_Intro, createCreateIntroPage());
  setPage(Page_Save, createSaveNetworkPage());
  setPage(Page_LatVol, createLatVolPage());
  setPage(Page_MeshBox, createEditMeshBoundingBoxPage());
  setPage(Page_Connection, createConnectionPage());
  setPage(Page_CalcData, createCalculateFieldDataPage());
  setPage(Page_Iso, createExtractIsosurfacePage());
  setPage(Page_ShowField, createShowFieldPage());
  setPage(Page_ViewScene, createViewScenePage());
  setPage(Page_Execute, createExecutePage());

  setPage(Page_Load_Intro, createLoadingNetworkIntroPage());
  setPage(Page_Load, createLoadNetworkPage());

  setPage(Page_Wand_Intro, createWandIntroPage());
  setPage(Page_Wand, createWandPage());

  setPage(Page_Int_Intro, createInterfaceIntroPage());
  setPage(Page_Base, createBaseNetworkPage());
  setPage(Page_IntPy, createAddIntPyPage());
  setPage(Page_SetPy, createSetPythonPage());

  setStartId(Page_Home);

  connect(this, SIGNAL(customButtonClicked(int)), this, SLOT(customClicked(int)));
}

void PythonWizard::customClicked(int which) {
  if (which == 6) {
    restart();
  }
}

PythonWizard::~PythonWizard()
{
  showPrefs();
}

void PythonWizard::switchPage(QAbstractButton* button)
{
  QString text = button -> text();
  if(text == "Creating Network with Python Console") {
    while(nextId()-1 != Page_Create_Intro) {
      next();
    }
  }

  if(text == "Loading a Network") {
    while(nextId()-1 != Page_Load_Intro) {
      next();
    }
  }

  if(text == "Using Wand Feature") {
    while(nextId()-1 != Page_Wand_Intro) {
      next();
    }
  }

  if(text == "Using InterfaceWithPython Module") {
    while(nextId()-1 != Page_Int_Intro) {
      next();
    }
  }

}


void PythonWizard::showPrefs()
{
  if (showPrefs_)
    SCIRunMainWindow::Instance()->actionPreferences_->trigger();
}

class PythonWizardPage : public QWizardPage, public Ui::PythonWizardPage
{
public:
  PythonWizardPage()
  {
    setupUi(this);
  }
};

QWizardPage* PythonWizard::createIntroPage()
{
  auto page = new PythonWizardPage;
  page->setTitle("Welcome to Python Wizard");

  page->textArea->setText("This wizard will help you implement Python in SCIRun for the first time.");
  auto docLabel = new QLabel(
    "<p><a href = \"https://github.com/SCIInstitute/SCIRun/blob/gh-pages/python.md\">Python Documentation</a>"
  );
  docLabel->setStyleSheet("QLabel {background-color : darkGray; color : blue; }");
  docLabel->setAlignment(Qt::AlignLeft);
  docLabel->setOpenExternalLinks(true);
  page->textLayout->addWidget(docLabel);
  auto pic = new QLabel;
  pic->setPixmap(QPixmap(":/general/Resources/scirunWizard.png"));
  page->textLayout->addWidget(pic);
  connect(page->tableOfContents, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(switchPage(QAbstractButton*)) );
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

QWizardPage* PythonWizard::createCreateIntroPage()
{
  auto page = new QWizardPage;
  page->setTitle("Creating a Network");
  page->setSubTitle("This section of the wizard will demonstrate how to create a network using the Python Console");
  auto layout = new QVBoxLayout;
  page -> setLayout(layout);
  return page;
}

QWizardPage* PythonWizard::createSaveNetworkPage()
{
  auto page = new PythonWizardCodePage;
  page->setTitle("Saving Network");
  page->infoText->setText("     Start by saving a new network to your SCIRun default path. "
  "To save to a different location, edit the parameter to the desired filename path."
  "\n\n"
  "     Click the \"Send to Python Console\"  to send and execute the code in the python console."
  "\n\n"
  "**The default path settings can be modified from Preferences > Paths**");

  auto dataDirString = Core::Preferences::Instance().dataDirectory().string();

  page->codeEdit->setPlainText("scirun_save_network(\"" +
  QString::fromStdString(dataDirString) +
  "\")");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}


QWizardPage* PythonWizard::createLatVolPage()
{
  auto page = new PythonWizardCodePage;
  page->setTitle("Working with the LatVol Module");
  page->infoText->setText("    Create a CreateLatVol module by using the command `scirun_add_module()`."
    "\n\n"
    "     Then edit the module's parameters using \'scirun_set_module_state\'."
    "**To view all module states of a module, toggle the Metadata Layer and hover over a module.**");

  page->codeEdit->setPlainText("lat = scirun_add_module(\"CreateLatVol\")\n"
    "\n\n"
    "scirun_set_module_state(lat, \"XSize\", 20)\n"
    "\n"
    "scirun_set_module_state(lat, \"YSize\", 20)\n"
    "\n"
    "scirun_set_module_state(lat, \"ZSize\", 20)\n"
    "\n"
    "scirun_set_module_state(lat, \"ElementSizeNormalized\", 1)");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}

QWizardPage* PythonWizard::createEditMeshBoundingBoxPage()
{
  auto page = new PythonWizardCodePage;
  page->setTitle("Working with the EditMeshBoundingBox Module");
  page->infoText->setText("     Create EditMeshBoundingBox module, and edit its parameters using the same commands."
    "\n\n"
    "**For more information about a module, click on the Question Mark icon on the module in the network, "
    "or visit https://sciinstitute.github.io/SCIRun/modules.html**");

  page->codeEdit->setPlainText("edit_box = scirun_add_module(\"EditMeshBoundingBox\")\n"
    "\n\n\n"
    "scirun_set_module_state(edit_box, \"OutputCenterX\", 10)\n"
    "\n"
    "scirun_set_module_state(edit_box, \"OutputCenterY\", 10)\n"
    "\n"
    "scirun_set_module_state(edit_box, \"OutputCenterZ\", 10)\n"
    "\n"
    "scirun_set_module_state(edit_box, \"OutputSizeX\", 20)\n"
    "\n"
    "scirun_set_module_state(edit_box, \"OutputSizeY\", 20)\n"
    "\n"
    "scirun_set_module_state(edit_box, \"OutputSizeZ\", 20)\n");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}

QWizardPage* PythonWizard::createConnectionPage()
{
  auto page = new PythonWizardCodePage;
  page->setTitle("Connecting Modules");
  page->infoText->setText("     Use the 'scirun_connect_modules()' command to connect the CreateLatVol module to the EditMeshBoundingBox module.");

  page->codeEdit->setPlainText("scirun_connect_modules(lat, 0, edit_box, 0)");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}

QWizardPage* PythonWizard::createCalculateFieldDataPage()
{
  auto page = new PythonWizardCodePage;
  page->setTitle("Working with the CalculateFieldData Module");
  page->infoText->setText("     Create a CalculateFieldData module, then  connect it to the EditMeshBoundingBox module."
    "\n\n"
    "     To make the LatVol spherical, assign the equation of a sphere to the CalculateFieldData module");

  page->codeEdit->setPlainText("calc = scirun_add_module(\"CalculateFieldData\")\n"
    "\n"
    "scirun_connect_modules(edit_box, 0, calc, 0)"
    "\n"
    "scirun_set_module_state(calc, \"FunctionString\", \"RESULT = sqrt((X * X) + (Y * Y) + (Z * Z))\")\n");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}

QWizardPage* PythonWizard::createExtractIsosurfacePage()
{
  auto page = new PythonWizardCodePage;
  page->setTitle("Working with the ExtractSimpleIsosurface Module");
  page->infoText->setText("     Create ExtractSimpleIsosurface module and connect it to the CalculateFieldData module."
    "\n\n"
    "     Change the module to a list and set the values.");

  page->codeEdit->setPlainText("iso = scirun_add_module(\"ExtractSimpleIsosurface\")\n"
    "\n"
    "scirun_connect_modules(calc, 0, iso, 0)\n"
    "\n"
    "scirun_set_module_state(iso, \"IsovalueChoice\", \"List\")\n"
    "\n"
    "scirun_set_module_state(iso, \"ListOfIsovalues\", \"1,5,10,15,18\")");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}

QWizardPage* PythonWizard::createShowFieldPage()
{
  auto page = new PythonWizardCodePage;
  page->setTitle("Working with ShowField Module");
  page->infoText->setText("Create a ShowField modules and turn off its ShowEdges parameter. "
    "Connect it to the ExtractSimpleIsosurface module."
    "\n\n"
    "Create a ColorMap module and rescale it by connecting it to a new RescaleColorMap module, "
    "then connecting the CalculateFieldData module to the RescaleColorMap module."
    "\n\n"
    "To show the color map, connect the RescaleColorMap module to the ShowField module.");

  page->codeEdit->setPlainText("iso_show_field = scirun_add_module(\"ShowField\")\n"
    "\n"
    "scirun_set_module_state(iso_show_field, \"ShowEdges\", 0)\n"
    "\n"
    "scirun_connect_modules(iso, 0, iso_show_field, 0)\n"
    "\n"
    "color_map = scirun_add_module(\"CreateStandardColorMap\")\n"
    "\n"
    "rescale_color_map = scirun_add_module(\"RescaleColorMap\")\n"
    "\n"
    "scirun_connect_modules(color_map, 0, rescale_color_map, 0)\n"
    "\n"
    "scirun_connect_modules(calc, 0, rescale_color_map, 1)\n"
    "\n"
    "scirun_connect_modules(rescale_color_map, 0, iso_show_field, 1)");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}


QWizardPage* PythonWizard::createViewScenePage()
{
  auto page = new PythonWizardCodePage;
  page->setTitle("Working with ViewScene Module");
  page->infoText->setText("To be able to view the volume, "
  "you will need to create a ViewScene module and connect it to the ShowField module.");

  page->codeEdit->setPlainText("view_scene = scirun_add_module(\"ViewScene\")\n"
    "\n\n"
    "scirun_connect_modules(iso_show_field, 0, view_scene, 0)");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}


QWizardPage* PythonWizard::createExecutePage()
{
  auto page = new PythonWizardCodePage;
  page->setTitle("Executing the Network");
  page->infoText->setText("Execute the network by running the command 'scirun_execute_all()'");

  page->codeEdit->setPlainText("scirun_execute_all()");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}

QWizardPage* PythonWizard::createLoadingNetworkIntroPage()
{
  auto page = new QWizardPage;
  page->setTitle("Loading a Network");
  page->setSubTitle("This section of the wizard will demonstrate how to load then execute a network");
  auto layout = new QVBoxLayout;
  page -> setLayout(layout);
  return page;
}


QWizardPage* PythonWizard::createLoadNetworkPage()
{
  auto page = new PythonWizardCodePage;
  page->setTitle("Load and Execute a Network");
  page->infoText->setText("Use the command `scirun_load_network()` to load a saved network from sample networks provided"
    "\n\n"
    "Execute network with `scirun_execute_all()` command");

  // TODO: direct path to a working example network
  auto dataDirString = Core::Preferences::Instance().dataDirectory().string();
  page->codeEdit->setPlainText("scirun_load_network(\"" +
    QString::fromStdString(dataDirString) +
    "\")"
    "\n\n\n"
    "scirun_execute_all()");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});
  return page;
}

QWizardPage* PythonWizard::createWandIntroPage()
{
  auto page = new QWizardPage;
  page->setTitle("Using the Wand Tool");
  page->setSubTitle("This section of the wizard will demonstrate how to use the wand tool to load a python script");
  auto layout = new QVBoxLayout;
  page -> setLayout(layout);
  return page;
}

QWizardPage* PythonWizard::createWandPage()
{
  auto page = new QWizardPage;
  page->setTitle("Using the Wand Tool");
  page->setSubTitle("Download the sample python script below"
    "\n\n"
    "Click the yellow wand icon at the top of the screen to select the script downloaded"
    "\n\n"
    "** If there is no wand at the top of the screen, you may need to go to Windows > Toolbars > Advanced"
    "to make the advanced toolbar appear**");
  auto layout = new QVBoxLayout;
  auto pic = new QLabel;
  pic->setPixmap(QPixmap(":/general/Resources/new/general/wand.png"));
  layout->addWidget(pic);
  page -> setLayout(layout);
  return page;
}

QWizardPage* PythonWizard::createInterfaceIntroPage()
{
  auto page = new QWizardPage;
  page->setTitle("Implementing InterfaceWithPython");
  page->setSubTitle("This section of the wizard will demonstrate how InterfaceWithPython can be used in a network");
  auto layout = new QVBoxLayout;
  page->setLayout(layout);
  return page;
}

QWizardPage* PythonWizard::createBaseNetworkPage()
{
  auto page = new PythonWizardCodePage;
  page->setTitle("Creating the base network");
  page->infoText->setText("Run this code in the python console to create a base network."
    "\n\n"
    "This network will create a latvol, create field data with the equation of a sphere, and extract its isosurface ");

  page->codeEdit->setPlainText("lat = scirun_add_module(\"CreateLatVol\");"
    "\n\n"
    "convert = scirun_add_module(\"ConvertHexVolToTetVol\")"
    "\n\n"
    "scirun_connect_modules(lat, 0, convert, 0)"
    "\n\n"
    "data = scirun_add_module(\"CreateFieldData\")"
    "\n\n"
    "scirun_set_module_state(data, \"FunctionString\", \"RESULT = X*X + Y*Y + Z*Z;\")"
    "\n\n"
    "scirun_connect_modules(convert, 0, data, 0)"
    "\n\n"
    "iso = scirun_add_module(\"ExtractIsosurface\")"
    "\n\n"
    "scirun_set_module_state(iso, \"SingleIsoValue\", 1)"
    "\n\n"
    "scirun_connect_modules(data, 0, iso, 0)"
    "\n\n"
    "show = scirun_add_module(\"ShowField\")"
    "\n\n"
    "scirun_connect_modules(iso, 0, show, 0)"
    "\n\n"
    "view = scirun_add_module(\"ViewScene\")"
    "\n\n"
    "scirun_connect_modules(show, 0, view, 0)"
    "\n\n"
    "scirun_execute_all();");

  connect(page->sendButton, &QPushButton::clicked, [this, page](){displayPython_(page->codeEdit->toPlainText());});

  return page;
}

QWizardPage* PythonWizard::createAddIntPyPage()
{
  auto page = new QWizardPage;
  page->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  page->setTitle("Implementing InterfaceWithPython");
  auto layout = new QHBoxLayout;

  auto text = new QTextBrowser;
  text->setText("Remove the connection pipe between the CreateFieldData module and the ExtractIsosurface module "
    "and add an IntefaceWithPython module. Connect the output from the CreateFieldData module to the second input of InterfaceWithPython. "
    "Connect the fourth output of InterfaceWithPython first input of the ExtractIsosurface module."
    "The network should look like the image to the right.");
  layout->addWidget(text);
  layout->setSizeConstraint(QLayout::SetDefaultConstraint);

  auto pic = new QLabel;
  pic-> setScaledContents(true);
  auto image = new QPixmap(":/general/Resources/Wizard/int_with_py_network.png");
  pic->setPixmap(image->scaledToHeight(pic->height(), Qt::SmoothTransformation));
  layout->addWidget(pic);

  page->setLayout(layout);
  page->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  return page;
}

class IntWithPythonPage : public QWizardPage, public Ui::IntWithPythonPage
{
public:
  IntWithPythonPage()
  {
    setupUi(this);
  }
};

QWizardPage* PythonWizard::createSetPythonPage()
{
  auto page = new IntWithPythonPage;
  page->setTitle("Implementing InterfaceWithPython");
  page->infoText->setText("Implement the python code provided to the InterfaceWithPython module"
    "For every node, if it is less than zero, it will halve the value. If it is larger than zero, it will remain the same"
    "It will then outout the new values of the nodes.");
  page->codeEdit->setPlainText("Nodes = Field[\"node\"]"
    "\n"
    "new_nodes = []"
    "\n\n"
    "for n in Nodes:"
    "\n"
    "  nn = [[]]*3"
    "  \n\n"
    "  if n[0]>0:"
    "  \n"
    "    nn[0] = n[0]"
    "    \n\n"
    "  else:"
    "  \n"
    "    nn[0] = n[0]/2"
    "  \n\n"
    "  nn[1] = n[1]"
    "  \n"
    "  nn[2] = n[2]"
    "  \n\n"
    "  new_nodes.append(nn)"
    "\n\n"
    "Field[\"node\"] = new_nodes"
    "\n\n"
    "fieldOutput1 = Field");
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
    QObject::connect(mainWindow_->actionPeakBehindModuleUIs_, &QAction::triggered,
      [this, editor]() {
        if (mainWindow_->actionPeakBehindModuleUIs_->isChecked())
          editor->seeThroughAllModuleUIs();
        else
          editor->normalOpacityAllModuleUIs();
        });
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

void SCIRunGuiRunner::reportIssue()
{
  SCIRunMainWindow::Instance()->reportIssue();
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
      toolkitDir_.setPath(dir);
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
