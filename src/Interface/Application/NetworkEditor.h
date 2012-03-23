#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#include <QGraphicsView>

class Node;
class Link;
class QMenu;
class QToolBar;
class QAction;
class QGraphicsScene;

namespace SCIRun
{

class NetworkEditor : public QGraphicsView
{
	Q_OBJECT
	
public:
  explicit NetworkEditor(QWidget* parent = 0);
  void addActions(QWidget* widget);
protected:
  virtual void dropEvent(QDropEvent* event);
  virtual void dragMoveEvent(QDragMoveEvent* event);
private slots:
  void addNode();
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
  typedef QPair<Node*, Node*> NodePair;
  void createActions();
  //void createMenus();
  //void createToolBars();
  void setZValue(int z);
  void setupNode(Node* node);
  Node* selectedNode() const;
  Link* selectedLink() const;
  NodePair selectedNodePair() const;

  //QMenu* fileMenu_;
  //QMenu* editMenu_;
  //QToolBar* editToolBar_;
  QAction* exitAction_;
  QAction* cutAction_;
  QAction* copyAction_;
  QAction* pasteAction_;
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
};

}

#endif