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

#include <iostream>
#include <QtGui>
#include "ui_Module.h"
#include <boost/thread.hpp>
#include <Core/Logging/Log.h>
#include <Core/Application/Application.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>

#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/PositionProvider.h>
#include <Interface/Application/ModuleLogWindow.h>
#include <Interface/Application/ClosestPortFinder.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Modules/Factory/ModuleDialogFactory.h>
#include <Interface/Application/PortWidgetManager.h>
#include <Core/Application/Preferences/Preferences.h>

//TODO: BAD, or will we have some sort of Application global anyway?
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/MainWindowCollaborators.h>

using namespace SCIRun;
using namespace Core;
using namespace Gui;
using namespace Dataflow::Networks;
using namespace Logging;

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
        << new QAction("Help", parent)
        << new QAction("Edit Notes...", parent)
        << new QAction("Duplicate", parent)
        << new QAction("Replace With", parent)
        //<< disabled(new QAction("Ignore*", parent))
        << new QAction("Show Log", parent)
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
    //TODO: becky's alpha number didn't look good here, it may be a Qt/coloring problem. Will wait until I get correct background.
    return SCIRunMainWindow::Instance()->newInterface() ? 100 : 255;
  }
  int portAlpha()
  {
    return SCIRunMainWindow::Instance()->newInterface() ? 230 : 255;
  }
  QString moduleRGBA(int r, int g, int b)
  {
    return QString("rgba(%1,%2,%3,%4)")
      .arg(r).arg(g).arg(b)
      .arg(moduleAlpha());
  }
}

class ModuleWidgetDisplay : public Ui::Module, public ModuleWidgetDisplayBase
{
public:
  virtual void setupFrame(QStackedWidget* stacked) override;
  virtual void setupTitle(const QString& name) override;
  virtual void setupProgressBar() override;
  virtual void setupSpecial() override;
  virtual void setupButtons(bool hasUI, QObject* module) override;
  virtual void setupIcons() override;
  virtual QAbstractButton* getOptionsButton() const override;
  virtual QAbstractButton* getExecuteButton() const override;
  virtual QAbstractButton* getHelpButton() const override;
  virtual QAbstractButton* getLogButton() const override;
  virtual QPushButton* getModuleActionButton() const override;

  virtual QProgressBar* getProgressBar() const override;

  virtual int getTitleWidth() const override;
  virtual QLabel* getTitle() const override;

  virtual void startExecuteMovie() override;
  virtual void stopExecuteMovie() override;
};

void ModuleWidgetDisplay::setupFrame(QStackedWidget* stacked)
{
  setupUi(stacked);
}

void ModuleWidgetDisplay::setupTitle(const QString& name)
{
  QFont titleFont("Helvetica", titleFontSize, QFont::Bold);
  titleLabel_->setFont(titleFont);
  titleLabel_->setText(name);
  QFont smallerTitleFont("Helvetica", titleFontSize - buttonPageFontSizeDiff);
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
  optionsButton_->setFont(QFont("Helvetica", viewFontSize));
  optionsButton_->setToolTip("View renderer output");

  //optionsButton_->setMaximumWidth(140);
  //optionsButton_->resize(140, optionsButton_->height());

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
  getExecuteButton()->setIcon(QPixmap(":/general/Resources/new/modules/run.png"));
  getOptionsButton()->setText("");
  getOptionsButton()->setIcon(QPixmap(":/general/Resources/new/modules/options.png"));
  getHelpButton()->setText("");
  getHelpButton()->setIcon(QPixmap(":/general/Resources/new/modules/help.png"));
  getLogButton()->setText("");
  getLogButton()->setIcon(QPixmap(":/general/Resources/new/modules/info.png"));
  getModuleActionButton()->setText("");
  getModuleActionButton()->setIcon(QPixmap(":/general/Resources/new/modules/settings.png"));

  auto movie = new QMovie(":/general/Resources/executing.gif");
  executingLabel_->setMovie(movie);
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

static const int UNSET = -1;
static const int SELECTED = -50;

typedef boost::bimap<QString, int> ColorStateLookup;
typedef ColorStateLookup::value_type ColorStatePair;
static ColorStateLookup colorStateLookup;
void fillColorStateLookup(const QString& background);

ModuleWidget::ModuleWidget(NetworkEditor* ed, const QString& name, ModuleHandle theModule, boost::shared_ptr<DialogErrorControl> dialogErrorControl,
  QWidget* parent /* = 0 */)
  : QStackedWidget(parent), HasNotes(theModule->get_id(), true),
  fullWidgetDisplay_(new ModuleWidgetDisplay),
  ports_(new PortWidgetManager),
  deletedFromGui_(true),
  colorLocked_(false),
  isMini_(false),
  errored_(false),
  executedOnce_(false),
  skipExecuteDueToFatalError_(false),
  disabled_(false),
  theModule_(theModule),
  previousModuleState_(UNSET),
  moduleId_(theModule->get_id()),
  dialog_(nullptr),
  dockable_(nullptr),
  dialogErrorControl_(dialogErrorControl),
  inputPortLayout_(nullptr),
  outputPortLayout_(nullptr),
  editor_(ed),
  deleting_(false),
  defaultBackgroundColor_(SCIRunMainWindow::Instance()->newInterface() ? moduleRGBA(99,99,104) : moduleRGBA(192,192,192)),
  isViewScene_(name == "ViewScene")
{
  fillColorStateLookup(defaultBackgroundColor_);

  setupModuleActions();
  setupLogging();

  setCurrentIndex(buildDisplay(fullWidgetDisplay_.get(), name));

  makeOptionsDialog();
  createPorts(*theModule_);
  addPorts(currentIndex());

  //qDebug() << width() << height() << currentWidget()->size();
  //resize(currentWidget()->size());
  //qDebug() << width() << height() << currentWidget()->size();

  connect(this, SIGNAL(backgroundColorUpdated(const QString&)), this, SLOT(updateBackgroundColor(const QString&)));
  theModule_->executionState().connectExecutionStateChanged([this](int state) { QtConcurrent::run(boost::bind(&ModuleWidget::updateBackgroundColorForModuleState, this, state)); });

  theModule_->connectExecuteSelfRequest([this]() { executeAgain(); });
  connect(this, SIGNAL(executeAgain()), this, SLOT(executeButtonPushed()));

  Preferences::Instance().modulesAreDockable.connectValueChanged(boost::bind(&ModuleWidget::adjustDockState, this, _1));

  connect(actionsMenu_->getAction("Destroy"), SIGNAL(triggered()), this, SIGNAL(deleteMeLater()));

  connectExecuteEnds(boost::bind(&ModuleWidget::executeEnds, this));
  connect(this, SIGNAL(executeEnds()), this, SLOT(changeExecuteButtonToPlay()));
  connect(this, SIGNAL(signalExecuteButtonIconChangeToStop()), this, SLOT(changeExecuteButtonToStop()));
  //qDebug() << width() << height() << currentWidget()->size();
}

int ModuleWidget::buildDisplay(ModuleWidgetDisplayBase* display, const QString& name)
{
  display->setupFrame(this);

  setupDisplayWidgets(display, name);

  addPortLayouts(0);

  resizeBasedOnModuleName(display, 0);

  setupDisplayConnections(display);

  return 0;
}

void ModuleWidget::setupLogging()
{
  logWindow_ = new ModuleLogWindow(QString::fromStdString(moduleId_), editor_, dialogErrorControl_, SCIRunMainWindow::Instance());
  connect(actionsMenu_->getAction("Show Log"), SIGNAL(triggered()), logWindow_, SLOT(show()));
  connect(actionsMenu_->getAction("Show Log"), SIGNAL(triggered()), logWindow_, SLOT(raise()));
  connect(logWindow_, SIGNAL(messageReceived(const QColor&)), this, SLOT(setLogButtonColor(const QColor&)));
  connect(logWindow_, SIGNAL(requestModuleVisible()), this, SIGNAL(requestModuleVisible()));

  LoggerHandle logger(boost::make_shared<ModuleLogger>(logWindow_));
  theModule_->setLogger(logger);
  theModule_->setUpdaterFunc(boost::bind(&ModuleWidget::updateProgressBarSignal, this, _1));
  if (theModule_->has_ui())
    theModule_->setUiToggleFunc([this](bool b){ dockable_->setVisible(b); });
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
  display->setupButtons(theModule_->has_ui(), this);
}

#ifdef WIN32
  const int ModuleWidgetDisplayBase::moduleWidthThreshold = 110;
  const int ModuleWidgetDisplayBase::extraModuleWidth = 5;
  const int ModuleWidgetDisplayBase::extraWidthThreshold = 5;
  const int ModuleWidgetDisplayBase::smushFactor = 15;
  const int ModuleWidgetDisplayBase::titleFontSize = 9;
  const int ModuleWidgetDisplayBase::viewFontSize = 6;
  const int ModuleWidgetDisplayBase::buttonPageFontSizeDiff = 1;
  const int ModuleWidgetDisplayBase::widgetHeightAdjust = -20;
  const int ModuleWidgetDisplayBase::widgetWidthAdjust = -10;
#else
  const int ModuleWidgetDisplayBase::moduleWidthThreshold = 120;
  const int ModuleWidgetDisplayBase::extraModuleWidth = 5;
  const int ModuleWidgetDisplayBase::extraWidthThreshold = 5;
  const int ModuleWidgetDisplayBase::smushFactor = 15;
  const int ModuleWidgetDisplayBase::titleFontSize = 13;
  const int ModuleWidgetDisplayBase::viewFontSize = 8;
  const int ModuleWidgetDisplayBase::buttonPageFontSizeDiff = 3;
  const int ModuleWidgetDisplayBase::widgetHeightAdjust = 1;
  const int ModuleWidgetDisplayBase::widgetWidthAdjust = -20;
#endif

void ModuleWidget::resizeBasedOnModuleName(ModuleWidgetDisplayBase* display, int index)
{
  auto frame = this;
  int pixelWidth = display->getTitleWidth();
  //qDebug() << moduleId_.c_str();
  //qDebug() << "\tPixelwidth = " << pixelWidth;
  int extraWidth = pixelWidth - ModuleWidgetDisplayBase::moduleWidthThreshold;
  //qDebug() << "\textraWidth = " << extraWidth;
  if (extraWidth > ModuleWidgetDisplayBase::extraWidthThreshold)
  {
    //qDebug() << "\tGROWING MODULE Current width: " << frame->width();
    frame->resize(frame->width() + extraWidth + ModuleWidgetDisplayBase::extraModuleWidth, frame->height());
    //qDebug() << "\tNew width: " << frame->width();
  }
  else
  {
    //qDebug() << "\tSHRINKING MODULE Current width: " << frame->width();
    frame->resize(frame->width() - ModuleWidgetDisplayBase::smushFactor, frame->height());
    //qDebug() << "\tNew width: " << frame->width();
  }
  //qDebug() << size() << frame->size();
  //frame->resize(frame->width() + ModuleWidgetDisplayBase::widgetWidthAdjust, frame->height() + ModuleWidgetDisplayBase::widgetHeightAdjust);
  //qDebug() << size() << frame->size();
}

void ModuleWidget::setupDisplayConnections(ModuleWidgetDisplayBase* display)
{
  connect(display->getExecuteButton(), SIGNAL(clicked()), this, SLOT(executeButtonPushed()));
  if (!theModule_->isStoppable())
  {
    addWidgetToExecutionDisableList(display->getExecuteButton());
  }
  connect(display->getOptionsButton(), SIGNAL(clicked()), this, SLOT(toggleOptionsDialog()));
  connect(display->getHelpButton(), SIGNAL(clicked()), this, SLOT(launchDocumentation()));
  connect(display->getLogButton(), SIGNAL(clicked()), logWindow_, SLOT(show()));
  connect(display->getLogButton(), SIGNAL(clicked()), logWindow_, SLOT(raise()));
  display->getModuleActionButton()->setMenu(actionsMenu_->getMenu());
}

void ModuleWidget::setLogButtonColor(const QColor& color)
{
  if (color == Qt::red)
  {
    //qDebug() << "errored set color";
    errored_ = true;
    //Q_EMIT backgroundColorUpdated(moduleRGBA(176, 23, 31));
    updateBackgroundColor(colorStateLookup.right.at(static_cast<int>(ModuleExecutionState::Errored)));
  }
  fullWidgetDisplay_->getLogButton()->setStyleSheet(
    QString("* { background-color: %1 }")
    .arg(moduleRGBA(color.red(), color.green(), color.blue())));
}

void ModuleWidget::resetLogButtonColor()
{
  fullWidgetDisplay_->getLogButton()->setStyleSheet("");
}

void ModuleWidget::resetProgressBar()
{
  fullWidgetDisplay_->getProgressBar()->setValue(0);
  fullWidgetDisplay_->getProgressBar()->setTextVisible(false);
}

size_t ModuleWidget::numInputPorts() const { return ports().numInputPorts(); }
size_t ModuleWidget::numOutputPorts() const { return ports().numOutputPorts(); }

void ModuleWidget::setupModuleActions()
{
  actionsMenu_.reset(new ModuleActionsMenu(this, moduleId_));
  addWidgetToExecutionDisableList(actionsMenu_->getAction("Execute"));

  connect(actionsMenu_->getAction("Execute"), SIGNAL(triggered()), this, SLOT(executeButtonPushed()));
  connect(this, SIGNAL(updateProgressBarSignal(double)), this, SLOT(updateProgressBar(double)));
  connect(actionsMenu_->getAction("Help"), SIGNAL(triggered()), this, SLOT(launchDocumentation()));
  connect(actionsMenu_->getAction("Duplicate"), SIGNAL(triggered()), this, SLOT(duplicate()));
  if (isViewScene_ || theModule_->hasDynamicPorts()) //TODO: buggy combination, will disable for now. Fix is #1035
    actionsMenu_->getMenu()->removeAction(actionsMenu_->getAction("Duplicate"));

  connectNoteEditorToAction(actionsMenu_->getAction("Notes"));
  connectUpdateNote(this);
}

void ModuleWidget::postLoadAction()
{
  auto replaceWith = actionsMenu_->getAction("Replace With");
  auto menu = new QMenu(this);
  replaceWith->setMenu(menu);
  fillReplaceWithMenu();
  connect(this, SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)), this, SLOT(fillReplaceWithMenu()));
  connect(this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)), this, SLOT(fillReplaceWithMenu()));
}

bool ModuleWidget::guiVisible() const
{
  if (dockable_)
    return dockable_->isVisible();
  return false;
}

void ModuleWidget::fillReplaceWithMenu()
{
  auto menu = getReplaceWithMenu();
  menu->clear();
  LOG_DEBUG("Filling menu for " << theModule_->get_module_name() << std::endl);
  auto replacements = Application::Instance().controller()->possibleReplacements(this->theModule_);
  auto isReplacement = [&](const ModuleDescription& md) { return replacements.find(md.lookupInfo_) != replacements.end(); };
  fillMenuWithFilteredModuleActions(menu, Application::Instance().controller()->getAllAvailableModuleDescriptions(),
    isReplacement,
    [=](QAction* action) { QObject::connect(action, SIGNAL(triggered()), this, SLOT(replaceModuleWith())); },
    fullWidgetDisplay_->getModuleActionButton());
}

QMenu* ModuleWidget::getReplaceWithMenu()
{
  return actionsMenu_->getAction("Replace With")->menu();
}

void ModuleWidget::replaceModuleWith()
{
  auto action = qobject_cast<QAction*>(sender());
  auto moduleToReplace = action->text();
  Q_EMIT replaceModuleWith(theModule_, moduleToReplace.toStdString());
}

void ModuleWidget::replaceMe()
{
  if (!executedOnce_)
    Q_EMIT replaceModuleWith(theModule_, theModule_->get_module_name());
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

void ModuleWidget::createInputPorts(const ModuleInfoProvider& moduleInfoProvider)
{
  const auto moduleId = moduleInfoProvider.get_id();
  size_t i = 0;
  const auto& inputs = moduleInfoProvider.inputPorts();
  for (const auto& port : inputs)
  {
    auto type = port->get_typename();
    //std::cout << "ADDING PORT: " << port->id() << "[" << port->isDynamic() << "] AT INDEX: " << i << std::endl;
    auto w = new InputPortWidget(QString::fromStdString(port->get_portname()), to_color(PortColorLookup::toColor(type),
      portAlpha()), type,
      moduleId, port->id(),
      i, port->isDynamic(), connectionFactory_,
      closestPortFinder_,
      PortDataDescriber(),
      this);
    hookUpGeneralPortSignals(w);
    connect(this, SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)), w, SLOT(MakeTheConnection(const SCIRun::Dataflow::Networks::ConnectionDescription&)));
    connect(w, SIGNAL(incomingConnectionStateChange(bool)), this, SLOT(incomingConnectionStateChanged(bool)));
    ports_->addPort(w);
    ++i;
    if (dialog_ && port->isDynamic())
    {
      auto portConstructionType = DynamicPortChange::INITIAL_PORT_CONSTRUCTION;
      auto nameMatches = [&](const InputPortHandle& in)
      {
        return in->id().name == port->id().name;
      };
      auto justAddedIndex = i - 1;
      bool isNotLastDynamicPortOfThisName = justAddedIndex < inputs.size() - 1
        && std::find_if(inputs.cbegin() + justAddedIndex + 1, inputs.cend(), nameMatches) != inputs.cend();
      //qDebug() << "UPDATE FROM PORT CHANGE TYPE CHECK:" << isNotLastDynamicPortOfThisName << justAddedIndex << inputs.size() << (justAddedIndex < inputs.size() - 1)
        //<< ((justAddedIndex < inputs.size() - 1) && (std::find_if(inputs.cbegin() + justAddedIndex + 1, inputs.cend(), nameMatches) != inputs.end()));
      if (isNotLastDynamicPortOfThisName)
        portConstructionType = DynamicPortChange::USER_ADDED_PORT_DURING_FILE_LOAD;
      dialog_->updateFromPortChange(i, port->id().toString(), portConstructionType);
    }
  }
}

void ModuleWidget::printInputPorts(const ModuleInfoProvider& moduleInfoProvider) const
{
  const auto moduleId = moduleInfoProvider.get_id();
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
  const ModuleId moduleId = moduleInfoProvider.get_id();
  size_t i = 0;
  for (const auto& port : moduleInfoProvider.outputPorts())
  {
    auto type = port->get_typename();
    auto w = new OutputPortWidget(QString::fromStdString(port->get_portname()), to_color(PortColorLookup::toColor(type), portAlpha()),
      type, moduleId, port->id(), i, port->isDynamic(),
      connectionFactory_,
      closestPortFinder_,
      port->getPortDataDescriber(),
      this);
    hookUpGeneralPortSignals(w);
    ports_->addPort(w);
    ++i;
  }
}

void ModuleWidget::hookUpGeneralPortSignals(PortWidget* port) const
{
  connect(port, SIGNAL(requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const SCIRun::Dataflow::Networks::PortDescriptionInterface*)),
    this, SIGNAL(requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const SCIRun::Dataflow::Networks::PortDescriptionInterface*)));
  connect(port, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)),
    this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)));
  connect(this, SIGNAL(cancelConnectionsInProgress()), port, SLOT(cancelConnectionsInProgress()));
  connect(this, SIGNAL(cancelConnectionsInProgress()), port, SLOT(clearPotentialConnections()));
  connect(port, SIGNAL(connectNewModule(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const std::string&)),
    this, SLOT(connectNewModule(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const std::string&)));
  connect(port, SIGNAL(connectionNoteChanged()), this, SIGNAL(noteChanged()));
  connect(port, SIGNAL(highlighted(bool)), this, SLOT(updatePortSpacing(bool)));
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
    qDebug() << "OOPS NO OUTPUT PORTS";
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

  for (PortWidget* port : inputPorts_)
    layout->addWidget(port);

  layout->setSizeConstraint(QLayout::SetMinimumSize);
  //qDebug() << "input port layout min size: " << layout->minimumSize();
  //qDebug() << "input port layout max size: " << layout->maximumSize();
}

void PortWidgetManager::addOutputsToLayout(QHBoxLayout* layout)
{
  if (outputPorts_.empty())
    layout->addWidget(new BlankPort(layout->parentWidget()));

  for (PortWidget* port : outputPorts_)
    layout->addWidget(port);

  layout->setSizeConstraint(QLayout::SetMinimumSize);
  //qDebug() << "output port layout min size: " << layout->minimumSize();
  //qDebug() << "output port layout max size: " << layout->maximumSize();
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
    qDebug() << "OOPS NO INPUT PORTS";
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
}

void PortWidgetManager::insertPort(int index, InputPortWidget* port)
{
  if (index > inputPorts_.size())
    inputPorts_.push_back(port);
  else
    inputPorts_.insert(inputPorts_.begin() + index, port);
}

void PortWidgetManager::setHighlightPorts(bool on)
{
  for (auto& port : getAllPorts())
  {
    port->setHighlight(on);
  }
}

void ModuleWidget::addDynamicPort(const ModuleId& mid, const PortId& pid)
{
  if (mid.id_ == moduleId_)
  {
    //std::cout << "ADDING NEW DYNAMIC PORT: " << pid << std::endl;
    auto port = theModule_->getInputPort(pid);
    auto type = port->get_typename();

    auto w = new InputPortWidget(QString::fromStdString(port->get_portname()), to_color(PortColorLookup::toColor(type)), type, mid, port->id(), port->getIndex(), port->isDynamic(), connectionFactory_, closestPortFinder_, PortDataDescriber(), this);
    hookUpGeneralPortSignals(w);
    connect(this, SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)), w, SLOT(MakeTheConnection(const SCIRun::Dataflow::Networks::ConnectionDescription&)));

    const int newPortIndex = port->getIndex();

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
  auto iter = std::find_if(inputPorts_.begin(), inputPorts_.end(), [&](const PortWidget* w) { return w->id() == pid; });
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
  Q_FOREACH(PortWidget* p, ports_->getAllPorts())
  {
    std::cout << "\t" << p->pos();
  }
  std::cout << std::endl;
}

enum ModuleWidgetPages
{
  TITLE_PAGE,
  PROGRESS_PAGE,
  BUTTON_PAGE
};

void ModuleWidget::enterEvent(QEvent* event)
{
  previousPageIndex_ = currentIndex();
  movePortWidgets(previousPageIndex_, BUTTON_PAGE);
  setCurrentIndex(BUTTON_PAGE);
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
  disconnect(this, SIGNAL(dynamicPortChanged(const std::string&, bool)), this, SLOT(updateDialogForDynamicPortChange(const std::string&, bool)));

  if (!theModule_->isStoppable())
  {
    removeWidgetFromExecutionDisableList(fullWidgetDisplay_->getExecuteButton());
  }
  removeWidgetFromExecutionDisableList(actionsMenu_->getAction("Execute"));
  if (dialog_)
    removeWidgetFromExecutionDisableList(dialog_->getExecuteAction());

  //TODO: would rather disconnect THIS from removeDynamicPort signaller in DynamicPortManager; need a method on NetworkEditor or something.
  //disconnect()
  deleting_ = true;
  Q_FOREACH (PortWidget* p, ports_->getAllPorts())
    p->deleteConnections();

  //GuiLogger::Instance().log("Module deleted.");

  theModule_->setLogger(nullptr);


  if (deletedFromGui_)
  {
    if (dialog_)
    {
      dialog_->close();
    }

    if (dockable_)
    {
      if (isViewScene_) // see bug #808
        dockable_->setFloating(false);
      SCIRunMainWindow::Instance()->removeDockWidget(dockable_);
      delete dockable_;
    }

    delete logWindow_;
    logWindow_ = nullptr;

    Q_EMIT removeModule(ModuleId(moduleId_));
  }
  Q_EMIT displayChanged();
}

void ModuleWidget::trackConnections()
{
  Q_FOREACH (PortWidget* p, ports_->getAllPorts())
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
    timer_.restart();
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

void fillColorStateLookup(const QString& background)
{
  if (colorStateLookup.empty())
  {
    colorStateLookup.insert(ColorStatePair(moduleRGBA(205,190,112), static_cast<int>(ModuleExecutionState::Waiting)));
    colorStateLookup.insert(ColorStatePair(moduleRGBA(170, 204, 170), static_cast<int>(ModuleExecutionState::Executing)));
    colorStateLookup.insert(ColorStatePair(background, static_cast<int>(ModuleExecutionState::Completed)));
    colorStateLookup.insert(ColorStatePair(moduleRGBA(164, 211, 238), SELECTED));
    colorStateLookup.insert(ColorStatePair(moduleRGBA(176, 23, 31), static_cast<int>(ModuleExecutionState::Errored)));
  }
}

//primitive state machine--updateBackgroundColor slot needs the thread-safe state machine too
void ModuleWidget::updateBackgroundColorForModuleState(int moduleState)
{
  //qDebug() << "color slot: " << moduleState;
  switch (moduleState)
  {
  case static_cast<int>(ModuleExecutionState::Waiting):
  {
    //qDebug() << "waiting color";
    Q_EMIT backgroundColorUpdated(colorStateLookup.right.at(static_cast<int>(ModuleExecutionState::Waiting)));
  }
  break;
  case static_cast<int>(ModuleExecutionState::Executing):
  {
    //qDebug() << "executing color";
    Q_EMIT backgroundColorUpdated(colorStateLookup.right.at(static_cast<int>(ModuleExecutionState::Executing)));
  }
  break;
  case static_cast<int>(ModuleExecutionState::Completed):
  {
    if (!errored_)
    {
      //qDebug() << "completed color";
      Q_EMIT backgroundColorUpdated(defaultBackgroundColor_);
    }
    //else qDebug() << "errored color";
  }
  break;
  }
}

void ModuleWidget::updateBackgroundColor(const QString& color)
{
  if (!colorLocked_)
  {
    //qDebug() << "color update: " << color;
    QString rounded;
    if (SCIRunMainWindow::Instance()->newInterface())
      rounded = "color: white; border-radius: 7px;";
    setStyleSheet(rounded + " background-color: " + color);
    previousModuleState_ = colorStateLookup.left.at(color);
  }
}

void ModuleWidget::setColorSelected()
{
  updateBackgroundColor(colorStateLookup.right.at(SELECTED));
  Q_EMIT moduleSelected(true);
}

void ModuleWidget::setColorUnselected()
{
  updateBackgroundColor(defaultBackgroundColor_);
  Q_EMIT moduleSelected(false);
}

boost::shared_ptr<ModuleDialogFactory> ModuleWidget::dialogFactory_;

void ModuleWidget::makeOptionsDialog()
{
  if (theModule_->has_ui())
  {
    if (!dialog_)
    {
      if (!dialogFactory_)
        dialogFactory_.reset(new ModuleDialogFactory(nullptr, addWidgetToExecutionDisableList, removeWidgetFromExecutionDisableList));

      dialog_ = dialogFactory_->makeDialog(moduleId_, theModule_->get_state());
      addWidgetToExecutionDisableList(dialog_->getExecuteAction());
      connect(dialog_, SIGNAL(executeActionTriggered()), this, SLOT(executeButtonPushed()));
      connect(this, SIGNAL(moduleExecuted()), dialog_, SLOT(moduleExecuted()));
      connect(this, SIGNAL(moduleSelected(bool)), dialog_, SLOT(moduleSelected(bool)));
      connect(this, SIGNAL(dynamicPortChanged(const std::string&, bool)), this, SLOT(updateDialogForDynamicPortChange(const std::string&, bool)));
      connect(dialog_, SIGNAL(setStartupNote(const QString&)), this, SLOT(setStartupNote(const QString&)));
      connect(dialog_, SIGNAL(fatalError(const QString&)), this, SLOT(handleDialogFatalError(const QString&)));
      connect(dialog_, SIGNAL(executionLoopStarted()), this, SIGNAL(disableWidgetDisabling()));
      connect(dialog_, SIGNAL(executionLoopHalted()), this, SIGNAL(reenableWidgetDisabling()));
      dockable_ = new QDockWidget(QString::fromStdString(moduleId_), nullptr);
      dockable_->setObjectName(dialog_->windowTitle());
      dockable_->setWidget(dialog_);
      dialog_->setDockable(dockable_);
      dockable_->setMinimumSize(dialog_->minimumSize());
      dockable_->setAllowedAreas(allowedDockArea());
      dockable_->setAutoFillBackground(true);
      SCIRunMainWindow::Instance()->addDockWidget(Qt::RightDockWidgetArea, dockable_);
      if (!isViewScene_)
        dockable_->setFloating(!Preferences::Instance().modulesAreDockable);
      dockable_->hide();
      connect(dockable_, SIGNAL(visibilityChanged(bool)), this, SLOT(colorOptionsButton(bool)));
      connect(dockable_, SIGNAL(topLevelChanged(bool)), this, SLOT(updateDockWidgetProperties(bool)));

      if (isViewScene_ && Application::Instance().parameters()->isRegressionMode())
      {
        dockable_->show();
        dockable_->setFloating(true);
      }

      dialog_->pull();
    }
  }
}

void ModuleWidget::updateDockWidgetProperties(bool isFloating)
{
  if (isFloating)
  {
    dockable_->setWindowFlags(Qt::Window);
    dockable_->show();
  }
}

void ModuleWidget::updateDialogForDynamicPortChange(const std::string& portId, bool adding)
{
  if (dialog_ && !deleting_ && !networkBeingCleared_)
    dialog_->updateFromPortChange(numInputPorts(), portId, adding ? DynamicPortChange::USER_ADDED_PORT : DynamicPortChange::USER_REMOVED_PORT);
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
    }
  }
}

boost::shared_ptr<ConnectionFactory> ModuleWidget::connectionFactory_;
boost::shared_ptr<ClosestPortFinder> ModuleWidget::closestPortFinder_;

void ModuleWidget::toggleOptionsDialog()
{
  if (dialog_)
  {
    if (dockable_->isHidden())
    {
      dockable_->show();
      dockable_->raise();
      dockable_->activateWindow();
      if (isViewScene_)
      {
        dockable_->setFloating(true);
      }
      colorOptionsButton(true);
    }
    else
    {
      hideUI();
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
  fullWidgetDisplay_->getProgressBar()->setFormat(QString("%1 s : %p%").arg(timer_.elapsed()));
}

void ModuleWidget::launchDocumentation()
{
  //TODO: push this help url construction to module layer
  openUrl("http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php/CIBC:Documentation:SCIRun:Reference:SCIRun:" + QString::fromStdString(getModule()->get_module_name()), "module help page");
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
  if (dialog_)
    dialog_->createStartupNote();
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

void ModuleWidget::connectNewModule(const PortDescriptionInterface* portToConnect, const std::string& newModuleName)
{
  setProperty(addNewModuleActionTypePropertyName(), sender()->property(addNewModuleActionTypePropertyName()));
  setProperty(insertNewModuleActionTypePropertyName(), sender()->property(insertNewModuleActionTypePropertyName()));

  Q_EMIT connectNewModule(theModule_, portToConnect, newModuleName);
}

bool ModuleWidget::hasDynamicPorts() const
{
  return theModule_->hasDynamicPorts();
}

void ModuleWidget::pinUI()
{
  if (dockable_)
    dockable_->setFloating(false);
}

void ModuleWidget::hideUI()
{
  if (dockable_)
    dockable_->hide();
}

void ModuleWidget::showUI()
{
  if (dockable_)
    dockable_->show();
}

void ModuleWidget::executeButtonPushed()
{
  LOG_DEBUG("Execute button pushed on module " << moduleId_ << std::endl);
  Q_EMIT executedManually(theModule_);
  changeExecuteButtonToStop();
}

void ModuleWidget::changeExecuteButtonToStop()
{
  fullWidgetDisplay_->getExecuteButton()->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaStop));
  disconnect(fullWidgetDisplay_->getExecuteButton(), SIGNAL(clicked()), this, SLOT(executeButtonPushed()));
  connect(fullWidgetDisplay_->getExecuteButton(), SIGNAL(clicked()), this, SLOT(stopButtonPushed()));
  movePortWidgets(currentIndex(), PROGRESS_PAGE);
  setCurrentIndex(PROGRESS_PAGE);

  fullWidgetDisplay_->startExecuteMovie();
}

void ModuleWidget::changeExecuteButtonToPlay()
{
  fullWidgetDisplay_->getExecuteButton()->setIcon(QPixmap(":/general/Resources/new/modules/run.png"));
  disconnect(fullWidgetDisplay_->getExecuteButton(), SIGNAL(clicked()), this, SLOT(stopButtonPushed()));
  connect(fullWidgetDisplay_->getExecuteButton(), SIGNAL(clicked()), this, SLOT(executeButtonPushed()));
  movePortWidgets(currentIndex(), TITLE_PAGE);
  setCurrentIndex(TITLE_PAGE);
}

void ModuleWidget::stopButtonPushed()
{
  //qDebug() << "stop pressed for " << QString::fromStdString(theModule_->get_id().id_);
  Q_EMIT interrupt(theModule_->get_id());
}

bool ModuleWidget::globalMiniMode_(false);

void ModuleWidget::movePortWidgets(int oldIndex, int newIndex)
{
  //qDebug() << "movePortWidgets" << oldIndex << newIndex;
  removeInputPortsFromWidget(oldIndex);
  removeOutputPortsFromWidget(oldIndex);
  addInputPortsToWidget(newIndex);
  addOutputPortsToWidget(newIndex);
  //auto size = widget(newIndex)->size();
  //setCurrentIndex(newIndex);
  //resize(size);
  Q_EMIT displayChanged();
}

void ModuleWidget::handleDialogFatalError(const QString& message)
{
  skipExecuteDueToFatalError_ = true;
  qDebug() << "Dialog error: " << message;
  updateBackgroundColor(colorStateLookup.right.at(static_cast<int>(ModuleExecutionState::Errored)));
  colorLocked_ = true;
  setStartupNote("MODULE FATAL ERROR, DO NOT USE THIS INSTANCE. \nClick \"Refresh\" button to replace module for proper execution.");

  disconnect(fullWidgetDisplay_->getOptionsButton(), SIGNAL(clicked()), this, SLOT(toggleOptionsDialog()));

  //This is entirely ViewScene-specific.
  fullWidgetDisplay_->getOptionsButton()->setText("");
  fullWidgetDisplay_->getOptionsButton()->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
  connect(fullWidgetDisplay_->getOptionsButton(), SIGNAL(clicked()), this, SLOT(replaceMe()));
}

void ModuleWidget::highlightPorts()
{
  ports_->setHighlightPorts(true);
  setPortSpacing(true);
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
  //qDebug() << "NEED TO UPDATE SPACING FOR " << sender();
  auto port = qobject_cast<PortWidget*>(sender());
  if (port)
  {
    if (port->isInput())
      setInputPortSpacing(highlighted);
    else
      setOutputPortSpacing(highlighted);
  }
}

void ModuleWidget::unhighlightPorts()
{
  ports_->setHighlightPorts(false);
  setPortSpacing(false);
  Q_EMIT displayChanged();
}

void ModuleWidget::updateMetadata(bool active)
{
  if (active)
  {
    auto metadata = theModule_->metadata().getFullMap();
    QStringList display;
    for (const auto& metaPair : metadata)
    {
      display.append(QString::fromStdString(metaPair.first) + " : " + QString::fromStdString(metaPair.second));
    }
    setToolTip("Metadata:\n" + display.join("\n"));
  }
  else
    setToolTip("");
}

void ModuleWidget::setExecutionDisabled(bool disabled)
{
  disabled_ = disabled;

  Q_EMIT executionDisabled(disabled_);

  theModule_->setExecutionDisabled(disabled_);
}

void ModuleWidget::incomingConnectionStateChanged(bool disabled)
{
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
