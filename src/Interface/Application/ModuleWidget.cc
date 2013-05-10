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
#include <boost/range/join.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/PositionProvider.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/ModuleLogWindow.h>
#include <Interface/Application/NoteEditor.h>
#include <Interface/Application/ClosestPortFinder.h>
#include <Interface/Modules/Factory/ModuleDialogFactory.h>

//TODO: BAD, or will we have some sort of Application global anyway?
#include <Interface/Application/SCIRunMainWindow.h>

#include <Dataflow/Network/Module.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;

QPointF ProxyWidgetPosition::currentPosition() const
{
  return widget_->pos() + offset_;
}

namespace {
//TODO move to separate header
  QColor to_color(const std::string& str)
  {
    if (str == "red")
      return Qt::red;
    if (str == "blue")
      return Qt::blue;
    if (str == "darkBlue")
      return Qt::darkBlue;
    if (str == "cyan")
      return Qt::cyan;
    if (str == "darkCyan")
      return Qt::darkCyan;
    if (str == "darkGreen")
      return Qt::darkGreen;
    if (str == "cyan")
      return Qt::cyan;
    if (str == "magenta")
      return Qt::magenta;
    if (str == "white")
      return Qt::white;
    if (str == "yellow")
      return Qt::yellow;
    if (str == "darkYellow")
      return Qt::darkYellow;
    if (str == "lightGray")
      return Qt::lightGray;
    if (str == "darkGray")
      return Qt::darkGray;
    if (str == "black")
      return Qt::black;
    else
      return Qt::black;
  }

  QAction* separatorAction(QWidget* parent)
  {
    auto sep = new QAction(parent);
    sep->setSeparator(true);
    return sep;
  }

  QAction* disabled(QAction* action)
  {
    action->setEnabled(false);
    return action;
  }
}

namespace SCIRun {
namespace Gui {
  class ModuleActionsMenu
  {
  public:
    ModuleActionsMenu(ModuleWidget* parent, const std::string& moduleId)
    {
      menu_ = new QMenu("Actions", parent);

      //TODO:  hook up disabled actions
      menu_->addActions(QList<QAction*>()
        << disabled(new QAction("ID: " + QString::fromStdString(moduleId), parent))
        << separatorAction(parent)
        << disabled(new QAction("Execute", parent))
        << new QAction("Help", parent)
        << new QAction("Notes", parent)
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

namespace
{
#ifdef WIN32
  const int moduleWidthThreshold = 220;
  const int extraModuleWidth = 40;
#else
  const int moduleWidthThreshold = 240;
  const int extraModuleWidth = 30;
#endif
}

ModuleWidget::ModuleWidget(const QString& name, SCIRun::Dataflow::Networks::ModuleHandle theModule, QWidget* parent /* = 0 */)
  : QFrame(parent),
  deletedFromGui_(true),
  moduleId_(theModule->get_id()),
  theModule_(theModule),
  inputPortLayout_(0),
  outputPortLayout_(0)
{
  setupUi(this);
  titleLabel_->setText("<b><h3>" + name + "</h3></b>");

  //TODO: ultra ugly. no other place for this code right now.
  //TODO: to be handled in issue #212
  if (name == "ViewScene")
  {
    optionsButton_->setText("VIEW");
    optionsButton_->setToolTip("View renderer output");
    optionsButton_->resize(100, optionsButton_->height());
    //progressBar_->setVisible(false); //this looks bad, need to insert a spacer or something. TODO later
  }
  progressBar_->setMaximum(100);
  progressBar_->setMinimum(0);
  progressBar_->setValue(0);
  
  addPortLayouts();
  addPorts(*theModule);

  int pixelWidth = titleLabel_->fontMetrics().width(titleLabel_->text());
  int extraWidth = pixelWidth - moduleWidthThreshold;
  if (extraWidth > 0)
  {
    resize(width() + extraWidth + extraModuleWidth, height());
  }
   
  connect(optionsButton_, SIGNAL(clicked()), this, SLOT(showOptionsDialog()));
  makeOptionsDialog();

  connect(helpButton_, SIGNAL(clicked()), this, SLOT(launchDocumentation()));
  connect(this, SIGNAL(styleSheetUpdated(const QString&)), this, SLOT(updateStyleSheet(const QString&)));

  setupModuleActions();

  progressBar_->setTextVisible(false);
  
  logWindow_ = new ModuleLogWindow(QString::fromStdString(moduleId_), SCIRunMainWindow::Instance());
  connect(logButton2_, SIGNAL(clicked()), logWindow_, SLOT(show()));
  connect(logButton2_, SIGNAL(clicked()), logWindow_, SLOT(raise()));
  connect(actionsMenu_->getAction("Show Log"), SIGNAL(triggered()), logWindow_, SLOT(show()));
  connect(actionsMenu_->getAction("Show Log"), SIGNAL(triggered()), logWindow_, SLOT(raise()));
  connect(logWindow_, SIGNAL(messageReceived(const QColor&)), this, SLOT(setLogButtonColor(const QColor&)));
  connect(this, SIGNAL(updateProgressBarSignal(double)), this, SLOT(updateProgressBar(double)));
  connect(actionsMenu_->getAction("Help"), SIGNAL(triggered()), this, SLOT(launchDocumentation()));

  noteEditor_ = new NoteEditor(QString::fromStdString(moduleId_), SCIRunMainWindow::Instance());
  connect(actionsMenu_->getAction("Notes"), SIGNAL(triggered()), noteEditor_, SLOT(show()));
  connect(actionsMenu_->getAction("Notes"), SIGNAL(triggered()), noteEditor_, SLOT(raise()));
  connect(noteEditor_, SIGNAL(noteChanged(const Note&)), this, SLOT(updateNote(const Note&)));

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
    QString("* { background-color: rgb(%1,%2,%3) }")
    .arg(color.red())
    .arg(color.green())
    .arg(color.blue()));
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
  verticalLayout->setContentsMargins(5,0,5,0);
}

void ModuleWidget::addPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider)
{
  const ModuleId moduleId = moduleInfoProvider.get_id();
  for (size_t i = 0; i < moduleInfoProvider.num_input_ports(); ++i)
  {
    InputPortHandle port = moduleInfoProvider.get_input_port(i);
    InputPortWidget* w = new InputPortWidget(QString::fromStdString(port->get_portname()), to_color(port->get_colorname()), moduleId, i, connectionFactory_, closestPortFinder_, this);
    hookUpSignals(w);
    connect(this, SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)), w, SLOT(MakeTheConnection(const SCIRun::Dataflow::Networks::ConnectionDescription&)));
    addPort(w);
  }
  for (size_t i = 0; i < moduleInfoProvider.num_output_ports(); ++i)
  {
    OutputPortHandle port = moduleInfoProvider.get_output_port(i);
    OutputPortWidget* w = new OutputPortWidget(QString::fromStdString(port->get_portname()), to_color(port->get_colorname()), moduleId, i, connectionFactory_, closestPortFinder_, this);
    hookUpSignals(w);
    addPort(w);
  }
  optionsButton_->setVisible(moduleInfoProvider.has_ui());
}

void ModuleWidget::hookUpSignals(PortWidget* port) const
{
  connect(port, SIGNAL(requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const SCIRun::Dataflow::Networks::PortDescriptionInterface*)), 
    this, SIGNAL(requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const SCIRun::Dataflow::Networks::PortDescriptionInterface*)));
  connect(port, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)), 
    this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)));
  connect(this, SIGNAL(cancelConnectionsInProgress()), port, SLOT(cancelConnectionsInProgress()));
}

void ModuleWidget::addPort(OutputPortWidget* port)
{
  if (!outputPortLayout_)
  {
    //TODO--extract method
    outputPortLayout_ = new QHBoxLayout;
    outputPortLayout_->setSpacing(3);
    outputPortLayout_->setAlignment(Qt::AlignLeft);
    verticalLayout->insertLayout(-1, outputPortLayout_);
  }
  outputPortLayout_->addWidget(port);
  outputPorts_.push_back(port);
}

void ModuleWidget::addPort(InputPortWidget* port)
{
  if (!inputPortLayout_)
  {
    inputPortLayout_ = new QHBoxLayout;
    inputPortLayout_->setSpacing(3);
    inputPortLayout_->setAlignment(Qt::AlignLeft);
    verticalLayout->insertLayout(0, inputPortLayout_);
  }
  inputPortLayout_->addWidget(port);
  inputPorts_.push_back(port);
}

ModuleWidget::~ModuleWidget()
{
  Q_FOREACH (PortWidget* p, boost::join(inputPorts_, outputPorts_))
    p->deleteConnections();
  GuiLogger::Instance().log("Module deleted.");
  dialog_.reset();
  theModule_->setLogger(LoggerHandle());
  delete logWindow_;
  delete noteEditor_;

  if (deletedFromGui_)
    Q_EMIT removeModule(ModuleId(moduleId_));
}

void ModuleWidget::trackConnections()
{
  Q_FOREACH (PortWidget* p, boost::join(inputPorts_, outputPorts_))
    p->trackConnections();
}

QPointF ModuleWidget::inputPortPosition() const 
{
  if (positionProvider_)
    return positionProvider_->currentPosition() + QPointF(20,10);
  return pos();
}

QPointF ModuleWidget::outputPortPosition() const 
{
  if (positionProvider_)
    return positionProvider_->currentPosition() + QPointF(20, height() - 10);
  return pos();
}

void ModuleWidget::execute()
{
  {
    auto sheet = styleSheet();
    styleSheetUpdated("background-color: #99FFCC;");
    timer_.restart();
    theModule_->do_execute();
    Q_EMIT updateProgressBarSignal(1);
    Q_EMIT styleSheetUpdated(sheet);
  }
  Q_EMIT moduleExecuted();
}

void ModuleWidget::updateStyleSheet(const QString& sheet)
{
  setStyleSheet(sheet);
}

boost::shared_ptr<ModuleDialogFactory> ModuleWidget::dialogFactory_;

void ModuleWidget::makeOptionsDialog()
{
  if (!dialog_)
  {
    if (!dialogFactory_)
      dialogFactory_.reset(new ModuleDialogFactory(SCIRunMainWindow::Instance()));

    dialog_.reset(dialogFactory_->makeDialog(moduleId_, theModule_->get_state()));
    dialog_->pull();
    connect(dialog_.get(), SIGNAL(executeButtonPressed()), this, SLOT(execute()));
    connect(this, SIGNAL(moduleExecuted()), dialog_.get(), SLOT(moduleExecuted()));
  }
}

boost::shared_ptr<ConnectionFactory> ModuleWidget::connectionFactory_;
boost::shared_ptr<ClosestPortFinder> ModuleWidget::closestPortFinder_;

void ModuleWidget::showOptionsDialog()
{
  makeOptionsDialog();
  dialog_->show();
  dialog_->raise();
  dialog_->activateWindow();
}

void ModuleWidget::updateProgressBar(double percent)
{
  progressBar_->setValue(percent * progressBar_->maximum());
  progressBar_->setTextVisible(true);
  updateModuleTime();
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
  currentNote_ = note;
  noteUpdated(note);
}

void ModuleWidget::duplicate()
{
  Q_EMIT duplicateModule(theModule_);
}