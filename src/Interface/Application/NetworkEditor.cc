#include <sstream>
#include <QtGui>
#include <iostream>
#include "NetworkEditor.h"
#include "Node.h"
#include "Connection.h"
#include "Module.h"
#include "ModuleProxyWidget.h"

using namespace SCIRun;
using namespace SCIRun::Gui;

template <class Point>
std::string to_string(const Point& p)
{
  std::ostringstream ostr;
  ostr << "QPoint(" << p.x() << "," << p.y() << ")";
  return ostr.str();
}

NetworkEditor::NetworkEditor(CurrentModuleSelection* moduleSelectionGetter, Logger* logger, QWidget* parent) : QGraphicsView(parent),
  moduleSelectionGetter_(moduleSelectionGetter),
  logger_(logger)
{
  scene_ = new QGraphicsScene(0, 0, 600, 500);

  setScene(scene_);
  setDragMode(QGraphicsView::RubberBandDrag);
  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  setContextMenuPolicy(Qt::ActionsContextMenu);

  minZ_ = 0;
  maxZ_ = 0;
  seqNumber_ = 0;

  createActions();
  //createMenus();
  //createToolBars();

  connect(scene_, SIGNAL(selectionChanged()), this, SLOT(updateActions()));

  updateActions();
}

void NetworkEditor::addNode()
{
  addNode(tr("Module %1").arg(seqNumber_ + 1), QPoint());
}

void NetworkEditor::addNode(const QString& text, const QPoint& pos)
{
  //Node* node = new Node;
  //node->setText(text);
  //setupNode(node, pos);
  logger_->log("Node added.");

  auto proxy = new ModuleProxyWidget(new Module("<b><h2>" + text + "</h2></b>"));
  scene_->addItem(proxy);
  proxy->setZValue(maxZ_);
  proxy->setVisible(true);
  proxy->setSelected(true);
  proxy->setPos(pos - QPoint(80,50));
  proxy->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
  connect(scene_, SIGNAL(selectionChanged()), proxy, SLOT(highlightIfSelected()));
  logger_->log("Module Frame: " + to_string(proxy->pos()));
}

void NetworkEditor::setupNode(Node* node, const QPoint& pos)
{
  if (pos.isNull())
    node->setPos(QPoint(80 + (100 * (seqNumber_ % 5)), 80 + (50 * ((seqNumber_ / 5) % 7))));
  else
    node->setPos(pos - QPoint(64,41));
  scene_->addItem(node);
  ++seqNumber_;

  scene_->clearSelection();
  node->setSelected(true);
  bringToFront();
}

void NetworkEditor::bringToFront()
{
  ++maxZ_;
  setZValue(maxZ_);
}

void NetworkEditor::sendToBack()
{
  --minZ_;
  setZValue(minZ_);
}

void NetworkEditor::setZValue(int z)
{
  Node* node = selectedNode();
  if (node)
    node->setZValue(z);
}

Node* NetworkEditor::selectedNode() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 1)
    return dynamic_cast<Node*>(items.first());
  return 0;
}

Link* NetworkEditor::selectedLink() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 1)
    return dynamic_cast<Link*>(items.first());
  return 0;
}

void NetworkEditor::addLink()
{
  NodePair nodes = selectedNodePair();
  if (nodes == NodePair())
    return;

  Link* link = new Link(nodes.first, nodes.second);
  scene_->addItem(link);
}

NetworkEditor::NodePair NetworkEditor::selectedNodePair() const
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

void NetworkEditor::del()
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

void NetworkEditor::properties()
{
  Node* node = selectedNode();
  Link* link = selectedLink();

  if (node)
  {
    //PropertiesDialog dialog(node, this);
    //dialog.exec();
  }
  else if (link)
  {
    //QColor color = QColorDialog::getColor(link->color(), this);
    //if (color.isValid())
    //  link->setColor(color);
  }
}

void NetworkEditor::cut()
{
  Node* node = selectedNode();
  if (!node)
    return;

  copy();
  delete node;
}

void NetworkEditor::copy()
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

void NetworkEditor::paste()
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

void NetworkEditor::updateActions()
{
  const bool hasSelection = !scene_->selectedItems().isEmpty();
  const bool isNode = (selectedNode() != 0);
  const bool isLink = (selectedLink() != 0);
  const bool isNodePair = (selectedNodePair() != NodePair());

  //cutAction_->setEnabled(isNode);
  //copyAction_->setEnabled(isNode);
  addLinkAction_->setEnabled(isNodePair);
  //deleteAction_->setEnabled(hasSelection);
  bringToFrontAction_->setEnabled(isNode);
  sendToBackAction_->setEnabled(isNode);
  propertiesAction_->setEnabled(isNode || isLink);

  foreach (QAction* action, actions())
    removeAction(action);

  //foreach (QAction* action, editToolBar_->actions())
  //{
  //  if (action->isEnabled())
  //    view_->addAction(action);
  //}
}

void NetworkEditor::createActions()
{
  //exitAction_ = new QAction(tr("E&xit"), this);
  //exitAction_->setShortcut(tr("Ctrl+Q"));
  //connect(exitAction_, SIGNAL(triggered()), this, SLOT(close()));

  addNodeAction_ = new QAction(tr("Add &Node"), this);
  addNodeAction_->setIcon(QIcon(":/images/node.png"));
  addNodeAction_->setShortcut(tr("Ctrl+N"));
  connect(addNodeAction_, SIGNAL(triggered()), this, SLOT(addNode()));

  addLinkAction_ = new QAction(tr("Add &Link"), this);
  addLinkAction_->setIcon(QIcon(":/images/link.png"));
  addLinkAction_->setShortcut(tr("Ctrl+L"));
  connect(addLinkAction_, SIGNAL(triggered()), this, SLOT(addLink()));

  //deleteAction_ = new QAction(tr("&Delete"), this);
  //deleteAction_->setIcon(QIcon(":/images/delete.png"));
  //deleteAction_->setShortcut(tr("Del"));
  //connect(deleteAction_, SIGNAL(triggered()), this, SLOT(del()));

  //cutAction_ = new QAction(tr("Cu&t"), this);
  //cutAction_->setIcon(QIcon(":/images/cut.png"));
  //cutAction_->setShortcut(tr("Ctrl+X"));
  //connect(cutAction_, SIGNAL(triggered()), this, SLOT(cut()));

  //copyAction_ = new QAction(tr("&Copy"), this);
  //copyAction_->setIcon(QIcon(":/images/copy.png"));
  //copyAction_->setShortcut(tr("Ctrl+C"));
  //connect(copyAction_, SIGNAL(triggered()), this, SLOT(copy()));

  //pasteAction_ = new QAction(tr("&Paste"), this);
  //pasteAction_->setIcon(QIcon(":/images/paste.png"));
  //pasteAction_->setShortcut(tr("Ctrl+V"));
  //connect(pasteAction_, SIGNAL(triggered()), this, SLOT(paste()));

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
}

void NetworkEditor::addActions(QWidget* widget)
{
  widget->addAction(addNodeAction_);
  widget->addAction(addLinkAction_);
  //widget->addAction(cutAction_);
  //widget->addAction(copyAction_);
  //widget->addAction(pasteAction_);
  //widget->addAction(deleteAction_);
}

void NetworkEditor::dropEvent(QDropEvent* event)
{
  //static int count = 1;
  //std::cout << "drop event " << count++ << std::endl;
  //std::cout << "Currently selected module: " << (*moduleSelectionGetter_)() << std::endl;
  //TODO: mime check here to ensure this only gets called for drags from treewidget
  if (moduleSelectionGetter_->isModule())
  {
    logger_->log(to_string(event->pos()));
    addNode(moduleSelectionGetter_->text().c_str(), event->pos());
  }
}

void NetworkEditor::dragEnterEvent(QDragEnterEvent* event)
{
  //static int count = 1;
  //std::cout << "dragEnterEvent event " << count++ << std::endl;
  //if (event->mimeData()->hasFormat(""))
    event->acceptProposedAction();
    logger_->log(to_string(event->pos()));
}

void NetworkEditor::dragMoveEvent(QDragMoveEvent* event)
{
  //static int count = 1;
  //std::cout << "dragMoveEvent event " << count++ << std::endl;
  logger_->log(to_string(event->pos()));
}

/*
void DiagramWindow::createMenus()
{
  fileMenu_ = menuBar()->addMenu(tr("&File"));
  fileMenu_->addAction(exitAction_);

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

void NetworkEditor::createToolBars()
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
}
*/