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

#include <iostream>
#include <QtGui>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <Core/Logging/Log.h>

#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/PositionProvider.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/ModuleLogWindow.h>
#include <Interface/Application/NoteEditor.h>
#include <Interface/Application/ClosestPortFinder.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Modules/Factory/ModuleDialogFactory.h>
#include <Interface/Application/PortWidgetManager.h>

//TODO: BAD, or will we have some sort of Application global anyway?
#include <Interface/Application/SCIRunMainWindow.h>

#include <Dataflow/Network/Module.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;

ProxyWidgetPosition::ProxyWidgetPosition(QGraphicsProxyWidget* widget, const QPointF& offset/* = QPointF()*/) : widget_(widget), offset_(offset)
{
}

QPointF ProxyWidgetPosition::currentPosition() const
{
  return widget_->pos() + offset_;
}

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
        << disabled(new QAction("Replace With->(TODO)", parent))
        << new QAction("Show Log", parent)
        << disabled(new QAction("Make Sub-Network", parent))
        << separatorAction(parent)
        << disabled(new QAction("Destroy", parent)));
    }
    QMenu* getMenu() { return menu_; }
    QAction* getAction(const char* name) const
    {
      BOOST_FOREACH(QAction* action, menu_->actions())
      {
        if (action->text().contains(name))
          return action;
      }
      return 0;
    }
  private:
    QMenu* menu_;
  };
}}

QColor SCIRun::Gui::to_color(const std::string& str, int alpha)
{
  QColor result;
  if (SCIRunMainWindow::Instance()->newInterface())
  {
    if (str == "red")
      result = Qt::red;
    else if (str == "blue")
      result = QColor(14,139,255);
    else if (str == "darkBlue")
      result = Qt::darkBlue;
    else if (str == "cyan")
      result = QColor(27,207,207);
    else if (str == "darkCyan")
      result = Qt::darkCyan;
    else if (str == "darkGreen")
      result = QColor(0,175,70);
    else if (str == "cyan")
      result = Qt::cyan;
    else if (str == "magenta")
      result = QColor(255,75,240);
    else if (str == "white")
      result = Qt::white;
    else if (str == "yellow")
      result = QColor(234,255,55);
    else if (str == "darkYellow")
      result = Qt::darkYellow;
    else if (str == "lightGray")
      result = Qt::lightGray;
    else if (str == "darkGray")
      result = Qt::darkGray;
    else if (str == "black")
      result = Qt::black;
    else if (str == "purple")
      result = QColor(122,119,226);
    else if (str == "orange")
      result = QColor(254, 139, 38);
    else
      result = Qt::black;
  }
  else
  {
    if (str == "red")
      result = Qt::red;
    else if (str == "blue")
      result = Qt::blue;
    else if (str == "darkBlue")
      result = Qt::darkBlue;
    else if (str == "cyan")
      result = Qt::cyan;
    else if (str == "darkCyan")
      result = Qt::darkCyan;
    else if (str == "darkGreen")
      result = Qt::darkGreen;
    else if (str == "cyan")
      result = Qt::cyan;
    else if (str == "magenta")
      result = Qt::magenta;
    else if (str == "white")
      result = Qt::white;
    else if (str == "yellow")
      result = Qt::yellow;
    else if (str == "darkYellow")
      result = Qt::darkYellow;
    else if (str == "lightGray")
      result = Qt::lightGray;
    else if (str == "darkGray")
      result = Qt::darkGray;
    else if (str == "black")
      result = Qt::black;
    else if (str == "purple")
      result = Qt::darkMagenta;
    else if (str == "orange")
      result = QColor(255, 165, 0);
    else
      result = Qt::black;
  }
  result.setAlpha(alpha);
  return result;
}

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

namespace
{
#ifdef WIN32
  const int moduleWidthThreshold = 110;
  const int extraModuleWidth = 5;
  const int extraWidthThreshold = 5;
  const int smushFactor = 15;
  const int titleFontSize = 8;
  const int widgetHeightAdjust = 0;
#else
  const int moduleWidthThreshold = 80;
  const int extraModuleWidth = 5;
  const int extraWidthThreshold = 5;
  const int smushFactor = 15;
  const int titleFontSize = 12;
  const int widgetHeightAdjust = 10;
#endif
}

ModuleWidget::ModuleWidget(NetworkEditor* ed, const QString& name, SCIRun::Dataflow::Networks::ModuleHandle theModule, boost::shared_ptr<SCIRun::Gui::DialogErrorControl> dialogErrorControl,
  QWidget* parent /* = 0 */)
  : QFrame(parent), HasNotes(theModule->get_id(), true),
  ports_(new PortWidgetManager),
  deletedFromGui_(true),
  colorLocked_(false),
  theModule_(theModule),
  moduleId_(theModule->get_id()),
  dialog_(0),
  dockable_(0),
  dialogErrorControl_(dialogErrorControl),
  inputPortLayout_(0),
  outputPortLayout_(0),
  editor_(ed),
  deleting_(false),
  defaultBackgroundColor_(SCIRunMainWindow::Instance()->newInterface() ? moduleRGBA(99,99,104) : moduleRGBA(192,192,192))
{
  setupUi(this);

  titleLabel_->setFont(QFont("Helvetica", titleFontSize, QFont::Bold));
  titleLabel_->setText(name);

  //TODO: ultra ugly. no other place for this code right now.
  //TODO: to be handled in issue #212
  if (name == "ViewScene")
  {
    optionsButton_->setText("VIEW");
    optionsButton_->setToolTip("View renderer output");
    optionsButton_->resize(100, optionsButton_->height());
    executePushButton_->hide();
    progressBar_->setVisible(false);
  }
  progressBar_->setMaximum(100);
  progressBar_->setMinimum(0);
  progressBar_->setValue(0);

  addPortLayouts();
  addPorts(*theModule_);
  optionsButton_->setVisible(theModule_->has_ui());

  executePushButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
  connect(executePushButton_, SIGNAL(clicked()), this, SLOT(executeButtonPushed()));

  int pixelWidth = titleLabel_->fontMetrics().boundingRect(titleLabel_->text()).width();
  //std::cout << titleLabel_->text().toStdString() << std::endl;
  //std::cout << "\tPixelwidth = " << pixelWidth << std::endl;
  int extraWidth = pixelWidth - moduleWidthThreshold;
  //std::cout << "\textraWidth = " << extraWidth << std::endl;
  if (extraWidth > extraWidthThreshold)
  {
    //std::cout << "\tGROWING MODULE Current width: " << width() << std::endl;
    resize(width() + extraWidth + extraModuleWidth, height());
    //std::cout << "\tNew width: " << width() << std::endl;
  }
  else
  {
    //std::cout << "\tSHRINKING MODULE Current width: " << width() << std::endl;
    resize(width() - smushFactor, height());
    //std::cout << "\tNew width: " << width() << std::endl;
  }
  resize(width(), height() + widgetHeightAdjust);

  connect(optionsButton_, SIGNAL(clicked()), this, SLOT(toggleOptionsDialog()));
  makeOptionsDialog();

  connect(helpButton_, SIGNAL(clicked()), this, SLOT(launchDocumentation()));
  connect(this, SIGNAL(backgroundColorUpdated(const QString&)), this, SLOT(updateBackgroundColor(const QString&)));
  theModule_->connectExecutionStateChanged(boost::bind(&ModuleWidget::moduleStateUpdated, this, _1));
  connect(this, SIGNAL(moduleStateUpdated(int)), this, SLOT(updateBackgroundColorForModuleState(int)));

  setupModuleActions();

  progressBar_->setTextVisible(false);

  logWindow_ = new ModuleLogWindow(QString::fromStdString(moduleId_), dialogErrorControl_, SCIRunMainWindow::Instance());
  connect(logButton2_, SIGNAL(clicked()), logWindow_, SLOT(show()));
  connect(logButton2_, SIGNAL(clicked()), logWindow_, SLOT(raise()));
  connect(actionsMenu_->getAction("Show Log"), SIGNAL(triggered()), logWindow_, SLOT(show()));
  connect(actionsMenu_->getAction("Show Log"), SIGNAL(triggered()), logWindow_, SLOT(raise()));
  connect(actionsMenu_->getAction("Execute"), SIGNAL(triggered()), this, SLOT(executeButtonPushed()));
  connect(logWindow_, SIGNAL(messageReceived(const QColor&)), this, SLOT(setLogButtonColor(const QColor&)));
  connect(this, SIGNAL(updateProgressBarSignal(double)), this, SLOT(updateProgressBar(double)));
  connect(actionsMenu_->getAction("Help"), SIGNAL(triggered()), this, SLOT(launchDocumentation()));

  connectNoteEditorToAction(actionsMenu_->getAction("Notes"));
  connectUpdateNote(this);

  connect(actionsMenu_->getAction("Duplicate"), SIGNAL(triggered()), this, SLOT(duplicate()));

  //TODO: doh, how do i destroy myself?
  //connect(actionsMenu_->getAction("Destroy"), SIGNAL(triggered()), this, SIGNAL(removeModule(const std::string&)));

  LoggerHandle logger(boost::make_shared<ModuleLogger>(logWindow_));
  theModule_->setLogger(logger);
  theModule_->setUpdaterFunc(boost::bind(&ModuleWidget::updateProgressBarSignal, this, _1));
  if (theModule_->has_ui())
    theModule_->setUiToggleFunc([&](bool b){ dialog_->setVisible(b); });
}

void ModuleWidget::setLogButtonColor(const QColor& color)
{
  logButton2_->setStyleSheet(
    QString("* { background-color: %1 }")
    .arg(moduleRGBA(color.red(), color.green(), color.blue())));
}

void ModuleWidget::resetLogButtonColor()
{
  logButton2_->setStyleSheet("");
}

void ModuleWidget::resetProgressBar()
{
  progressBar_->setValue(0);
  progressBar_->setTextVisible(false);
}

void ModuleWidget::setupModuleActions()
{
  actionsMenu_.reset(new ModuleActionsMenu(this, moduleId_));

  moduleActionButton_->setMenu(actionsMenu_->getMenu());
}

void ModuleWidget::addPortLayouts()
{
  layout()->setContentsMargins(5,0,5,0);
}

void ModuleWidget::addPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider)
{
  addInputPorts(moduleInfoProvider);
  addOutputPorts(moduleInfoProvider);
}

void ModuleWidget::addInputPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider)
{
  const ModuleId moduleId = moduleInfoProvider.get_id();
  size_t i = 0;
  BOOST_FOREACH(InputPortHandle port, moduleInfoProvider.inputPorts())
  {
    auto type = port->get_typename();
    //std::cout << "ADDING PORT: " << port->id() << "[" << port->isDynamic() << "] AT INDEX: " << i << std::endl;
    InputPortWidget* w = new InputPortWidget(QString::fromStdString(port->get_portname()), to_color(PortColorLookup::toColor(type), portAlpha()), type, moduleId, port->id(), i, port->isDynamic(), connectionFactory_, closestPortFinder_, this);
    hookUpGeneralPortSignals(w);
    connect(this, SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)), w, SLOT(MakeTheConnection(const SCIRun::Dataflow::Networks::ConnectionDescription&)));
    ports_->addPort(w);
    ++i;
  }
  addInputPortsToLayout();
}

void ModuleWidget::printInputPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider)
{
  const ModuleId moduleId = moduleInfoProvider.get_id();
  std::cout << "Module input ports: " << moduleId << std::endl;
  size_t i = 0;
  BOOST_FOREACH(InputPortHandle port, moduleInfoProvider.inputPorts())
  {
    auto type = port->get_typename();
    std::cout << "\t" << i << " : " << port->get_portname() << " : " << type << " dyn = " << port->isDynamic() << std::endl;
     ++i;
  }
}

void ModuleWidget::addOutputPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider)
{
  const ModuleId moduleId = moduleInfoProvider.get_id();
  size_t i = 0;
  BOOST_FOREACH(OutputPortHandle port, moduleInfoProvider.outputPorts())
  {
    auto type = port->get_typename();
    OutputPortWidget* w = new OutputPortWidget(QString::fromStdString(port->get_portname()), to_color(PortColorLookup::toColor(type), portAlpha()), type, moduleId, port->id(), i, port->isDynamic(), connectionFactory_, closestPortFinder_, this);
    hookUpGeneralPortSignals(w);
    ports_->addPort(w);
    ++i;
  }
  addOutputPortsToLayout();
}

void ModuleWidget::hookUpGeneralPortSignals(PortWidget* port) const
{
  connect(port, SIGNAL(requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const SCIRun::Dataflow::Networks::PortDescriptionInterface*)),
    this, SIGNAL(requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const SCIRun::Dataflow::Networks::PortDescriptionInterface*)));
  connect(port, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)),
    this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)));
  connect(this, SIGNAL(cancelConnectionsInProgress()), port, SLOT(cancelConnectionsInProgress()));
  connect(port, SIGNAL(connectNewModule(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const std::string&)),
    this, SLOT(connectNewModule(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const std::string&)));
  connect(port, SIGNAL(connectionNoteChanged()), this, SIGNAL(noteChanged()));
}

void ModuleWidget::addOutputPortsToLayout()
{
  if (!outputPortLayout_)
  {
    //TODO--extract method
    outputPortLayout_ = new QHBoxLayout;
    outputPortLayout_->setSpacing(PORT_SPACING);
    outputPortLayout_->setAlignment(Qt::AlignLeft);
    qobject_cast<QVBoxLayout*>(layout())->insertLayout(-1, outputPortLayout_, 1);
  }
  ports_->addOutputsToLayout(outputPortLayout_);
}

void PortWidgetManager::addInputsToLayout(QHBoxLayout* layout)
{
  if (inputPorts_.empty())
    layout->addWidget(new BlankPort(layout->parentWidget()));

  BOOST_FOREACH(PortWidget* port, inputPorts_)
    layout->addWidget(port);
}

void PortWidgetManager::addOutputsToLayout(QHBoxLayout* layout)
{
  if (outputPorts_.empty())
    layout->addWidget(new BlankPort(layout->parentWidget()));

  BOOST_FOREACH(PortWidget* port, outputPorts_)
    layout->addWidget(port);
}

void ModuleWidget::addInputPortsToLayout()
{
  if (!inputPortLayout_)
  {
    inputPortLayout_ = new QHBoxLayout;
    inputPortLayout_->setSpacing(PORT_SPACING);
    inputPortLayout_->setAlignment(Qt::AlignLeft);
    qobject_cast<QVBoxLayout*>(layout())->insertLayout(0, inputPortLayout_, 1);
  }
  ports_->addInputsToLayout(inputPortLayout_);
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

void ModuleWidget::addDynamicPort(const ModuleId& mid, const PortId& pid)
{
  if (mid.id_ == moduleId_)
  {
    InputPortHandle port = theModule_->getInputPort(pid);
    auto type = port->get_typename();

    InputPortWidget* w = new InputPortWidget(QString::fromStdString(port->get_portname()), to_color(PortColorLookup::toColor(type)), type, mid, port->id(), port->getIndex(), port->isDynamic(), connectionFactory_, closestPortFinder_, this);
    hookUpGeneralPortSignals(w);
    connect(this, SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)), w, SLOT(MakeTheConnection(const SCIRun::Dataflow::Networks::ConnectionDescription&)));
    ports_->addPort(w);
    inputPortLayout_->addWidget(w);
    Q_EMIT dynamicPortChanged();
  }
}

void ModuleWidget::removeDynamicPort(const ModuleId& mid, const PortId& pid)
{
  if (mid.id_ == moduleId_ && !deleting_)
  {
    if (ports_->removeDynamicPort(pid, inputPortLayout_))
    {
      Q_EMIT dynamicPortChanged();
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

ModuleWidget::~ModuleWidget()
{
  //TODO: would rather disconnect THIS from removeDynamicPort signaller in DynamicPortManager; need a method on NetworkEditor or something.
  //disconnect()
  deleting_ = true;
  Q_FOREACH (PortWidget* p, ports_->getAllPorts())
    p->deleteConnections();

  //GuiLogger::Instance().log("Module deleted.");

  theModule_->setLogger(LoggerHandle());


  if (deletedFromGui_)
  {
    if (dialog_)
    {
      dialog_->close();
    }

    if (dockable_)
    {
      SCIRunMainWindow::Instance()->removeDockWidget(dockable_);
      delete dockable_;
    }

    delete logWindow_;
    logWindow_ = 0;

    Q_EMIT removeModule(ModuleId(moduleId_));
  }
}

void ModuleWidget::trackConnections()
{
  Q_FOREACH (PortWidget* p, ports_->getAllPorts())
    p->trackConnections();
}

void ModuleWidget::execute()
{
  {
    //colorLocked_ = true; //TODO
    timer_.restart();
    theModule_->do_execute();
    Q_EMIT updateProgressBarSignal(1);
    //colorLocked_ = false;
  }
  Q_EMIT moduleExecuted();
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

void ModuleWidget::updateBackgroundColorForModuleState(int moduleState)
{
  switch (moduleState)
  {
  case (int)ModuleInterface::Waiting:
    Q_EMIT backgroundColorUpdated(moduleRGBA(205,190,112));
    break;
  case (int)ModuleInterface::Executing:
    Q_EMIT backgroundColorUpdated(moduleRGBA(170,204,170));
    break;
  case (int)ModuleInterface::Completed:
    Q_EMIT backgroundColorUpdated(defaultBackgroundColor_);
    break;
  }
}

void ModuleWidget::updateBackgroundColor(const QString& color)
{
  if (!colorLocked_)
  {
    QString rounded;
    if (SCIRunMainWindow::Instance()->newInterface())
      rounded = "color: white; border-radius: 7px;";
    setStyleSheet(rounded + " background-color: " + color);
  }
}

void ModuleWidget::setColorSelected()
{
  updateBackgroundColor(moduleRGBA(0,255,255));
}

void ModuleWidget::setColorUnselected()
{
  updateBackgroundColor(defaultBackgroundColor_);
}

boost::shared_ptr<ModuleDialogFactory> ModuleWidget::dialogFactory_;

void ModuleWidget::makeOptionsDialog()
{
  if (theModule_->has_ui())
  {
    if (!dialog_)
    {
      if (!dialogFactory_)
        dialogFactory_.reset(new ModuleDialogFactory(0));

      dialog_ = dialogFactory_->makeDialog(moduleId_, theModule_->get_state());
      dialog_->pull();
      connect(dialog_, SIGNAL(executeActionTriggered()), this, SLOT(executeButtonPushed()));
      connect(this, SIGNAL(moduleExecuted()), dialog_, SLOT(moduleExecuted()));
      dockable_ = new QDockWidget(QString::fromStdString(moduleId_), 0);
      dockable_->setObjectName(dialog_->windowTitle());
      dockable_->setWidget(dialog_);
      dockable_->setMinimumSize(dialog_->minimumSize());
      dockable_->setAllowedAreas(Qt::RightDockWidgetArea);
      dockable_->setAutoFillBackground(true);
      SCIRunMainWindow::Instance()->addDockWidget(Qt::RightDockWidgetArea, dockable_);
      dockable_->hide();
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
      //TODO--more special viewscene code...
      if (dialog_->windowTitle().startsWith("ViewScene"))
      {
        dockable_->setFloating(true);
      }
    }
    else
      hideUI();
  }
}

void ModuleWidget::updateProgressBar(double percent)
{
  progressBar_->setValue(percent * progressBar_->maximum());

  //TODO: make this configurable
  //progressBar_->setTextVisible(true);
  updateModuleTime();
  progressBar_->setToolTip(progressBar_->text());
}

void ModuleWidget::updateModuleTime()
{
  progressBar_->setFormat(QString("%1 s : %p%").arg(timer_.elapsed()));
}

void ModuleWidget::launchDocumentation()
{
  //TODO: push this help url construction to module layer
  std::string url = "http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php/CIBC:Documentation:SCIRun:Reference:SCIRun:" + getModule()->get_module_name();

  QUrl qurl(QString::fromStdString(url), QUrl::TolerantMode);

  if (!QDesktopServices::openUrl(qurl))
    GuiLogger::Instance().log("Failed to open help page: " + qurl.toString());
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

void ModuleWidget::connectNewModule(const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName)
{
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
}
