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

#ifndef NETWORKEDITOR_H
#define NETWORKEDITOR_H

#include <QGraphicsView>

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
    virtual std::string text() const = 0;
    virtual bool isModule() const = 0;
  };

  class Logger
  {
  public:
    virtual ~Logger() {}
    virtual void log(const std::string& message) const = 0;
  };

  class Connection;
  class Module;
  class ModuleProxyWidget;

class NetworkEditor : public QGraphicsView
{
	Q_OBJECT
	
public:
  //TODO change to boost::shared_ptr
  explicit NetworkEditor(CurrentModuleSelection* moduleSelectionGetter, Logger* logger, QWidget* parent = 0);
  void addActions(QWidget* widget);
protected:
  virtual void dropEvent(QDropEvent* event);
  virtual void dragEnterEvent(QDragEnterEvent* event);
  virtual void dragMoveEvent(QDragMoveEvent* event);
private slots:
  void addModule();
  void addModule(const QString& text, const QPoint& pos);
  void addLink();
  void del();
  void cut();
  void copy();
  void paste();
  void bringToFront();
  void sendToBack();
  void properties();
  void updateActions();

private:
  typedef QPair<Module*, Module*> ModulePair;
  void createActions();
  //void createMenus();
  //void createToolBars();
  void setZValue(int z);
  void setupModule(Module* node, const QPoint& pos = QPoint());
  Module* selectedModule() const;
  ModuleProxyWidget* selectedModuleProxy() const;
  Connection* selectedLink() const;
  ModulePair selectedModulePair() const;

  //QMenu* fileMenu_;
  //QMenu* editMenu_;
  //QToolBar* editToolBar_;
  //QAction* exitAction_;
  //QAction* cutAction_;
  //QAction* copyAction_;
  //QAction* pasteAction_;
  QAction* deleteAction_;
  QAction* addLinkAction_;
  QAction* addNodeAction_;
  QAction* bringToFrontAction_;
  QAction* sendToBackAction_;
  QAction* propertiesAction_;

  QGraphicsScene* scene_;
  
  int minZ_;
  int maxZ_;
  int seqNumber_;

  //TODO change to boost::shared_ptr
  CurrentModuleSelection* moduleSelectionGetter_;
  Logger* logger_;
};

}
}

#endif