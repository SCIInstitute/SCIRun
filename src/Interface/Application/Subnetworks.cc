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
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h> //TODO: remove
#include <Dataflow/Network/Module.h> //TODO: remove
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Application/Application.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <boost/lambda/lambda.hpp>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Engine;

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

SubnetworkEditor::SubnetworkEditor(NetworkEditor* editor, const QString& name, QWidget* parent) : QDockWidget(parent),
  editor_(editor)
{
  setupUi(this);
  groupBox->setTitle(name);
  auto vbox = new QVBoxLayout;
  vbox->addWidget(editor);
  groupBox->setLayout(vbox);
}

void SubnetworkEditor::expand()
{
  editor_->sendItemsToParent();
}

void NetworkEditor::sendItemsToParent()
{
  if (parentNetwork_)
  {
    for (auto& item : scene_->items())
      parentNetwork_->scene_->addItem(item);
  }
}

void NetworkEditor::addSubnetChild(const QString& name)
{
  auto it = childrenNetworks_.find(name);
  if (it == childrenNetworks_.end())
  {
    auto subnet = new NetworkEditor(ctorParams_);
    initializeSubnet(name, subnet);
  }
  else
  {
    it->second->show();
  }
}

void NetworkEditor::initializeSubnet(const QString& name, NetworkEditor* subnet)
{
  subnet->parentNetwork_ = this;
  subnet->setNetworkEditorController(getNetworkEditorController());

  for (auto& item : childrenNetworkItems_[name])
    subnet->scene_->addItem(item);

  auto dock = new SubnetworkEditor(subnet, name, parentWidget());
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
  virtual void execute() override
  {
    //std::ostringstream ostr;
    //ostr << "Module " << get_module_name() << " executing for " << 3.14 << " seconds." << std::endl;
    //status(ostr.str());
  }

  static const AlgorithmParameterName ModuleInfo;

  virtual void setStateDefaults() override
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
      underlyingModules.push_back(module->getModule());
  }
  if (underlyingModules.empty())
    return;

  bool ok;
  auto name = QInputDialog::getText(nullptr, "Make subnet", "Enter subnet name:", QLineEdit::Normal, "subnet", &ok);
  if (!ok || name.isEmpty())
    return;

  auto pic = grabSubnetPic(rect);
  auto subnetModule = boost::make_shared<SubnetModule>(underlyingModules, items);
  subnetModule->setStateDefaults();
  subnetModule->get_state()->setValue(Name("Name"), name.toStdString());
  auto moduleWidget = new SubnetWidget(this, name, subnetModule, dialogErrorControl_);

  auto tooltipPic = convertToTooltip(pic);
  auto proxy = setupModuleWidget(moduleWidget);

  //TODO: file loading case, duplicated
  moduleWidget->postLoadAction();
  proxy->setScale(1.6);
  proxy->setToolTip(tooltipPic);

  childrenNetworkItems_[name] = items;

  addSubnetChild(name);
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

SubnetWidget::SubnetWidget(NetworkEditor* ed, const QString& name, ModuleHandle theModule, boost::shared_ptr<DialogErrorControl> dialogErrorControl,
  QWidget* parent /* = 0 */) : ModuleWidget(ed, name, theModule, dialogErrorControl, parent)
{

}
