/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


#include <sstream>
#include <QtGui>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/ModuleProxyWidget.h>
#include <Interface/Application/PortWidgetManager.h>
#include <Interface/Application/ClosestPortFinder.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/Subnetworks.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h> //TODO
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h> //TODO: remove
#include <Dataflow/Network/Module.h> //TODO: remove
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Application/Application.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <boost/lambda/lambda.hpp>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Utils/StringUtil.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Engine;

NetworkEditor::~NetworkEditor()
{
  if (parentNetwork_)
    controller_.reset();

  for (auto& child : childrenNetworks_)
  {
    child.second->get()->controller_.reset();
    delete child.second->get();
    delete child.second;
    child.second = nullptr;
  }
  childrenNetworks_.clear();

  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    auto module = getModule(item);
    if (module)
      module->setDeletedFromGui(false);
  }
  NetworkEditor::clear();
}

SubnetworkEditor::SubnetworkEditor(NetworkEditor* editor, const ModuleId& subnetModuleId, const QString& name, QWidget* parent) : QFrame(parent),
editor_(editor), name_(name), subnetModuleId_(subnetModuleId)
{
  setupUi(this);
  setWindowTitle(windowTitle() + " - " + name);
  auto vbox = qobject_cast<QVBoxLayout*>(layout());
  vbox->insertWidget(0, editor);
  connect(expandPushButton_, SIGNAL(clicked()), this, SLOT(expand()));
  editor_->setParent(this);
  editor_->setAcceptDrops(true);

  auto subnetBar = new QToolBar("Subnet");
  WidgetStyleMixin::toolbarStyle(subnetBar);
  subnetBar->setObjectName("SubnetToolbar");
  SCIRunMainWindow::Instance()->addNetworkActionsToBar(subnetBar);
  subnetBar->setIconSize(QSize(25, 25));
  vbox->setMenuBar(subnetBar);

  saveAsTemplatePushButton_->hide();
}

void SubnetworkEditor::expand()
{
  editor_->sendItemsToParent();
  editor_->parentNetwork()->removeModuleWidget(subnetModuleId_);
}

SubnetworkEditor::~SubnetworkEditor()
{
}

const char* SUBNET_PORT_ID_TO_FIND = "SUBNET_PORT_ID_TO_FIND";

void NetworkEditor::sendItemsToParent()
{
  if (parentNetwork_)
  {
    removeSubnetPortHolders();

    for (auto& item : subnetItemsToMove())
    {
      auto conn = qgraphicsitem_cast<ConnectionLine*>(item);
      if (conn)
      {
        conn->deleteCompanion();
      }

      auto proxy = dynamic_cast<ModuleProxyWidget*>(item);
      if (proxy)
      {
        connect(parentNetwork_->scene_, SIGNAL(selectionChanged()), proxy, SLOT(highlightIfSelected()));
      }

      parentNetwork_->scene_->addItem(item);
      item->setVisible(true);
      item->setData(SUBNET_KEY, 0);
    }
  }
}

void NetworkEditor::removeSubnetPortHolders()
{
  for (auto& item : subnetPortHolders_)
    scene_->removeItem(item);
  subnetPortHolders_.clear();
}

std::vector<QGraphicsItem*> NetworkEditor::subnetItemsToMove()
{
  std::vector<QGraphicsItem*> nonCompanionItems(scene_->items().begin(), scene_->items().end());

  nonCompanionItems.erase(std::remove_if(nonCompanionItems.begin(), nonCompanionItems.end(),
    [](QGraphicsItem* item)
  {
    auto conn = qgraphicsitem_cast<ConnectionLine*>(item);
    return conn && conn->isCompanion();
  }),
    nonCompanionItems.end());

  return nonCompanionItems;
}

void NetworkEditor::addSubnetChild(const QString& name, ModuleHandle mod)
{
  auto it = childrenNetworks_.find(name);
  if (it == childrenNetworks_.end())
  {
    auto subnet = new NetworkEditor(ctorParams_);
    subnet->portRewiringMap_.swap(portRewiringMap_);
    initializeSubnet(name, mod, subnet);
  }
  else
  {
    auto subnet = it->second;
    subnet->show();
    subnet->activateWindow();
    subnet->raise();
  }
}

void NetworkEditor::showSubnetChild(const QString& name)
{
  auto it = childrenNetworks_.find(name);
  if (it == childrenNetworks_.end())
  {
    throw "logical error";
  }
  else
  {
    auto subnet = it->second;
    subnet->show();
    subnet->activateWindow();
    subnet->raise();
  }
}

void NetworkEditor::subnetMenuActionTriggered()
{
  auto action = qobject_cast<QAction*>(sender());
  auto subnetId = qobject_cast<QMenu*>(action->parent())->title();
  auto actionText = action->text();
  if ("Show" == actionText)
  {
    showSubnetChild(subnetNameMap_[subnetId.toStdString()]);
  }
  else if ("Rename..." == actionText)
  {
    bool ok;
    auto text = QInputDialog::getText(this, tr("Rename subnet"), tr("Enter new subnet name:"),
      QLineEdit::Normal, subnetNameMap_[subnetId.toStdString()], &ok);
    if (ok && !text.isEmpty())
    {
      qDebug() << "how to rename subnet";
    }
  }
}

QRectF NetworkEditor::visibleRect() const
{
  return mapToScene(rect()).boundingRect();
}

NetworkEditor* NetworkEditor::inEditingContext_(nullptr);
NetworkEditor::ConnectorFunc NetworkEditor::connectorFunc_;

std::function<QPointF(const QRectF&)> NetworkEditor::topSubnetPortHolderPositioner_([](const QRectF& rect) { return rect.topLeft(); });
std::function<QPointF(const QRectF&)> NetworkEditor::bottomSubnetPortHolderPositioner_([](const QRectF& rect) { return rect.bottomLeft() + QPointF(0, -40); });

void NetworkEditor::setupPortHolder(const std::vector<SharedPointer<PortDescriptionInterface>>& ports, const QString& name, std::function<QPointF(const QRectF&)> position)
{
  auto portsBridge = new SubnetPortsBridgeWidget(this, name);
  portsBridge->setToolTip(name);

  auto layout = new QHBoxLayout;
  layout->setSpacing(15);
  layout->setAlignment(Qt::AlignLeft);
  layout->setContentsMargins(15, 0, 5, 0);

  auto visible = visibleRect();

  auto proxy = new SubnetPortsBridgeProxyWidget(portsBridge);
  proxy->setWidget(portsBridge);
  proxy->setAcceptDrops(true);

  proxy->setMinimumWidth(visible.width());
  proxy->setData(123, name);

  int offset = 40;
  for (const auto& port : ports)
  {
    SubnetPortWidgetCtorArgs args { QString::fromStdString(port->get_portname()),
      to_color(PortColorLookup::toColor(port->get_typename()), 230), port->get_typename(),
      [this](){ return boost::make_shared<ConnectionFactory>([this]() { return scene_; }); },
      [this](){ return boost::make_shared<ClosestPortFinder>([this]() { return scene_; }); },
      port.get()};

    PortWidget* portRepl;
    if (name == "Outputs") // flip input and output designation here.
      portRepl = new SubnetInputPortWidget(args);
    else // Inputs
      portRepl = new SubnetOutputPortWidget(args);

    layout->addWidget(portRepl);
    portRepl->setSceneFunc([this]() { return scene_; });
    portRepl->setPositionObject(boost::make_shared<LambdaPositionProvider>([proxy, offset]() { return proxy->pos() + QPointF(offset, 0); }));

    // qDebug() << "port subnet in editor" << QString::fromStdString(port->id().toString());
    //   << portRewiringMap2_[port->id().toString()]->id().id_.c_str();

    portRewiringMap_[port->id().toString()]->addSubnetCompanion(portRepl);
    offset += portRepl->properWidth() + 10;
    portsBridge->addPort(portRepl);
  }

  //TODO: get from somewhere else
  std::vector<QString> types{"Field", "Matrix", "String", "Geometry", "Bundle", "Nrrd"};
  for (const auto& type : types)
  {
    SubnetPortWidgetCtorArgs args { "Test" + type,
      to_color(PortColorLookup::toColor(type.toStdString()), 230), type.toStdString(),
      [this](){ return boost::make_shared<ConnectionFactory>([this]() { return scene_; }); },
      [this](){ return boost::make_shared<ClosestPortFinder>([this]() { return scene_; }); },
      nullptr};

    PortWidget* testPort;
    if (name == "Outputs") // flip input and output designation here.
      testPort = new SubnetInputPortWidget(args);
    else // Inputs
      testPort = new SubnetOutputPortWidget(args);

    layout->addWidget(testPort);
    testPort->setSceneFunc([this]() { return scene_; });
    testPort->setPositionObject(boost::make_shared<LambdaPositionProvider>([proxy, offset]() { return proxy->pos() + QPointF(offset, 0); }));
    offset += testPort->properWidth() + 3;
    portsBridge->addPort(testPort);
    //testPort->hide();
  }

  portsBridge->setLayout(layout);

  scene_->addItem(proxy);
  subnetPortHolders_.append(proxy);

  proxy->setPos(position(visibleRect()));
}

SubnetInputPortWidget::SubnetInputPortWidget(const SubnetPortWidgetCtorArgs& args, QWidget* parent)
  : InputPortWidget(args.name, args.color, args.datatype, {}, PortId(), 0, true, args.connectionFactory, args.closestPortFinder, {}, parent), realPort_(args.realPort)
{

}


SubnetOutputPortWidget::SubnetOutputPortWidget(const SubnetPortWidgetCtorArgs& args, QWidget* parent)
  : OutputPortWidget(args.name, args.color, args.datatype, {}, PortId(), 0, true, args.connectionFactory, args.closestPortFinder, {}, parent), realPort_(args.realPort)
{

}

void NetworkEditor::setupPortHolders(ModuleHandle mod)
{
  setupPortHolder(upcast_range<PortDescriptionInterface>(mod->inputPorts()), "Inputs", topSubnetPortHolderPositioner_);
  setupPortHolder(upcast_range<PortDescriptionInterface>(mod->outputPorts()), "Outputs", bottomSubnetPortHolderPositioner_);
  portRewiringMap_.clear();
  boost::dynamic_pointer_cast<SubnetModule>(mod)->setSubnet(this);
}

void NetworkEditor::clearSiblingSelections()
{
  auto active = sender();
  for (auto& child : childrenNetworks_)
  {
    auto scene = child.second->get()->scene_;
    if (scene != active)
      scene->clearSelection();
  }
}

void NetworkEditor::initializeSubnet(const QString& name, ModuleHandle mod, NetworkEditor* subnet)
{
  subnet->parentNetwork_ = this;
  subnet->setDragMode(dragMode());
  subnet->setNetworkEditorController(getNetworkEditorController()->withSubnet(subnet));

  subnet->setSceneRect(QRectF(0, 0, NetworkBoundaries::sceneWidth / 2, NetworkBoundaries::sceneHeight / 2));
  connect(subnet->scene_, SIGNAL(selectionChanged()), scene_, SLOT(clearSelection()));
  connect(subnet->scene_, SIGNAL(selectionChanged()), this, SLOT(clearSiblingSelections()));
  connect(scene_, SIGNAL(selectionChanged()), subnet->scene_, SLOT(clearSelection()));

  for (auto& item : childrenNetworkItems_[name])
  {
    subnet->scene_->addItem(item);
    if (auto proxy = qgraphicsitem_cast<ModuleProxyWidget*>(item))
    {
      item->setVisible(true);
      connect(subnet->scene_, SIGNAL(selectionChanged()), proxy, SLOT(highlightIfSelected()));
    }
    else
    {
      auto conn = qgraphicsitem_cast<ConnectionLine*>(item);
      if (conn)
      {
        item->setVisible(true);
        if (item->data(SUBNET_KEY).toInt() == EXTERNAL_SUBNET_CONNECTION)
        {
        }
      }
    }
    item->ensureVisible();
  }

  connectorFunc_(subnet);
  subnet->setupPortHolders(mod);

  //TODO: later
  //auto mdi = new SubnetworkWindow;

  auto dock = new SubnetworkEditor(subnet, mod->id(), name, nullptr);
  dock->setStyleSheet(SCIRunMainWindow::Instance()->styleSheet());
  //dock->setWindowFlags(Qt::WindowStaysOnTopHint);
  //mdi->addSubWindow(dock, Qt::WindowStaysOnTopHint);
  dock->show();
  subnet->centerView();

  childrenNetworks_[name] = dock;
}

SubnetModule::SubnetModule(const std::vector<ModuleHandle>& underlyingModules, const QList<QGraphicsItem*>& items,
  NetworkEditor* parent) : Module(ModuleLookupInfo()),
  underlyingModules_(underlyingModules), items_(items), connector_(parent)
{
  setId("Subnet:" + boost::lexical_cast<std::string>(subnetCount_));
  subnetCount_++;
  connector_.setModule(this);
}

void SubnetModule::execute()
{
}

void SubnetModule::setStateDefaults()
{
  auto state = get_state();

  auto table = makeHomogeneousVariableList(
    [this](size_t i)
    {
      return makeAnonymousVariableList(underlyingModules_[i]->id().id_,
        std::string("Push me"),
        boost::lexical_cast<std::string>(underlyingModules_[i]->numInputPorts()),
        boost::lexical_cast<std::string>(underlyingModules_[i]->numOutputPorts()));
    },
    underlyingModules_.size());

  state->setValue(ModuleInfo, table);
}

std::string SubnetModule::listComponentIds() const
{
  std::ostringstream ostr;
  std::transform(underlyingModules_.begin(), underlyingModules_.end(),
    std::ostream_iterator<std::string>(ostr, ", "),
    [](const ModuleHandle& mod) { return mod->id(); });
  return ostr.str();
}

int SubnetModule::subnetCount_(0);
const AlgorithmParameterName SubnetModule::ModuleInfo("ModuleInfo");

//TODO: breaks older compilers. Will disable for now
#if 0
template <typename Iter>
QSet<typename Iter::value_type> toSet(Iter b, Iter e)
{
  return QSet<typename Iter::value_type>(b, e);
}
#endif

QList<QGraphicsItem*> NetworkEditor::includeConnections(QList<QGraphicsItem*> items) const
{
  throw "not implemented";
#if 0
  auto subnetItems = toSet(items.begin(), items.end());
  Q_FOREACH(QGraphicsItem* item, items)
  {
    auto module = getModule(item);
    if (module)
    {
      auto cs = module->connections();
      subnetItems.unite(toSet(cs.begin(), cs.end()));
    }
  }
  return subnetItems.values();
#endif
}

namespace
{
  QRectF updateRect(QGraphicsItem* item, QRectF rect)
  {
    auto r = item->boundingRect();
    r = item->mapRectToParent(r);

    if (rect.isEmpty())
      return r;
    else
      return rect.united(r);
  }
}

static const char* noRecursiveSubnetsWarning = "Subnets only support a depth level of one at this time. See issue #1641.";

void NetworkEditor::makeSubnetwork()
{
  if (parentNetwork_)
  {
    QMessageBox::information(this, "Make subnetwork", noRecursiveSubnetsWarning);
    return;
  }

  QRectF rect;
  QPointF position;

  std::vector<ModuleHandle> underlyingModules;
  QList<QGraphicsItem*> items;
  Q_FOREACH(QGraphicsItem* item, scene_->selectedItems())
  {
    position = item->pos();
    rect = updateRect(item, rect);

    auto module = getModule(item);
    if (module)
    {
      items.append(item);
      auto mod = module->getModule();
      if (mod->id().id_.find("Subnet") != std::string::npos)
      {
        QMessageBox::information(this, "Make subnetwork", noRecursiveSubnetsWarning);
        return;
      }
      underlyingModules.push_back(mod);
    }
  }

  if (underlyingModules.empty())
  {
    QMessageBox::information(this, "Make subnetwork", "Please select at least one module.");
    return;
  }

  bool ok;
  auto name = QInputDialog::getText(nullptr, "Make subnet", "Enter subnet name:", QLineEdit::Normal, "subnet" + QString::number(currentSubnetNames_.size()), &ok);
  if (!ok)
    return;

  if (name.isEmpty())
  {
    QMessageBox::information(this, "Make subnetwork", "Invalid name: cannot be empty.");
    return;
  }

  if (currentSubnetNames_.contains(name))
  {
    QMessageBox::information(this, "Make subnetwork", "Invalid name: a subnet by that name already exists.");
    return;
  }

  if (Application::Instance().moduleNameExists(name.toStdString()))
  {
    QMessageBox::information(this, "Make subnetwork", "Invalid name: matches a module name.");
    return;
  }

  makeSubnetworkFromComponents(name, underlyingModules, includeConnections(items), rect);
}

class SubnetModuleFactory : public Modules::Factory::HardCodedModuleFactory
{
public:
  explicit SubnetModuleFactory(NetworkEditor* parent) : parent_(parent) {}
  ModuleHandle makeSubnet(const QString& name, const std::vector<ModuleHandle>& modules, QList<QGraphicsItem*> items) const
  {
    ModuleDescription desc;

    for (const auto& i : items)
    {
      auto conn = qgraphicsitem_cast<ConnectionLine*>(i);
      if (conn)
      {
        auto mods = conn->getConnectedToModuleIds();
        auto foundFirst = std::find_if(modules.cbegin(), modules.cend(),
          [&mods](const ModuleHandle& mod) { return mod->id().id_ == mods.first.id_; });
        auto foundSecond = std::find_if(modules.cbegin(), modules.cend(),
          [&mods](const ModuleHandle& mod) { return mod->id().id_ == mods.second.id_; });
        auto isInternalConnection = foundFirst != modules.cend() && foundSecond != modules.cend();
        conn->setData(SUBNET_KEY, isInternalConnection ? INTERNAL_SUBNET_CONNECTION : EXTERNAL_SUBNET_CONNECTION);

        if (!isInternalConnection)
        {
          auto ports = conn->connectedPorts();

          auto addSubnetToId = [](PortWidget* port) { return PortId{ port->id().id,
            port->id().name + (port->isInput() ? std::string("[To:") : std::string("[From:")) + port->getUnderlyingModuleId().id_ + "]" }; };
          if (foundFirst != modules.cend())
          {
            auto portToReplicate = ports.second;
            auto id = addSubnetToId(portToReplicate);

            //qDebug() << "port being replicated" << id.toString().c_str() <<
            //  portToReplicate->id().toString().c_str() <<
            //  portToReplicate->getUnderlyingModuleId().id_.c_str();

            map_[id.toString()] = conn;

            desc.input_ports_.emplace_back(id, portToReplicate->get_typename(), portToReplicate->isDynamic());
            ports.first->setProperty(SUBNET_PORT_ID_TO_FIND, QString::fromStdString(id.toString()));
          }
          else
          {
            auto portToReplicate = ports.first;
            auto id = addSubnetToId(portToReplicate);

            //qDebug() << "port being replicated" << id.toString().c_str() <<
            //  portToReplicate->id().toString().c_str() <<
            //  portToReplicate->getUnderlyingModuleId().id_.c_str();

            map_[id.toString()] = conn;

            desc.output_ports_.emplace_back(id, portToReplicate->get_typename(), portToReplicate->isDynamic());
            ports.second->setProperty(SUBNET_PORT_ID_TO_FIND, QString::fromStdString(id.toString()));
          }
        }
      }
    }

    desc.maker_ = [&modules, items, this]() { return new SubnetModule(modules, items, parent_); };

    auto mod = create(desc);

    mod->get_state()->setValue(Name("Name"), name.toStdString());

    return mod;
  }

  const PortRewiringMap& getMap() const
  {
    return map_;
  }
private:
  mutable PortRewiringMap map_;
  NetworkEditor* parent_;
};

void NetworkEditor::makeSubnetworkFromComponents(const QString& name, const std::vector<ModuleHandle>& modules,
  QList<QGraphicsItem*> items, const QRectF& rect)
{
  currentSubnetNames_.insert(name);

  SubnetModuleFactory factory(this);
  auto subnetModule = factory.makeSubnet(name, modules, items);
  portRewiringMap_ = factory.getMap();

  auto moduleWidget = new SubnetWidget(this, name, subnetModule, dialogErrorControl_);
  auto proxy = setupModuleWidget(moduleWidget);
  //TODO: file loading case, duplicated
  moduleWidget->postLoadAction();
  //proxy->setScale(1.6);--problematic with port positions

  auto dropShadow = new QGraphicsDropShadowEffect;
  dropShadow->setColor(Qt::darkGray);
  dropShadow->setOffset(5, 5);
  dropShadow->setBlurRadius(2);
  proxy->setGraphicsEffect(dropShadow);

  auto pic = grabSubnetPic(rect, items);
  auto tooltipPic = convertToTooltip(pic);
  proxy->setToolTip(tooltipPic);

  {
    QMap<QString, PortWidget*> subPortMap;
    {
      auto subPorts = moduleWidget->ports().getAllPorts();
      for (const auto& subP : subPorts)
      {
        subPortMap[QString::fromStdString(subP->realId().toString())] = subP;
      }
    }

    for (const auto& i : items)
    {
      auto conn = qgraphicsitem_cast<ConnectionLine*>(i);
      if (conn)
      {
        auto mods = conn->getConnectedToModuleIds();
        if (conn->data(SUBNET_KEY).toInt() == EXTERNAL_SUBNET_CONNECTION)
        {
          auto ports = conn->connectedPorts();
          if (!ports.first->property(SUBNET_PORT_ID_TO_FIND).toString().isEmpty())
          {
            ports.first->connectToSubnetPort(subPortMap[ports.first->property(SUBNET_PORT_ID_TO_FIND).toString()]);
          }
          else
          {
            ports.second->connectToSubnetPort(subPortMap[ports.second->property(SUBNET_PORT_ID_TO_FIND).toString()]);
          }
        }
      }
    }
  }

  auto size = proxy->getModuleWidget()->size();
  if (!rect.isEmpty())
  {
    auto pos = rect.center();
    pos.rx() -= size.width() / 2;
    pos.ry() -= size.height() / 2;
    proxy->setPos(pos);
  }
  else
  {
    //qDebug() << "rect is empty, pos is" << proxy->pos();
  }

  childrenNetworkItems_[name] = items;

  addSubnetChild(name, subnetModule);
  subnetNameMap_[subnetModule->id()] = name;

  Q_EMIT modified();
  Q_EMIT newModule(QString::fromStdString(subnetModule->id()), subnetModule->hasUI());
}

QPixmap NetworkEditor::grabSubnetPic(const QRectF& rect, const QList<QGraphicsItem*>& items)
{
  QList<QGraphicsItem*> toHide;
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (dynamic_cast<QGraphicsPixmapItem*>(item) || !items.contains(item))
    {
      item->setVisible(false);
      toHide.append(item);
    }
  }

  auto pic = grab(mapFromScene(rect).boundingRect());

  Q_FOREACH(QGraphicsItem* item, toHide)
  {
    item->setVisible(true);
  }

  return pic;
}

QString NetworkEditor::convertToTooltip(const QPixmap& pic) const
{
  QByteArray byteArray;
  QBuffer buffer(&byteArray);
  pic.scaled(pic.size() * 0.5).save(&buffer, "PNG");
  return QString("<html><img src=\"data:image/png;base64,") + byteArray.toBase64() + "\"/></html>";
}

void NetworkEditor::dumpSubnetworksImpl(const QString& name, Subnetworks& data, ModuleFilter modFilter) const
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (auto mod = dynamic_cast<ModuleProxyWidget*>(item))
    {
      if (parentNetwork_ && modFilter(mod->getModuleWidget()->getModule()))
      {
        data.subnets[name.toStdString()].push_back(mod->getModuleWidget()->getModuleId());
      }
    }
  }
}

void NetworkEditor::updateSubnetworks(const Subnetworks& subnets)
{
  for (const auto& sub : subnets.subnets)
  {
    std::vector<ModuleHandle> underlying;
    QList<QGraphicsItem*> items;

    QRectF rect;
    Q_FOREACH(QGraphicsItem* item, scene_->items())
    {
      if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
      {
        if (std::find(sub.second.begin(), sub.second.end(), w->getModuleWidget()->getModuleId()) != sub.second.end())
        {
          underlying.push_back(w->getModuleWidget()->getModule());
          items.append(w);
          rect = updateRect(item, rect);
        }
      }
    }
    makeSubnetworkFromComponents(QString::fromStdString(sub.first), underlying, includeConnections(items), rect);
  }
}

SubnetWidget::SubnetWidget(NetworkEditor* ed, const QString& name, ModuleHandle theModule, boost::shared_ptr<DialogErrorControl> dialogErrorControl,
  QWidget* parent /* = 0 */) : ModuleWidget(ed, name, theModule, dialogErrorControl, parent), editor_(ed), name_(name)
{
}

SubnetWidget::~SubnetWidget()
{
  editor_->killChild(name_, deleteSubnetImmediately_);
}

SubnetPortsBridgeWidget::SubnetPortsBridgeWidget(NetworkEditor* ed, const QString& name, QWidget* parent /* = 0 */) :
  QWidget(parent), editor_(ed), name_(name)
{
  setFixedHeight(8);
  QString rounded("color: white; border-radius: 7px;");
  setStyleSheet(rounded + " background-color: darkGray");
}

void NetworkEditor::killChild(const QString& name, bool force)
{
  auto subnetIter = childrenNetworks_.find(name);
  if (subnetIter != childrenNetworks_.end())
  {
    subnetIter->second->get()->clear();
    if (force)
      delete subnetIter->second;
    else
      subnetIter->second->deleteLater();
    childrenNetworks_.erase(subnetIter);
    currentSubnetNames_.remove(name);

    auto idNameIter = std::find_if(subnetNameMap_.begin(), subnetNameMap_.end(),
      [&name](const std::map<std::string, QString>::value_type& p) { return p.second == name; });
    if (idNameIter != subnetNameMap_.end())
    {
      subnetNameMap_.erase(idNameIter);
    }
  }
}

void NetworkEditor::resizeEvent(QResizeEvent *event)
{
  if (event->oldSize() != QSize(-1, -1))
  {
    for (auto& item : subnetPortHolders_)
    {
      item->resize(QSize(item->size().width() * (event->size().width() / static_cast<double>(event->oldSize().width())), item->size().height()));
      auto isInput = item->data(123).toString() == "Inputs";
      item->setPos(item->pos() + QPointF(0, (isInput ? 0 : 1) * (event->size().height() - event->oldSize().height())));
      item->updateConnections();
    }
  }

  QGraphicsView::resizeEvent(event);
}

SubnetModuleConnector::SubnetModuleConnector(NetworkEditor* parent) :
  parent_(parent), subnet_(nullptr)
{
  connect(parent, SIGNAL(connectionDeleted(const SCIRun::Dataflow::Networks::ConnectionId&)),
    this, SLOT(connectionDeletedFromParent()));

  connect(parent_->getNetworkEditorController().get(), SIGNAL(moduleAdded(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle, const SCIRun::Dataflow::Engine::ModuleCounter&)),
    this, SLOT(moduleAddedToSubnet(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle)));
}

void SubnetModuleConnector::setSubnet(NetworkEditor* subnet)
{
  subnet_ = subnet;

  connect(subnet_->getNetworkEditorController().get(), SIGNAL(moduleAdded(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle, const SCIRun::Dataflow::Engine::ModuleCounter&)),
    this, SLOT(moduleAddedToSubnet(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle)));
}

bool SubnetModuleConnector::signalFromParent(QObject* sender) const
{
  return qobject_cast<NetworkEditorControllerGuiProxy*>(sender)->activeNetwork() == parent_;
}

bool SubnetModuleConnector::signalFromSubnet(QObject* sender) const
{
  return qobject_cast<NetworkEditorControllerGuiProxy*>(sender)->activeNetwork() == subnet_;
}

void SubnetModuleConnector::moduleAddedToSubnet(const std::string& s, ModuleHandle module)
{
  if (signalFromSubnet(sender()) && subnet_->containsModule(module->id().id_))
  {
    //qDebug() << "was:" << module_->underlyingModules_.size();
    module_->underlyingModules_.push_back(module);
    //qDebug() << "now:" << module_->underlyingModules_.size() << "added" << s.c_str();
  }
}

bool NetworkEditor::containsModule(const std::string& id) const
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    auto module = getModule(item);
    if (module && module->getModuleId() == id)
      return true;
  }
  return false;
}

void SubnetModuleConnector::connectionDeletedFromParent()
{
}
