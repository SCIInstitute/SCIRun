/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <iostream>
#include <QtGui>
#include "DiagramWindow.h"
#include "Node.h"
#include "Link.h"
#include "propertiesdialog.h"
#include "PythonConsoleWidget.h"
#include <Interface/PythonTestGui/API/DiagramView.h>
#include <Core/Python/PythonInterpreter.h>

#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>

class DiagramViewImpl : public DiagramViewInterface
{
public:
  explicit DiagramViewImpl(DiagramWindow* window) : window_(window) {}
  virtual int numEdges() const
  {
    return window_->numEdges();
  }
  virtual int numNodes() const
  {
    return window_->numNodes();
  }
  virtual void addNode() 
  {
    window_->addNode();
  }
  virtual std::string removeNode(const std::string& name)
  {
    return window_->removeNode(name);
  }
  virtual std::string firstNodeName() const 
  {
    return listNodeNames().at(0);
  }
  virtual std::vector<std::string> listNodeNames() const
  {
    return window_->listNodeNames();
  }
private:
  DiagramWindow* window_;
};

DiagramWindow::DiagramWindow()
{
  pythonConsole_ = new PythonConsoleWidget( this );

  scene_ = new QGraphicsScene(0, 0, 600, 500);

  view_ = new QGraphicsView;
  view_->setScene(scene_);
  view_->setDragMode(QGraphicsView::RubberBandDrag);
  view_->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  view_->setContextMenuPolicy(Qt::ActionsContextMenu);
  setCentralWidget(view_);

  minZ_ = 0;
  maxZ_ = 0;
  seqNumber_ = 0;

  createActions();
  createMenus();
  createToolBars();

  connect(scene_, SIGNAL(selectionChanged()), this, SLOT(updateActions()));

  setWindowTitle(tr("Diagram"));
  updateActions();

  boost::shared_ptr<DiagramViewInterface> view(new DiagramViewImpl(this));
  DiagramView::setImpl(view);
  SCIRun::Core::PythonInterpreter::Instance().run_string("import PythonAPI; from PythonAPI import *");

  log4cpp::Appender *appender1 = new log4cpp::OstreamAppender("console", &std::cout);
  auto layout = new log4cpp::PatternLayout();
  layout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S.%l} [%p] %m%n");
  appender1->setLayout(layout);

  log4cpp::Category& root = log4cpp::Category::getRoot();
  root.setPriority(log4cpp::Priority::WARN);
  root.addAppender(appender1);
}

void DiagramWindow::addNode()
{
  Node* node = new Node;
  node->setText(tr("Node %1").arg(seqNumber_ + 1));
  setupNode(node);
}

void DiagramWindow::setupNode(Node* node)
{
  node->setPos(QPoint(80 + (100 * (seqNumber_ % 5)), 80 + (50 * ((seqNumber_ / 5) % 7))));
  scene_->addItem(node);
  ++seqNumber_;

  scene_->clearSelection();
  node->setSelected(true);
  bringToFront();
}

void DiagramWindow::bringToFront()
{
  ++maxZ_;
  setZValue(maxZ_);
}

void DiagramWindow::sendToBack()
{
  --minZ_;
  setZValue(minZ_);
}

void DiagramWindow::setZValue(int z)
{
  Node* node = selectedNode();
  if (node)
    node->setZValue(z);
}

Node* DiagramWindow::selectedNode() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 1)
    return dynamic_cast<Node*>(items.first());
  return 0;
}

Link* DiagramWindow::selectedLink() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 1)
    return dynamic_cast<Link*>(items.first());
  return 0;
}

void DiagramWindow::addLink()
{
  NodePair nodes = selectedNodePair();
  if (nodes == NodePair())
    return;

  Link* link = new Link(nodes.first, nodes.second);
  scene_->addItem(link);
}

DiagramWindow::NodePair DiagramWindow::selectedNodePair() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 2)
  {
    Node* first = dynamic_cast<Node*>(items.first());
    Node* second = dynamic_cast<Node*>(items.last());
    if (first && second)
      return NodePair(first, second);
  }
  return NodePair();
}

void DiagramWindow::del()
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  QMutableListIterator<QGraphicsItem*> i(items);
  while (i.hasNext())
  {
    Link* link = dynamic_cast<Link*>(i.next());
    if (link)
    {
      delete link;
      i.remove();
    }
  }
  qDeleteAll(items);
}

void DiagramWindow::properties()
{
  Node* node = selectedNode();
  Link* link = selectedLink();

  if (node)
  {
    PropertiesDialog dialog(node, this);
    dialog.exec();
  }
  else if (link)
  {
    QColor color = QColorDialog::getColor(link->color(), this);
    if (color.isValid())
      link->setColor(color);
  }
}

void DiagramWindow::cut()
{
  Node* node = selectedNode();
  if (!node)
    return;

  copy();
  delete node;
}

void DiagramWindow::copy()
{
  Node* node = selectedNode();
  if (!node)
    return;

  QString str = QString("Node %1 %2 %3 %4")
                .arg(node->textColor().name())
                .arg(node->outlineColor().name())
                .arg(node->backgroundColor().name())
                .arg(node->text());
  QApplication::clipboard()->setText(str);
}

void DiagramWindow::paste()
{
  QString str = QApplication::clipboard()->text();
  QStringList parts = str.split(" ");
  if (parts.count() >= 5 && parts.first() == "Node")
  {
    Node* node = new Node;
    node->setText(QStringList(parts.mid(4)).join(" "));
    node->setTextColor(QColor(parts[1]));
    node->setOutlineColor(QColor(parts[2]));
    node->setBackgroundColor(QColor(parts[3]));
    setupNode(node);
  }
}

void DiagramWindow::updateActions()
{
  bool hasSelection = !scene_->selectedItems().isEmpty();
  bool isNode = (selectedNode() != 0);
  bool isNodePair = (selectedNodePair() != NodePair());

  cutAction_->setEnabled(isNode);
  copyAction_->setEnabled(isNode);
  addLinkAction_->setEnabled(isNodePair);
  deleteAction_->setEnabled(hasSelection);
  bringToFrontAction_->setEnabled(isNode);
  sendToBackAction_->setEnabled(isNode);
  propertiesAction_->setEnabled(isNode);

  Q_FOREACH (QAction* action, view_->actions())
    view_->removeAction(action);

  Q_FOREACH (QAction* action, editMenu_->actions())
  {
    if (action->isEnabled())
      view_->addAction(action);
  }
}

void DiagramWindow::createActions()
{
  exitAction_ = new QAction(tr("E&xit"), this);
  exitAction_->setShortcut(tr("Ctrl+Q"));
  connect(exitAction_, SIGNAL(triggered()), this, SLOT(close()));

  addNodeAction_ = new QAction(tr("Add &Node"), this);
  addNodeAction_->setIcon(QIcon(":/images/node.png"));
  addNodeAction_->setShortcut(tr("Ctrl+N"));
  connect(addNodeAction_, SIGNAL(triggered()), this, SLOT(addNode()));

  addLinkAction_ = new QAction(tr("Add &Link"), this);
  addLinkAction_->setIcon(QIcon(":/images/link.png"));
  addLinkAction_->setShortcut(tr("Ctrl+L"));
  connect(addLinkAction_, SIGNAL(triggered()), this, SLOT(addLink()));

  deleteAction_ = new QAction(tr("&Delete"), this);
  deleteAction_->setIcon(QIcon(":/images/delete.png"));
  deleteAction_->setShortcut(tr("Del"));
  connect(deleteAction_, SIGNAL(triggered()), this, SLOT(del()));

  cutAction_ = new QAction(tr("Cu&t"), this);
  cutAction_->setIcon(QIcon(":/images/cut.png"));
  cutAction_->setShortcut(tr("Ctrl+X"));
  connect(cutAction_, SIGNAL(triggered()), this, SLOT(cut()));

  copyAction_ = new QAction(tr("&Copy"), this);
  copyAction_->setIcon(QIcon(":/images/copy.png"));
  copyAction_->setShortcut(tr("Ctrl+C"));
  connect(copyAction_, SIGNAL(triggered()), this, SLOT(copy()));

  pasteAction_ = new QAction(tr("&Paste"), this);
  pasteAction_->setIcon(QIcon(":/images/paste.png"));
  pasteAction_->setShortcut(tr("Ctrl+V"));
  connect(pasteAction_, SIGNAL(triggered()), this, SLOT(paste()));

  bringToFrontAction_ = new QAction(tr("Bring to &Front"), this);
  bringToFrontAction_->setIcon(QIcon(":/images/bringtofront.png"));
  connect(bringToFrontAction_, SIGNAL(triggered()),
    this, SLOT(bringToFront()));

  sendToBackAction_ = new QAction(tr("&Send to Back"), this);
  sendToBackAction_->setIcon(QIcon(":/images/sendtoback.png"));
  connect(sendToBackAction_, SIGNAL(triggered()),
    this, SLOT(sendToBack()));

  propertiesAction_ = new QAction(tr("P&roperties..."), this);
  connect(propertiesAction_, SIGNAL(triggered()),
    this, SLOT(properties()));

  pythonConsoleAction_ = new QAction(tr("Python Console"), this);
  pythonConsoleAction_->setShortcut( tr( "Ctrl+Shift+Y" ) );
  pythonConsoleAction_->setCheckable( true );
  connect(pythonConsoleAction_, SIGNAL(triggered()),
    this, SLOT(launchPython()));

  countNodesAction_ = new QAction("#Nodes", this);
  connect(countNodesAction_, SIGNAL(triggered()), this, SLOT(printNodeCount()));
  countEdgesAction_ = new QAction("#Edges", this);
  connect(countEdgesAction_, SIGNAL(triggered()), this, SLOT(printEdgeCount()));
}

void DiagramWindow::createMenus()
{
  fileMenu_ = menuBar()->addMenu(tr("&File"));
  fileMenu_->addAction(exitAction_);
  fileMenu_->addAction(pythonConsoleAction_);

  editMenu_ = menuBar()->addMenu(tr("&Edit"));
  editMenu_->addAction(addNodeAction_);
  editMenu_->addAction(addLinkAction_);
  editMenu_->addAction(deleteAction_);
  editMenu_->addSeparator();
  editMenu_->addAction(cutAction_);
  editMenu_->addAction(copyAction_);
  editMenu_->addAction(pasteAction_);
  editMenu_->addSeparator();
  editMenu_->addAction(bringToFrontAction_);
  editMenu_->addAction(sendToBackAction_);
  editMenu_->addSeparator();
  editMenu_->addAction(propertiesAction_);
}

void DiagramWindow::createToolBars()
{
  editToolBar_ = addToolBar(tr("Edit"));
  editToolBar_->addAction(addNodeAction_);
  editToolBar_->addAction(addLinkAction_);
  editToolBar_->addAction(deleteAction_);
  editToolBar_->addSeparator();
  editToolBar_->addAction(cutAction_);
  editToolBar_->addAction(copyAction_);
  editToolBar_->addAction(pasteAction_);
  editToolBar_->addSeparator();
  editToolBar_->addAction(bringToFrontAction_);
  editToolBar_->addAction(sendToBackAction_);
  editToolBar_->addSeparator();
  editToolBar_->addAction(countNodesAction_);
  editToolBar_->addAction(countEdgesAction_);
}

void DiagramWindow::launchPython()
{
  pythonConsole_->setVisible(!pythonConsole_->isVisible());
  pythonConsole_->showBanner();
}

template <class U, class T>
int countOfType(const QList<T*>& list)
{
  auto items = list.toStdList();
  return std::count_if(items.begin(), items.end(), [](T* item) { return dynamic_cast<U*>(item) != 0; });
}

int DiagramWindow::numNodes() const
{
  return countOfType<Node>(scene_->items());
}

void DiagramWindow::printNodeCount()
{
  std::cout << "Number of nodes = " << numNodes() << std::endl;
}

int DiagramWindow::numEdges() const
{
  return countOfType<Link>(scene_->items());
}

void DiagramWindow::printEdgeCount()
{
  std::cout << "Number of edges = " << numEdges() << std::endl;
}

std::vector<std::string> DiagramWindow::listNodeNames() const
{
  auto items = scene_->items().toStdList();
  std::vector<std::string> names;
  std::transform(items.begin(), items.end(), std::back_inserter(names), 
    [](QGraphicsItem* item) -> std::string
    { 
      auto node = dynamic_cast<Node*>(item);
      return node ? node->text().toStdString() : "";
    }
  );
  names.erase(std::remove_if(names.begin(), names.end(), [](const std::string& s) -> bool { return s.empty(); }), names.end());
  return names;
}

std::string DiagramWindow::removeNode(const std::string& name)
{
  QList<QGraphicsItem*> items = scene_->items();
  QMutableListIterator<QGraphicsItem*> i(items);
  while (i.hasNext())
  {
    auto node = dynamic_cast<Node*>(i.next());
    if (node && node->text().toStdString() == name)
    {
      delete node;
      i.remove();
      return "Removed node: " + name;
    }
  }
  return "No such node: " + name;
}