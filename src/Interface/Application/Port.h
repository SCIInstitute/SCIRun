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


#ifndef INTERFACE_APPLICATION_PORTWIDGET_H
#define INTERFACE_APPLICATION_PORTWIDGET_H

#include <QGraphicsWidget>
#include <QPushButton>
#include <QColor>
#ifndef Q_MOC_RUN
#include <Core/Utils/SmartPointers.h>
#include <set>
#include <vector>
#include <Interface/Application/PositionProvider.h>
#include <Dataflow/Network/PortInterface.h>
#include <Dataflow/Network/ConnectionId.h>
#endif

class QGraphicsScene;

namespace SCIRun {
namespace Gui {

class ConnectionLine;
class PositionProvider;
class ConnectionInProgress;
class ConnectionFactory;
class ClosestPortFinder;
class PortActionsMenu;
using SceneFunc = std::function<QGraphicsScene*()>;

class PortWidgetBase : public QPushButton//, public SCIRun::Dataflow::Networks::PortDescriptionInterface
{
public:
  //SCIRun::Dataflow::Networks::PortId id() const override = 0;
  //size_t nconnections() const override = 0;
  //std::string get_typename() const override = 0;
  //std::string get_portname() const override = 0;
  //bool isInput() const override = 0;
  //bool isDynamic() const override = 0;
  //SCIRun::Dataflow::Networks::ModuleId getUnderlyingModuleId() const override = 0;
  //size_t getIndex() const override = 0;
  //std::optional<Dataflow::Networks::ConnectionId> firstConnectionId() const override = 0;
  virtual const SCIRun::Dataflow::Networks::PortDescriptionInterface* description() const = 0;

  virtual QColor color() const = 0;
  virtual bool isLightOn() const = 0;

  bool isHighlighted() const { return isHighlighted_; }
  int properWidth() const { return sizeHint().width(); }
  void setSceneFunc(SceneFunc getScene) { getScene_ = getScene; }
  SceneFunc sceneFunc() const { return getScene_; }
  bool sameScene(const PortWidgetBase* other) const;

protected:
  static const int DEFAULT_WIDTH = 11;
  explicit PortWidgetBase(QWidget* parent);
  QSize sizeHint() const override;
  void paintEvent(QPaintEvent* event) override;
  bool isHighlighted_;
  SceneFunc getScene_;
};

class PortWidget : public PortWidgetBase, public NeedsScenePositionProvider
{
  Q_OBJECT

public:
  PortWidget(const QString& name, const QColor& color, const std::string& datatype, const SCIRun::Dataflow::Networks::ModuleId& moduleId,
    size_t index, bool isInput, bool isDynamic,
    const SCIRun::Dataflow::Networks::PortHandle& port,
    std::function<SharedPointer<ConnectionFactory>()> connectionFactory,
    std::function<SharedPointer<ClosestPortFinder>()> closestPortFinder,
    SCIRun::Dataflow::Networks::PortDataDescriber portDataDescriber,
    QWidget* parent = nullptr);
  virtual ~PortWidget();

  QString name() const { return name_; }
  QColor color() const override { return color_; }
  //bool isInput() const override { return isInput_; }
  //bool isDynamic() const override { return isDynamic_; }
  bool isConnected() const { return isConnected_; }
  void setConnected(bool connected) { isConnected_ = connected; }

  //virtual Dataflow::Networks::PortDescriptionInterface* getRealPort() { return this; }

  const SCIRun::Dataflow::Networks::PortDescriptionInterface* description() const override;

  //size_t nconnections() const override;
  //std::string get_typename() const override;
  //std::string get_portname() const override;
  //Dataflow::Networks::ModuleId getUnderlyingModuleId() const override;
  //size_t getIndex() const override;
  void setIndex(size_t i);
  //Dataflow::Networks::PortId realId() const { return portId_; }

  //SCIRun::Dataflow::Networks::PortId id() const override;
  //SCIRun::Dataflow::Networks::PortId externalId() const override { return id(); }

  void toggleLight();
  void turn_on_light();
  void turn_off_light();
  bool isLightOn() const override { return lightOn_; }
#if 0
  void connectToSubnetPort(PortWidget* subnetPort);
#endif

  void setHighlight(bool on, bool individual = false);
  void setPositionObject(PositionProviderPtr provider) override;

  void addConnection(ConnectionLine* c);
  void removeConnection(ConnectionLine* c);

  void makePotentialConnectionLine(PortWidget* other);

  void trackConnections();
  void deleteConnections();
  void deleteConnectionsLater();

  std::vector<PortWidget*> connectedPorts() const;

  QPointF position() const;

  bool sharesParentModule(const PortWidget& other) const;
  bool isFullInputPort() const;

  void connectionDisabled(bool disabled);
  void setConnectionsDisabled(bool disabled);

  void doMousePress(Qt::MouseButton button, const QPointF& pos);
  QGraphicsItem* doMouseMove(Qt::MouseButtons buttons, const QPointF& pos);
  void doMouseRelease(Qt::MouseButton button, const QPointF& pos, Qt::KeyboardModifiers modifiers);

  SCIRun::Dataflow::Networks::PortDataDescriber getPortDataDescriber() const { return portDataDescriber_; }

  const ConnectionLine* firstConnection() const { return !connections_.empty() ? *connections_.cbegin() : nullptr; }

  //std::optional<Dataflow::Networks::ConnectionId> firstConnectionId() const override;

  QGraphicsTextItem* makeNameLabel() const;

  const std::set<ConnectionLine*>& connections() const { return connections_; }

protected:
  void moveEvent(QMoveEvent * event) override;

public Q_SLOTS:
  void makeConnection(const SCIRun::Dataflow::Networks::ConnectionDescription& cd);
  void cancelConnectionsInProgress();
  void portCachingChanged(bool checked);
  void connectModule();
  void clearPotentialConnections();
  void insertNewModule(const QMap<QString, std::string>& info);
  void pickConnectModule();
Q_SIGNALS:
  void requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to);
  void connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId& id);
  void connectNewModuleHere(const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);
  void insertNewModuleHere(const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const QMap<QString, std::string>& info);
  void portMoved();
  void connectionNoteChanged();
  void highlighted(bool highlighted);
  void incomingConnectionStateChange(bool disabled, int index);
protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

private:
  template <typename Func, typename Pred>
  static void forEachPort(Func func, Pred pred);

  QGraphicsItem* dragImpl(const QPointF& endPos);
  void makeConnection(const QPointF& pos);
  void tryConnectPort(const QPointF& pos, PortWidget* port, double threshold);
  bool matches(const SCIRun::Dataflow::Networks::ConnectionDescription& cd) const;

  const QString name_;

  const SCIRun::Dataflow::Networks::ModuleId moduleId_;
  const SCIRun::Dataflow::Networks::PortHandle port_;
  size_t index_;
  const QColor color_;
  const std::string typename_;
  const bool isInput_;
  const bool isDynamic_;
  bool isConnected_;
  bool lightOn_;
  QPointF startPos_;
  ConnectionInProgress* currentConnection_;
  friend struct DeleteCurrentConnectionAtEndOfBlock;
  std::set<ConnectionLine*> connections_;
  boost::function<SharedPointer<ConnectionFactory>()> connectionFactory_;
  boost::function<SharedPointer<ClosestPortFinder>()> closestPortFinder_;
  PortActionsMenu* menu_;
  SCIRun::Dataflow::Networks::PortDataDescriber portDataDescriber_;
  //TODO
  typedef std::map<std::string, std::map<bool, std::map<SCIRun::Dataflow::Networks::PortId, PortWidget*>>> PortWidgetMap;
  static PortWidgetMap portWidgetMap_;

  typedef std::map<PortWidget*, std::map<PortWidget*, bool>> PotentialConnectionMap;
  static PotentialConnectionMap potentialConnectionsMap_;
  std::set<ConnectionInProgress*> potentialConnections_;
  std::set<QGraphicsTextItem*> potentialConnectionPortNames_;
};

// To fill the layout
class BlankPort : public PortWidgetBase
{
public:
  explicit BlankPort(QWidget* parent);
  const SCIRun::Dataflow::Networks::PortDescriptionInterface* description() const override;
  QColor color() const override;
  bool isLightOn() const override { return false; }
};

class InputPortWidget : public PortWidget
{
public:
  InputPortWidget(const QString& name, const QColor& color, const std::string& datatype, const SCIRun::Dataflow::Networks::ModuleId& moduleId,
    size_t index, bool isDynamic,
    const SCIRun::Dataflow::Networks::PortHandle& port,
    std::function<SharedPointer<ConnectionFactory>()> connectionFactory,
    std::function<SharedPointer<ClosestPortFinder>()> closestPortFinder,
    SCIRun::Dataflow::Networks::PortDataDescriber portDataDescriber,
    QWidget* parent = nullptr);
};

class OutputPortWidget : public PortWidget
{
public:
  OutputPortWidget(const QString& name, const QColor& color, const std::string& datatype, const SCIRun::Dataflow::Networks::ModuleId& moduleId,
    size_t index, bool isDynamic,
    const SCIRun::Dataflow::Networks::PortHandle& port,
    std::function<SharedPointer<ConnectionFactory>()> connectionFactory,
    std::function<SharedPointer<ClosestPortFinder>()> closestPortFinder,
    SCIRun::Dataflow::Networks::PortDataDescriber portDataDescriber,
    QWidget* parent = nullptr);
};

#if 0
struct SubnetPortWidgetCtorArgs
{
  QString name;
  QColor color;
  std::string datatype;
  boost::function<SharedPointer<ConnectionFactory>()> connectionFactory;
  boost::function<SharedPointer<ClosestPortFinder>()> closestPortFinder;
  Dataflow::Networks::PortDescriptionInterface* realPort;
};

class SubnetInputPortWidget : public InputPortWidget
{
public:
  SubnetInputPortWidget(const SubnetPortWidgetCtorArgs& args, QWidget* parent = nullptr);
  Dataflow::Networks::PortDescriptionInterface* getRealPort() override { return realPort_; }
private:
  Dataflow::Networks::PortDescriptionInterface* realPort_;
};

class SubnetOutputPortWidget : public OutputPortWidget
{
public:
  SubnetOutputPortWidget(const SubnetPortWidgetCtorArgs& args, QWidget* parent = nullptr);
  Dataflow::Networks::PortDescriptionInterface* getRealPort() override { return realPort_; }
private:
  Dataflow::Networks::PortDescriptionInterface* realPort_;
};
#endif

class DataInfoDialog
{
public:
  static void show(SCIRun::Dataflow::Networks::PortDataDescriber portDataDescriber, const QString& label, const std::string& id);
};

}
}

#endif
