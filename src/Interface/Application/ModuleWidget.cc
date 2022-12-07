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


#include "ui_Module.h"
#include <iostream>
#include <QtConcurrent>
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Network/Connection.h>
#include <Interface/qt_include.h>
#include <Interface/Application/ClosestPortFinder.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/ModuleOptionsDialogConfiguration.h>
#include <Interface/Modules/Base/ModuleLogWindow.h>
#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/PortWidgetManager.h>
#include <Interface/Application/Utility.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Interface/Modules/Factory/ModuleDialogFactory.h>

//TODO
#include <Interface/Modules/Render/ViewScene.h>

using namespace SCIRun;
using namespace Core;
using namespace Gui;
using namespace Dataflow::Networks;
using namespace Logging;

//#define MODULE_POSITION_LOGGING

namespace SCIRun {
namespace Gui {
  class ModuleActionsMenu
  {
  public:
    ModuleActionsMenu(QWidget* parent, const std::string& moduleId)
    {
      menu_ = new QMenu("Actions", parent);

      //TODO:  hook up disabled actions
      menu_->addActions(QList<QAction*>()
        << disabled(new QAction("ID: " + QString::fromStdString(moduleId), parent))
        << separatorAction(parent)
        << new QAction("Execute", parent)
        << new QAction("... Downstream Only", parent)
        << new QAction("Help", parent)
        << new QAction("Edit Notes...", parent)
        << new QAction("Duplicate", parent)
        << new QAction("Replace With...", parent)
        //<< disabled(new QAction("Ignore*", parent))
        << new QAction("Show Log", parent)
        << new QAction("Toggle Programmable Input Port", parent)
        //<< disabled(new QAction("Make Sub-Network", parent))  // Issue #287
        << separatorAction(parent)
        << new QAction("Destroy", parent));
    }
    QMenu* getMenu() { return menu_; }
    QAction* getAction(const char* name) const
    {
      for (const auto& action : menu_->actions())
      {
        if (action->text().contains(name))
          return action;
      }
      return nullptr;
    }
  private:
    QMenu* menu_;
  };
}}

namespace
{
  //TODO: make run-time configurable
  int moduleAlpha()
  {
    return 100;
  }
  int portAlpha()
  {
    return 230;
  }
  QString moduleRGBA(int r, int g, int b)
  {
    return QString("rgba(%1,%2,%3,%4)")
      .arg(r).arg(g).arg(b)
      .arg(moduleAlpha());
  }

  QString scirunModuleFontName()
  {
    return "Helvetica";
  }
}

class ModuleWidgetDisplay : public Ui::Module, public ModuleWidgetDisplayBase
{
public:
  ModuleWidgetDisplay() : subnetButton_(new QPushButton("Subnet"))
  { }
  void setupFrame(QStackedWidget* stacked) override;
  void setupTitle(const QString& name) override;
  void setupProgressBar() override;
  void setupSpecial() override;
  void setupButtons(bool hasUI, QObject* module) override;
  void setupIcons() override;
  QAbstractButton* getOptionsButton() const override;
  QAbstractButton* getExecuteButton() const override;
  QAbstractButton* getHelpButton() const override;
  QAbstractButton* getLogButton() const override;
  void setStatusColor(const QString& color) override;
  QPushButton* getModuleActionButton() const override;
  QAbstractButton* getSubnetButton() const override;

  QProgressBar* getProgressBar() const override;

  void setupSubnetWidgets() override;

  int getTitleWidth() const override;
  QLabel* getTitle() const override;
  QGroupBox* getButtonGroup() const override;

  void startExecuteMovie() override;
  void stopExecuteMovie() override;

private:
  QAbstractButton* subnetButton_;
};

void ModuleWidgetDisplay::setupFrame(QStackedWidget* stacked)
{
  setupUi(stacked);
}

void ModuleWidgetDisplay::setupTitle(const QString& name)
{
  auto hiDpi = ModuleWidget::highResolutionExpandFactor_ > 1;
  auto fontSize = titleFontSize;
  if (hiDpi)
    fontSize--;
  QFont titleFont(scirunModuleFontName(), fontSize, QFont::Bold);
  if (hiDpi)
    titleFont.setBold(false);
  titleLabel_->setFont(titleFont);
  titleLabel_->setText(name);
  if (hiDpi)
  {
    titleLabel_->setMinimumHeight(1.2 * titleLabel_->minimumHeight());
    titleLabel_->setMinimumWidth(1.2 * titleLabel_->minimumWidth());
  }
  QFont smallerTitleFont(scirunModuleFontName(), fontSize - buttonPageFontSizeDiff);
  buttonGroup_->setFont(smallerTitleFont);
  buttonGroup_->setTitle(name);
  progressGroupBox_->setFont(smallerTitleFont);
  progressGroupBox_->setTitle(name);
}

void ModuleWidgetDisplay::setupProgressBar()
{
  progressBar_->setMaximum(100);
  progressBar_->setMinimum(0);
  progressBar_->setValue(0);
  progressBar_->setTextVisible(false);
}

void ModuleWidgetDisplay::setupSpecial()
{
  optionsButton_->setText("VIEW");
  optionsButton_->setFont(QFont(scirunModuleFontName(), viewFontSize));
  optionsButton_->setToolTip("View renderer output");
  optionsButton_->setIcon(QIcon());
  executePushButton_->hide();
  progressBar_->setVisible(false);
}

void ModuleWidgetDisplay::setupButtons(bool hasUI, QObject*)
{
  optionsButton_->setVisible(hasUI);
}

void ModuleWidgetDisplay::setupIcons()
{
  auto currentExecuteIcon = Preferences::Instance().moduleExecuteDownstreamOnly ? ModuleWidget::downstreamOnlyIcon : ModuleWidget::allIcon;
  getExecuteButton()->setIcon(QPixmap(currentExecuteIcon));
  getOptionsButton()->setText("");
  getOptionsButton()->setIcon(QPixmap(":/general/Resources/new/modules/options.png"));
  getHelpButton()->setText("");
  getHelpButton()->setIcon(QPixmap(":/general/Resources/new/modules/help.png"));
  getLogButton()->setText("");
  getLogButton()->setIcon(QPixmap(":/general/Resources/new/modules/info.png"));
  getModuleActionButton()->setText("");
  getModuleActionButton()->setIcon(QPixmap(":/general/Resources/new/modules/settings.png"));

  getSubnetButton()->setIcon(QPixmap(":/general/Resources/editSubnet.png"));
  getSubnetButton()->setText("Edit");

  auto movie = new QMovie(":/general/Resources/executing.gif");
  executingLabel_->setMovie(movie);
}

void ModuleWidget::adjustExecuteButtonToDownstream(bool downOnly)
{
  currentExecuteIcon_ = downOnly ? &downstreamOnlyIcon : &allIcon;
  fullWidgetDisplay_->getExecuteButton()->setIcon(QPixmap(*currentExecuteIcon_));
}

void ModuleWidgetDisplay::startExecuteMovie()
{
  executingLabel_->movie()->start();
}

void ModuleWidgetDisplay::stopExecuteMovie()
{
  executingLabel_->movie()->stop();
}

QAbstractButton* ModuleWidgetDisplay::getOptionsButton() const
{
  return optionsButton_;
}

QAbstractButton* ModuleWidgetDisplay::getExecuteButton() const
{
  return executePushButton_;
}

QAbstractButton* ModuleWidgetDisplay::getHelpButton() const
{
  return helpButton_;
}

QAbstractButton* ModuleWidgetDisplay::getLogButton() const
{
  return logButton2_;
}

QAbstractButton* ModuleWidgetDisplay::getSubnetButton() const
{
  return subnetButton_;
}

void ModuleWidgetDisplay::setupSubnetWidgets()
{
  getExecuteButton()->setVisible(false);
  getLogButton()->setVisible(false);
  getHelpButton()->setVisible(false);

  subnetButton_->setMinimumWidth(50);
  auto layout = qobject_cast<QHBoxLayout*>(buttonGroup_->layout());
  if (layout)
    layout->insertWidget(0, subnetButton_);
}

void ModuleWidgetDisplay::setStatusColor(const QString& color)
{
  if (color.isEmpty())
  {
    getLogButton()->setStyleSheet("");
  }
  else
  {
    auto style = QString("* { background-color: %1 }").arg(color);
    getLogButton()->setStyleSheet(style);
  }
}

QPushButton* ModuleWidgetDisplay::getModuleActionButton() const
{
  return moduleActionButton_;
}

QProgressBar* ModuleWidgetDisplay::getProgressBar() const
{
  return progressBar_;
}

int ModuleWidgetDisplay::getTitleWidth() const
{
  return titleLabel_->fontMetrics().boundingRect(titleLabel_->text()).width();
}

QLabel* ModuleWidgetDisplay::getTitle() const
{
  return titleLabel_;
}

QGroupBox* ModuleWidgetDisplay::getButtonGroup() const
{
  return buttonGroup_;
}

static const int UNSET = -1;
static const int SELECTED = -50;

namespace
{
  ModuleId id(ModuleHandle mod)
  {
    return mod ? mod->id() : ModuleId();
  }

  QString backgroundColorByName(const QString& name)
  {
    if (name.contains("Loop"))
      return moduleRGBA(13, 152, 186);

    return moduleRGBA(99,99,104);
  }
}

ModuleWidget::ModuleWidget(ModuleErrorDisplayer* ed, const QString& name, ModuleHandle theModule,
  QWidget* parent /* = 0 */)
  : QStackedWidget(parent), HasNotes(id(theModule), true),
  fullWidgetDisplay_(new ModuleWidgetDisplay),
  ports_(new PortWidgetManager),
  deletedFromGui_(true),
  colorLocked_(false),
  executedOnce_(false),
  skipExecuteDueToFatalError_(false),
  disabled_(false),
  errored_(false),
  theModule_(theModule),
  previousModuleState_(UNSET),
  moduleId_(id(theModule)),
  name_(name),
  dialogManager_(theModule),
  dockable_(nullptr),
  inputPortLayout_(nullptr),
  outputPortLayout_(nullptr),
  deleting_(false),
  defaultBackgroundColor_(backgroundColorByName(name)),
  isViewScene_(name == "ViewScene") //TODO
{
  fillColorStateLookup(defaultBackgroundColor_);

  setupModuleActions();

  setupLoggingAndProgress(ed);

  setCurrentIndex(buildDisplay(fullWidgetDisplay_.get(), name));

  makeOptionsDialog();

  createPorts(*theModule_);
  addPorts(currentIndex());
  updateProgrammablePorts();

  connect(this, &ModuleWidget::backgroundColorUpdated, this, &ModuleWidget::updateBackgroundColor);
  theModule_->executionState().connectExecutionStateChanged([this](int state) { (void)QtConcurrent::run(
      [this, state] { updateBackgroundColorForModuleState(state); }); });

  theModule_->connectExecuteSelfRequest([this](bool upstream) { executeAgain(upstream); });
  connect(this, &ModuleWidget::executeAgain, this, &ModuleWidget::executeTriggeredProgrammatically);

  Preferences::Instance().modulesAreDockable.connectValueChanged([this](bool d) { adjustDockState(d); });

  connect(actionsMenu_->getAction("Destroy"), &QAction::triggered, this, &ModuleWidget::deleteMeLater);

  connectExecuteEnds([this] (double, const ModuleId&) { executeEnds(); });
  connect(this, &ModuleWidget::executeEnds, this, &ModuleWidget::changeExecuteButtonToPlay);
  connect(this, &ModuleWidget::signalExecuteButtonIconChangeToStop, this, &ModuleWidget::changeExecuteButtonToStop);
  connect(this, &ModuleWidget::dynamicPortChanged, this, &ModuleWidget::updateDialogForDynamicPortChange);

  if (theModule->isDeprecated() && !Core::Application::Instance().parameters()->isRegressionMode())
  {
    QMessageBox::warning(nullptr,
      "Module deprecation alert",
      tr("Module %1 is deprecated, please use this replacement instead: %2")
        .arg(QString::fromStdString(theModule->name()))
        .arg(QString::fromStdString(theModule->replacementModuleName())));
  }

  currentExecuteIcon_ = Preferences::Instance().moduleExecuteDownstreamOnly ? &downstreamOnlyIcon : &allIcon;
}

void ModuleWidget::setupLoggingAndProgress(ModuleErrorDisplayer* ed)
{
  auto logWindow = dialogManager_.setupLogging(ed, actionsMenu_->getAction("Show Log"), mainWindowWidget());
  QObject::connect(logWindow, &ModuleLogWindow::messageReceived, this, &ModuleWidget::setLogButtonColor);
  QObject::connect(logWindow, &ModuleLogWindow::requestModuleVisible, this, &ModuleWidget::requestModuleVisible);
  theModule_->setUpdaterFunc([this](int i) { updateProgressBarSignal(i); });
  if (theModule_->hasUI())
    theModule_->setUiToggleFunc([this](bool b) {
      if (dockable()) dockable()->setVisible(b);
    });
}

QString ModuleWidget::downstreamOnlyIcon(":/general/Resources/new/modules/run_down.png");
QString ModuleWidget::allIcon(":/general/Resources/new/modules/run_all.png");

int ModuleWidget::buildDisplay(ModuleWidgetDisplayBase* display, const QString& name)
{
  display->setupFrame(this);

  setupDisplayWidgets(display, name);

  addPortLayouts(0);

  resizeBasedOnModuleName(display, 0);

  setupDisplayConnections(display);

  return 0;
}

void ModuleWidget::setupDisplayWidgets(ModuleWidgetDisplayBase* display, const QString& name)
{
  display->setupTitle(name);
  display->setupIcons();

  //TODO: ultra ugly. no other place for this code right now.
  //TODO: to be handled in issue #212
  if (isViewScene_)
  {
    display->setupSpecial();
  }
  display->setupProgressBar();
  display->setupButtons(theModule_->hasUI(), this);
}

#if defined (WIN32)
  const int ModuleWidgetDisplayBase::moduleWidthThreshold = 110;
  const int ModuleWidgetDisplayBase::extraModuleWidth = 5;
  const int ModuleWidgetDisplayBase::extraWidthThreshold = 5;
  const int ModuleWidgetDisplayBase::smushFactor = 15;
  const int ModuleWidgetDisplayBase::titleFontSize = 9;
  const int ModuleWidgetDisplayBase::viewFontSize = 6;
  const int ModuleWidgetDisplayBase::buttonPageFontSizeDiff = 1;
  const int ModuleWidgetDisplayBase::widgetHeightAdjust = -20;
  const int ModuleWidgetDisplayBase::widgetWidthAdjust = -10;
#elif defined (__APPLE__)
  const int ModuleWidgetDisplayBase::moduleWidthThreshold = 116;
  const int ModuleWidgetDisplayBase::extraModuleWidth = 2;
  const int ModuleWidgetDisplayBase::extraWidthThreshold = 5;
  const int ModuleWidgetDisplayBase::smushFactor = 15;
  const int ModuleWidgetDisplayBase::titleFontSize = 13;
  const int ModuleWidgetDisplayBase::viewFontSize = 8;
  const int ModuleWidgetDisplayBase::buttonPageFontSizeDiff = 3;
  const int ModuleWidgetDisplayBase::widgetHeightAdjust = 1;
  const int ModuleWidgetDisplayBase::widgetWidthAdjust = -20;
#else // Linux

  //TODO: need a parameter for spacer between buttons and group box title

  const int ModuleWidgetDisplayBase::moduleWidthThreshold = 110;
  const int ModuleWidgetDisplayBase::extraModuleWidth = 5;
  const int ModuleWidgetDisplayBase::extraWidthThreshold = 5;
  const int ModuleWidgetDisplayBase::smushFactor = 15;
  const int ModuleWidgetDisplayBase::titleFontSize = 8;
  const int ModuleWidgetDisplayBase::viewFontSize = 5;
  const int ModuleWidgetDisplayBase::buttonPageFontSizeDiff = 1;
  const int ModuleWidgetDisplayBase::widgetHeightAdjust = -20;
  const int ModuleWidgetDisplayBase::widgetWidthAdjust = -10;
#endif

void ModuleWidget::resizeBasedOnModuleName(ModuleWidgetDisplayBase* display, int)
{
  auto frame = this;
  int pixelWidth = display->getTitleWidth();
  int extraWidth = pixelWidth - ModuleWidgetDisplayBase::moduleWidthThreshold;
  if (extraWidth > ModuleWidgetDisplayBase::extraWidthThreshold)
  {
    frame->resize(frame->width() + extraWidth + ModuleWidgetDisplayBase::extraModuleWidth, frame->height());
  }
  else
  {
    frame->resize(frame->width() - ModuleWidgetDisplayBase::smushFactor, frame->height());
  }
}

void ModuleWidget::setupDisplayConnections(ModuleWidgetDisplayBase* display)
{
  connect(display->getExecuteButton(), &QPushButton::clicked, this, &ModuleWidget::executeButtonPushed);
  if (!theModule_->isStoppable())
  {
    addWidgetToExecutionDisableList(display->getExecuteButton());
  }
  connect(display->getOptionsButton(), &QPushButton::clicked, this, &ModuleWidget::toggleOptionsDialog);
  connect(display->getHelpButton(), &QPushButton::clicked, this, &ModuleWidget::launchDocumentation);
  dialogManager_.connectDisplayLogButton(display->getLogButton());
  connect(display->getSubnetButton(), &QPushButton::clicked, this, &ModuleWidget::subnetButtonClicked);
  display->getModuleActionButton()->setMenu(actionsMenu_->getMenu());
}

void ModuleWidget::subnetButtonClicked()
{
  Q_EMIT showSubnetworkEditor(QString::fromStdString(theModule_->get_state()->getValue(Core::Algorithms::Name("Name")).toString()));
}

void ModuleWidget::setLogButtonColor(const QColor& color)
{
  if (color == Qt::red)
  {
    errored_ = true;
    updateBackgroundColor(colorStateLookup_.right.at(static_cast<int>(ModuleExecutionState::Value::Errored)));
  }
  fullWidgetDisplay_->setStatusColor(moduleRGBA(color.red(), color.green(), color.blue()));
}

void ModuleWidget::resetLogButtonColor()
{
  fullWidgetDisplay_->setStatusColor("");
}

void ModuleWidget::resetProgressBar()
{
  fullWidgetDisplay_->getProgressBar()->setValue(0);
  fullWidgetDisplay_->getProgressBar()->setTextVisible(false);
}

size_t ModuleWidget::numOutputPorts() const { return ports().numOutputPorts(); }
int ModuleWidget::numDynamicInputPortsForGuiUpdates() const
{
  const auto inputs = ports().inputs();
  return std::count_if(inputs.begin(), inputs.end(), [](PortWidget* p) { return p->description()->isDynamic(); });
}

void ModuleWidget::setupModuleActions()
{
  actionsMenu_.reset(new ModuleActionsMenu(this, moduleId_));
  addWidgetToExecutionDisableList(actionsMenu_->getAction("Execute"));
  addWidgetToExecutionDisableList(actionsMenu_->getAction("... Downstream Only"));

  connect(actionsMenu_->getAction("Execute"), &QAction::triggered, this, &ModuleWidget::executeButtonPushed);
  connect(actionsMenu_->getAction("... Downstream Only"), &QAction::triggered, this, &ModuleWidget::executeTriggeredViaStateChange);
  connect(this, &ModuleWidget::updateProgressBarSignal, this, &ModuleWidget::updateProgressBar);
  connect(actionsMenu_->getAction("Help"), &QAction::triggered, this, &ModuleWidget::launchDocumentation);
  connect(actionsMenu_->getAction("Duplicate"), &QAction::triggered, this, &ModuleWidget::duplicate);
  connect(actionsMenu_->getAction("Toggle Programmable Input Port"), &QAction::triggered, this, &ModuleWidget::toggleProgrammableInputPort);
  if (theModule_->id().name_ == "Subnet")
    actionsMenu_->getMenu()->removeAction(actionsMenu_->getAction("Duplicate"));
  if (theModule_->id().name_ == "Subnet")
    actionsMenu_->getMenu()->removeAction(actionsMenu_->getAction("Replace With..."));

  connectNoteEditorToAction(actionsMenu_->getAction("Notes"));
  connectUpdateNote(this);
}

void ModuleWidget::postLoadAction()
{
  auto replaceWith = actionsMenu_->getAction("Replace With...");
  if (replaceWith)
    connect(replaceWith, &QAction::triggered, this, &ModuleWidget::showReplaceWithWidget);
}

void ModuleWidget::showReplaceWithWidget()
{
#ifndef __APPLE__
  replaceWithDialog_ = new QDialog;
  replaceWithDialog_->setWindowTitle("Replace a module");
  auto layout = new QHBoxLayout;
  layout->addWidget(new QLabel("Replace " + name_ + " with:"));
  auto button = new QPushButton("Choose a compatible module");
  auto menu = new QMenu;
  button->setMenu(menu);
  fillReplaceWithMenu(menu);
  layout->addWidget(button);
  auto cancel = new QPushButton("Cancel");
  connect(cancel, &QPushButton::clicked, replaceWithDialog_, &QDialog::reject);
  layout->addWidget(cancel);
  replaceWithDialog_->setLayout(layout);
  replaceWithDialog_->exec();
#else
  QMessageBox::information(nullptr, "Replace with disabled", "The replace with command is disabled on OSX until the Qt 5 upgrade is complete.");
#endif
}

bool ModuleWidget::guiVisible() const
{
  if (dockable_)
    return dockable_->isVisible();
  return false;
}

void ModuleWidget::fillReplaceWithMenu(QMenu* menu)
{
  if (deleting_ || networkBeingCleared_)
    return;

  menu->clear();
  LOG_DEBUG("Filling menu for {}", theModule_->name());
  auto replacements = Application::Instance().controller()->possibleReplacements(this->theModule_);
  auto isReplacement = [&](const ModuleDescription& md) { return replacements.find(md.lookupInfo_) != replacements.end(); };
  fillMenuWithFilteredModuleActions(menu, Application::Instance().controller()->getAllAvailableModuleDescriptions(),
    isReplacement,
    [=](QAction* action) { QObject::connect(action, &QAction::triggered, this, &ModuleWidget::replaceModule); },
    replaceWithDialog_);
}

void ModuleWidget::replaceModule()
{
  delete replaceWithDialog_;
  replaceWithDialog_ = nullptr;
  auto action = qobject_cast<QAction*>(sender());
  auto moduleToReplace = action->text();
  Q_EMIT replaceModuleWith(theModule_, moduleToReplace.toStdString());
}

void ModuleWidget::replaceMe()
{
  if (!executedOnce_)
    Q_EMIT replaceModuleWith(theModule_, theModule_->name());
  else
  {
    setStartupNote("MODULE FATAL ERROR, DO NOT USE THIS INSTANCE. \nPlease manually replace module for proper execution.");
  }
}

void ModuleWidget::addPortLayouts(int index)
{
  widget(index)->layout()->setContentsMargins(0, 0, 0, 0);
}

void ModuleWidget::createPorts(const ModuleInfoProvider& moduleInfoProvider)
{
  createInputPorts(moduleInfoProvider);
  createOutputPorts(moduleInfoProvider);
}

void ModuleWidget::addPorts(int index)
{
  addInputPortsToLayout(index);
  addOutputPortsToLayout(index);
}

class PortBuilder
{
public:
  void buildInputs(ModuleWidget* widget, const ModuleInfoProvider& moduleInfoProvider)
  {
    const auto moduleId = moduleInfoProvider.id();
    size_t i = 0;
    const auto& inputs = moduleInfoProvider.inputPorts();
    for (const auto& port : inputs)
    {
      auto type = port->get_typename();
      auto w = new InputPortWidget(QString::fromStdString(port->get_portname()), to_color(PortColorLookup::toColor(type),
        portAlpha()), type,
        moduleId, i, port->isDynamic(), port,
        [widget]() { return widget->connectionFactory_; },
        [widget]() { return widget->closestPortFinder_; },
        {},
        widget);
      widget->hookUpGeneralPortSignals(w);
      QObject::connect(widget, &ModuleWidget::connectionAdded, w, &InputPortWidget::makeConnection);
      QObject::connect(w, &InputPortWidget::incomingConnectionStateChange, widget, &ModuleWidget::incomingConnectionStateChanged);
      QObject::connect(widget, &ModuleWidget::connectionStatusChanged, w, &InputPortWidget::connectionStatusChanged);
      widget->ports_->addPort(w);
      ++i;
      if (widget->dialogManager_.hasOptions() && port->isDynamic())
      {
        auto portConstructionType = DynamicPortChange::INITIAL_PORT_CONSTRUCTION;
        auto nameMatches = [&](const InputPortHandle& in)
        {
          return in->externalId().name == port->externalId().name;
        };
        auto justAddedIndex = i - 1;
        bool isNotLastDynamicPortOfThisName = justAddedIndex < inputs.size() - 1
          && std::find_if(inputs.cbegin() + justAddedIndex + 1, inputs.cend(), nameMatches) != inputs.cend();
        if (isNotLastDynamicPortOfThisName)
          portConstructionType = DynamicPortChange::USER_ADDED_PORT_DURING_FILE_LOAD;
        widget->dialogManager_.options()->updateFromPortChange(static_cast<int>(i), port->externalId().toString(), portConstructionType);
      }
    }
  }
  void buildOutputs(ModuleWidget* widget, const ModuleInfoProvider& moduleInfoProvider)
  {
    const ModuleId moduleId = moduleInfoProvider.id();
    size_t i = 0;
    for (const auto& port : moduleInfoProvider.outputPorts())
    {
      auto type = port->get_typename();
      auto w = new OutputPortWidget(
        QString::fromStdString(port->get_portname()),
        to_color(PortColorLookup::toColor(type), portAlpha()),
        type, moduleId, i, port->isDynamic(), port,
        [widget]() { return widget->connectionFactory_; },
        [widget]() { return widget->closestPortFinder_; },
        port->getPortDataDescriber(),
        widget);
      widget->hookUpGeneralPortSignals(w);
      widget->ports_->addPort(w);
      ++i;
    }
  }
private:

};

void ModuleWidget::createInputPorts(const ModuleInfoProvider& moduleInfoProvider)
{
  PortBuilder builder;
  builder.buildInputs(this, moduleInfoProvider);
}

bool ModuleWidget::hasOptions() const
{
  return dialogManager_.hasOptions();
}

void ModuleWidget::printInputPorts(const ModuleInfoProvider& moduleInfoProvider) const
{
  const auto moduleId = moduleInfoProvider.id();
  std::cout << "Module input ports: " << moduleId << std::endl;
  size_t i = 0;
  for (const auto& port : moduleInfoProvider.inputPorts())
  {
    auto type = port->get_typename();
    std::cout << "\t" << i << " : " << port->get_portname() << " : " << type << " dyn = " << port->isDynamic() << std::endl;
     ++i;
  }
}

void ModuleWidget::createOutputPorts(const ModuleInfoProvider& moduleInfoProvider)
{
  PortBuilder builder;
  builder.buildOutputs(this, moduleInfoProvider);
}

void ModuleWidget::updateProgrammablePorts()
{
  #if 0 //later maybe?
  auto state = theModule_->get_state();
  auto hasKey = state->containsKey(Core::Algorithms::Variables::ProgrammableInputPortEnabled);
  qDebug() << moduleId_.c_str() << "port state: hasKey " << hasKey;
  if (hasKey)
    qDebug() << "\tvalue: " << state->getValue(Core::Algorithms::Variables::ProgrammableInputPortEnabled).toBool();

  if (hasKey && state->getValue(Core::Algorithms::Variables::ProgrammableInputPortEnabled).toBool())
  {
    // do nothing, port is on by default
  }
  else
  {
    qDebug() << "\t\t" << "NEED TO TURN OFF PROG INPUT";
  }
  #endif
}

void ModuleWidget::hookUpGeneralPortSignals(PortWidget* port) const
{
  connect(port, &PortWidget::requestConnection, this, &ModuleWidget::requestConnection);
  connect(port, &PortWidget::connectionDeleted, this, &ModuleWidget::connectionDeleted);
  connect(this, &ModuleWidget::cancelConnectionsInProgress, port, &PortWidget::cancelConnectionsInProgress);
  connect(this, &ModuleWidget::cancelConnectionsInProgress, port, &PortWidget::clearPotentialConnections);
  connect(port, &PortWidget::connectNewModuleHere, this, &ModuleWidget::connectNewModuleTo);
  connect(port, &PortWidget::insertNewModuleHere, this, &ModuleWidget::insertNewModuleTo);
  connect(port, &PortWidget::connectionNoteChanged, this, &ModuleWidget::noteChanged);
  connect(port, &PortWidget::highlighted, this, &ModuleWidget::updatePortSpacing);
}

void ModuleWidget::addOutputPortsToLayout(int index)
{
  if (!outputPortLayout_)
  {
    //TODO--extract method
    outputPortLayout_ = new QHBoxLayout;
    outputPortLayout_->setSpacing(SMALL_PORT_SPACING);
    outputPortLayout_->setAlignment(Qt::AlignLeft);
    outputPortLayout_->setContentsMargins(0, 0, 0, 0);
    addOutputPortsToWidget(index);
  }
  ports_->addOutputsToLayout(outputPortLayout_);
}

void ModuleWidget::addOutputPortsToWidget(int index)
{
  auto vbox = qobject_cast<QVBoxLayout*>(widget(index)->layout());
  if (vbox)
  {
    vbox->setContentsMargins(5, 0, 0, 0);
    vbox->insertLayout(-1, outputPortLayout_, 1);
    vbox->setAlignment(outputPortLayout_, Qt::AlignBottom);
    vbox->setAlignment(fullWidgetDisplay_->getTitle(), Qt::AlignBottom);
  }
  else
  {
  }
}

void ModuleWidget::removeOutputPortsFromWidget(int index)
{
  auto vbox = qobject_cast<QVBoxLayout*>(widget(index)->layout());
  if (vbox)
  {
    vbox->removeItem(outputPortLayout_);
  }
  else
    qDebug() << "removeOutputPortsFromWidget failed";
}

void PortWidgetManager::addInputsToLayout(QHBoxLayout* layout)
{
  if (inputPorts_.empty())
    layout->addWidget(new BlankPort(layout->parentWidget()));

  for (auto port : inputPorts_)
    layout->addWidget(port);

  layout->setSizeConstraint(QLayout::SetMinimumSize);
}

void PortWidgetManager::addOutputsToLayout(QHBoxLayout* layout)
{
  if (outputPorts_.empty())
    layout->addWidget(new BlankPort(layout->parentWidget()));

  for (auto port : outputPorts_)
    layout->addWidget(port);

  layout->setSizeConstraint(QLayout::SetMinimumSize);
}

void PortWidgetManager::setSceneFunc(SceneFunc f)
{
  getScene_ = f;
  for (auto& p : getAllPorts())
    p->setSceneFunc(f);
}

void ModuleWidget::addInputPortsToLayout(int index)
{
  if (!inputPortLayout_)
  {
    inputPortLayout_ = new QHBoxLayout;
    inputPortLayout_->setSpacing(SMALL_PORT_SPACING);
    inputPortLayout_->setAlignment(Qt::AlignLeft);
    inputPortLayout_->setContentsMargins(0, 0, 0, 0);
    addInputPortsToWidget(index);
  }
  ports_->addInputsToLayout(inputPortLayout_);
}

void ModuleWidget::addInputPortsToWidget(int index)
{
  auto vbox = qobject_cast<QVBoxLayout*>(widget(index)->layout());
  if (vbox)
  {
    vbox->setContentsMargins(5, 0, 0, 3);
    vbox->insertLayout(0, inputPortLayout_, 1);
  }
  else
  {
  }
}

void ModuleWidget::removeInputPortsFromWidget(int index)
{
  auto vbox = qobject_cast<QVBoxLayout*>(widget(index)->layout());
  if (vbox)
    vbox->removeItem(inputPortLayout_);
  else
    qDebug() << "removeInputPortsFromWidget failed";
}

void PortWidgetManager::reindexInputs()
{
  for (size_t i = 0; i < inputPorts_.size(); ++i)
  {
    auto port = inputPorts_[i];
    port->setIndex(i);
  }
}

void PortWidgetManager::addPort(OutputPortWidget* port)
{
  outputPorts_.push_back(port);
}

void PortWidgetManager::addPort(InputPortWidget* port)
{
  inputPorts_.push_back(port);
  port->setSceneFunc(getScene_);
}

void PortWidgetManager::insertPort(int index, InputPortWidget* port)
{
  if (index > inputPorts_.size())
    inputPorts_.push_back(port);
  else
    inputPorts_.insert(inputPorts_.begin() + index, port);
  port->setSceneFunc(getScene_);
}

void PortWidgetManager::setHighlightPorts(bool on)
{
  for (auto& port : getAllPorts())
  {
    port->setHighlight(on);
  }
}

QList<QGraphicsItem*> ModuleWidget::connections() const
{
  QList<QGraphicsItem*> conns;
  for (const auto& port : ports().getAllPorts())
  {
    for (const auto& conn : port->connections())
      conns.append(conn);
  }
  return conns;
}

void ModuleWidget::addDynamicPort(const ModuleId& mid, const PortId& pid)
{
  if (mid.id_ == moduleId_)
  {
    auto port = theModule_->getInputPort(pid);
    auto type = port->get_typename();

    auto w = new InputPortWidget(QString::fromStdString(port->get_portname()), to_color(PortColorLookup::toColor(type)), type, mid, port->getIndex(), port->isDynamic(),
      port,
      [this]() { return connectionFactory_; },
      [this]() { return closestPortFinder_; },
      PortDataDescriber(), this);
    hookUpGeneralPortSignals(w);

    connect(this, &ModuleWidget::connectionAdded, w, &InputPortWidget::makeConnection);

    const auto newPortIndex = static_cast<int>(port->getIndex());

    ports_->insertPort(newPortIndex, w);
    ports_->reindexInputs();

    inputPortLayout_->insertWidget(newPortIndex, w);

    Q_EMIT dynamicPortChanged(pid.toString(), true);
  }
}

void ModuleWidget::removeDynamicPort(const ModuleId& mid, const PortId& pid)
{
  if (mid.id_ == moduleId_ && !deleting_ && !networkBeingCleared_)
  {
    if (ports_->removeDynamicPort(pid, inputPortLayout_))
    {
      Q_EMIT dynamicPortChanged(pid.toString(), false);
    }
  }
}

bool PortWidgetManager::removeDynamicPort(const PortId& pid, QHBoxLayout* layout)
{
  auto iter = std::find_if(inputPorts_.begin(), inputPorts_.end(), [&](const PortWidget* w) { return w->description()->externalId() == pid; });
  if (iter != inputPorts_.end())
  {
    auto widget = *iter;
    inputPorts_.erase(iter);

    reindexInputs();

    layout->removeWidget(widget);
    delete widget;

    return true;
  }
  return false;
}

void ModuleWidget::printPortPositions() const
{
  std::cout << "Port positions for module " << moduleId_ << std::endl;
  for (const auto& p : ports_->getAllPorts())
  {
    std::cout << "\t" << p->pos();
  }
  std::cout << std::endl;
}

enum class ModuleWidgetPages
{
  TITLE_PAGE,
  PROGRESS_PAGE,
  BUTTON_PAGE
};

void ModuleWidget::enterEvent(Q_ENTER_EVENT_CLASS* event)
{
  previousPageIndex_ = currentIndex();
  movePortWidgets(previousPageIndex_, static_cast<int>(ModuleWidgetPages::BUTTON_PAGE));
  setCurrentIndex(static_cast<int>(ModuleWidgetPages::BUTTON_PAGE));
  QStackedWidget::enterEvent(event);
}

void ModuleWidget::leaveEvent(QEvent* event)
{
  movePortWidgets(currentIndex(), previousPageIndex_);
  setCurrentIndex(previousPageIndex_);
  QStackedWidget::leaveEvent(event);
}

bool ModuleWidget::networkBeingCleared_(false);

ModuleWidget::NetworkClearingScope::NetworkClearingScope()
{
  networkBeingCleared_ = true;
}

ModuleWidget::NetworkClearingScope::~NetworkClearingScope()
{
  networkBeingCleared_ = false;
}

ModuleWidget::~ModuleWidget()
{
  disconnect(this, &ModuleWidget::dynamicPortChanged, this, &ModuleWidget::updateDialogForDynamicPortChange);

  if (!theModule_->isStoppable())
  {
    removeWidgetFromExecutionDisableList(fullWidgetDisplay_->getExecuteButton());
  }
  removeWidgetFromExecutionDisableList(actionsMenu_->getAction("Execute"));
  removeWidgetFromExecutionDisableList(actionsMenu_->getAction("... Downstream Only"));
  if (hasOptions())
    removeWidgetFromExecutionDisableList(dialogManager_.options()->getExecuteAction());

  //TODO: would rather disconnect THIS from removeDynamicPort signaller in DynamicPortManager; need a method on NetworkEditor or something.
  //disconnect()
  deleting_ = true;
  theModule_->disconnectStateListeners();
  for (auto& p : ports_->getAllPorts())
    p->deleteConnections();

  theModule_->setLogger(nullptr);

  if (deletedFromGui_)
  {
    dialogManager_.closeOptions();

    if (dockable_)
    {
      if (isViewScene_) // see bug #808
        dockable_->setFloating(false);
      mainWindowWidget()->removeDockWidget(dockable_);
      delete dockable_;
    }

    dialogManager_.destroyLog();

    Q_EMIT removeModule(ModuleId(moduleId_));
  }
#ifdef MODULE_POSITION_LOGGING
  logCritical("Q_EMIT display Changed {},{}", LOG_FUNC, __LINE__);
#endif

  Q_EMIT displayChanged();
}

void ModuleWidget::trackConnections()
{
  for (auto& p : ports_->getAllPorts())
    p->trackConnections();
}

bool ModuleWidget::executeWithSignals()
{
  executedOnce_ = true;
  if (skipExecuteDueToFatalError_)
    return false;
  {
    Q_EMIT signalExecuteButtonIconChangeToStop();
    errored_ = false;
    //colorLocked_ = true; //TODO
    timer_.reset(new SimpleScopedTimer);
    theModule_->executeWithSignals();
    if (!disabled_)
      Q_EMIT updateProgressBarSignal(1);
    //colorLocked_ = false;
  }
  Q_EMIT moduleExecuted();
  return errored_;
}

boost::signals2::connection ModuleWidget::connectExecuteBegins(const ExecuteBeginsSignalType::slot_type& subscriber)
{
  return theModule_->connectExecuteBegins(subscriber);
}

boost::signals2::connection ModuleWidget::connectExecuteEnds(const ExecuteEndsSignalType::slot_type& subscriber)
{
  return theModule_->connectExecuteEnds(subscriber);
}

boost::signals2::connection ModuleWidget::connectErrorListener(const ErrorSignalType::slot_type& subscriber)
{
  return theModule_->connectErrorListener(subscriber);
}

void ModuleWidget::fillColorStateLookup(const QString& background)
{
  colorStateLookup_.insert(ColorStatePair(moduleRGBA(205,190,112), static_cast<int>(ModuleExecutionState::Value::Waiting)));
  colorStateLookup_.insert(ColorStatePair(moduleRGBA(170, 204, 170), static_cast<int>(ModuleExecutionState::Value::Executing)));
  colorStateLookup_.insert(ColorStatePair(background, static_cast<int>(ModuleExecutionState::Value::Completed)));
  colorStateLookup_.insert(ColorStatePair(moduleRGBA(164, 211, 238), SELECTED));
  colorStateLookup_.insert(ColorStatePair(moduleRGBA(176, 23, 31), static_cast<int>(ModuleExecutionState::Value::Errored)));
}

//primitive state machine--updateBackgroundColor slot needs the thread-safe state machine too
void ModuleWidget::updateBackgroundColorForModuleState(int moduleState)
{
  switch (moduleState)
  {
  case static_cast<int>(ModuleExecutionState::Value::Waiting):
  {
    Q_EMIT backgroundColorUpdated(colorStateLookup_.right.at(static_cast<int>(ModuleExecutionState::Value::Waiting)));
  }
  break;
  case static_cast<int>(ModuleExecutionState::Value::Executing):
  {
    Q_EMIT backgroundColorUpdated(colorStateLookup_.right.at(static_cast<int>(ModuleExecutionState::Value::Executing)));
  }
  break;
  case static_cast<int>(ModuleExecutionState::Value::Completed):
  {
    Q_EMIT backgroundColorUpdated(defaultBackgroundColor_);
  }
  break;
  }
}

void ModuleWidget::updateBackgroundColor(const QString& color)
{
  if (!colorLocked_)
  {
    auto colorToUse(color);

    if (errored_)
    {
      colorToUse = colorStateLookup_.right.at(static_cast<int>(ModuleExecutionState::Value::Errored));
    }

    static const QString rounded("color: white; border-radius: 7px;");
    auto style = rounded + " background-color: " + colorToUse;
    setStyleSheet(style);
    fullWidgetDisplay_->getTitle()->setStyleSheet(style);
    fullWidgetDisplay_->getButtonGroup()->setStyleSheet(style);

    previousModuleState_ = colorStateLookup_.left.at(colorToUse);
  }
}

void ModuleWidget::setColorSelected()
{
  Q_EMIT backgroundColorUpdated(colorStateLookup_.right.at(SELECTED));
  Q_EMIT moduleSelected(true);
}

void ModuleWidget::setColorUnselected()
{
  Q_EMIT backgroundColorUpdated(defaultBackgroundColor_);
  Q_EMIT moduleSelected(false);
}

double ModuleWidget::highResolutionExpandFactor_ = 1;


void ModuleWidget::makeOptionsDialog()
{
  if (theModule_->hasUI())
  {
    if (!dialogManager_.hasOptions())
    {
      {
        if (!ModuleDialogGeneric::factory())
          ModuleDialogGeneric::setFactory(makeShared<ModuleDialogFactory>(nullptr, addWidgetToExecutionDisableList, removeWidgetFromExecutionDisableList));
      }
      dialogManager_.createOptions();

      ModuleOptionsDialogConfiguration config(this);
      dockable_ = config.config(dialogManager_.options());
    }
  }
}

QDialog* ModuleWidget::dialog()
{
  return dialogManager_.options();
}

void ModuleWidget::updateDockWidgetProperties(bool isFloating)
{
  if (isFloating)
  {
    dockable_->setWindowFlags(Qt::Window);
    dockable_->show();
    Q_EMIT showUIrequested(dialogManager_.options());
  }
  dialogManager_.options()->setButtonBarTitleVisible(!isFloating);

  if (isViewScene_) //ugh
  {
    qobject_cast<ViewSceneDialog*>(dialogManager_.options())->setFloatingState(isFloating);
  }
}

void ModuleWidget::updateDialogForDynamicPortChange(const std::string& portId, bool adding)
{
  if (dialogManager_.hasOptions() && !deleting_ && !networkBeingCleared_)
    dialogManager_.options()->updateFromPortChange(numDynamicInputPortsForGuiUpdates(), portId, adding ? DynamicPortChange::USER_ADDED_PORT : DynamicPortChange::USER_REMOVED_PORT);
}

Qt::DockWidgetArea ModuleWidget::allowedDockArea() const
{
  return Preferences::Instance().modulesAreDockable ? Qt::RightDockWidgetArea : Qt::NoDockWidgetArea;
}

void ModuleWidget::adjustDockState(bool dockEnabled)
{
  if (dockable_)
  {
    dockable_->setAllowedAreas(allowedDockArea());
  }

  if (!dockEnabled)
  {
    if (dockable_ && !dockable_->isHidden())
    {
      dockable_->setFloating(true);
      updateDockWidgetProperties(true);
    }
  }
}

QList<QPoint> ModuleWidget::positions_;

void ModuleWidget::toggleOptionsDialog()
{
  if (dialogManager_.hasOptions())
  {
    if (dockable_->isHidden())
    {
      if (firstTimeShown_)
      {
        firstTimeShown_ = false;
        if (!positions_.empty())
        {
          auto maxX = *std::max_element(positions_.begin(), positions_.end(), [](const QPoint& p1, const QPoint& p2) { return p1.x() < p2.x(); });
          auto maxY = *std::max_element(positions_.begin(), positions_.end(), [](const QPoint& p1, const QPoint& p2) { return p1.y() < p2.y(); });
          static const auto rec = QGuiApplication::screens()[0]->size();
          dockable_->move((maxX.x() + 30) % rec.width(), (maxY.y() + 30) % rec.height());
        }
        else
        {
          dockable_->move(isViewScene_ ? 700 : 400, isViewScene_ ? 400 : 200);
        }
        positions_.append(dockable_->pos());
      }
      dockable_->show();
      Q_EMIT showUIrequested(dialogManager_.options());
      dockable_->raise();
      dockable_->activateWindow();
      if (isViewScene_)
      {
        //TODO: figure out why this was needed.
        //dockable_->setFloating(true);
      }
      colorOptionsButton(true);
    }
    else
    {
      hideUI();
      positions_.removeAll(dockable_->pos());
      colorOptionsButton(false);
    }
  }
}

void ModuleWidget::colorOptionsButton(bool visible)
{
  QString styleSheet = visible ? "background-color: rgb(0,0,220); color: white;" : "";
  fullWidgetDisplay_->getOptionsButton()->setStyleSheet(styleSheet);
}

void ModuleWidget::updateProgressBar(double percent)
{
  fullWidgetDisplay_->getProgressBar()->setValue(percent * fullWidgetDisplay_->getProgressBar()->maximum());

  //TODO: make this configurable
  //progressBar_->setTextVisible(true);
  updateModuleTime();
  fullWidgetDisplay_->getProgressBar()->setToolTip(fullWidgetDisplay_->getProgressBar()->text());
}

void ModuleWidget::updateModuleTime()
{
  fullWidgetDisplay_->getProgressBar()->setFormat(QString("%1 s : %p%").arg(timer_->elapsedSeconds()));
}

void ModuleWidget::launchDocumentation()
{
  openUrl(QString::fromStdString(theModule_->newHelpPageUrl()), "module help page");
}

void ModuleWidget::setStartupNote(const QString& text)
{
  if (isViewScene_ || Preferences::Instance().autoNotes)
  {
    auto note = getCurrentNote();
    note.plainText_ = text;
    note.html_ = "<p style=\"color:white\">" + text;
    updateNoteFromFile(note);
  }
}

void ModuleWidget::createStartupNote()
{
  if (dialogManager_.hasOptions())
    dialogManager_.options()->createStartupNote();
}

void ModuleWidget::updateNote(const Note& note)
{
  setCurrentNote(note, false);
  Q_EMIT noteUpdated(note);
}

void ModuleWidget::updateNoteFromFile(const Note& note)
{
  setCurrentNote(note, true);
  Q_EMIT noteUpdated(note);
}

void ModuleWidget::duplicate()
{
  Q_EMIT duplicateModule(theModule_);
}

void ModuleWidget::connectNewModuleTo(const PortDescriptionInterface* portToConnect, const std::string& newModuleName)
{
  Q_EMIT connectNewModule(theModule_, portToConnect, newModuleName);
}

void ModuleWidget::insertNewModuleTo(const PortDescriptionInterface* portToConnect, const QMap<QString, std::string>& info)
{
  Q_EMIT insertNewModule(theModule_, portToConnect, info);
}

bool ModuleWidget::hasDynamicPorts() const
{
  return theModule_->hasDynamicPorts();
}

void ModuleWidget::pinUI()
{
  if (dockable_)
  {
    dockable_->setFloating(false);
    Q_EMIT showUIrequested(dialogManager_.options());
  }
}

void ModuleWidget::hideUI()
{
  if (dockable_)
  {
    dockable_->hide();
  }
}

void ModuleWidget::seeThroughUI()
{
  if (dockable_)
  {
    dockable_->setWindowOpacity(0.6);
  }
}

void ModuleWidget::normalOpacityUI()
{
  if (dockable_)
  {
    dockable_->setWindowOpacity(1);
  }
}

void ModuleWidget::showUI()
{
  if (dockable_)
  {
    dockable_->show();
    dialogManager_.options()->expand();
    Q_EMIT showUIrequested(dialogManager_.options());
  }
}

void ModuleWidget::collapsePinnedDialog()
{
  if (!isViewScene_ && dockable_ && !dockable_->isFloating())
  {
    dialogManager_.options()->collapse();
  }
}

void ModuleWidget::executeButtonPushed()
{
  auto skipUpstream = currentExecuteIcon_ == &downstreamOnlyIcon;
  Q_EMIT executedManually(theModule_, !skipUpstream);
  changeExecuteButtonToStop();
}

void ModuleWidget::executeTriggeredProgrammatically(bool upstream)
{
  Q_EMIT executedManually(theModule_, upstream);
  changeExecuteButtonToStop();
}

void ModuleWidget::executeTriggeredViaStateChange()
{
  Q_EMIT executedManually(theModule_, false);
  changeExecuteButtonToStop();
}

void ModuleWidget::changeExecuteButtonToStop()
{
  fullWidgetDisplay_->getExecuteButton()->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaStop));
  disconnect(fullWidgetDisplay_->getExecuteButton(), &QPushButton::clicked, this, &ModuleWidget::executeButtonPushed);
  connect(fullWidgetDisplay_->getExecuteButton(), &QPushButton::clicked, this, &ModuleWidget::stopButtonPushed);
  movePortWidgets(currentIndex(), static_cast<int>(ModuleWidgetPages::PROGRESS_PAGE));
  setCurrentIndex(static_cast<int>(ModuleWidgetPages::PROGRESS_PAGE));

  fullWidgetDisplay_->startExecuteMovie();
}

void ModuleWidget::changeExecuteButtonToPlay()
{
  fullWidgetDisplay_->getExecuteButton()->setIcon(QPixmap(*currentExecuteIcon_));
  disconnect(fullWidgetDisplay_->getExecuteButton(), &QPushButton::clicked, this, &ModuleWidget::stopButtonPushed);
  connect(fullWidgetDisplay_->getExecuteButton(), &QPushButton::clicked, this, &ModuleWidget::executeButtonPushed);
  movePortWidgets(currentIndex(), static_cast<int>(ModuleWidgetPages::TITLE_PAGE));
  setCurrentIndex(static_cast<int>(ModuleWidgetPages::TITLE_PAGE));
}

void ModuleWidget::stopButtonPushed()
{
  //TODO: doesn't quite work yet
  #if 0
  auto stoppable = std::dynamic_pointer_cast<SCIRun::Core::Thread::Stoppable>(theModule_);
  if (stoppable)
    stoppable->sendStopRequest();
  #endif
}

void ModuleWidget::movePortWidgets(int oldIndex, int newIndex)
{
  removeInputPortsFromWidget(oldIndex);
  removeOutputPortsFromWidget(oldIndex);
  addInputPortsToWidget(newIndex);
  addOutputPortsToWidget(newIndex);

  #ifdef MODULE_POSITION_LOGGING
    logCritical("Q_EMIT display Changed {},{}", LOG_FUNC, __LINE__);
  #endif

  Q_EMIT displayChanged();
}

void ModuleWidget::handleDialogFatalError(const QString& message)
{
  skipExecuteDueToFatalError_ = true;
  qDebug() << "Dialog error: " << message;
  updateBackgroundColor(colorStateLookup_.right.at(static_cast<int>(ModuleExecutionState::Value::Errored)));
  colorLocked_ = true;
  setStartupNote("MODULE FATAL ERROR, DO NOT USE THIS INSTANCE. \nClick \"Refresh\" button to replace module for proper execution.");

  disconnect(fullWidgetDisplay_->getOptionsButton(), &QPushButton::clicked, this, &ModuleWidget::toggleOptionsDialog);

  //This is entirely ViewScene-specific.
  fullWidgetDisplay_->getOptionsButton()->setText("");
  fullWidgetDisplay_->getOptionsButton()->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
  connect(fullWidgetDisplay_->getOptionsButton(), &QPushButton::clicked, this, &ModuleWidget::replaceMe);

  auto id = QString::fromStdString(getModuleId());
  QMessageBox::critical(nullptr, "Critical module error: " + id,
    "Please note the broken module, " + id + ", and replace it with a new instance. This is most likely due to this known bug: https://github.com/SCIInstitute/SCIRun/issues/881");
}

void ModuleWidget::highlightPorts()
{
  ports_->setHighlightPorts(true);
  setPortSpacing(true);

  #ifdef MODULE_POSITION_LOGGING
    logCritical("Q_EMIT display Changed {},{}", LOG_FUNC, __LINE__);
  #endif

  Q_EMIT displayChanged();
}

void ModuleWidget::setPortSpacing(bool highlighted)
{
  int spacing = highlighted ? LARGE_PORT_SPACING : SMALL_PORT_SPACING;
  inputPortLayout_->setSpacing(spacing);
  outputPortLayout_->setSpacing(spacing);
}

void ModuleWidget::setInputPortSpacing(bool highlighted)
{
  int spacing = highlighted ? LARGE_PORT_SPACING : SMALL_PORT_SPACING;
  inputPortLayout_->setSpacing(spacing);
}

void ModuleWidget::setOutputPortSpacing(bool highlighted)
{
  int spacing = highlighted ? LARGE_PORT_SPACING : SMALL_PORT_SPACING;
  outputPortLayout_->setSpacing(spacing);
}

int ModuleWidget::portSpacing() const
{
  return inputPortLayout_->spacing();
}

void ModuleWidget::updatePortSpacing(bool highlighted)
{
  auto port = qobject_cast<PortWidget*>(sender());
  if (port)
  {
    if (port->description()->isInput())
      setInputPortSpacing(highlighted);
    else
      setOutputPortSpacing(highlighted);
  }
}

void ModuleWidget::unhighlightPorts()
{
  ports_->setHighlightPorts(false);
  setPortSpacing(false);

  #ifdef MODULE_POSITION_LOGGING
    logCritical("Q_EMIT display Changed {},{}", LOG_FUNC, __LINE__);
  #endif

  Q_EMIT displayChanged();
}

QString ModuleWidget::metadataToString() const
{
  auto metadata = theModule_->metadata().getFullMap();
  QStringList display;
  for (const auto& metaPair : metadata)
  {
    display.append(QString::fromStdString(metaPair.first) + " : " + QString::fromStdString(metaPair.second));
  }
  return display.join("\n");
}

void ModuleWidget::updateMetadata(bool active)
{
  setToolTip(active ? "    ~ " + QString::fromStdString(moduleId_) + " Metadata ~\n\n" + metadataToString() : "");
}

void ModuleWidget::setExecutionDisabled(bool disabled)
{
  disabled_ = disabled;

  Q_EMIT executionDisabled(disabled_);

  theModule_->setExecutionDisabled(disabled_);
}

void ModuleWidget::incomingConnectionStateChanged(bool disabled, int index)
{
  if (index < theModule_->numInputPorts())
  {
    theModule_->inputPorts()[index]->connection(0)->setDisable(disabled);
  }

  bool shouldDisable;
  if (disabled)
  {
    if (isViewScene_)
      shouldDisable = !std::any_of(ports().inputs().cbegin(), ports().inputs().cend(), [](const PortWidget* input) { return input->firstConnection() && !input->firstConnection()->disabled(); });
    else // here is where to consider optional ports, see issue #?
      shouldDisable = true;
  }
  else
  {
    if (isViewScene_)
      shouldDisable = !std::any_of(ports().inputs().cbegin(), ports().inputs().cend(), [](const PortWidget* input) { return input->firstConnection() && !input->firstConnection()->disabled(); });
    else
      shouldDisable = std::any_of(ports().inputs().cbegin(), ports().inputs().cend(), [](const PortWidget* input) { return input->isConnected() && input->firstConnection()->disabled(); });
  }

  setExecutionDisabled(shouldDisable);

  for (const auto& output : ports().outputs())
  {
    output->setConnectionsDisabled(disabled_ || disabled);
  }
}

void ModuleWidget::saveImagesFromViewScene()
{
  if (isViewScene_)
  {
    qobject_cast<ViewSceneDialog*>(dialogManager_.options())->autoSaveScreenshot();
  }
}

void ModuleWidget::setupPortSceneCollaborator(QGraphicsProxyWidget* proxy)
{
  connectionFactory_ = makeShared<ConnectionFactory>(proxy);
  closestPortFinder_ = makeShared<ClosestPortFinder>(proxy);
  ports().setSceneFunc([proxy]() { return proxy->scene(); });
}

void ModuleWidget::toggleProgrammableInputPort()
{
  programmablePortEnabled_ = !programmablePortEnabled_;
  theModule_->setProgrammableInputPortEnabled(programmablePortEnabled_);
}
#if 0
void SubnetWidget::postLoadAction()
{
  fullWidgetDisplay_->setupSubnetWidgets();
}
#endif
