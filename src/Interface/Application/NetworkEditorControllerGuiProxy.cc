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


#include <iostream>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Network/NetworkSettings.h>
#include <QDebug>
#include <Core/Logging/Log.h>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Logging;

NetworkEditorControllerGuiProxy::NetworkEditorControllerGuiProxy(SharedPointer<NetworkEditorController> controller, NetworkEditor* editor)
  : controller_(controller), editor_(editor)
{
  connections_.emplace_back(controller_->connectModuleAdded([this](const std::string& name, ModuleHandle module, const ModuleCounter& count)
    { moduleAdded(name, module, count); }));
  connections_.emplace_back(controller_->connectModuleRemoved([this](const ModuleId& id)
    { moduleRemoved(id); }));
  connections_.emplace_back(controller_->connectConnectionAdded([this](const ConnectionDescription& cd)
    { connectionAdded(cd); }));
  connections_.emplace_back(controller_->connectConnectionRemoved([this](const ConnectionId& id)
    { connectionRemoved(id); }));
  connections_.emplace_back(controller_->connectPortAdded([this](const ModuleId& mid, const PortId& pid)
    { portAdded(mid, pid); }));
  connections_.emplace_back(controller_->connectPortRemoved([this](const ModuleId& mid, const PortId& pid)
    { portRemoved(mid, pid); }));
  connections_.emplace_back(controller_->connectStaticNetworkExecutionStarts([this]()
    { executionStarted(); }));
  connections_.emplace_back(controller_->connectStaticNetworkExecutionFinished([this](int ret)
    { executionFinished(ret); }));
  connections_.emplace_back(controller_->connectNetworkDoneLoading([this](int nMod)
    { networkDoneLoading(nMod); }));
}

NetworkEditorControllerGuiProxy::~NetworkEditorControllerGuiProxy()
{
  for (auto& c : connections_)
    c.disconnect();
}

SCIRun::SharedPointer<NetworkEditorControllerGuiProxy> NetworkEditorControllerGuiProxy::withSubnet(NetworkEditor* subnet) const
{
  return makeShared<NetworkEditorControllerGuiProxy>(controller_, subnet);
}

void NetworkEditorControllerGuiProxy::addModule(const std::string& moduleName)
{
  try
  {
    controller_->addModule(moduleName);
  }
  catch (SCIRun::Core::InvalidArgumentException& e)
  {
    logError("CAUGHT EXCEPTION: {}", e.what());
  }
}

void NetworkEditorControllerGuiProxy::removeModule(const ModuleId& id)
{
  controller_->removeModule(id);
}

std::optional<ConnectionId> NetworkEditorControllerGuiProxy::requestConnection(const PortDescriptionInterface* from, const PortDescriptionInterface* to)
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

void NetworkEditorControllerGuiProxy::setExecutableLookup(const ExecutableLookup* lookup)
{
  controller_->setExecutableLookup(lookup);
}

void NetworkEditorControllerGuiProxy::executeAll()
{
  controller_->executeAll();
}

void NetworkEditorControllerGuiProxy::executeModule(const ModuleHandle& module, bool executeUpstream)
{
  controller_->executeModule(module, executeUpstream);
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

void NetworkEditorControllerGuiProxy::duplicateModule(const ModuleHandle& module)
{
  controller_->duplicateModule(module);
}

void NetworkEditorControllerGuiProxy::connectNewModule(const PortDescriptionInterface* portToConnect, const std::string& newModuleName)
{
  controller_->connectNewModule(portToConnect, newModuleName);
}

void NetworkEditorControllerGuiProxy::insertNewModule(const PortDescriptionInterface* portToConnect, const QMap<QString, std::string>& info)
{
  controller_->insertNewModule(portToConnect, { info["moduleToAdd"], info["endModuleId"], info["inputPortName"], info["inputPortId"]});
}

const ModuleDescriptionMap& NetworkEditorControllerGuiProxy::getAllAvailableModuleDescriptions() const
{
  return controller_->getAllAvailableModuleDescriptions();
}

SCIRun::SharedPointer<DisableDynamicPortSwitch> NetworkEditorControllerGuiProxy::createDynamicPortSwitch()
{
  return controller_->createDynamicPortSwitch();
}

void NetworkEditorControllerGuiProxy::cleanUpNetwork()
{
  controller_->cleanUpNetwork();
}

std::vector<ModuleExecutionState::Value> NetworkEditorControllerGuiProxy::moduleExecutionStates() const
{
  if (!controller_)
  {
    return {};
  }
  return controller_->moduleExecutionStates();
}
