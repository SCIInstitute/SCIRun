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
#include <Core/Dataflow/Network/NetworkFwd.h>

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
  
  class ConnectionLine;
  class ModuleWidget;
  class ModuleProxyWidget;
  class NetworkEditorControllerGuiProxy;

  class NetworkEditor : public QGraphicsView
  {
	  Q_OBJECT
	
  public:
    explicit NetworkEditor(boost::shared_ptr<CurrentModuleSelection> moduleSelectionGetter, QWidget* parent = 0);
    void addActions(QWidget* widget);
    void setExecuteAction(QAction* action) { executeAction_ = action; }
    void setNetworkEditorController(boost::shared_ptr<NetworkEditorControllerGuiProxy> controller);
  protected:
    virtual void dropEvent(QDropEvent* event);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
  public Q_SLOTS:
    void addModule(const std::string& name, SCIRun::Domain::Networks::ModuleHandle module);
  Q_SIGNALS:
    void addConnection(const SCIRun::Domain::Networks::ConnectionDescription&);
    void connectionDeleted(const SCIRun::Domain::Networks::ConnectionId& id);
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
    //QMenu* fileMenu_;
    //QMenu* editMenu_;
    //QToolBar* editToolBar_;
    //QAction* cutAction_;
    //QAction* copyAction_;
    //QAction* pasteAction_;
    QAction* deleteAction_;
    //QAction* addLinkAction_;
    //QAction* addNodeAction_;
    QAction* bringToFrontAction_;
    QAction* sendToBackAction_;
    QAction* propertiesAction_;
    QAction* executeAction_;

    QGraphicsScene* scene_;
  
    int minZ_;
    int maxZ_;
    int seqNumber_;

    QPointF lastModulePosition_;

    boost::shared_ptr<CurrentModuleSelection> moduleSelectionGetter_;
    boost::shared_ptr<NetworkEditorControllerGuiProxy> controller_;
  };

}
}

#endif