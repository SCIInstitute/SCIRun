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

#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Engine/Network/NetworkEditorController.h>

using namespace SCIRun::Engine;
using namespace SCIRun::Gui;
  
NetworkEditorControllerGuiProxy::NetworkEditorControllerGuiProxy(boost::shared_ptr<SCIRun::Engine::NetworkEditorController> controller)
  : controller_(controller)
{
  controller_->connectModuleAdded(boost::bind(&NetworkEditorControllerGuiProxy::moduleAdded, this, _1, _2));
}

void NetworkEditorControllerGuiProxy::addModule(const std::string& moduleName)
{
  controller_->addModule(moduleName);
}

void NetworkEditorControllerGuiProxy::removeModule(const std::string& id)
{
  controller_->removeModule(id);
}

void NetworkEditorControllerGuiProxy::addConnection(const SCIRun::Domain::Networks::ConnectionDescription& desc)
{
  controller_->addConnection(desc);
}

void NetworkEditorControllerGuiProxy::removeConnection(const SCIRun::Domain::Networks::ConnectionId& id)
{
  controller_->removeConnection(id);
}
