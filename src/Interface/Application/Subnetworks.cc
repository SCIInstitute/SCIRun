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
#include <Interface/Application/PortWidgetManager.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/Subnetworks.h>
#include <Interface/Application/SCIRunMainWindow.h>
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
    child.second = nullptr;
  }

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
  qobject_cast<QVBoxLayout*>(layout())->insertWidget(0, editor);
  connect(expandPushButton_, SIGNAL(clicked()), this, SLOT(expand()));
  editor_->setParent(this);
  editor_->setAcceptDrops(true);
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
    for (auto& item : subnetPortHolders_)
      scene_->removeItem(item);
    subnetPortHolders_.clear();

    for (auto& item : scene_->items())
    {
      parentNetwork_->scene_->addItem(item);
      item->setVisible(true);
      item->setData(SUBNET_KEY, 0);
    }
  }
}

void NetworkEditor::removeSubnetChild(const QString& name)
{
  childrenNetworks_.erase(name);
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
    it->second->show();
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
    it->second->show();
  }
}

NetworkEditor* NetworkEditor::inEditingContext_(nullptr);
NetworkEditor::ConnectorFunc NetworkEditor::connectorFunc_;

void NetworkEditor::setupPortHolder(const std::vector<SharedPointer<PortDescriptionInterface>>& ports, const QString& name, std::function<QPointF(const QRectF&)> position)
{
  auto portsBridge = new SubnetPortsBridgeWidget(this, name);
  portsBridge->setToolTip(name);

  auto layout = new QHBoxLayout;
  layout->setSpacing(4);
  layout->setAlignment(Qt::AlignLeft);
  layout->setContentsMargins(5, 0, 5, 0);

  for (const auto& port : ports)
  {
    auto portRepl = new SubnetOutputPortWidget(QString::fromStdString(port->get_portname()),
      to_color(PortColorLookup::toColor(port->get_typename()), 230), port->get_typename());
    layout->addWidget(portRepl);

    //qDebug() << "port subnet in editor" << QString::fromStdString(port->id().toString());
      //<< portRewiringMap2_[port->id().toString()]->id().id_.c_str();

    portRewiringMap_[port->id().toString()]->addSubnetCompanion(portRepl);
  }

  portsBridge->setLayout(layout);

  auto proxy = new QGraphicsProxyWidget;
  proxy->setWidget(portsBridge);
  proxy->setAcceptDrops(true);

  auto visible = mapToScene(rect()).boundingRect();
  proxy->setMinimumWidth(visible.width());
  scene_->addItem(proxy);
  subnetPortHolders_.append(proxy);

  proxy->setPos(position(visible));
}

SubnetOutputPortWidget::SubnetOutputPortWidget(const QString& name, const QColor& color, const std::string& datatype, QWidget* parent)
  : OutputPortWidget(name, color, datatype, ModuleId(), PortId(), 0, false, {}, {}, {})
{

}

void NetworkEditor::setupPortHolders(ModuleHandle mod)
{
  setupPortHolder(upcast_range<PortDescriptionInterface>(mod->inputPorts()), "Inputs", [](const QRectF& rect) { return rect.topLeft(); });
  setupPortHolder(upcast_range<PortDescriptionInterface>(mod->outputPorts()), "Outputs", [](const QRectF& rect) { return rect.bottomLeft() + QPointF(0, -23); });
  portRewiringMap_.clear();
}

void NetworkEditor::initializeSubnet(const QString& name, ModuleHandle mod, NetworkEditor* subnet)
{
  subnet->parentNetwork_ = this;
  subnet->setNetworkEditorController(getNetworkEditorController()->withSubnet(subnet));

  subnet->setSceneRect(QRectF(-500, -500, 1000, 1000));

  for (auto& item : childrenNetworkItems_[name])
  {
    subnet->scene_->addItem(item);
    if (qgraphicsitem_cast<ModuleProxyWidget*>(item))
      item->setVisible(true);
    else
    {
      auto conn = qgraphicsitem_cast<ConnectionLine*>(item);
      if (conn)
      {
        //item->setVisible(item->data(IS_INTERNAL).toBool());
        item->setVisible(true);
        if (item->data(SUBNET_KEY).toInt() == EXTERNAL_SUBNET_CONNECTION)
        {

          //qDebug() << "hidden external connection ports" << conn->connectedPorts().first->id().toString().c_str()
          //  << conn->connectedPorts().first->getUnderlyingModuleId().id_.c_str()
          //  << conn->connectedPorts().second->id().toString().c_str()
          //  << conn->connectedPorts().second->getUnderlyingModuleId().id_.c_str()
          //  ;



          //auto firstMatch = subnet->portRewiringMap_.find(conn->connectedPorts().first->getUnderlyingModuleId().id_);
          //if (firstMatch != subnet->portRewiringMap_.end())
          //{
          //  //qDebug() << "found match for conn end--first";
          //  auto portMatch = firstMatch->second.find(conn->connectedPorts().first->id().toString());
          //  if (portMatch != firstMatch->second.end())
          //  {
          //    //qDebug() << "\tand found port match at" << portMatch->second.toString().c_str();
          //    subnet->portRewiringMap2_[portMatch->second.toString()] = conn;
          //  }
          //}
          //else
          //{
          //  auto secondMatch = subnet->portRewiringMap_.find(conn->connectedPorts().second->getUnderlyingModuleId().id_);
          //  if (secondMatch != subnet->portRewiringMap_.end())
          //  {
          //    //qDebug() << "found match for conn end--second";
          //    auto portMatch = secondMatch->second.find(conn->connectedPorts().second->id().toString());
          //    if (portMatch != secondMatch->second.end())
          //    {
          //      //qDebug() << "\tand found port match at" << portMatch->second.toString().c_str();
          //      subnet->portRewiringMap2_[portMatch->second.toString()] = conn;
          //    }
          //  }
          //}
        }
      }
    }

    item->ensureVisible();
  }

  connectorFunc_(subnet);
  subnet->setupPortHolders(mod);


  auto dock = new SubnetworkEditor(subnet, mod->get_id(), name, nullptr);
  dock->setStyleSheet(SCIRunMainWindow::Instance()->styleSheet());
  dock->show();

  childrenNetworks_[name] = dock;
}

class SubnetModule : public Module
{
public:
  SubnetModule(const std::vector<ModuleHandle>& underlyingModules, const QList<QGraphicsItem*>& items) : Module(ModuleLookupInfo()),
    underlyingModules_(underlyingModules), items_(items)
  {
    set_id("Subnet:" + boost::lexical_cast<std::string>(subnetCount_));
    subnetCount_++;
  }

  void execute() override
  {
  }

  static const AlgorithmParameterName ModuleInfo;

  void setStateDefaults() override
  {
    auto state = get_state();

    auto table = makeHomogeneousVariableList(
      [this](size_t i)
      {
        return makeAnonymousVariableList(underlyingModules_[i]->get_id().id_,
          std::string("Push me"),
          boost::lexical_cast<std::string>(underlyingModules_[i]->num_input_ports()),
          boost::lexical_cast<std::string>(underlyingModules_[i]->num_output_ports()));
      },
      underlyingModules_.size());

    state->setValue(ModuleInfo, table);
  }

  std::string listComponentIds() const
  {
    std::ostringstream ostr;
    std::transform(underlyingModules_.begin(), underlyingModules_.end(),
      std::ostream_iterator<std::string>(ostr, ", "),
      [](const ModuleHandle& mod) { return mod->get_id(); });
    return ostr.str();
  }
private:
  std::vector<ModuleHandle> underlyingModules_;
  QList<QGraphicsItem*> items_;
  static int subnetCount_;
};

int SubnetModule::subnetCount_(0);
const AlgorithmParameterName SubnetModule::ModuleInfo("ModuleInfo");

QList<QGraphicsItem*> NetworkEditor::includeConnections(QList<QGraphicsItem*> items) const
{
  auto subnetItems = items.toSet();
  Q_FOREACH(QGraphicsItem* item, items)
  {
    auto module = getModule(item);
    if (module)
    {
      subnetItems.unite(module->connections().toSet());
    }
  }
  return subnetItems.toList();
}

void NetworkEditor::makeSubnetwork()
{
  QRectF rect;
  QPointF position;

  std::vector<ModuleHandle> underlyingModules;
  auto items = scene_->selectedItems();
  Q_FOREACH(QGraphicsItem* item, items)
  {
    auto r = item->boundingRect();
    position = item->pos();
    r = item->mapRectToParent(r);

    if (rect.isEmpty())
      rect = r;
    else
      rect = rect.united(r);

    auto module = getModule(item);
    if (module)
    {
      underlyingModules.push_back(module->getModule());
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
    QMessageBox::information(this, "Make subnetwork", "Invalid name.");
    return;
  }

  if (currentSubnetNames_.contains(name))
  {
    QMessageBox::information(this, "Make subnetwork", "A subnet by that name already exists.");
    return;
  }

  makeSubnetworkFromComponents(name, underlyingModules, includeConnections(items), rect);
}

class SubnetModuleFactory : public Modules::Factory::HardCodedModuleFactory
{
public:
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
          [&mods](const ModuleHandle& mod) { return mod->get_id().id_ == mods.first.id_; });
        auto foundSecond = std::find_if(modules.cbegin(), modules.cend(),
          [&mods](const ModuleHandle& mod) { return mod->get_id().id_ == mods.second.id_; });
        auto isInternalConnection = foundFirst != modules.cend() && foundSecond != modules.cend();
        conn->setData(SUBNET_KEY, isInternalConnection ? INTERNAL_SUBNET_CONNECTION : EXTERNAL_SUBNET_CONNECTION);

        if (!isInternalConnection)
        {
          auto ports = conn->connectedPorts();

          auto addSubnetToId = [](PortWidget* port) { return PortId{ port->id().id, port->id().name + (port->isInput() ? std::string("[To:") : std::string("[From:")) + port->getUnderlyingModuleId().id_ + "]" }; };
          if (foundFirst != modules.cend())
          {
            auto portToReplicate = ports.second;
            auto id = addSubnetToId(portToReplicate);

            qDebug() << "port being replicated" << id.toString().c_str() <<
              portToReplicate->id().toString().c_str() <<
              portToReplicate->getUnderlyingModuleId().id_.c_str();

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

    desc.maker_ = [&modules, items]() { return new SubnetModule(modules, items); };

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
};

void NetworkEditor::makeSubnetworkFromComponents(const QString& name, const std::vector<ModuleHandle>& modules,
  QList<QGraphicsItem*> items, const QRectF& rect)
{
  currentSubnetNames_.insert(name);

  SubnetModuleFactory factory;
  auto subnetModule = factory.makeSubnet(name, modules, items);
  portRewiringMap_ = factory.getMap();

  auto moduleWidget = new SubnetWidget(this, name, subnetModule, dialogErrorControl_);
  auto proxy = setupModuleWidget(moduleWidget);
  //TODO: file loading case, duplicated
  moduleWidget->postLoadAction();
  //proxy->setScale(1.6);--problematic with port positions

  auto colorize = new QGraphicsDropShadowEffect;
  colorize->setColor(QColor(255,182,193,200));
  colorize->setOffset(10, 6);
  colorize->setBlurRadius(30);
  proxy->setGraphicsEffect(colorize);

  auto pic = grabSubnetPic(rect);
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
  //qDebug() << "port repl map out of scope";
}

QPixmap NetworkEditor::grabSubnetPic(const QRectF& rect)
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (dynamic_cast<QGraphicsPixmapItem*>(item))
      item->setVisible(false);
  }

  auto pic = QPixmap::grabWidget(this, mapFromScene(rect).boundingRect());

  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (dynamic_cast<QGraphicsPixmapItem*>(item))
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

    Q_FOREACH(QGraphicsItem* item, scene_->items())
    {
      if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
      {
        if (std::find(sub.second.begin(), sub.second.end(), w->getModuleWidget()->getModuleId()) != sub.second.end())
        {
          underlying.push_back(w->getModuleWidget()->getModule());
          items.append(w);
        }
      }
    }
    makeSubnetworkFromComponents(QString::fromStdString(sub.first), underlying, includeConnections(items), {});
  }
}

SubnetWidget::SubnetWidget(NetworkEditor* ed, const QString& name, ModuleHandle theModule, boost::shared_ptr<DialogErrorControl> dialogErrorControl,
  QWidget* parent /* = 0 */) : ModuleWidget(ed, name, theModule, dialogErrorControl, parent), editor_(ed), name_(name)
{
}

SubnetWidget::~SubnetWidget()
{
  editor_->killChild(name_);
}

SubnetPortsBridgeWidget::SubnetPortsBridgeWidget(NetworkEditor* ed, const QString& name, QWidget* parent /* = 0 */) :
  QWidget(parent), editor_(ed), name_(name)
{
  setFixedHeight(8);
  QString rounded("color: white; border-radius: 7px;");
  setStyleSheet(rounded + " background-color: darkGray");
}

void NetworkEditor::killChild(const QString& name)
{
  auto subnetIter = childrenNetworks_.find(name);
  if (subnetIter != childrenNetworks_.end())
  {
    subnetIter->second->get()->clear();
    subnetIter->second->deleteLater();
    childrenNetworks_.erase(subnetIter);
    currentSubnetNames_.remove(name);
  }
}
