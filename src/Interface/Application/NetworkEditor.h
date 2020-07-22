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


#ifndef INTERFACE_APPLICATION_NETWORKEDITOR_H
#define INTERFACE_APPLICATION_NETWORKEDITOR_H

#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include "ui_NetworkSearch.h"
#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#include <atomic>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Dataflow/Engine/Controller/ControllerInterfaces.h>
#include <Dataflow/Serialization/Network/ModulePositionGetter.h>
#include <Interface/Application/Note.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/Subnetworks.h>
#endif

class QMenu;
class QToolBar;
class QAction;
class QGraphicsScene;
class QTimeLine;
Q_DECLARE_METATYPE (std::string)

namespace SCIRun {

  namespace Dataflow { namespace Engine { class NetworkEditorController; struct DisableDynamicPortSwitch; struct ModuleCounter; }}

namespace Gui {

  class DialogErrorControl;
  class SubnetPortsBridgeProxyWidget;

  class CurrentModuleSelection
  {
  public:
    virtual ~CurrentModuleSelection() {}
    virtual QString text() const = 0;
    virtual bool isModule() const = 0;
    virtual QString clipboardXML() const = 0;
    virtual bool isClipboardXML() const = 0;
  };

  //almost just want to pass a boost::function for this one.
  class DefaultNotePositionGetter
  {
  public:
    virtual ~DefaultNotePositionGetter() {}
    virtual NotePosition position() const = 0;
    virtual int size() const = 0;
  };

  class ModuleErrorDisplayer
  {
  public:
    virtual ~ModuleErrorDisplayer() {}
    virtual void displayError(const QString& msg, std::function<void()> showModule) = 0;
  };

  class FloatingTextItem : public QGraphicsTextItem
  {
    Q_OBJECT
  public:
    FloatingTextItem(const QString& text, std::function<void()> action, QGraphicsItem* parent = nullptr);
    ~FloatingTextItem();
    int num() const { return counter_; }
  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
  private Q_SLOTS:
    void animate(qreal val);
  private:
    QTimeLine* timeLine_;
    std::function<void()> action_;
    const int counter_;
    QGraphicsRectItem* rect_;
    static std::atomic<int> instanceCounter_;
  };

  class ErrorItem : public FloatingTextItem
  {
    Q_OBJECT
  public:
    ErrorItem(const QString& text, std::function<void()> showModule, QGraphicsItem* parent = nullptr);
  };

  class SearchResultItem : public FloatingTextItem
  {
    Q_OBJECT
  public:
    SearchResultItem(const QString& text, const QColor& color, std::function<void()> action, QGraphicsItem* parent = nullptr);
    ~SearchResultItem();
    static void removeAll();
    static std::set<SearchResultItem*> items_;
  };

  class NetworkSearchWidget : public QWidget, public Ui::NetworkSearch
  {
    Q_OBJECT
  public:
    explicit NetworkSearchWidget(class NetworkEditor* ned);
  };

  class ModuleEventProxy : public QObject
  {
    Q_OBJECT
  public:
    ModuleEventProxy();
    void trackModule(SCIRun::Dataflow::Networks::ModuleHandle module);
  Q_SIGNALS:
    void moduleExecuteStart(const std::string& id);
    void moduleExecuteEnd(double execTime, const std::string& id);
  };

  class ModuleProxyWidget;

  class ZLevelManager
  {
  public:
    explicit ZLevelManager(QGraphicsScene* scene);
    int get_max() const { return maxZ_; }
    void bringToFront();
    void sendToBack();
  private:
    void setZValue(int z);
    ModuleProxyWidget* selectedModuleProxy() const;
    QGraphicsScene* scene_;
    int minZ_;
    int maxZ_;
    enum { INITIAL_Z = 1000 };
  };

  class ModuleWidgetPlacementManager
  {
  public:
    void updateLatestFromDuplicate(const QPointF& scenePos);
    void updateLatestFromConnectNew(const QPointF& scenePos, bool isInputPort);
    void updateLatestFromReplace(const QPointF& scenePos);
    QPointF getLast() const { return lastModulePosition_; }
    QPointF getLastForDoubleClickedItem(const QPointF& p) const;
    void setLastFromAddingNew(const QPointF& p) { lastModulePosition_ = p; }
  private:
    QPointF lastModulePosition_{ 30, 30 };
    static QPointF connectNewIncrement(bool isInput);
  };

  class ConnectionLine;
  class ModuleWidget;
  class NetworkEditorControllerGuiProxy;
	class DialogErrorControl;
  class PortWidget;
  using PortRewiringMap = std::map<std::string, ConnectionLine*>;

  struct NetworkEditorParameters
  {
    SharedPointer<CurrentModuleSelection> moduleSelectionGetter;
    SharedPointer<DefaultNotePositionGetter> dnpg;
    SharedPointer<DialogErrorControl> dialogErrorControl;
    PreexecuteFunc preexecuteFunc;
    TagColorFunc tagColor;
    TagNameFunc tagName;
    double highResolutionExpandFactor;
    class DockManager* dockManager_;
  };

  namespace NetworkBoundaries
  {
    const int sceneWidth = 1600;
    const int sceneHeight = 3200;

    const double highDPIExpandFactorDefault = 1.7;
  }

  class NetworkEditor : public QGraphicsView,
    public Dataflow::Networks::ExecutableLookup,
    public Dataflow::Networks::NetworkEditorSerializationManager,
    public Dataflow::Engine::NetworkIOInterface<Dataflow::Networks::NetworkFileHandle>,
    public Dataflow::Networks::ConnectionMakerService,
    public ModuleErrorDisplayer
  {
	  Q_OBJECT

  public:
    explicit NetworkEditor(const NetworkEditorParameters& params, QWidget* parent = nullptr);
    NetworkEditor(const NetworkEditor& rhs) = delete;
    NetworkEditor& operator=(const NetworkEditor&) = delete;
    ~NetworkEditor();
    void setNetworkEditorController(SharedPointer<NetworkEditorControllerGuiProxy> controller);
    SharedPointer<NetworkEditorControllerGuiProxy> getNetworkEditorController() const;
    Dataflow::Networks::ExecutableObject* lookupExecutable(const Dataflow::Networks::ModuleId& id) const override;
    bool containsViewScene() const override;

    Dataflow::Networks::NetworkFileHandle saveNetwork() const override;
    void loadNetwork(const Dataflow::Networks::NetworkFileHandle& file) override;
    void appendToNetwork(const Dataflow::Networks::NetworkFileHandle& xml);

    Dataflow::Networks::ModulePositionsHandle dumpModulePositions(Dataflow::Networks::ModuleFilter filter) const override;
    void updateModulePositions(const Dataflow::Networks::ModulePositions& modulePositions, bool selectAll) override;

    Dataflow::Networks::ModuleNotesHandle dumpModuleNotes(Dataflow::Networks::ModuleFilter filter) const override;
    void updateModuleNotes(const Dataflow::Networks::ModuleNotes& moduleNotes) override;

    Dataflow::Networks::ConnectionNotesHandle dumpConnectionNotes(Dataflow::Networks::ConnectionFilter filter) const override;
    void updateConnectionNotes(const Dataflow::Networks::ConnectionNotes& notes) override;

    Dataflow::Networks::ModuleTagsHandle dumpModuleTags(Dataflow::Networks::ModuleFilter filter) const override;
    void updateModuleTags(const Dataflow::Networks::ModuleTags& notes) override;

    Dataflow::Networks::DisabledComponentsHandle dumpDisabledComponents(Dataflow::Networks::ModuleFilter modFilter, Dataflow::Networks::ConnectionFilter connFilter) const override;
    void updateDisabledComponents(const Dataflow::Networks::DisabledComponents& disabled) override;

    Dataflow::Networks::SubnetworksHandle dumpSubnetworks(Dataflow::Networks::ModuleFilter filter) const override;
    void updateSubnetworks(const Dataflow::Networks::Subnetworks& subnets) override;

    void copyNote(Dataflow::Networks::ModuleHandle from, Dataflow::Networks::ModuleHandle to) const override;

    size_t numModules() const;

    SharedPointer<ModuleEventProxy> moduleEventProxy() const { return moduleEventProxy_; }
    int errorCode() const override;

    void disableInputWidgets();
    void enableInputWidgets();

    void disableViewScenes();
    void enableViewScenes();
    void saveImages();

    //TODO: this class is getting too big and messy, schedule refactoring

    void setBackground(const QBrush& brush);
    QBrush background() const;
    void updateBackground(bool forceGrid);
    void updateExecuteButtons(bool downstream);

    int connectionPipelineType() const;

    SharedPointer<Dataflow::Engine::DisableDynamicPortSwitch> createDynamicPortDisabler();

    int currentZoomPercentage() const;

    void setVisibility(bool visible);

    void metadataLayer(bool active);
    void tagLayer(bool active, int tag);
    bool tagLayerActive() const { return tagLayerActive_; }
    bool tagGroupsActive() const { return tagGroupsActive_; }

    void displayError(const QString& msg, std::function<void()> showModule) override;

    bool showTagGroupsOnFileLoad() const { return showTagGroupsOnFileLoad_; }
    void setShowTagGroupsOnFileLoad(bool show) { showTagGroupsOnFileLoad_ = show; }

    void setHighResolutionExpandFactor(double factor = NetworkBoundaries::highDPIExpandFactorDefault);

    void adjustExecuteButtonsToDownstream(bool downOnly);

    NetworkEditor* parentNetwork() { return parentNetwork_; }
    size_t childCount() const { return childrenNetworks_.size(); }
    void killChild(const QString& name, bool force);
    void sendItemsToParent();
    bool containsModule(const std::string& moduleId) const;

    void hidePipesByType(const std::string& type);

    using ConnectorFunc = std::function<void(NetworkEditor*)>;
    static void setConnectorFunc(ConnectorFunc func) { connectorFunc_ = func; }

    using ViewUpdateFunc = std::function<void(const QString&)>;
    static void setViewUpdateFunc(ViewUpdateFunc func) { viewUpdateFunc_ = func; }

    static void setMiniview(QGraphicsView* miniview) { miniview_ = miniview; }

    static const int ConnectionHideTimeMS_ {15000};

    struct InEditingContext
    {
      explicit InEditingContext(NetworkEditor* ed)
      {
        inEditingContext_ = ed;
      }
      ~InEditingContext()
      {
        inEditingContext_ = nullptr;
      }
    };
  protected:
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent* event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void scrollContentsBy(int dx, int dy) override;
    void resizeEvent(QResizeEvent *event) override;

  public Q_SLOTS:
    void addModuleWidget(const std::string& name, SCIRun::Dataflow::Networks::ModuleHandle module, const SCIRun::Dataflow::Engine::ModuleCounter& count);
    boost::optional<SCIRun::Dataflow::Networks::ConnectionId> requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to) override;
    void duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle& module);
    void connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo, const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);
    void insertNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo, const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const QMap<QString, std::string>& info);
    void replaceModuleWith(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToReplace, const std::string& newModuleName);
    void executeAll();
    void executeModule(const SCIRun::Dataflow::Networks::ModuleHandle& module, bool fromButton);
    void removeModuleWidget(const SCIRun::Dataflow::Networks::ModuleId& id);
    void clear() override;
    void setConnectionPipelineType(int type);
    void addModuleViaDoubleClickedTreeItem();
    void selectAll();
    void del();
    void pinAllModuleUIs();
    void hideAllModuleUIs();
    void restoreAllModuleUIs();
    void seeThroughAllModuleUIs();
    void normalOpacityAllModuleUIs();
    void updateViewport();
    void connectionAddedQueued(const SCIRun::Dataflow::Networks::ConnectionDescription& cd);
    void setMouseAsDragMode();
    void setMouseAsSelectMode();
    void showStateViewer();
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void zoomBestFit();
    void centerView();
    void highlightTaggedItem(int tagValue);
    void resetNetworkDueToCycle();
    void moduleWindowAction();
    void cleanUpNetwork();
    void redrawTagGroups();
    void adjustModuleWidth(int delta);
    void adjustModuleHeight(int delta);
    void saveTagGroupRectInFile();
    void renameTagGroupInFile();
    void makeSubnetwork();
    void makeSubnetworkFromComponents(const QString& name,
      const std::vector<SCIRun::Dataflow::Networks::ModuleHandle>& modules,
      QList<QGraphicsItem*> items, const QRectF& rect);
    void showSubnetChild(const QString& name);
    void addSubnetChild(const QString& name, SCIRun::Dataflow::Networks::ModuleHandle mod);
    void subnetMenuActionTriggered();

  Q_SIGNALS:
    void connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId& id);
    void modified();
    void networkExecuted();
    void networkExecutionFinished();
    void networkEditorMouseButtonPressed();
    void middleMouseClicked();
    void moduleMoved(const SCIRun::Dataflow::Networks::ModuleId& id, double newX, double newY);
    void defaultNotePositionChanged(NotePosition position);
    void defaultNoteSizeChanged(int size);
    void snapToModules();
    void highlightPorts(int state);
    void zoomLevelChanged(int zoom);
    void disableWidgetDisabling();
    void reenableWidgetDisabling();
    void resetModulesDueToCycle();
    void newModule(const QString& modId, bool hasUI);
    void newSubnetworkCopied(const QString& xml);
    void requestLoadNetwork(const QString& file);
  private Q_SLOTS:
    void cut();
    void copy();
    void paste();
    void bringToFront();
    void sendToBack();
    void searchTextChanged(const QString& text);
    void clearSiblingSelections();

  private:
    using ModulePair = QPair<ModuleWidget*, ModuleWidget*>;
    ModuleProxyWidget* setupModuleWidget(ModuleWidget* node);
    ModuleWidget* selectedModule() const;
    ConnectionLine* selectedLink() const;
    ModulePair selectedModulePair() const;
    void addNewModuleAtPosition(const QPointF& position);
    ConnectionLine* getSingleConnectionSelected();
    void unselectConnectionGroup();
    void fillModulePositionMap(SCIRun::Dataflow::Networks::ModulePositions& positions, SCIRun::Dataflow::Networks::ModuleFilter filter) const;
    void highlightTaggedItem(QGraphicsItem* item, int tagValue);
    void pasteImpl(const QString& xml);
    void connectNewModuleImpl(const Dataflow::Networks::ModuleHandle& moduleToConnectTo, const Dataflow::Networks::PortDescriptionInterface* portToConnect,
      const std::string& newModuleName);
    void drawTagGroups();
    void removeTagGroups();
    QString checkForOverriddenTagName(int tag) const;
    void renameTagGroup(int tag, const QString& name);
    QPointF positionOfFloatingText(int num, bool top, int horizontalIndent, int verticalSpacing) const;
    QPixmap grabSubnetPic(const QRectF& rect, const QList<QGraphicsItem*>& items);
    QString convertToTooltip(const QPixmap& pic) const;
    void initializeSubnet(const QString& name, SCIRun::Dataflow::Networks::ModuleHandle mod, NetworkEditor* subnet);
    void dumpSubnetworksImpl(const QString& name, Dataflow::Networks::Subnetworks& data, Dataflow::Networks::ModuleFilter modFilter) const;
    QList<QGraphicsItem*> includeConnections(QList<QGraphicsItem*> items) const;
    QRectF visibleRect() const;
    void alignViewport();
    void deleteImpl(QList<QGraphicsItem*> items);
    QPointF getModulePositionAdjustment(const SCIRun::Dataflow::Networks::ModulePositions& modulePositions);
    void deselectAll();

    // default constructed
    bool modulesSelectedByCL_{ false };
    double currentScale_ { 1.0 };
    bool tagLayerActive_{ false };
    bool tagGroupsActive_ {false};
    bool fileLoading_{ false };
    bool insertingNewModuleAlongConnection_{ false };
    bool showTagGroupsOnFileLoad_{ false };
    bool visibleItems_{ true };
    ModuleWidgetPlacementManager modulePlacement_;
    std::string latestModuleId_;
    std::map<int, std::string> tagLabelOverrides_;
    QTimeLine* connectionDimmingTimeLine_{nullptr};

    // for subnets
    NetworkEditorParameters ctorParams_;

    // unpacked constructor parameters
    TagColorFunc tagColor_;
    TagNameFunc tagName_;
		SharedPointer<DialogErrorControl> dialogErrorControl_;
    SharedPointer<CurrentModuleSelection> moduleSelectionGetter_;
    SharedPointer<DefaultNotePositionGetter> defaultNotePositionGetter_;
    PreexecuteFunc preexecute_;
    double highResolutionExpandFactor_;

    // special
    QGraphicsScene* scene_;
    SharedPointer<NetworkEditorControllerGuiProxy> controller_;

    // newed up
    SharedPointer<ModuleEventProxy> moduleEventProxy_;
    SharedPointer<ZLevelManager> zLevelManager_;

    // tree structure
    NetworkEditor* parentNetwork_ {nullptr};
    std::map<QString, class SubnetworkEditor*> childrenNetworks_;
    std::map<QString, QList<QGraphicsItem*>> childrenNetworkItems_;
    QList<SubnetPortsBridgeProxyWidget*> subnetPortHolders_;
    void setupPortHolders(Dataflow::Networks::ModuleHandle mod);
    void setupPortHolder(const std::vector<SharedPointer<SCIRun::Dataflow::Networks::PortDescriptionInterface>>& ports, const QString& name,
      std::function<QPointF(const QRectF&)> position);
    void removeSubnetPortHolders();
    void resizeSubnetPortHolders(double scaleFactor);
    std::vector<QGraphicsItem*> subnetItemsToMove();
    PortRewiringMap portRewiringMap_;
    QSet<QString> currentSubnetNames_;
    std::map<std::string, QString> subnetNameMap_;

    template <typename Func>
    void tailRecurse(Func func)
    {
      for (auto& child : childrenNetworks_)
      {
        func(child.second->get());
      }
    }

    static NetworkEditor* inEditingContext_;

    static ConnectorFunc connectorFunc_;
    static ViewUpdateFunc viewUpdateFunc_;
    void logViewerDims(const QString& msg);
    static std::function<QPointF(const QRectF&)> topSubnetPortHolderPositioner_;
    static std::function<QPointF(const QRectF&)> bottomSubnetPortHolderPositioner_;
    static QGraphicsView* miniview_;
  };

  ModuleWidget* getModule(QGraphicsItem* item);
}
}

#endif
