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

#include <iostream>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Network/NetworkSettings.h>
#include <QDebug>
#include <Core/Logging/Log.h>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;

NetworkEditorControllerGuiProxy::NetworkEditorControllerGuiProxy(boost::shared_ptr<NetworkEditorController> controller)
  : controller_(controller)
{
  controller_->connectModuleAdded(boost::bind(&NetworkEditorControllerGuiProxy::moduleAdded, this, _1, _2, _3));
  controller_->connectModuleRemoved(boost::bind(&NetworkEditorControllerGuiProxy::moduleRemoved, this, _1));
  controller_->connectConnectionAdded(boost::bind(&NetworkEditorControllerGuiProxy::connectionAdded, this, _1));
  controller_->connectConnectionRemoved(boost::bind(&NetworkEditorControllerGuiProxy::connectionRemoved, this, _1));
  controller_->connectPortAdded(boost::bind(&NetworkEditorControllerGuiProxy::portAdded, this, _1, _2));
  controller_->connectPortRemoved(boost::bind(&NetworkEditorControllerGuiProxy::portRemoved, this, _1, _2));
  controller_->connectNetworkExecutionStarts([&]() { executionStarted(); });
  controller_->connectNetworkExecutionFinished(boost::bind(&NetworkEditorControllerGuiProxy::executionFinished, this, _1));
  controller_->connectNetworkDoneLoading(boost::bind(&NetworkEditorControllerGuiProxy::networkDoneLoading, this, _1));
}

void NetworkEditorControllerGuiProxy::addModule(const std::string& moduleName)
{
  try
  {
    controller_->addModule(moduleName);
  }
  catch (SCIRun::Core::InvalidArgumentException& e)
  {
    qDebug() << "CAUGHT EXCEPTION";
    Core::Logging::Log::get() << Core::Logging::ERROR_LOG << e.what() << std::endl;
  }
}

void NetworkEditorControllerGuiProxy::removeModule(const ModuleId& id)
{
  controller_->removeModule(id);
}

void NetworkEditorControllerGuiProxy::interrupt(const ModuleId& id)
{
  controller_->interruptModule(id);
}

boost::optional<ConnectionId> NetworkEditorControllerGuiProxy::requestConnection(const PortDescriptionInterface* from, const PortDescriptionInterface* to)
{
  return controller_->requestConnection(from, to);
}

void NetworkEditorControllerGuiProxy::removeConnection(const ConnectionId& id)
{
  controller_->removeConnection(id);
}

NetworkFileHandle NetworkEditorControllerGuiProxy::saveNetwork() const
{
  return controller_->saveNetwork();
}

NetworkFileHandle NetworkEditorControllerGuiProxy::serializeNetworkFragment(ModuleFilter modFilter, ConnectionFilter connFilter) const
{
  return controller_->serializeNetworkFragment(modFilter, connFilter);
}

void NetworkEditorControllerGuiProxy::loadNetwork(const NetworkFileHandle& xml)
{
  controller_->loadNetwork(xml);
}

void NetworkEditorControllerGuiProxy::appendToNetwork(const NetworkFileHandle& xml)
{
  controller_->appendToNetwork(xml);
}

void NetworkEditorControllerGuiProxy::executeAll(const ExecutableLookup& lookup)
{
  controller_->executeAll(&lookup);
}

void NetworkEditorControllerGuiProxy::executeModule(const ModuleHandle& module, const ExecutableLookup& lookup)
{
  controller_->executeModule(module, &lookup);
}

size_t NetworkEditorControllerGuiProxy::numModules() const
{
  return controller_->getNetwork()->nmodules();
}

int NetworkEditorControllerGuiProxy::errorCode() const
{
  return controller_->getNetwork()->errorCode();
}

NetworkGlobalSettings& NetworkEditorControllerGuiProxy::getSettings()
{
  return controller_->getSettings();
}

void NetworkEditorControllerGuiProxy::setExecutorType(int type)
{
  controller_->setExecutorType(type);
}

void NetworkEditorControllerGuiProxy::duplicateModule(const SCIRun::Dataflow::Networks::ModuleHandle& module)
{
  controller_->duplicateModule(module);
}

void NetworkEditorControllerGuiProxy::connectNewModule(const SCIRun::Dataflow::Networks::ModuleHandle& moduleToConnectTo, const SCIRun::Dataflow::Networks::PortDescriptionInterface* portToConnect, const std::string& newModuleName)
{
  controller_->connectNewModule(moduleToConnectTo, portToConnect, newModuleName);
}

const ModuleDescriptionMap& NetworkEditorControllerGuiProxy::getAllAvailableModuleDescriptions() const
{
  return controller_->getAllAvailableModuleDescriptions();
}

boost::shared_ptr<DisableDynamicPortSwitch> NetworkEditorControllerGuiProxy::createDynamicPortSwitch()
{
  return controller_->createDynamicPortSwitch();
}

void NetworkEditorControllerGuiProxy::cleanUpNetwork()
{
  controller_->cleanUpNetwork();
}
