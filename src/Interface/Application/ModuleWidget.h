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


#ifndef INTERFACE_APPLICATION_MODULEWIDGET_H
#define INTERFACE_APPLICATION_MODULEWIDGET_H

#include <QStackedWidget>
#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>
#include <boost/bimap.hpp>
#include <set>
#include <deque>
#include <atomic>
#include <Interface/Application/Note.h>
#include <Interface/Application/HasNotes.h>
#include <Interface/Application/PositionProvider.h>

#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/ExecutableObject.h>
#endif

class QDockWidget;
class QProgressBar;
class QTimeLine;
class QGroupBox;
class PortBuilder;

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
  virtual void setupFrame(QStackedWidget* stacked) = 0; //TODO
  virtual void setupTitle(const QString& name) = 0;
  virtual void setupProgressBar() = 0;
  virtual void setupSpecial() = 0;
  virtual void setupButtons(bool hasUI, QObject* module) = 0;
  virtual void setupIcons() = 0;
  virtual QAbstractButton* getOptionsButton() const = 0;
  virtual QAbstractButton* getExecuteButton() const = 0;
  virtual QAbstractButton* getHelpButton() const = 0;
  virtual QAbstractButton* getLogButton() const = 0;
  virtual void setStatusColor(const QString& color) = 0;
  virtual QPushButton* getModuleActionButton() const = 0;

  virtual QProgressBar* getProgressBar() const = 0;

  virtual void setupSubnetWidgets() = 0;
  virtual QAbstractButton* getSubnetButton() const = 0;

  virtual int getTitleWidth() const = 0;
  virtual QLabel* getTitle() const = 0;

  virtual QGroupBox* getButtonGroup() const = 0;

  virtual void startExecuteMovie() = 0;
  virtual void stopExecuteMovie() = 0;

  //The following have platform-specific values
  static const int moduleWidthThreshold;
  static const int extraModuleWidth;
  static const int extraWidthThreshold;
  static const int smushFactor;
  static const int titleFontSize;
  static const int viewFontSize;
  static const int buttonPageFontSizeDiff;
  static const int widgetHeightAdjust;
  static const int widgetWidthAdjust;
};

typedef boost::shared_ptr<ModuleWidgetDisplayBase> ModuleWidgetDisplayPtr;

class ModuleWidget : public QStackedWidget,
  public Dataflow::Networks::ExecutableObject, public HasNotes
{
	Q_OBJECT

public:
  ModuleWidget(NetworkEditor* ed, const QString& name, SCIRun::Dataflow::Networks::ModuleHandle theModule,
    boost::shared_ptr<DialogErrorControl> dialogErrorControl,
    QWidget* parent = nullptr);
  ~ModuleWidget();

  void trackConnections();

  int numDynamicInputPortsForGuiUpdates() const;
  size_t numOutputPorts() const;

  const PortWidgetManager& ports() const { return *ports_; }
  PortWidgetManager& ports() { return *ports_; }
  QList<QGraphicsItem*> connections() const;

  std::string getModuleId() const { return moduleId_; }
  Dataflow::Networks::ModuleHandle getModule() const { return theModule_; }

  void setDeletedFromGui(bool b) { deletedFromGui_ = b; }

  void setColorSelected();
  void setColorUnselected();

  bool executionDisabled() const { return disabled_; }
  void setExecutionDisabled(bool disabled);

  void saveImagesFromViewScene();

  void highlightPorts();
  void unhighlightPorts();

  void printPortPositions() const;

  bool hasDynamicPorts() const;

  void createStartupNote();
  virtual void postLoadAction();

  bool guiVisible() const;

  static const int SMALL_PORT_SPACING = 3;
  static const int LARGE_PORT_SPACING = SMALL_PORT_SPACING * 2;
  int portSpacing() const;
  void setPortSpacing(bool highlighted);

  boost::signals2::connection connectExecuteBegins(const SCIRun::Dataflow::Networks::ExecuteBeginsSignalType::slot_type& subscriber) override final;
  boost::signals2::connection connectExecuteEnds(const SCIRun::Dataflow::Networks::ExecuteEndsSignalType::slot_type& subscriber) override final;
  boost::signals2::connection connectErrorListener(const SCIRun::Dataflow::Networks::ErrorSignalType::slot_type& subscriber) override final;

  void updateNoteFromFile(const Note& note);

  void adjustExecuteButtonToDownstream(bool downOnly);

  struct NetworkClearingScope
  {
    NetworkClearingScope();
    ~NetworkClearingScope();
  };

  QString metadataToString() const;
  QDialog* dialog();
  void collapsePinnedDialog();

  static double highResolutionExpandFactor_;
  static QString downstreamOnlyIcon;
  static QString allIcon;

  void setupPortSceneCollaborator(QGraphicsProxyWidget* proxy);

public Q_SLOTS:
  bool executeWithSignals() override;
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
  void insertNewModule(const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const QMap<QString, std::string>& info);
  void addDynamicPort(const SCIRun::Dataflow::Networks::ModuleId& mid, const SCIRun::Dataflow::Networks::PortId& pid);
  void removeDynamicPort(const SCIRun::Dataflow::Networks::ModuleId& mid, const SCIRun::Dataflow::Networks::PortId& pid);
  void pinUI();
  void hideUI();
  void showUI();
  void seeThroughUI();
  void normalOpacityUI();
  void updateMetadata(bool active);
  void updatePortSpacing(bool highlighted);
  void replaceMe();
Q_SIGNALS:
  void removeModule(const SCIRun::Dataflow::Networks::ModuleId& moduleId);
  void interrupt(const SCIRun::Dataflow::Networks::ModuleId& moduleId);
  void requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to);
  void connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription& desc);
  void connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId& id);
  void moduleExecuted();
  void executedManually(const SCIRun::Dataflow::Networks::ModuleHandle& module, bool fromButton);
  void updateProgressBarSignal(double percent);
  void cancelConnectionsInProgress();
  void noteUpdated(const Note& note);
  void duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle& module);
  void connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo, const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);
  void insertNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo,
    const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect,
    const QMap<QString, std::string>& info);
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
  void executeAgain(bool upstream);
  void executionDisabled(bool disabled);
  void findInNetwork();
  void showSubnetworkEditor(const QString& name);
  void showUIrequested(class ModuleDialogGeneric* dialog);
private Q_SLOTS:
  void subnetButtonClicked();
  void updateBackgroundColorForModuleState(int moduleState);
  void updateBackgroundColor(const QString& color);
  void executeButtonPushed();
  void executeTriggeredViaStateChange();
  void executeTriggeredProgrammatically(bool upstream);
  void stopButtonPushed();
  void colorOptionsButton(bool visible);
  void replaceModuleWith();
  void updateDialogForDynamicPortChange(const std::string& portName, bool adding);
  void handleDialogFatalError(const QString& message);
  void changeExecuteButtonToPlay();
  void changeExecuteButtonToStop();
  void updateDockWidgetProperties(bool isFloating);
  void incomingConnectionStateChanged(bool disabled, int index);
  void showReplaceWithWidget();
  void toggleProgrammableInputPort();
protected:
  virtual void enterEvent(QEvent* event) override;
  virtual void leaveEvent(QEvent* event) override;
  ModuleWidgetDisplayPtr fullWidgetDisplay_;
private:
  boost::shared_ptr<PortWidgetManager> ports_;
  boost::timer timer_;
  bool deletedFromGui_, colorLocked_;
  bool executedOnce_, skipExecuteDueToFatalError_, disabled_, programmablePortEnabled_{false};
  std::atomic<bool> errored_;
  int previousPageIndex_ {0};
  QDialog* replaceWithDialog_{ nullptr };

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
  QString name_;
  class ModuleDialogGeneric* dialog_;
  QDockWidget* dockable_;
  bool firstTimeShown_{ true };
  static QList<QPoint> positions_;
  void makeOptionsDialog();
  int buildDisplay(ModuleWidgetDisplayBase* display, const QString& name);
  void setupDisplayWidgets(ModuleWidgetDisplayBase* display, const QString& name);
  void setupModuleActions();
  void setupLogging(class ModuleErrorDisplayer* displayer);
  void adjustDockState(bool dockEnabled);
  Qt::DockWidgetArea allowedDockArea() const;
  void printInputPorts(const SCIRun::Dataflow::Networks::ModuleInfoProvider& moduleInfoProvider) const;
  void setInputPortSpacing(bool highlighted);
  void setOutputPortSpacing(bool highlighted);
  void fillReplaceWithMenu(QMenu* menu);

  class ModuleLogWindow* logWindow_;
  boost::scoped_ptr<class ModuleActionsMenu> actionsMenu_;

  static boost::shared_ptr<class ModuleDialogFactory> dialogFactory_;
	boost::shared_ptr<DialogErrorControl> dialogErrorControl_;

  void movePortWidgets(int oldIndex, int newIndex);
  void addPortLayouts(int index);
  void addInputPortsToLayout(int index);
  void addInputPortsToWidget(int index);
  void removeInputPortsFromWidget(int index);
  void addOutputPortsToLayout(int index);
  void addOutputPortsToWidget(int index);
  void removeOutputPortsFromWidget(int index);
  void updateProgrammablePorts();
  QHBoxLayout* inputPortLayout_;
  QHBoxLayout* outputPortLayout_;
  bool deleting_;
  static bool networkBeingCleared_;
  const QString defaultBackgroundColor_;
  bool isViewScene_; //TODO: lots of special logic around this case.

  typedef boost::bimap<QString, int> ColorStateLookup;
  typedef ColorStateLookup::value_type ColorStatePair;
  ColorStateLookup colorStateLookup_;
  void fillColorStateLookup(const QString& background);

  boost::shared_ptr<class ConnectionFactory> connectionFactory_;
  boost::shared_ptr<class ClosestPortFinder> closestPortFinder_;
  QString* currentExecuteIcon_ {nullptr};

  friend class ::PortBuilder;
};

class SubnetWidget : public ModuleWidget
{
	Q_OBJECT
public:
  SubnetWidget(NetworkEditor* ed, const QString& name, Dataflow::Networks::ModuleHandle theModule, boost::shared_ptr<DialogErrorControl> dialogErrorControl, QWidget* parent = nullptr);
  ~SubnetWidget();
  void postLoadAction() override;
  void deleteSubnetImmediately() { deleteSubnetImmediately_ = true; }
private:
  NetworkEditor* editor_;
  QString name_;
  bool deleteSubnetImmediately_{ false };
};

class SubnetPortsBridgeWidget : public QWidget
{
	Q_OBJECT
public:
  SubnetPortsBridgeWidget(NetworkEditor* ed, const QString& name, QWidget* parent = nullptr);
  void addPort(PortWidget* port) { ports_.push_back(port); }
  const std::vector<PortWidget*>& ports() const { return ports_; }
private:
  NetworkEditor* editor_;
  QString name_;
  std::vector<PortWidget*> ports_;
};

}
}

#endif
