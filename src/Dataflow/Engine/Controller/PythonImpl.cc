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

#ifdef BUILD_WITH_PYTHON

#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Engine/Controller/PythonImpl.h>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

PythonImpl::PythonImpl(NetworkEditorController& nec) : nec_(nec) {}

std::string PythonImpl::addModule(const std::string& name)
{
  auto m = nec_.addModule(name);
  if (m)
    return "Module added: " + m->get_id().id_;
  else
    return "Module add failed, no such module type";
}

std::string PythonImpl::removeModule(const std::string& id)
{
  try
  {
    nec_.removeModule(ModuleId(id));
    return "Module removed";
  }
  catch (...)
  {
    return "No module by that id";
  }
}

std::string PythonImpl::executeAll(const ExecutableLookup& lookup)
{
  nec_.executeAll(lookup);
  return "Execution finished.";
}

std::string PythonImpl::connect(const std::string& moduleId1, int port1, const std::string& moduleId2, int port2)
{
  auto network = nec_.getNetwork();
  auto mod1 = network->lookupModule(ModuleId(moduleId1));
  auto mod2 = network->lookupModule(ModuleId(moduleId2));
  return "PythonImpl::connect does nothing";
}

std::string PythonImpl::disconnect(const std::string& moduleId1, int port1, const std::string& moduleId2, int port2)
{
  return "PythonImpl::disconnect does nothing";
}

std::string PythonImpl::saveNetwork(const std::string& filename)
{
  return "PythonImpl::saveNetwork does nothing";
}

std::string PythonImpl::loadNetwork(const std::string& filename)
{
  return "PythonImpl::loadNetwork does nothing";
}

std::string PythonImpl::quit(bool force)
{
  return "PythonImpl::quit does nothing";
}

#endif