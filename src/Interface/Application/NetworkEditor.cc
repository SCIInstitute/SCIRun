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

#include <sstream>
#include <QtGui>
#include <iostream>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/Node.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/Module.h>
#include <Interface/Application/ModuleProxyWidget.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/Logger.h>

#include <Core/Dataflow/Network/Network.h>
#include <Core/Dataflow/Network/HardCodedModuleFactory.h>
#include <Core/Dataflow/Network/ModuleDescription.h>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Domain::Networks;

boost::shared_ptr<Logger> Logger::Instance;

NetworkEditor::NetworkEditor(boost::shared_ptr<CurrentModuleSelection> moduleSelectionGetter, QWidget* parent) : QGraphicsView(parent),
  moduleSelectionGetter_(moduleSelectionGetter),
  executeAction_(0)
{
  scene_ = new QGraphicsScene(0, 0, 1000, 1000);
  scene_->setBackgroundBrush(Qt::darkGray);
  Port::TheScene = scene_;

  setScene(scene_);
  setDragMode(QGraphicsView::RubberBandDrag);
  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  setContextMenuPolicy(Qt::ActionsContextMenu);

  minZ_ = 0;
  maxZ_ = 0;
  seqNumber_ = 0;

  createActions();

  connect(scene_, SIGNAL(selectionChanged()), this, SLOT(updateActions()));

  updateActions();

  //DOMAIN HOOKUP

  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  theNetwork_.reset(new Network(mf));
}

void NetworkEditor::addModule()
{
  addModule(tr("Module %1").arg(seqNumber_ + 1), QPointF());
}

void NetworkEditor::addModule(const QString& text, const QPointF& pos)
{
  Logger::Instance->log("Module added.");

  ModuleLookupInfo info;
  info.module_name_ = text.toStdString();
  ModuleHandle realModule = theNetwork_->add_module(info);


  ModuleWidget* module = new ModuleWidget("<b><h2>" + text + "</h2></b>", realModule);
  setupModule(module, pos);
}

void NetworkEditor::setupModule(ModuleWidget* module, const QPointF& pos)
{
  ModuleProxyWidget* proxy = new ModuleProxyWidget(module);
  connect(executeAction_, SIGNAL(triggered()), module, SLOT(incrementProgressFake()));
  proxy->setZValue(maxZ_);
  proxy->setVisible(true);
  proxy->setSelected(true);
  proxy->setPos(pos);
  proxy->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
  connect(scene_, SIGNAL(selectionChanged()), proxy, SLOT(highlightIfSelected()));
  connect(proxy, SIGNAL(selected()), this, SLOT(bringToFront()));
  proxy->createPortPositionProviders();

  scene_->addItem(proxy);
  ++seqNumber_;

  scene_->clearSelection();
  proxy->setSelected(true);
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
  ModuleProxyWidget* node = selectedModuleProxy();
  if (node)
    node->setZValue(z);
}

ModuleProxyWidget* getModuleProxy(QGraphicsItem* item)
{
  return dynamic_cast<ModuleProxyWidget*>(item);
}

ModuleWidget* getModule(QGraphicsItem* item)
{
  ModuleProxyWidget* proxy = getModuleProxy(item);
  if (proxy)
    return static_cast<ModuleWidget*>(proxy->widget());
  return 0;
}

//TODO copy/paste
ModuleWidget* NetworkEditor::selectedModule() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 1)
  {
    return getModule(items.first());
  }
  return 0;
}

ModuleProxyWidget* NetworkEditor::selectedModuleProxy() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 1)
  {
    return getModuleProxy(items.first());
  }
  return 0;
}

ConnectionLine* NetworkEditor::selectedLink() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 1)
    return dynamic_cast<ConnectionLine*>(items.first());
  return 0;
}

NetworkEditor::ModulePair NetworkEditor::selectedModulePair() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 2)
  {
    ModuleWidget* first = getModule(items.first());
    ModuleWidget* second = getModule(items.last());
    if (first && second)
      return ModulePair(first, second);
  }
  return ModulePair();
}

void NetworkEditor::del()
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  QMutableListIterator<QGraphicsItem*> i(items);
  while (i.hasNext())
  {
    ConnectionLine* link = dynamic_cast<ConnectionLine*>(i.next());
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
  ModuleWidget* node = selectedModule();
  ConnectionLine* link = selectedLink();

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
  //Module* node = selectedModule();
  //if (!node)
  //  return;

  //copy();
  //delete node;
}

void NetworkEditor::copy()
{
  //Module* node = selectedModule();
  //if (!node)
  //  return;

  //QString str = QString("Module %1 %2 %3 %4")
  //              .arg(node->textColor().name())
  //              .arg(node->outlineColor().name())
  //              .arg(node->backgroundColor().name())
  //              .arg(node->text());
  //QApplication::clipboard()->setText(str);
}

void NetworkEditor::paste()
{
  //QString str = QApplication::clipboard()->text();
  //QStringList parts = str.split(" ");
  //if (parts.count() >= 5 && parts.first() == "Node")
  //{
  //  Module* node = new Module;
  //  node->setText(QStringList(parts.mid(4)).join(" "));
  //  node->setTextColor(QColor(parts[1]));
  //  node->setOutlineColor(QColor(parts[2]));
  //  node->setBackgroundColor(QColor(parts[3]));
  //  setupNode(node);
  //}
}

void NetworkEditor::updateActions()
{
  const bool hasSelection = !scene_->selectedItems().isEmpty();
  const bool isNode = (selectedModule() != 0);
  const bool isLink = (selectedLink() != 0);
  const bool isNodePair = (selectedModulePair() != ModulePair());

  //cutAction_->setEnabled(isNode);
  //copyAction_->setEnabled(isNode);
  //addLinkAction_->setEnabled(isNodePair);
  deleteAction_->setEnabled(hasSelection);
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

  addNodeAction_ = new QAction(tr("Add &Module"), this);
  addNodeAction_->setIcon(QIcon(":/images/node.png"));
  addNodeAction_->setShortcut(tr("Ctrl+N"));
  connect(addNodeAction_, SIGNAL(triggered()), this, SLOT(addModule()));

  //addLinkAction_ = new QAction(tr("Add &Connection"), this);
  //addLinkAction_->setIcon(QIcon(":/images/link.png"));
  //addLinkAction_->setShortcut(tr("Ctrl+L"));
  //connect(addLinkAction_, SIGNAL(triggered()), this, SLOT(addLink()));

  deleteAction_ = new QAction(tr("&Delete"), this);
  deleteAction_->setIcon(QIcon(":/images/delete.png"));
  deleteAction_->setShortcut(tr("Del"));
  connect(deleteAction_, SIGNAL(triggered()), this, SLOT(del()));

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
  //widget->addAction(addLinkAction_);
  widget->addAction(bringToFrontAction_);
  widget->addAction(sendToBackAction_);
  //widget->addAction(cutAction_);
  //widget->addAction(copyAction_);
  //widget->addAction(pasteAction_);
  widget->addAction(deleteAction_);
}

void NetworkEditor::dropEvent(QDropEvent* event)
{
  //TODO: mime check here to ensure this only gets called for drags from treewidget
  if (moduleSelectionGetter_->isModule())
  {
    addModule(moduleSelectionGetter_->text(), mapToScene(event->pos()));
  }
}

void NetworkEditor::dragEnterEvent(QDragEnterEvent* event)
{
  //???
  event->acceptProposedAction();
}

void NetworkEditor::dragMoveEvent(QDragMoveEvent* event)
{
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
