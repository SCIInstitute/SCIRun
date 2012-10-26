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

#ifndef INTERFACE_APPLICATION_NETWORKEDITOR_H
#define INTERFACE_APPLICATION_NETWORKEDITOR_H

#include <boost/shared_ptr.hpp>
#include <QGraphicsView>
#include <map>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/NetworkInterface.h>

class QMenu;
class QToolBar;
class QAction;
class QGraphicsScene;

namespace SCIRun {
namespace Gui {

  class CurrentModuleSelection
  {
  public:
    virtual ~CurrentModuleSelection() {}
    virtual QString text() const = 0;
    virtual bool isModule() const = 0;
  };

  Q_DECLARE_METATYPE (std::string)

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
  
  class ConnectionLine;
  class ModuleWidget;
  class ModuleProxyWidget;
  class NetworkEditorControllerGuiProxy;

  class NetworkEditor : public QGraphicsView, public SCIRun::Dataflow::Networks::ExecutableLookup
  {
	  Q_OBJECT
	
  public:
    explicit NetworkEditor(boost::shared_ptr<CurrentModuleSelection> moduleSelectionGetter, QWidget* parent = 0);
    QList<QAction*> getModuleSpecificActions() const;
    void setModuleDumpAction(QAction* action);
    void setNetworkEditorController(boost::shared_ptr<NetworkEditorControllerGuiProxy> controller);
    virtual SCIRun::Dataflow::Networks::ExecutableObject* lookupExecutable(const std::string& id) const; 
    void moveModules(const SCIRun::Dataflow::Networks::ModulePositions& modulePositions);

    SCIRun::Dataflow::Networks::NetworkXMLHandle saveNetwork();
    void loadNetwork(const SCIRun::Dataflow::Networks::NetworkXML& xml);

    int numModules() const;

    boost::shared_ptr<ModuleEventProxy> moduleEventProxy() { return moduleEventProxy_; }

  protected:
    virtual void dropEvent(QDropEvent* event);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
  public Q_SLOTS:
    void addModule(const std::string& name, SCIRun::Dataflow::Networks::ModuleHandle module);
    void needConnection(const SCIRun::Dataflow::Networks::ConnectionDescription&);
    void executeAll(SCIRun::Dataflow::Networks::NetworkExecutionFinishedCallback func = 0);
    void clear();

    //TODO: break out, unit test
    SCIRun::Dataflow::Networks::ModulePositionsHandle dumpModulePositions();
  Q_SIGNALS:
    void addConnection(const SCIRun::Dataflow::Networks::ConnectionDescription&);
    void connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId& id);
    void modified();
  private Q_SLOTS:
    void del();
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
    void setupModule(ModuleWidget* node);
    ModuleWidget* selectedModule() const;
    ModuleProxyWidget* selectedModuleProxy() const;
    ConnectionLine* selectedLink() const;
    ModulePair selectedModulePair() const;
    
    //QToolBar* editToolBar_;
    //QAction* cutAction_;
    //QAction* copyAction_;
    //QAction* pasteAction_;
    QAction* deleteAction_;
    //QAction* bringToFrontAction_;
    QAction* sendToBackAction_;
    QAction* propertiesAction_;
    //QAction* executeAction_;
    QAction* moduleDumpAction_;

    QGraphicsScene* scene_;
  
    int minZ_;
    int maxZ_;
    int seqNumber_;

    QPointF lastModulePosition_;

    boost::shared_ptr<CurrentModuleSelection> moduleSelectionGetter_;
    boost::shared_ptr<NetworkEditorControllerGuiProxy> controller_;

    boost::shared_ptr<ModuleEventProxy> moduleEventProxy_;
  };

}
}

#endif