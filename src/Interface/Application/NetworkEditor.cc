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
#include <fstream>
#include <QtGui>
#include <iostream>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/Node.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/ModuleProxyWidget.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/ClosestPortFinder.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h> //TODO: remove
#include <Dataflow/Network/NetworkSettings.h> //TODO: push
#ifdef BUILD_WITH_PYTHON
#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>
#endif

#include <boost/bind.hpp>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Engine;

NetworkEditor::NetworkEditor(boost::shared_ptr<CurrentModuleSelection> moduleSelectionGetter, 
  boost::shared_ptr<DefaultNotePositionGetter> dnpg, boost::shared_ptr<SCIRun::Gui::DialogErrorControl> dialogErrorControl, QWidget* parent) 
  : QGraphicsView(parent),
  scene_(new QGraphicsScene(parent)),
  lastModulePosition_(0,0),
  defaultModulePosition_(0,0),
  moduleSelectionGetter_(moduleSelectionGetter),
  defaultNotePositionGetter_(dnpg),
  moduleEventProxy_(new ModuleEventProxy),
  zLevelManager_(new ZLevelManager(scene_)),
	dialogErrorControl_(dialogErrorControl) 
{
  scene_->setBackgroundBrush(Qt::darkGray);
  ModuleWidget::connectionFactory_.reset(new ConnectionFactory(scene_));
  ModuleWidget::closestPortFinder_.reset(new ClosestPortFinder(scene_));

  setScene(scene_);
  setDragMode(QGraphicsView::RubberBandDrag);
  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  //setContextMenuPolicy(Qt::ActionsContextMenu);

  createActions();

  connect(scene_, SIGNAL(selectionChanged()), this, SLOT(updateActions()));
  connect(scene_, SIGNAL(changed(const QList<QRectF>&)), this, SIGNAL(sceneChanged(const QList<QRectF>&)));

  updateActions();
  ensureVisible(0,0,0,0);
#ifdef BUILD_WITH_PYTHON
  NetworkEditorPythonAPI::setExecutionContext(this);
#endif
}

void NetworkEditor::setNetworkEditorController(boost::shared_ptr<NetworkEditorControllerGuiProxy> controller)
{
  if (controller_ == controller)
    return;

  if (controller_) 
  {
    disconnect(controller_.get(), SIGNAL(moduleAdded(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle)), 
      this, SLOT(addModuleWidget(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle)));

    disconnect(this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)), 
      controller_.get(), SLOT(removeConnection(const SCIRun::Dataflow::Networks::ConnectionId&)));
  }
  
  controller_ = controller;
  
  if (controller_) 
  {
    connect(controller_.get(), SIGNAL(moduleAdded(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle)), 
      this, SLOT(addModuleWidget(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle)));
    
    connect(this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)), 
      controller_.get(), SLOT(removeConnection(const SCIRun::Dataflow::Networks::ConnectionId&)));
  }
}

boost::shared_ptr<NetworkEditorControllerGuiProxy> NetworkEditor::getNetworkEditorController() const
{
  return controller_;
}

void NetworkEditor::addModuleWidget(const std::string& name, SCIRun::Dataflow::Networks::ModuleHandle module)
{
  ModuleWidget* moduleWidget = new ModuleWidget(this, QString::fromStdString(name), module, dialogErrorControl_);
  moduleEventProxy_->trackModule(module);
  
  setupModuleWidget(moduleWidget);
  Q_EMIT modified();
}

boost::shared_ptr<DisableDynamicPortSwitch> NetworkEditor::createDynamicPortDisabler()
{
  return controller_->createDynamicPortSwitch();
}

void NetworkEditor::requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface* from, const SCIRun::Dataflow::Networks::PortDescriptionInterface* to)
{
  controller_->requestConnection(from, to);
  Q_EMIT modified();
}

namespace
{
  ModuleProxyWidget* findById(const QList<QGraphicsItem*>& list, const std::string& id)
  {
    Q_FOREACH(QGraphicsItem* item, list)
    {
      if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
      {
        if (id == w->getModuleWidget()->getModuleId())
          return w;
      }
    }
    return 0;
  }
}

void NetworkEditor::duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle& module)
{
  auto widget = findById(scene_->items(), module->get_id());
  lastModulePosition_ = widget->scenePos() + QPointF(0, 110);
  //TODO: need better duplicate placement. hard code it for now.
  controller_->duplicateModule(module);
}

void NetworkEditor::connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo, const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName)
{
  auto widget = findById(scene_->items(), moduleToConnectTo->get_id());
  QPointF increment(0, portToConnect->isInput() ? -110 : 110);
  lastModulePosition_ = widget->scenePos() + increment;

  controller_->connectNewModule(moduleToConnectTo, portToConnect, newModuleName);
}


namespace 
{
  QPointF moduleAddIncrement(20,90);
}

void NetworkEditor::setupModuleWidget(ModuleWidget* module)
{
  ModuleProxyWidget* proxy = new ModuleProxyWidget(module);

  connect(module, SIGNAL(removeModule(const SCIRun::Dataflow::Networks::ModuleId&)), controller_.get(), SLOT(removeModule(const SCIRun::Dataflow::Networks::ModuleId&)));
  connect(module, SIGNAL(removeModule(const SCIRun::Dataflow::Networks::ModuleId&)), this, SIGNAL(modified()));
  connect(module, SIGNAL(requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const SCIRun::Dataflow::Networks::PortDescriptionInterface*)), 
    this, SLOT(requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const SCIRun::Dataflow::Networks::PortDescriptionInterface*)));
  connect(module, SIGNAL(duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle&)), this, SLOT(duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle&)));
  connect(this, SIGNAL(networkEditorMouseButtonPressed()), module, SIGNAL(cancelConnectionsInProgress()));
  connect(controller_.get(), SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)), 
    module, SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)));
  connect(module, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)), 
    this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)));
  connect(module, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)), this, SIGNAL(modified()));
  connect(module, SIGNAL(connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle&, const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const std::string&)), 
    this, SLOT(connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle&, const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const std::string&)));
  
  if (module->hasDynamicPorts())
  {
    connect(controller_.get(), SIGNAL(portAdded(const SCIRun::Dataflow::Networks::ModuleId&, const SCIRun::Dataflow::Networks::PortId&)), module, SLOT(addDynamicPort(const SCIRun::Dataflow::Networks::ModuleId&, const SCIRun::Dataflow::Networks::PortId&)));
    connect(controller_.get(), SIGNAL(portRemoved(const SCIRun::Dataflow::Networks::ModuleId&, const SCIRun::Dataflow::Networks::PortId&)), module, SLOT(removeDynamicPort(const SCIRun::Dataflow::Networks::ModuleId&, const SCIRun::Dataflow::Networks::PortId&)));
    connect(module, SIGNAL(dynamicPortChanged()), proxy, SLOT(createPortPositionProviders()));
  }
  
  LOG_DEBUG("NetworkEditor connecting to state" << std::endl);
  module->getModule()->get_state()->connect_state_changed(boost::bind(&NetworkEditor::modified, this));
  
  connect(this, SIGNAL(networkExecuted()), module, SLOT(resetLogButtonColor()));
  connect(this, SIGNAL(networkExecuted()), module, SLOT(resetProgressBar()));

  proxy->setZValue(zLevelManager_->max());
  proxy->setVisible(true);
  proxy->setPos(lastModulePosition_);
  lastModulePosition_ += moduleAddIncrement;
  proxy->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
  connect(scene_, SIGNAL(selectionChanged()), proxy, SLOT(highlightIfSelected()));
  connect(proxy, SIGNAL(selected()), this, SLOT(bringToFront()));
  connect(proxy, SIGNAL(widgetMoved(const SCIRun::Dataflow::Networks::ModuleId&, double, double)), this, SIGNAL(modified()));
  connect(proxy, SIGNAL(widgetMoved(const SCIRun::Dataflow::Networks::ModuleId&, double, double)), this, SIGNAL(moduleMoved(const SCIRun::Dataflow::Networks::ModuleId&, double, double)));
  connect(this, SIGNAL(defaultNotePositionChanged(NotePosition)), proxy, SLOT(setDefaultNotePosition(NotePosition)));
 
  proxy->setDefaultNotePosition(defaultNotePositionGetter_->position());
  proxy->createPortPositionProviders();
  
  scene_->addItem(proxy);

  scene_->clearSelection();
  proxy->setSelected(true);
  bringToFront();

  GuiLogger::Instance().log("Module added.");
}

void NetworkEditor::bringToFront()
{
  zLevelManager_->bringToFront();
}

void ZLevelManager::bringToFront()
{
  ++maxZ_;
  setZValue(maxZ_);
}

void NetworkEditor::sendToBack()
{
  zLevelManager_->sendToBack();
}

void ZLevelManager::sendToBack()
{
  --minZ_;
  setZValue(minZ_);
}

void ZLevelManager::setZValue(int z)
{
  ModuleProxyWidget* node = selectedModuleProxy();
  if (node)
  {
    node->setZValue(z);
  }
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

ModuleProxyWidget* ZLevelManager::selectedModuleProxy() const
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
    auto link = dynamic_cast<QGraphicsPathItem*>(i.next());
    if (link)
    {
      delete link;
      i.remove();
    }
  }
  qDeleteAll(items);
  Q_EMIT modified();
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
  //const bool isNodePair = (selectedModulePair() != ModulePair());

  //cutAction_->setEnabled(isNode);
  //copyAction_->setEnabled(isNode);
  //addLinkAction_->setEnabled(isNodePair);
  deleteAction_->setEnabled(hasSelection);
  sendToBackAction_->setEnabled(isNode);
  propertiesAction_->setEnabled(isNode || isLink);

  Q_FOREACH (QAction* action, actions())
    removeAction(action);
}

void NetworkEditor::createActions()
{
  //exitAction_ = new QAction(tr("E&xit"), this);
  //exitAction_->setShortcut(tr("Ctrl+Q"));
  //connect(exitAction_, SIGNAL(triggered()), this, SLOT(close()));

  deleteAction_ = new QAction(tr("&Delete selected objects"), this);
  deleteAction_->setIcon(QIcon(":/images/delete.png"));
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

  sendToBackAction_ = new QAction(tr("&Send selected to back"), this);
  sendToBackAction_->setIcon(QIcon(":/images/sendtoback.png"));
  connect(sendToBackAction_, SIGNAL(triggered()),
    this, SLOT(sendToBack()));

  propertiesAction_ = new QAction(tr("P&roperties..."), this);
  connect(propertiesAction_, SIGNAL(triggered()),
    this, SLOT(properties()));
}

QList<QAction*> NetworkEditor::getModuleSpecificActions() const
{
  return QList<QAction*>() 
    << sendToBackAction_
    << deleteAction_;
  //widget->addAction(addNodeAction_);
  //widget->addAction(addLinkAction_);
  //widget->addAction(cutAction_);
  //widget->addAction(copyAction_);
  //widget->addAction(pasteAction_);
}

void NetworkEditor::dropEvent(QDropEvent* event)
{
  //TODO: mime check here to ensure this only gets called for drags from treewidget
  if (moduleSelectionGetter_->isModule())
  {
    addNewModuleAtPosition(event->pos());
  }
}

void NetworkEditor::addNewModuleAtPosition(const QPoint& position)
{
  lastModulePosition_ = mapToScene(position);
  controller_->addModule(moduleSelectionGetter_->text().toStdString());
  Q_EMIT modified();
}

void NetworkEditor::addModuleViaDoubleClickedTreeItem()
{
  if (moduleSelectionGetter_->isModule())
  {
    defaultModulePosition_ += moduleAddIncrement.toPoint();
    addNewModuleAtPosition(defaultModulePosition_);
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

void NetworkEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton)
		Q_EMIT networkEditorMouseButtonPressed();
	
	if(ConnectionLine* cL = getSingleConnectionSelected())
		if(event->buttons() & Qt::LeftButton)
			if(!(event->modifiers() & Qt::ControlModifier))
		{
			auto selectedPair = cL->getConnectedToModuleId();

			findById(scene_->items(),selectedPair.front())->setSelected(true);
			findById(scene_->items(),selectedPair.back())->setSelected(true);
		}
	QGraphicsView::mouseMoveEvent(event); 
}

void NetworkEditor::mouseReleaseEvent(QMouseEvent *event)
{
	unselectConnectionGroup(); 
	QGraphicsView::mouseReleaseEvent(event);
}

ConnectionLine* NetworkEditor::getSingleConnectionSelected()
{
	ConnectionLine* connectionSelected = 0;
	auto item = scene_->selectedItems();
	if(item.count() == 1 && (connectionSelected = qgraphicsitem_cast<ConnectionLine*>(item.first())))
		return connectionSelected; 
	return connectionSelected; 
}
void NetworkEditor::unselectConnectionGroup()
{
	QList<QGraphicsItem*> items = scene_->selectedItems(); 
	if(items.count() == 3) 
	{
		int hasConnection = 0; 
		int	hasWidgets = 0;
		ConnectionLine* cL; ModuleProxyWidget* mPW;

		Q_FOREACH(QGraphicsItem* item, items)
		{
			if(cL = qgraphicsitem_cast<ConnectionLine*>(item))
			{
				++hasConnection;
				items.push_front(cL); 
			}
			if(mPW = qgraphicsitem_cast<ModuleProxyWidget*>(item))
				++hasWidgets;
		}
		if(hasConnection == 1 && hasWidgets == 2)
		{
			if(cL = qgraphicsitem_cast<ConnectionLine*>(items.first()))
			{
				auto selectedPair = cL->getConnectedToModuleId(); 

				cL->setSelected(false);  
				findById(scene_->items(),selectedPair.front())->setSelected(false);
				findById(scene_->items(),selectedPair.back())->setSelected(false);
			}
		}	
	}
}

SCIRun::Dataflow::Networks::ModulePositionsHandle NetworkEditor::dumpModulePositions() const
{
  ModulePositionsHandle positions(boost::make_shared<ModulePositions>());
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (ModuleProxyWidget* w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      positions->modulePositions[w->getModuleWidget()->getModuleId()] = std::make_pair(item->scenePos().x(), item->scenePos().y());
    }
  }
  return positions;
}

void NetworkEditor::executeAll()
{ 
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (ModuleProxyWidget* w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      w->setAsWaiting();
    }
  }

  controller_->executeAll(*this);
  //TODO: not sure about this right now.
  //Q_EMIT modified();
  Q_EMIT networkExecuted();
}

ExecutableObject* NetworkEditor::lookupExecutable(const ModuleId& id) const
{
  auto widget = findById(scene_->items(), id.id_);
  return widget ? widget->getModuleWidget() : 0;
}

void NetworkEditor::removeModuleWidget(const SCIRun::Dataflow::Networks::ModuleId& id)
{
  auto widget = findById(scene_->items(), id.id_);
  if (widget)
  {
    widget->getModuleWidget()->setDeletedFromGui(false);
    delete widget;
    Q_EMIT modified();
  }
}

void NetworkEditor::clear()
{
  //auto portSwitch = createDynamicPortDisabler();
  scene_->clear();
  //TODO: this (unwritten) method does not need to be called here.  the dtors of all the module widgets get called when the scene_ is cleared, which triggered removal from the underlying network.
  // we'll need a similar hook when programming the scripting interface (moduleWidgets<->modules).
  //controller_->clear();
  Q_EMIT modified();
}

void NetworkEditor::moveModules(const ModulePositions& modulePositions)
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (ModuleProxyWidget* w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      ModulePositions::Data::const_iterator posIter = modulePositions.modulePositions.find(w->getModuleWidget()->getModuleId());
      if (posIter != modulePositions.modulePositions.end())
        w->setPos(posIter->second.first, posIter->second.second);
    }
  }
}

SCIRun::Dataflow::Networks::NetworkFileHandle NetworkEditor::saveNetwork() const
{
  return controller_->saveNetwork();
}

void NetworkEditor::loadNetwork(const SCIRun::Dataflow::Networks::NetworkFileHandle& xml)
{
  controller_->loadNetwork(xml);
}

size_t NetworkEditor::numModules() const
{
  return controller_->numModules();
}

void NetworkEditor::setConnectionPipelineType(int type)
{
  ModuleWidget::connectionFactory_->setType(ConnectionDrawType(type));
}

int NetworkEditor::connectionPipelineType() const
{
  return (int) ModuleWidget::connectionFactory_->getType();
}

int NetworkEditor::errorCode() const
{
  return controller_->errorCode();
}

ModuleEventProxy::ModuleEventProxy()
{
  qRegisterMetaType<std::string>("std::string");
}

void ModuleEventProxy::trackModule(SCIRun::Dataflow::Networks::ModuleHandle module)
{
  module->connectExecuteBegins(boost::bind(&ModuleEventProxy::moduleExecuteStart, this, _1));
  module->connectExecuteEnds(boost::bind(&ModuleEventProxy::moduleExecuteEnd, this, _1));
}

void NetworkEditor::disableInputWidgets()
{
  deleteAction_->setDisabled(true);
}

void NetworkEditor::enableInputWidgets()
{
  deleteAction_->setEnabled(true);
}

void NetworkEditor::setRegressionTestDataDir(const QString& dir)
{
  controller_->getSettings().setValue("regressionTestDataDir", dir.toStdString());
}

void NetworkEditor::setBackground(const QBrush& brush)
{
  scene_->setBackgroundBrush(brush);
}

QBrush NetworkEditor::background() const
{
  return scene_->backgroundBrush();
}

QPixmap NetworkEditor::sceneGrab()
{
  //TODO: this approach may not be able to show the hidden parts of the network.
  return QPixmap::grabWidget(this);
}

void NetworkEditor::selectAll()
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (ModuleProxyWidget* mpw = dynamic_cast<ModuleProxyWidget*>(item))
      mpw->setSelected(true);
  }
}

void NetworkEditor::pinAllModuleUIs()
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    auto module = getModule(item);
    if (module)
      module->pinUI();
  }
}

void NetworkEditor::hideAllModuleUIs()
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    auto module = getModule(item);
    if (module)
      module->hideUI();
  }
}

void NetworkEditor::restoreAllModuleUIs()
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    auto module = getModule(item);
    if (module)
      module->showUI();
  }
}

NetworkEditor::~NetworkEditor()
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    auto module = getModule(item);
    if (module)
      module->setDeletedFromGui(false);
  }
  clear();
}

ZLevelManager::ZLevelManager(QGraphicsScene* scene)
  : scene_(scene), minZ_(INITIAL_Z), maxZ_(INITIAL_Z)
{

}