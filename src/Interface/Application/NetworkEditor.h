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

#ifndef INTERFACE_APPLICATION_NETWORKEDITOR_H
#define INTERFACE_APPLICATION_NETWORKEDITOR_H

#include <QGraphicsView>
#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#include <map>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Engine/Controller/ControllerInterfaces.h>
#include <Dataflow/Serialization/Network/ModulePositionGetter.h>
#include <Interface/Application/Note.h>
#include <Interface/Application/Utility.h>
#endif

class QMenu;
class QToolBar;
class QAction;
class QGraphicsScene;
class DialogErrorControl;
Q_DECLARE_METATYPE (std::string)

namespace SCIRun {

  namespace Dataflow { namespace Engine { class NetworkEditorController; struct DisableDynamicPortSwitch; struct ModuleCounter; }}

namespace Gui {

  class CurrentModuleSelection
  {
  public:
    virtual ~CurrentModuleSelection() {}
    virtual QString text() const = 0;
    virtual bool isModule() const = 0;
  };

  //almost just want to pass a boost::function for this one.
  class DefaultNotePositionGetter
  {
  public:
    virtual ~DefaultNotePositionGetter() {}
    virtual NotePosition position() const = 0;
  };

  class ModuleEventProxy : public QObject
  {
    Q_OBJECT
  public:
    ModuleEventProxy();
    void trackModule(SCIRun::Dataflow::Networks::ModuleHandle module);
Q_SIGNALS:
    void moduleExecuteStart(const std::string& id);
    void moduleExecuteEnd(const std::string& id);
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

  class ConnectionLine;
  class ModuleWidget;
  class NetworkEditorControllerGuiProxy;
	class DialogErrorControl;

  class NetworkEditor : public QGraphicsView,
    public SCIRun::Dataflow::Networks::ExecutableLookup,
    public SCIRun::Dataflow::Networks::NetworkEditorSerializationManager,
    public SCIRun::Dataflow::Engine::NetworkIOInterface<SCIRun::Dataflow::Networks::NetworkFileHandle>,
    public SCIRun::Dataflow::Networks::ConnectionMakerService
  {
	  Q_OBJECT

  public:
    explicit NetworkEditor(boost::shared_ptr<CurrentModuleSelection> moduleSelectionGetter, boost::shared_ptr<DefaultNotePositionGetter> dnpg,
				boost::shared_ptr<DialogErrorControl> dialogErrorControl, 
        TagColorFunc tagColor = defaultTagColor,
        QWidget* parent = 0);
    ~NetworkEditor();
    QList<QAction*> getModuleSpecificActions() const;
    void setNetworkEditorController(boost::shared_ptr<NetworkEditorControllerGuiProxy> controller);
    boost::shared_ptr<NetworkEditorControllerGuiProxy> getNetworkEditorController() const;
    virtual SCIRun::Dataflow::Networks::ExecutableObject* lookupExecutable(const SCIRun::Dataflow::Networks::ModuleId& id) const;
    virtual bool containsViewScene() const;

    SCIRun::Dataflow::Networks::NetworkFileHandle saveNetwork() const;
    void loadNetwork(const SCIRun::Dataflow::Networks::NetworkFileHandle& file);

    virtual SCIRun::Dataflow::Networks::ModulePositionsHandle dumpModulePositions() const;
    virtual void updateModulePositions(const SCIRun::Dataflow::Networks::ModulePositions& modulePositions);

    virtual SCIRun::Dataflow::Networks::ModuleNotesHandle dumpModuleNotes() const;
    virtual void updateModuleNotes(const SCIRun::Dataflow::Networks::ModuleNotes& moduleNotes);

    virtual SCIRun::Dataflow::Networks::ConnectionNotesHandle dumpConnectionNotes() const;
    virtual void updateConnectionNotes(const SCIRun::Dataflow::Networks::ConnectionNotes& notes);

    virtual SCIRun::Dataflow::Networks::ModuleTagsHandle dumpModuleTags() const override;
    virtual void updateModuleTags(const SCIRun::Dataflow::Networks::ModuleTags& notes) override;

    size_t numModules() const;

    boost::shared_ptr<ModuleEventProxy> moduleEventProxy() { return moduleEventProxy_; }
    virtual int errorCode() const;

    void disableInputWidgets();
    void enableInputWidgets();

    //TODO: this class is getting too big and messy, schedule refactoring
    void setRegressionTestDataDir(const QString& dir);

    void setBackground(const QBrush& brush);
    QBrush background() const;

    int connectionPipelineType() const;

    QPixmap sceneGrab();

    boost::shared_ptr<Dataflow::Engine::DisableDynamicPortSwitch> createDynamicPortDisabler();

    int currentZoomPercentage() const;

    void setVisibility(bool visible);

    void metadataLayer(bool active);
    void tagLayer(bool active, int tag);
    bool tagLayerActive() const { return tagLayerActive_; }

  protected:
    virtual void dropEvent(QDropEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

  public Q_SLOTS:
    void addModuleWidget(const std::string& name, SCIRun::Dataflow::Networks::ModuleHandle module, const SCIRun::Dataflow::Engine::ModuleCounter& count);
    void requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to);
    void duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle& module);
    void connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo, const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName);
    void replaceModuleWith(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToReplace, const std::string& newModuleName);
    void executeAll();
    void executeModule(const SCIRun::Dataflow::Networks::ModuleHandle& module);
    void removeModuleWidget(const SCIRun::Dataflow::Networks::ModuleId& id);
    virtual void clear();
    void setConnectionPipelineType(int type);
    void addModuleViaDoubleClickedTreeItem();
    void selectAll();
    void del();
    void pinAllModuleUIs();
    void hideAllModuleUIs();
    void restoreAllModuleUIs();
    void updateViewport();
    void connectionAddedQueued(const SCIRun::Dataflow::Networks::ConnectionDescription& cd);
    void setMouseAsDragMode();
    void setMouseAsSelectMode();
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void centerView();
    void setModuleMini(bool mini);
    void highlightTaggedItem(int tagValue);

  Q_SIGNALS:
    void addConnection(const SCIRun::Dataflow::Networks::ConnectionDescription&);
    void connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId& id);
    void modified();
    void networkExecuted();
    void networkExecutionFinished();
    void networkEditorMouseButtonPressed();
    void moduleMoved(const SCIRun::Dataflow::Networks::ModuleId& id, double newX, double newY);
    void defaultNotePositionChanged(NotePosition position);
    void sceneChanged(const QList<QRectF>& region);
    void snapToModules();
    void highlightPorts(int state);
    void zoomLevelChanged(int zoom);
    void disableWidgetDisabling();
    void reenableWidgetDisabling();
  private Q_SLOTS:
    void cut();
    void copy();
    void paste();
    void bringToFront();
    void sendToBack();
    void properties();
    void updateActions();

  private:
    typedef QPair<ModuleWidget*, ModuleWidget*> ModulePair;
    void createActions();
    //void createMenus();
    //void createToolBars();
    void setZValue(int z);
    void setupModuleWidget(ModuleWidget* node);
    ModuleWidget* selectedModule() const;
    ConnectionLine* selectedLink() const;
    ModulePair selectedModulePair() const;
    void addNewModuleAtPosition(const QPointF& position);
    ConnectionLine* getSingleConnectionSelected();
    void unselectConnectionGroup();
    void fillModulePositionMap(SCIRun::Dataflow::Networks::ModulePositions& positions) const;
    void highlightTaggedItem(QGraphicsItem* item, int tagValue);
    //QToolBar* editToolBar_;
    //QAction* cutAction_;
    //QAction* copyAction_;
    //QAction* pasteAction_;
    QAction* deleteAction_;
    QAction* sendToBackAction_;
    QAction* propertiesAction_;
    //QAction* executeAction_;
		bool modulesSelectedByCL_;
    double currentScale_;
    bool tagLayerActive_;
    TagColorFunc tagColor_;

    QGraphicsScene* scene_;

    bool visibleItems_;
    QPointF lastModulePosition_;
		boost::shared_ptr<DialogErrorControl> dialogErrorControl_;
    boost::shared_ptr<CurrentModuleSelection> moduleSelectionGetter_;
    boost::shared_ptr<NetworkEditorControllerGuiProxy> controller_;
    boost::shared_ptr<DefaultNotePositionGetter> defaultNotePositionGetter_;
    boost::shared_ptr<ModuleEventProxy> moduleEventProxy_;
    boost::shared_ptr<ZLevelManager> zLevelManager_;
    std::string latestModuleId_;
  };
}
}

#endif
