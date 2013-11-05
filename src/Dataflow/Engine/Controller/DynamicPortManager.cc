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

#include <Dataflow/Engine/Controller/DynamicPortManager.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/ConnectionId.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

DynamicPortManager::DynamicPortManager(ConnectionAddedSignalType& addedSignal, ConnectionRemovedSignalType& removeSignal, const NetworkInterface* network) : network_(network)
{
  ENSURE_NOT_NULL(network, "DPM needs network object");
  addedSignal.connect(boost::bind(&DynamicPortManager::connectionAddedNeedToCloneAPort, this, _1));
  removeSignal.connect(boost::bind(&DynamicPortManager::connectionRemovedNeedToRemoveAPort, this, _1));
}

void DynamicPortManager::connectionAddedNeedToCloneAPort(const SCIRun::Dataflow::Networks::ConnectionDescription& cd)
{
  //std::cout << "need to clone a port: " << ConnectionId::create(cd).id_ << std::endl;
  //TODO: assumption: dynamic = input
  auto moduleIn = network_->lookupModule(cd.in_.moduleId_);
  Module::Builder builder;
  builder.cloneInputPort(moduleIn, cd.in_.port_);
  portAdded_(moduleIn, cd.in_.port_);
}

void DynamicPortManager::connectionRemovedNeedToRemoveAPort(const SCIRun::Dataflow::Networks::ConnectionId& id)
{
  //std::cout << "need to remove a port: " << id.id_ << std::endl;
  auto desc = id.describe();
  auto moduleIn = network_->lookupModule(desc.in_.moduleId_);
  Module::Builder builder;
  builder.removeInputPort(moduleIn, desc.in_.port_);
  portRemoved_(moduleIn, desc.in_.port_);
}

boost::signals2::connection DynamicPortManager::connectPortAdded(const PortAddedSignalType::slot_type& subscriber)
{
  return portAdded_.connect(subscriber);
}

boost::signals2::connection DynamicPortManager::connectPortRemoved(const PortRemovedSignalType::slot_type& subscriber)
{
  return portRemoved_.connect(subscriber);
}
