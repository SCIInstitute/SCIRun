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

#include <sstream>
#include <QtGui>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/Node.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/ModuleProxyWidget.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/PortWidgetManager.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/ClosestPortFinder.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h> //TODO: remove
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Application/Application.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#ifdef BUILD_WITH_PYTHON
#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>
#endif

#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Engine;

NetworkEditor::NetworkEditor(boost::shared_ptr<CurrentModuleSelection> moduleSelectionGetter,
  boost::shared_ptr<DefaultNotePositionGetter> dnpg,
  boost::shared_ptr<DialogErrorControl> dialogErrorControl,
  PreexecuteFunc preexecuteFunc,
  TagColorFunc tagColor,
  TagNameFunc tagName,
  QWidget* parent)
  : QGraphicsView(parent),
  modulesSelectedByCL_(false),
  currentScale_(1),
  tagLayerActive_(false),
  tagColor_(tagColor),
  tagName_(tagName),
  scene_(new QGraphicsScene(parent)),
  visibleItems_(true),
  lastModulePosition_(0,0),
  dialogErrorControl_(dialogErrorControl),
  moduleSelectionGetter_(moduleSelectionGetter),
  defaultNotePositionGetter_(dnpg),
  moduleEventProxy_(new ModuleEventProxy),
  zLevelManager_(new ZLevelManager(scene_)),
  fileLoading_(false),
  preexecute_(preexecuteFunc)
{
  scene_->setBackgroundBrush(Qt::darkGray);
  ModuleWidget::connectionFactory_.reset(new ConnectionFactory(scene_));
  ModuleWidget::closestPortFinder_.reset(new ClosestPortFinder(scene_));

  setScene(scene_);
  setDragMode(RubberBandDrag);
  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

  connect(scene_, SIGNAL(changed(const QList<QRectF>&)), this, SIGNAL(sceneChanged(const QList<QRectF>&)));

  setSceneRect(QRectF(-1000, -1000, 2000, 2000));
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  centerOn(100, 100);

  setMouseAsDragMode();

#ifdef BUILD_WITH_PYTHON
  NetworkEditorPythonAPI::setExecutionContext(this);
#endif

  connect(this, SIGNAL(moduleMoved(const SCIRun::Dataflow::Networks::ModuleId&, double, double)), this, SLOT(redrawTagGroups()));
}

void NetworkEditor::setNetworkEditorController(boost::shared_ptr<NetworkEditorControllerGuiProxy> controller)
{
  if (controller_ == controller)
    return;

  if (controller_)
  {
    disconnect(controller_.get(), SIGNAL(moduleAdded(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle, const SCIRun::Dataflow::Engine::ModuleCounter&)),
      this, SLOT(addModuleWidget(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle, const SCIRun::Dataflow::Engine::ModuleCounter&)));

    disconnect(this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)),
      controller_.get(), SLOT(removeConnection(const SCIRun::Dataflow::Networks::ConnectionId&)));

    disconnect(controller_.get(), SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)),
      this, SLOT(connectionAddedQueued(const SCIRun::Dataflow::Networks::ConnectionDescription&)));
  }

  controller_ = controller;

  if (controller_)
  {
    connect(controller_.get(), SIGNAL(moduleAdded(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle, const SCIRun::Dataflow::Engine::ModuleCounter&)),
      this, SLOT(addModuleWidget(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle, const SCIRun::Dataflow::Engine::ModuleCounter&)));

    connect(this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)),
      controller_.get(), SLOT(removeConnection(const SCIRun::Dataflow::Networks::ConnectionId&)));

    connect(controller_.get(), SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)),
      this, SLOT(connectionAddedQueued(const SCIRun::Dataflow::Networks::ConnectionDescription&)));
  }
}

boost::shared_ptr<NetworkEditorControllerGuiProxy> NetworkEditor::getNetworkEditorController() const
{
  return controller_;
}

void NetworkEditor::addModuleWidget(const std::string& name, ModuleHandle module, const ModuleCounter& count)
{
  //qDebug() << "addModuleWidget " << module->get_id().id_.c_str();
  latestModuleId_ = module->get_id().id_;
  //std::cout << "\tNE modules done (start): " << *count.count << std::endl;
  ModuleWidget* moduleWidget = new ModuleWidget(this, QString::fromStdString(name), module, dialogErrorControl_);
  moduleEventProxy_->trackModule(module);

  setupModuleWidget(moduleWidget);
  if (!fileLoading_)
  {
    moduleWidget->postLoadAction();
  }
  count.increment();
  //std::cout << "\tNE modules done (end): " << *count.count << std::endl;
  Q_EMIT modified();
  Q_EMIT newModule(QString::fromStdString(module->get_id()), module->has_ui());
}

void NetworkEditor::connectionAddedQueued(const ConnectionDescription& cd)
{
  //std::cout << "Received queued connection request: " << ConnectionId::create(cd).id_ << std::endl;
}

boost::shared_ptr<DisableDynamicPortSwitch> NetworkEditor::createDynamicPortDisabler()
{
  return controller_->createDynamicPortSwitch();
}

boost::optional<ConnectionId> NetworkEditor::requestConnection(const PortDescriptionInterface* from, const PortDescriptionInterface* to)
{
  auto id = controller_->requestConnection(from, to);
  Q_EMIT modified();
  return id;
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
    return nullptr;
  }

  ModuleProxyWidget* findFirstByName(const QList<QGraphicsItem*>& list, const std::string& name)
  {
    Q_FOREACH(QGraphicsItem* item, list)
    {
      if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
      {
        if (w->getModuleWidget()->getModuleId().find(name) != std::string::npos)
          return w;
      }
    }
    return nullptr;
  }
}

void NetworkEditor::duplicateModule(const ModuleHandle& module)
{
  auto widget = findById(scene_->items(), module->get_id());
  lastModulePosition_ = widget->scenePos() + QPointF(0, 110);
  //TODO: need better duplicate placement. hard code it for now.
  controller_->duplicateModule(module);
}

namespace
{
  QPointF moduleAddIncrement(20, 20);
}

void NetworkEditor::connectNewModule(const ModuleHandle& moduleToConnectTo, const PortDescriptionInterface* portToConnect, const std::string& newModuleName)
{
  auto prop = sender()->property(addNewModuleActionTypePropertyName());

  auto widget = findById(scene_->items(), moduleToConnectTo->get_id());
  QPointF increment(0, portToConnect->isInput() ? -110 : 50);
  lastModulePosition_ = widget->scenePos() + increment;
  moduleAddIncrement = { 20.0, portToConnect->isInput() ? -20.0 : 20.0 };

  PortWidget* newConnectionInputPort = nullptr;
  auto q = dynamic_cast<const PortWidget*>(portToConnect);
  if (q)
  {
    for (size_t i = 0; i < q->nconnections(); ++i)
    {
      auto cpi = q->connectedPorts()[i];
      if (QString::fromStdString(cpi->id().toString()) == sender()->property(insertNewModuleActionTypePropertyName()))
        newConnectionInputPort = cpi;
    }
  }

  if (newConnectionInputPort)
  {
    controller_->removeConnection(*newConnectionInputPort->firstConnectionId());
    newConnectionInputPort->deleteConnectionsLater();
  }

  controller_->connectNewModule(portToConnect, newModuleName, newConnectionInputPort);
}

void NetworkEditor::replaceModuleWith(const ModuleHandle& moduleToReplace, const std::string& newModuleName)
{
  auto oldModule = findById(scene_->items(), moduleToReplace->get_id());
  lastModulePosition_ = oldModule->scenePos();
  controller_->addModule(newModuleName);

  // connect up same ports
  auto newModule = findById(scene_->items(), latestModuleId_);

  const auto& oldModPorts = oldModule->getModuleWidget()->ports();
  const auto& newModPorts = newModule->getModuleWidget()->ports();

  {
    int nextInputIndex = 0;
    for (const auto& iport : oldModPorts.inputs())
    {
      if (iport->isConnected())
      {
        const auto& newInputs = newModPorts.inputs();
        auto toConnect = std::find_if(newInputs.begin(), newInputs.end(),
          [&](const PortWidget* port) { return port->get_typename() == iport->get_typename() && port->getIndex() >= nextInputIndex; });
        if (toConnect == newInputs.end())
        {
          qDebug() << "Logical error: could not find input port to connect to" << iport << nextInputIndex;
          break;
        }
        requestConnection(iport->connectedPorts()[0], *toConnect);
        nextInputIndex = (*toConnect)->getIndex() + 1;
      }
    }
  }

  {
    int nextOutputIndex = 0;
    auto newOutputs = newModPorts.outputs();
    for (const auto& oport : oldModPorts.outputs())
    {
      if (oport->isConnected())
      {
        auto toConnect = std::find_if(newOutputs.begin(), newOutputs.end(),
          [&](const PortWidget* port) { return port->get_typename() == oport->get_typename() && port->getIndex() >= nextOutputIndex; });
        if (toConnect == newOutputs.end())
        {
          qDebug() << "Logical error: could not find output port to connect to" << oport;
          break;
        }
        auto connectedPorts = oport->connectedPorts();
        std::vector<PortWidget*> dynamicPortsNeedSpecialHandling;
        std::copy_if(connectedPorts.begin(), connectedPorts.end(), std::back_inserter(dynamicPortsNeedSpecialHandling), [](const PortWidget* p) { return p->isDynamic(); });
        connectedPorts.erase(std::remove_if(connectedPorts.begin(), connectedPorts.end(), [](const PortWidget* p) { return p->isDynamic(); }), connectedPorts.end());
        oport->deleteConnections();
        for (const auto& connected : connectedPorts)
        {
          requestConnection(connected, *toConnect);
        }
        nextOutputIndex = (*toConnect)->getIndex() + 1;
      }
    }
  }

  oldModule->deleteLater();
}

void NetworkEditor::setupModuleWidget(ModuleWidget* module)
{
  auto proxy = new ModuleProxyWidget(module);

  connect(module, SIGNAL(removeModule(const SCIRun::Dataflow::Networks::ModuleId&)), controller_.get(), SLOT(removeModule(const SCIRun::Dataflow::Networks::ModuleId&)));
  connect(module, SIGNAL(interrupt(const SCIRun::Dataflow::Networks::ModuleId&)), controller_.get(), SLOT(interrupt(const SCIRun::Dataflow::Networks::ModuleId&)));
  connect(module, SIGNAL(removeModule(const SCIRun::Dataflow::Networks::ModuleId&)), this, SIGNAL(modified()));
  connect(module, SIGNAL(noteChanged()), this, SIGNAL(modified()));
  connect(module, SIGNAL(executionDisabled(bool)), this, SIGNAL(modified()));
  connect(module, SIGNAL(requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const SCIRun::Dataflow::Networks::PortDescriptionInterface*)),
    this, SLOT(requestConnection(const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const SCIRun::Dataflow::Networks::PortDescriptionInterface*)));
  connect(module, SIGNAL(duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle&)), this, SLOT(duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle&)));
  connect(this, SIGNAL(networkEditorMouseButtonPressed()), module, SIGNAL(cancelConnectionsInProgress()));
  connect(controller_.get(), SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)),
    module, SIGNAL(connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription&)));
  connect(module, SIGNAL(executedManually(const SCIRun::Dataflow::Networks::ModuleHandle&)),
    this, SLOT(executeModule(const SCIRun::Dataflow::Networks::ModuleHandle&)));
  connect(module, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)),
    this, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)));
  connect(module, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)), this, SIGNAL(modified()));
  connect(module, SIGNAL(connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle&, const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const std::string&)),
    this, SLOT(connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle&, const SCIRun::Dataflow::Networks::PortDescriptionInterface*, const std::string&)));
  connect(module, SIGNAL(replaceModuleWith(const SCIRun::Dataflow::Networks::ModuleHandle&, const std::string&)),
    this, SLOT(replaceModuleWith(const SCIRun::Dataflow::Networks::ModuleHandle&, const std::string&)));
  connect(module, SIGNAL(disableWidgetDisabling()), this, SIGNAL(disableWidgetDisabling()));
  connect(module, SIGNAL(reenableWidgetDisabling()), this, SIGNAL(reenableWidgetDisabling()));

  if (module->hasDynamicPorts())
  {
    connect(controller_.get(), SIGNAL(portAdded(const SCIRun::Dataflow::Networks::ModuleId&, const SCIRun::Dataflow::Networks::PortId&)), module, SLOT(addDynamicPort(const SCIRun::Dataflow::Networks::ModuleId&, const SCIRun::Dataflow::Networks::PortId&)));
    connect(controller_.get(), SIGNAL(portRemoved(const SCIRun::Dataflow::Networks::ModuleId&, const SCIRun::Dataflow::Networks::PortId&)), module, SLOT(removeDynamicPort(const SCIRun::Dataflow::Networks::ModuleId&, const SCIRun::Dataflow::Networks::PortId&)));
    connect(module, SIGNAL(dynamicPortChanged(const std::string&, bool)), proxy, SLOT(createPortPositionProviders()));
  }

  LOG_DEBUG("NetworkEditor connecting to state" << std::endl);
  module->getModule()->get_state()->connect_state_changed(boost::bind(&NetworkEditor::modified, this));

  connect(this, SIGNAL(networkExecuted()), module, SLOT(resetLogButtonColor()));
  connect(this, SIGNAL(networkExecuted()), module, SLOT(resetProgressBar()));

  proxy->setZValue(zLevelManager_->get_max());

  while (!scene_->items(lastModulePosition_.x() - 20, lastModulePosition_.y() - 20, 40, 40).isEmpty())
  {
    lastModulePosition_ += moduleAddIncrement;
  }
  proxy->setPos(lastModulePosition_);

  proxy->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
  connect(scene_, SIGNAL(selectionChanged()), proxy, SLOT(highlightIfSelected()));
  connect(proxy, SIGNAL(selected()), this, SLOT(bringToFront()));
  connect(proxy, SIGNAL(widgetMoved(const SCIRun::Dataflow::Networks::ModuleId&, double, double)), this, SIGNAL(modified()));
  connect(proxy, SIGNAL(widgetMoved(const SCIRun::Dataflow::Networks::ModuleId&, double, double)), this, SIGNAL(moduleMoved(const SCIRun::Dataflow::Networks::ModuleId&, double, double)));
  connect(this, SIGNAL(snapToModules()), proxy, SLOT(snapToGrid()));
  connect(this, SIGNAL(highlightPorts(int)), proxy, SLOT(highlightPorts(int)));
  connect(this, SIGNAL(resetModulesDueToCycle()), module, SLOT(changeExecuteButtonToPlay()));
  connect(this, SIGNAL(defaultNotePositionChanged(NotePosition)), proxy, SLOT(setDefaultNotePosition(NotePosition)));
  connect(module, SIGNAL(displayChanged()), this, SLOT(updateViewport()));
  connect(module, SIGNAL(displayChanged()), proxy, SLOT(createPortPositionProviders()));
  connect(proxy, SIGNAL(tagChanged(int)), this, SLOT(highlightTaggedItem(int)));

  proxy->setDefaultNotePosition(defaultNotePositionGetter_->position());
  proxy->createPortPositionProviders();
  proxy->highlightPorts(Preferences::Instance().highlightPorts ? 1 : 0);

  scene_->addItem(proxy);
  ensureVisible(proxy);
  proxy->createStartupNote();

  scene_->clearSelection();
  proxy->setSelected(true);
  bringToFront();
  proxy->setVisible(visibleItems_);

  GuiLogger::Instance().logInfoStd("Module added: " + module->getModuleId());
}

void NetworkEditor::setMouseAsDragMode()
{
  setDragMode(ScrollHandDrag);
}

void NetworkEditor::setMouseAsSelectMode()
{
  setDragMode(RubberBandDrag);
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
  auto node = selectedModuleProxy();
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
  auto proxy = getModuleProxy(item);
  if (proxy)
    return static_cast<ModuleWidget*>(proxy->widget());
  return nullptr;
}

void NetworkEditor::setVisibility(bool visible)
{
  visibleItems_ = visible;
  ModuleWidget::connectionFactory_->setVisibility(visible);
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto p = getModuleProxy(item))
      p->setVisible(visibleItems_);
    else if (auto c = dynamic_cast<ConnectionLine*>(item))
      c->setVisible(visibleItems_);
  }
}

//TODO copy/paste
ModuleWidget* NetworkEditor::selectedModule() const
{
  auto items = scene_->selectedItems();
  if (items.count() == 1)
  {
    return getModule(items.first());
  }
  return nullptr;
}

ModuleProxyWidget* ZLevelManager::selectedModuleProxy() const
{
  auto items = scene_->selectedItems();
  if (items.count() == 1)
  {
    return getModuleProxy(items.first());
  }
  return nullptr;
}

ConnectionLine* NetworkEditor::selectedLink() const
{
  auto items = scene_->selectedItems();
  if (items.count() == 1)
    return dynamic_cast<ConnectionLine*>(items.first());
  return nullptr;
}

NetworkEditor::ModulePair NetworkEditor::selectedModulePair() const
{
  auto items = scene_->selectedItems();
  if (items.count() == 2)
  {
    auto first = getModule(items.first());
    auto second = getModule(items.last());
    if (first && second)
		return ModulePair(first, second);
  }
  return ModulePair();
}

void NetworkEditor::del()
{
  auto items = scene_->selectedItems();
  QMutableListIterator<QGraphicsItem*> i(items);
  while (i.hasNext())
  {
    auto link = dynamic_cast<QGraphicsPathItem*>(i.next());
    if (link)
    {
      scene()->removeItem(link);
      delete link;
      i.remove();
    }
  }
  qDeleteAll(items);
  updateViewport();
  Q_EMIT modified();
}

void NetworkEditor::cut()
{
  copy();
  del();
}

void NetworkEditor::copy()
{
  auto selected = scene_->selectedItems();
  auto modSelected = [&selected](ModuleHandle mod)
  {
    for (const auto& item : selected)
    {
      if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
      {
        if (w->getModuleWidget()->getModuleId() == mod->get_id().id_)
          return true;
      }
    }
    return false;
  };
  auto connSelected = [&selected](const ConnectionDescription& conn)
  {
    for (const auto& item : selected)
    {
      if (auto connLine = dynamic_cast<ConnectionLine*>(item))
      {
        if (connLine->id().describe() == conn)
          return true;
      }
    }
    return false;
  };

  auto file = controller_->serializeNetworkFragment(modSelected, connSelected);

  if (file)
  {
    //TODO encapsulate
    std::ostringstream ostr;
    XMLSerializer::save_xml(*file, ostr, "networkFragment");
    auto xml = QString::fromStdString(ostr.str());

    QApplication::clipboard()->setText(xml);
    Q_EMIT newSubnetworkCopied(xml);
  }
  else
  {
    qDebug() << "null net fragment";
  }
}

void NetworkEditor::paste()
{
  pasteImpl(QApplication::clipboard()->text());
}

void NetworkEditor::pasteImpl(const QString& xml)
{
  std::istringstream istr(xml.toStdString());
  try
  {
    appendToNetwork(XMLSerializer::load_xml<NetworkFile>(istr));
  }
  catch (...)
  {
    QMessageBox::critical(this, "Paste error", "Invalid clipboard contents: " + xml);
  }
}

void NetworkEditor::contextMenuEvent(QContextMenuEvent *event)
{
  //TODO: this menu needs to check for certain editing conditions. Disabling for now.
  if (false)
  {
    auto items = scene_->items(mapToScene(event->pos()));
    if (items.isEmpty())
    {
      QMenu menu(this);
      menu.addActions(actions());
      menu.exec(event->globalPos());
    }
  }
}

void NetworkEditor::dropEvent(QDropEvent* event)
{
  //TODO: mime check here to ensure this only gets called for drags from treewidget
  if (moduleSelectionGetter_->isModule())
  {
    addNewModuleAtPosition(mapToScene(event->pos()));
  }
  else if (moduleSelectionGetter_->isClipboardXML())
    pasteImpl(moduleSelectionGetter_->clipboardXML());
}

void NetworkEditor::addNewModuleAtPosition(const QPointF& position)
{
  lastModulePosition_ = position;
  controller_->addModule(moduleSelectionGetter_->text().toStdString());
  Q_EMIT modified();
}

void NetworkEditor::addModuleViaDoubleClickedTreeItem()
{
  if (moduleSelectionGetter_->isModule())
  {
    auto upperLeft = mapToScene(viewport()->geometry()).boundingRect().center();
    addNewModuleAtPosition(upperLeft);
  }
  else if (moduleSelectionGetter_->isClipboardXML())
    pasteImpl(moduleSelectionGetter_->clipboardXML());
}

void NetworkEditor::dragEnterEvent(QDragEnterEvent* event)
{
  //???
  event->acceptProposedAction();
}

void NetworkEditor::dragMoveEvent(QDragMoveEvent* event)
{
}

void NetworkEditor::updateViewport()
{
  viewport()->update();
}

void NetworkEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton)
		Q_EMIT networkEditorMouseButtonPressed();

  if (auto cL = getSingleConnectionSelected())
  {
    if (event->buttons() & Qt::LeftButton)
    {
      if (!(event->modifiers() & Qt::ControlModifier))
      {
        auto selectedPair = cL->getConnectedToModuleIds();

        findById(scene_->items(), selectedPair.first)->setSelected(true);
        findById(scene_->items(), selectedPair.second)->setSelected(true);
        modulesSelectedByCL_ = true;
      }
    }
  }
  QGraphicsView::mouseMoveEvent(event);
}

void NetworkEditor::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::MiddleButton)
    Q_EMIT middleMouseClicked();
  QGraphicsView::mousePressEvent(event);
}

void NetworkEditor::mouseReleaseEvent(QMouseEvent *event)
{
  if (modulesSelectedByCL_)
  {
    unselectConnectionGroup();
    Q_EMIT modified();
  }
  modulesSelectedByCL_ = false;
  QGraphicsView::mouseReleaseEvent(event);
}

ConnectionLine* NetworkEditor::getSingleConnectionSelected()
{
	ConnectionLine* connectionSelected = nullptr;
	auto item = scene_->selectedItems();
	if(item.count() == 1 && (connectionSelected = qgraphicsitem_cast<ConnectionLine*>(item.first())))
		return connectionSelected;
	return connectionSelected;
}

void NetworkEditor::unselectConnectionGroup()
{
  auto items = scene_->selectedItems();
	if (items.count() == 3)
	{
		int hasConnection = 0;
		int	hasWidgets = 0;

		Q_FOREACH(QGraphicsItem* item, items)
		{
			if (auto cL = qgraphicsitem_cast<ConnectionLine*>(item))
			{
				++hasConnection;
				items.push_front(cL);
			}
			if (qgraphicsitem_cast<ModuleProxyWidget*>(item))
				++hasWidgets;
		}
		if(hasConnection == 1 && hasWidgets == 2)
		{
			if (auto cL = qgraphicsitem_cast<ConnectionLine*>(items.first()))
			{
				auto selectedPair = cL->getConnectedToModuleIds();

				cL->setSelected(false);
				findById(scene_->items(),selectedPair.first)->setSelected(false);
				findById(scene_->items(),selectedPair.second)->setSelected(false);
			}
		}
	}
}

ModulePositionsHandle NetworkEditor::dumpModulePositions(ModuleFilter filter) const
{
  auto positions(boost::make_shared<ModulePositions>());
  fillModulePositionMap(*positions, filter);
  return positions;
}

void NetworkEditor::fillModulePositionMap(ModulePositions& positions, ModuleFilter filter) const
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      if (filter(w->getModuleWidget()->getModule()))
        positions.modulePositions[w->getModuleWidget()->getModuleId()] = std::make_pair(item->scenePos().x(), item->scenePos().y());
    }
  }
}

void NetworkEditor::centerView()
{
  ModulePositions positions;
  fillModulePositionMap(positions, boost::lambda::constant(true));
  centerOn(findCenterOfNetwork(positions));
}

ModuleNotesHandle NetworkEditor::dumpModuleNotes(ModuleFilter filter) const
{
  auto notes(boost::make_shared<ModuleNotes>());
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      auto note = w->currentNote();
      if (filter(w->getModuleWidget()->getModule()) &&
        !note.plainText_.isEmpty())
        notes->notes[w->getModuleWidget()->getModuleId()] = NoteXML(note.html_.toStdString(), note.position_, note.plainText_.toStdString(), note.fontSize_);
    }
  }
  return notes;
}

namespace
{
  std::string connectionNoteId(const ModuleIdPair& ms)
  {
    return ms.first.id_ + "--" + ms.second.id_;
  }
}

ConnectionNotesHandle NetworkEditor::dumpConnectionNotes(ConnectionFilter filter) const
{
  auto notes(boost::make_shared<ConnectionNotes>());
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto conn = dynamic_cast<ConnectionLine*>(item))
    {
      auto note = conn->currentNote();
      if (filter(conn->id().describe()) &&
        !note.plainText_.isEmpty())
      {
        //TODO hacky
        auto id = connectionNoteId(conn->getConnectedToModuleIds());
        notes->notes[id] = NoteXML(note.html_.toStdString(), note.position_, note.plainText_.toStdString(), note.fontSize_);
      }
    }
  }
  return notes;
}

ModuleTagsHandle NetworkEditor::dumpModuleTags(ModuleFilter filter) const
{
  auto tags(boost::make_shared<ModuleTags>());
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto mod = dynamic_cast<ModuleProxyWidget*>(item))
    {
      if (filter(mod->getModuleWidget()->getModule()))
        tags->tags[mod->getModuleWidget()->getModuleId()] = mod->data(TagDataKey).toInt();
    }
  }
  return tags;
}

DisabledComponentsHandle NetworkEditor::dumpDisabledComponents(ModuleFilter modFilter, ConnectionFilter connFilter) const
{
  auto disabled(boost::make_shared<DisabledComponents>());
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto mod = dynamic_cast<ModuleProxyWidget*>(item))
    {
      if (mod->getModuleWidget()->executionDisabled() && modFilter(mod->getModuleWidget()->getModule()))
        disabled->disabledModules.push_back(mod->getModuleWidget()->getModuleId());
    }
    if (auto conn = dynamic_cast<ConnectionLine*>(item))
    {
      if (conn->disabled() && connFilter(conn->id().describe()))
      {
        disabled->disabledConnections.emplace_back(connectionNoteId(conn->getConnectedToModuleIds()));
      }
    }
  }
  return disabled;
}

void NetworkEditor::updateModulePositions(const ModulePositions& modulePositions)
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      auto posIter = modulePositions.modulePositions.find(w->getModuleWidget()->getModuleId());
      if (posIter != modulePositions.modulePositions.end())
      {
        w->setPos(posIter->second.first, posIter->second.second);
        ensureVisible(w);
      }
    }
  }
}

void NetworkEditor::updateModuleNotes(const ModuleNotes& moduleNotes)
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      auto noteIter = moduleNotes.notes.find(w->getModuleWidget()->getModuleId());
      if (noteIter != moduleNotes.notes.end())
      {
        auto noteXML = noteIter->second;
        Note note(QString::fromStdString(noteXML.noteHTML), QString::fromStdString(noteXML.noteText), noteXML.fontSize, noteXML.position);
        w->getModuleWidget()->updateNoteFromFile(note);
      }
    }
  }
}

void NetworkEditor::updateModuleTags(const ModuleTags& moduleTags)
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      auto tagIter = moduleTags.tags.find(w->getModuleWidget()->getModuleId());
      if (tagIter != moduleTags.tags.end())
      {
        w->setData(TagDataKey, tagIter->second);
      }
    }
  }
}

void NetworkEditor::updateConnectionNotes(const ConnectionNotes& notes)
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto conn = dynamic_cast<ConnectionLine*>(item))
    {
      auto id = connectionNoteId(conn->getConnectedToModuleIds());
      auto noteIter = notes.notes.find(id);
      if (noteIter != notes.notes.end())
      {
        auto noteXML = noteIter->second;
        Note note(QString::fromStdString(noteXML.noteHTML), QString::fromStdString(noteXML.noteText), noteXML.fontSize, noteXML.position);
        conn->updateNoteFromFile(note);
      }
    }
  }
}

void NetworkEditor::updateDisabledComponents(const DisabledComponents& disabled)
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto conn = dynamic_cast<ConnectionLine*>(item))
    {
      auto id = connectionNoteId(conn->getConnectedToModuleIds());
      if (std::find(disabled.disabledConnections.begin(), disabled.disabledConnections.end(), id) != disabled.disabledConnections.end())
      {
        conn->setDisabled(true);
      }
    }

    if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      if (std::find(disabled.disabledModules.begin(), disabled.disabledModules.end(), w->getModuleWidget()->getModuleId()) != disabled.disabledModules.end())
      {
        w->getModuleWidget()->setExecutionDisabled(true);
      }
    }
  }
}

void NetworkEditor::executeAll()
{
  preexecute_();
  // explicit type needed for older Qt and/or clang
  std::function<void()> exec = [this]() { controller_->executeAll(*this); };
  QtConcurrent::run(exec);

  //TODO: not sure about this right now.
  //Q_EMIT modified();
  Q_EMIT networkExecuted();
}

void NetworkEditor::executeModule(const ModuleHandle& module)
{
  preexecute_();
  // explicit type needed for older Qt and/or clang
  std::function<void()> exec = [this, &module]() { controller_->executeModule(module, *this); };
  QtConcurrent::run(exec);
  //TODO: not sure about this right now.
  //Q_EMIT modified();
  Q_EMIT networkExecuted();
}

ExecutableObject* NetworkEditor::lookupExecutable(const ModuleId& id) const
{
  auto widget = findById(scene_->items(), id.id_);
  return widget ? widget->getModuleWidget() : 0;
}

void NetworkEditor::resetNetworkDueToCycle()
{
  Q_EMIT resetModulesDueToCycle();
  //TODO: ??reset module colors--right now they stay yellow
}

void NetworkEditor::removeModuleWidget(const ModuleId& id)
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
  ModuleWidget::NetworkClearingScope clearing;
  //auto portSwitch = createDynamicPortDisabler();
  scene_->clear();
  //TODO: this (unwritten) method does not need to be called here.  the dtors of all the module widgets get called when the scene_ is cleared, which triggered removal from the underlying network.
  // we'll need a similar hook when programming the scripting interface (moduleWidgets<->modules).
  //controller_->clear();
  Q_EMIT modified();
}

NetworkFileHandle NetworkEditor::saveNetwork() const
{
  return controller_->saveNetwork();
}

void NetworkEditor::loadNetwork(const NetworkFileHandle& xml)
{
  fileLoading_ = true;
  controller_->loadNetwork(xml);
  fileLoading_ = false;

  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      w->getModuleWidget()->postLoadAction();
    }
  }

  setSceneRect(QRectF());
}

void NetworkEditor::appendToNetwork(const NetworkFileHandle& xml)
{
  auto originalItems = scene_->items();
  fileLoading_ = true;
  controller_->appendToNetwork(xml);
  fileLoading_ = false;

  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (!originalItems.contains(item))
      if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
      {
        w->getModuleWidget()->postLoadAction();
      }
  }

  setSceneRect(QRectF());
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
  return static_cast<int>(ModuleWidget::connectionFactory_->getType());
}

int NetworkEditor::errorCode() const
{
  return controller_->errorCode();
}

ModuleEventProxy::ModuleEventProxy()
{
  qRegisterMetaType<std::string>("std::string");
  qRegisterMetaType<SCIRun::Dataflow::Networks::ModuleHandle>("SCIRun::Dataflow::Networks::ModuleHandle");
  qRegisterMetaType<SCIRun::Dataflow::Networks::ConnectionDescription>("SCIRun::Dataflow::Networks::ConnectionDescription");
  qRegisterMetaType<SCIRun::Dataflow::Networks::ModuleId>("SCIRun::Dataflow::Networks::ModuleId");
  qRegisterMetaType<SCIRun::Dataflow::Networks::ConnectionId>("SCIRun::Dataflow::Networks::ConnectionId");
  qRegisterMetaType<SCIRun::Dataflow::Engine::ModuleCounter>("SCIRun::Dataflow::Engine::ModuleCounter");
}

void ModuleEventProxy::trackModule(ModuleHandle module)
{
  module->connectExecuteBegins([this](const std::string& id) { moduleExecuteStart(id); });
  module->connectExecuteEnds([this](double t, const std::string& id) { moduleExecuteEnd(t, id); });
}

void NetworkEditor::disableInputWidgets()
{
  //deleteAction_->setDisabled(true);
}

void NetworkEditor::enableInputWidgets()
{
  //deleteAction_->setEnabled(true);
}

void NetworkEditor::setBackground(const QBrush& brush)
{
  scene_->setBackgroundBrush(brush);
  Preferences::Instance().networkBackgroundColor.setValue(brush.color().name().toStdString());
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
    //if (ModuleProxyWidget* mpw = dynamic_cast<ModuleProxyWidget*>(item))
    //mpw->setSelected(true);
    item->setSelected(true);
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

namespace
{
  const double minScale = 0.03;
  const double maxScale = 4.0;
  const double scaleFactor = 1.15;
}

void NetworkEditor::wheelEvent(QWheelEvent* event)
{
  if (event->modifiers() & Qt::ShiftModifier)
  {
    setTransformationAnchor(AnchorUnderMouse);

    if (event->delta() > 0)
    {
      zoomIn();
    }
    else
    {
      zoomOut();
    }
    // Don't call superclass handler here
    // as wheel is normally used for moving scrollbars
  }
  else
    QGraphicsView::wheelEvent(event);
}

void NetworkEditor::zoomIn()
{
  if (currentScale_ < maxScale)
  {
    double factor = std::min(scaleFactor, 4.0/currentScale_);
    scale(factor, factor);
    currentScale_ *= factor;
    Q_EMIT zoomLevelChanged(currentZoomPercentage());
  }
}

void NetworkEditor::zoomOut()
{
  if (currentScale_ > minScale)
  {
    scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    currentScale_ /= scaleFactor;
    Q_EMIT zoomLevelChanged(currentZoomPercentage());
  }
}

void NetworkEditor::zoomReset()
{
  scale(1.0 / currentScale_, 1.0 / currentScale_);
  currentScale_ = 1;
  Q_EMIT zoomLevelChanged(currentZoomPercentage());
}

void NetworkEditor::zoomBestFit()
{
  //TODO not quite working yet.
  auto oldRect = sceneRect();
  setSceneRect(QRectF());
  fitInView(sceneRect(), Qt::KeepAspectRatio);
  //qDebug() << "old rect: " << oldRect << "new rect:" << sceneRect();
  currentScale_ *= sceneRect().x() / oldRect.x();
  //scale(1.0 / currentScale_, 1.0 / currentScale_);
  //currentScale_ = 1;
  Q_EMIT zoomLevelChanged(currentZoomPercentage());
}

int NetworkEditor::currentZoomPercentage() const
{
  return static_cast<int>(currentScale_ * 100);
}

bool NetworkEditor::containsViewScene() const
{
  return findFirstByName(scene_->items(), "ViewScene") != nullptr;
}

void NetworkEditor::moduleWindowAction()
{
  auto action = qobject_cast<QAction*>(sender());
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    auto module = getModule(item);
    if (module && module->getModuleId() == action->text().toStdString())
    {
      if (module->guiVisible())
        module->hideUI();
      else
        module->showUI();
      break;
    }
  }
}

void NetworkEditor::adjustModuleWidth(int delta)
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    auto proxy = getModuleProxy(item);
    if (proxy)
    {
      proxy->adjustWidth(delta);
      //qDebug() << module->size() << proxy->minimumSize() << proxy->maximumSize() << proxy->preferredSize();
    }
  }
}

void NetworkEditor::adjustModuleHeight(int delta)
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    auto proxy = getModuleProxy(item);
    if (proxy)
    {
      proxy->adjustHeight(delta);
      //qDebug() << module->size() << proxy->minimumSize() << proxy->maximumSize() << proxy->preferredSize();
    }
  }
}

void NetworkEditor::setModuleMini(bool mini)
{
  ModuleWidget::setGlobalMiniMode(mini);
  for (const auto& item : scene_->items())
  {
    auto module = getModule(item);
    if (module)
      module->setMiniMode(mini);
  }
}

void NetworkEditor::metadataLayer(bool active)
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    item->setOpacity(active ? 0.4 : 1);
    auto module = getModule(item);
    if (module)
      module->updateMetadata(active);
  }
}

QColor Gui::defaultTagColor(int tag)
{
  switch (tag)
  {
  case 0:
    return Qt::blue;
  case 1:
    return Qt::green;
  case 2:
    return Qt::darkYellow;
  case 3:
    return Qt::darkMagenta;
  case 4:
    return Qt::darkCyan;
  case 5:
    return Qt::darkRed;
  case 6:
    return Qt::darkGray;
  case 7:
    return Qt::darkGreen;
  case 8:
    return Qt::darkBlue;
  case 9:
    return Qt::black;
  default:
    return Qt::white;
  }
}

QString Gui::colorToString(const QColor& color)
{
  return QString("rgb(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());
}

QGraphicsEffect* Gui::blurEffect(double radius)
{
  auto blur = new QGraphicsBlurEffect;
  blur->setBlurRadius(radius);
  return blur;
}

void NetworkEditor::tagLayer(bool active, int tag)
{
  tagLayerActive_ = active;

  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    item->setData(TagLayerKey, active);
    item->setData(CurrentTagKey, tag);
    if (active)
    {
      const auto itemTag = item->data(TagDataKey).toInt();
      if (AllTags == tag || ShowGroups == tag)
      {
        highlightTaggedItem(item, itemTag);
      }
      else if (tag != NoTag)
      {
        if (tag == itemTag)
        {
          highlightTaggedItem(item, itemTag);
        }
        else
          item->setGraphicsEffect(blurEffect());
      }
    }
    else
      item->setGraphicsEffect(nullptr);
  }
  if (ShowGroups == tag)
  {
    redrawTagGroups();
  }
  if (HideGroups == tag)
  {
    removeTagGroups();
  }
}

void NetworkEditor::drawTagGroups()
{
  QMap<int,QRectF> tagItemRects;

  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (dynamic_cast<ModuleProxyWidget*>(item))
    {
      const auto itemTag = item->data(TagDataKey).toInt();

      if (itemTag != NoTag)
      {
        auto r = item->boundingRect();
        r.translate(item->pos());
        if (!tagItemRects.contains(itemTag))
        {
          tagItemRects.insert(itemTag, r);
        }
        else
        {
          tagItemRects[itemTag] = tagItemRects[itemTag].united(r);
        }
      }
    }
  }

  for (auto rectIter = tagItemRects.constBegin(); rectIter != tagItemRects.constEnd(); ++rectIter)
  {
    auto rectBounds = rectIter.value().adjusted(-10, -10, 10, 10);
    QPen pen(tagColor_(rectIter.key()));
    pen.setWidth(3);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    auto rect = scene_->addRect(rectBounds, pen);
    rect->setFlags(QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);

    auto fill = new QGraphicsRectItem(rectBounds);
    auto c = pen.color();
    c.setAlphaF(0.15);
    fill->setBrush(c);
    scene_->addItem(fill);

    static const QFont labelFont("Courier", 20, QFont::Bold);
    auto label = scene_->addSimpleText(tagName_(rectIter.key()), labelFont);
    label->setBrush(pen.color());
    static const QFontMetrics fm(labelFont);
    auto textWidthInPixels = fm.width(label->text());
    label->setPos((rect->rect().topLeft() + rect->rect().topRight()) / 2 + QPointF(-textWidthInPixels/2, -30));
  }
}

void NetworkEditor::removeTagGroups()
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (dynamic_cast<QGraphicsRectItem*>(item) || dynamic_cast<QGraphicsSimpleTextItem*>(item))
    {
      delete item;
    }
  }
}

void NetworkEditor::redrawTagGroups()
{
  removeTagGroups();
  drawTagGroups();
}

void NetworkEditor::highlightTaggedItem(int tagValue)
{
  highlightTaggedItem(qobject_cast<QGraphicsItem*>(sender()), tagValue);
  Q_EMIT modified();
}

void NetworkEditor::highlightTaggedItem(QGraphicsItem* item, int tagValue)
{
  if (tagValue == NoTag)
  {
    item->setGraphicsEffect(blurEffect());
  }
  else
  {
    auto colorize = new QGraphicsColorizeEffect;
    auto color = tagColor_(tagValue);
    colorize->setColor(color);
    item->setGraphicsEffect(colorize);
  }
}

void NetworkEditor::cleanUpNetwork()
{
  controller_->cleanUpNetwork();
  centerView();
}

std::atomic<int> ErrorItem::instanceCounter_(0);

ErrorItem::ErrorItem(const QString& text, std::function<void()> showModule, QGraphicsItem* parent) : QGraphicsTextItem(text, parent),
  showModule_(showModule), counter_(instanceCounter_), rect_(nullptr)
{
  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
  setZValue(10000);
  ++instanceCounter_;
  setDefaultTextColor(Qt::red);

  {
    timeLine_ = new QTimeLine(10000, this);
    connect(timeLine_, SIGNAL(valueChanged(qreal)), this, SLOT(animate(qreal)));
    connect(timeLine_, SIGNAL(finished()), this, SLOT(deleteLater()));
  }
  timeLine_->start();
}

ErrorItem::~ErrorItem()
{
  --instanceCounter_;
  delete rect_;
}

void ErrorItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if (event->buttons() & Qt::LeftButton)
  {
    showModule_();
  }
  else if (event->buttons() & Qt::RightButton)
  {
    if (rect_)
    {
      scene()->removeItem(rect_);
      rect_ = nullptr;
    }
    scene()->removeItem(this);
  }
  QGraphicsTextItem::mousePressEvent(event);
}

void ErrorItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  timeLine_->setCurrentTime(0);
  QGraphicsTextItem::hoverEnterEvent(event);
}

void ErrorItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  if (!rect_)
  {
    timeLine_->stop();
    timeLine_->setCurrentTime(0);
    auto f = font();
    f.setBold(true);
    setFont(f);
    setFlags(flags() ^ ItemIsMovable);
    rect_ = scene()->addRect(boundingRect(), QPen(Qt::red, 2, Qt::DotLine));
    rect_->setPos(pos());
  }
  else
  {
    auto f = font();
    f.setBold(false);
    setFont(f);
    setFlags(flags() & ItemIsMovable);
    scene()->removeItem(rect_);
    rect_ = nullptr;
    timeLine_->start();
  }

  QGraphicsTextItem::mouseDoubleClickEvent(event);
}

void ErrorItem::animate(qreal val)
{
  if (val < 1)
    show();
  else
    hide();
  setOpacity(val < 0.5 ? 1 : 2 - 2*val);
}

void NetworkEditor::displayError(const QString& msg, std::function<void()> showModule)
{
  if (Preferences::Instance().showModuleErrorInlineMessages)
  {
    auto errorItem = new ErrorItem(msg, showModule);
    scene()->addItem(errorItem);

    QPointF tl(horizontalScrollBar()->value(), verticalScrollBar()->value());
    QPointF br = tl + viewport()->rect().bottomRight();
    QMatrix mat = matrix().inverted();
    auto rect = mat.mapRect(QRectF(tl, br));

    auto corner = rect.bottomLeft();
    errorItem->setPos(corner + QPointF(100, -(40*errorItem->num() + 100)));

#if 0
    auto xMin = rect.topLeft().x();
    auto xMax = rect.topRight().x();
    auto yMin = rect.topLeft().y();
    auto yMax = rect.bottomLeft().y();
    for (double x = xMin; x < xMax; x += 100)
      for (double y = yMin; y < yMax; y += 100)
      {
        QString xy = QString::number(x) + "," + QString::number(y);
        auto item = scene()->addText(xy);
        item->setDefaultTextColor(Qt::white);
        item->setPos(x, y);
  }
#endif
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
  NetworkEditor::clear();
}

ZLevelManager::ZLevelManager(QGraphicsScene* scene)
  : scene_(scene), minZ_(INITIAL_Z), maxZ_(INITIAL_Z)
{

}
