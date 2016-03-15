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

#ifndef INTERFACE_APPLICATION_MODULEWIDGET_H
#define INTERFACE_APPLICATION_MODULEWIDGET_H

#include <QStackedWidget>
#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>
#include <set>
#include <deque>
#include <atomic>
#include <Interface/Application/Note.h>
#include <Interface/Application/HasNotes.h>
#include <Interface/Application/PositionProvider.h>

#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/ExecutableObject.h>
#endif

class QGraphicsProxyWidget;
class QDockWidget;
class QProgressBar;

namespace SCIRun {
namespace Gui {

class PortWidget;
class InputPortWidget;
class OutputPortWidget;
class PositionProvider;
class NetworkEditor;
class PortWidgetManager;
class DialogErrorControl;

class ModuleWidgetDisplayBase
{
public:
  virtual ~ModuleWidgetDisplayBase() {}
  virtual void setupFrame(QFrame* frame) = 0;
  virtual void setupTitle(const QString& name) = 0;
  virtual void setupProgressBar() = 0;
  virtual void setupSpecial() = 0;
  virtual void setupButtons(bool hasUI, QObject* module) = 0;
  virtual void setupIcons() = 0;
  virtual QAbstractButton* getOptionsButton() const = 0;
  virtual QAbstractButton* getExecuteButton() const = 0;
  virtual QAbstractButton* getHelpButton() const = 0;
  virtual QAbstractButton* getLogButton() const = 0;
  virtual QPushButton* getModuleActionButton() const = 0;

  virtual QProgressBar* getProgressBar() const = 0;

  virtual int getTitleWidth() const = 0;

  virtual void adjustLayout(QLayout* layout) = 0;
};

typedef boost::shared_ptr<ModuleWidgetDisplayBase> ModuleWidgetDisplayPtr;

class ModuleWidget : public QStackedWidget,
  public Dataflow::Networks::ExecutableObject, public HasNotes
{
	Q_OBJECT

public:
  ModuleWidget(NetworkEditor* ed, const QString& name, SCIRun::Dataflow::Networks::ModuleHandle theModule, boost::shared_ptr<DialogErrorControl> dialogErrorControl,
    QWidget* parent = nullptr);
  ~ModuleWidget();

  void trackConnections();

  size_t numInputPorts() const;
  size_t numOutputPorts() const;

  const PortWidgetManager& ports() const { return *ports_; }

  std::string getModuleId() const { return moduleId_; }
  Dataflow::Networks::ModuleHandle getModule() const { return theModule_; }

  void setDeletedFromGui(bool b) { deletedFromGui_ = b; }

  //TODO: initialize in a new class
  static boost::shared_ptr<class ConnectionFactory> connectionFactory_;
  static boost::shared_ptr<class ClosestPortFinder> closestPortFinder_;

  void setColorSelected();
  void setColorUnselected();

  void highlightPorts();
  void unhighlightPorts();

  void printPortPositions() const;

  bool hasDynamicPorts() const;

  void createStartupNote();
  void postLoadAction();

  bool guiVisible() const;

  static const int SMALL_PORT_SPACING = 3;
  static const int LARGE_PORT_SPACING = SMALL_PORT_SPACING * 2;
  int portSpacing() const;
  void setPortSpacing(bool highlighted);

  virtual boost::signals2::connection connectExecuteBegins(const SCIRun::Dataflow::Networks::ExecuteBeginsSignalType::slot_type& subscriber) override;
  virtual boost::signals2::connection connectExecuteEnds(const SCIRun::Dataflow::Networks::ExecuteEndsSignalType::slot_type& subscriber) override;
  virtual boost::signals2::connection connectErrorListener(const SCIRun::Dataflow::Networks::ErrorSignalType::slot_type& subscriber) override;

  void updateNoteFromFile(const Note& note);

public Q_SLOTS:
  virtual void execute() override;
  void toggleOptionsDialog();
  void setLogButtonColor(const QColor& color);
  void resetLogButtonColor();
  void resetProgressBar();
  void updateProgressBar(double percent);
  void updateModuleTime();
  void launchDocumentation();
  void setStartupNote(const QString& text);
  void updateNote(const Note& note);
  void duplicate();
  void connectNewModule(const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);
  void addDynamicPort(const SCIRun::Dataflow::Networks::ModuleId& mid, const SCIRun::Dataflow::Networks::PortId& pid);
  void removeDynamicPort(const SCIRun::Dataflow::Networks::ModuleId& mid, const SCIRun::Dataflow::Networks::PortId& pid);
  void pinUI();
  void hideUI();
  void showUI();
  void setMiniMode(bool mini);
  void collapseToMiniMode();
  void expandToFullMode();
  void updateMetadata(bool active);
  void updatePortSpacing(bool highlighted);
  void replaceMe();
  static void setGlobalMiniMode(bool mini);
Q_SIGNALS:
  void removeModule(const SCIRun::Dataflow::Networks::ModuleId& moduleId);
  void interrupt(const SCIRun::Dataflow::Networks::ModuleId& moduleId);
  void requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to);
  void connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription& desc);
  void connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId& id);
  void moduleExecuted();
  void executedManually(const SCIRun::Dataflow::Networks::ModuleHandle& module);
  void updateProgressBarSignal(double percent);
  void cancelConnectionsInProgress();
  void noteUpdated(const Note& note);
  void duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle& module);
  void connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo, const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);
  void replaceModuleWith(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToReplace, const std::string& newModuleName);
  void backgroundColorUpdated(const QString& color);
  void dynamicPortChanged(const std::string& portID, bool adding);
  void noteChanged();
  void moduleStateUpdated(int state);
  void moduleSelected(bool selected);
  void displayChanged();
  void requestModuleVisible();
  void deleteMeLater();
  void executeEnds();
  void signalExecuteButtonIconChangeToStop();
  void disableWidgetDisabling();
  void reenableWidgetDisabling();
  void executeAgain();
private Q_SLOTS:
  void updateBackgroundColorForModuleState(int moduleState);
  void updateBackgroundColor(const QString& color);
  void executeButtonPushed();
  void stopButtonPushed();
  void colorOptionsButton(bool visible);
  void fillReplaceWithMenu();
  void replaceModuleWith();
  void updateDialogForDynamicPortChange(const std::string& portName, bool adding);
  void handleDialogFatalError(const QString& message);
  void changeExecuteButtonToPlay();
  void changeExecuteButtonToStop();
  void updateDockWidgetProperties(bool isFloating);
private:
  ModuleWidgetDisplayBase* currentDisplay_;
  ModuleWidgetDisplayPtr fullWidgetDisplay_;
  ModuleWidgetDisplayPtr miniWidgetDisplay_;
  boost::shared_ptr<PortWidgetManager> ports_;
  boost::timer timer_;
  bool deletedFromGui_, colorLocked_;
  bool isMini_, errored_, executedOnce_, skipExecute_;

  SCIRun::Dataflow::Networks::ModuleHandle theModule_;
  std::atomic<int> previousModuleState_;

  void addPorts(int index);
  void createPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider);
  void createInputPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider);
  void createOutputPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider);
  void hookUpGeneralPortSignals(PortWidget* port) const;
  void setupDisplayConnections(ModuleWidgetDisplayBase* display);
  void resizeBasedOnModuleName(ModuleWidgetDisplayBase* display, int index);
  std::string moduleId_;
  class ModuleDialogGeneric* dialog_;
  QDockWidget* dockable_;
  void makeOptionsDialog();
  int buildDisplay(ModuleWidgetDisplayBase* display, const QString& name);
  void setupDisplayWidgets(ModuleWidgetDisplayBase* display, const QString& name);
  void setupModuleActions();
  void setupLogging();
  void adjustDockState(bool dockEnabled);
  Qt::DockWidgetArea allowedDockArea() const;
  void printInputPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider);
  QMenu* getReplaceWithMenu();
  void setInputPortSpacing(bool highlighted);
  void setOutputPortSpacing(bool highlighted);

  class ModuleLogWindow* logWindow_;
  boost::scoped_ptr<class ModuleActionsMenu> actionsMenu_;

  static boost::shared_ptr<class ModuleDialogFactory> dialogFactory_;
	boost::shared_ptr<DialogErrorControl> dialogErrorControl_;

  void changeDisplay(int oldIndex, int newIndex);
  void addPortLayouts(int index);
  void addInputPortsToLayout(int index);
  void addInputPortsToWidget(int index);
  void removeInputPortsFromWidget(int index);
  void addOutputPortsToLayout(int index);
  void addOutputPortsToWidget(int index);
  void removeOutputPortsFromWidget(int index);
  QHBoxLayout* inputPortLayout_;
  QHBoxLayout* outputPortLayout_;
  NetworkEditor* editor_;
  bool deleting_;
  const QString defaultBackgroundColor_;
  int fullIndex_, miniIndex_;
  bool isViewScene_; //TODO: lots of special logic around this case.

  static bool globalMiniMode_;
};

}
}

#endif
