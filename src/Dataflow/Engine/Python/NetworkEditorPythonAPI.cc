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
/// @todo Documentation Dataflow/Engine/Python/NetworkEditorPythonAPI.cc

#include <Python.h>
#include <iostream>
#include <Dataflow/Engine/Python/NetworkEditorPythonInterface.h>
#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>
#include <boost/range/adaptors.hpp>
#include <Core/Python/PythonDatatypeConverter.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;

boost::shared_ptr<NetworkEditorPythonInterface> NetworkEditorPythonAPI::impl_;
ExecutableLookup* NetworkEditorPythonAPI::lookup_ = nullptr;
Mutex NetworkEditorPythonAPI::pythonLock_("Python");
std::atomic<bool> NetworkEditorPythonAPI::executeLockedFromPython_(false);
std::atomic<bool> NetworkEditorPythonAPI::convertersRegistered_(false);

template< class T >
class StdVectorToListConverter : public boost::python::converter::wrap_pytype< &PyList_Type >
{
public:
  static PyObject* convert( const std::vector< T >& v )
  {
    boost::python::list result;
    for ( size_t i = 0; i < v.size(); ++i )
    {
      result.append( v[ i ] );
    }

    return boost::python::incref( result.ptr() );
  }
};

void NetworkEditorPythonAPI::setImpl(boost::shared_ptr<NetworkEditorPythonInterface> impl)
{
  if (!impl_)
  {
    impl_ = impl;
    impl_->setUnlockFunc([]() { unlock(); });

    if (!convertersRegistered_)
    {
      boost::python::to_python_converter< std::vector< boost::shared_ptr<PyModule> >,
        StdVectorToListConverter< boost::shared_ptr<PyModule> >, true >();
      boost::python::to_python_converter< std::vector< std::string >,
        StdVectorToListConverter< std::string >, true >();
      convertersRegistered_ = true;
    }
  }
}

void NetworkEditorPythonAPI::clearImpl()
{
  impl_.reset();
}

void NetworkEditorPythonAPI::setExecutionContext(ExecutableLookup* lookup)
{
  lookup_ = lookup;
}

boost::shared_ptr<PyModule> NetworkEditorPythonAPI::addModule(const std::string& name)
{
  Guard g(pythonLock_.get());

  if (impl_ && impl_->isModuleContext())
    return nullptr;

  if (impl_)
  {
    return impl_->addModule(name);
  }
  else
  {
    std::cout << "Null implementation: NetworkEditorPythonAPI::addModule()" << std::endl;
    return nullptr;
  }
}

std::string NetworkEditorPythonAPI::removeModule(const std::string& id)
{
  Guard g(pythonLock_.get());

  if (impl_ && impl_->isModuleContext())
    return "In module context--function not available";

  if (impl_)
  {
    return impl_->removeModule(id);
  }
  else
  {
    return "Null implementation: NetworkEditorPythonAPI::removeModule()";
  }
}

std::vector<boost::shared_ptr<PyModule>> NetworkEditorPythonAPI::modules()
{
  Guard g(pythonLock_.get());
  return impl_->moduleList();
}

std::string NetworkEditorPythonAPI::executeAll()
{
  if (impl_ && impl_->isModuleContext())
    return "In module context--function not available";

  if (impl_)
  {
    //std::cout << "executionMutex_->lock attempt " << boost::this_thread::get_id() << std::endl;
    pythonLock_.lock();
    executeLockedFromPython_ = true;
    //std::cout << "executionMutex_->lock call done" << boost::this_thread::get_id() << std::endl;
    return impl_->executeAll(lookup_);
  }
  else
  {
    return "Null implementation or execution context: NetworkEditorPythonAPI::executeAll()";
  }
}

void NetworkEditorPythonAPI::unlock()
{
  if (executeLockedFromPython_)
    pythonLock_.unlock();
  executeLockedFromPython_ = false;
}

std::string NetworkEditorPythonAPI::connect(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex)
{
  Guard g(pythonLock_.get());

  if (impl_ && impl_->isModuleContext())
    return "In module context--function not available";

  if (impl_)
    return impl_->connect(moduleIdFrom, fromIndex, moduleIdTo, toIndex);
  else
  {
    return "Null implementation or execution context: NetworkEditorPythonAPI::connect()";
  }
}

std::string NetworkEditorPythonAPI::disconnect(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex)
{
  Guard g(pythonLock_.get());

  if (impl_ && impl_->isModuleContext())
    return "In module context--function not available";

  if (impl_)
    return impl_->disconnect(moduleIdFrom, fromIndex, moduleIdTo, toIndex);
  else
  {
    return "Null implementation or execution context: NetworkEditorPythonAPI::disconnect()";
  }
}

std::string NetworkEditorPythonAPI::saveNetwork(const std::string& filename)
{
  Guard g(pythonLock_.get());

  if (impl_ && impl_->isModuleContext())
    return "In module context--function not available";

  if (impl_)
    return impl_->saveNetwork(filename);
  else
  {
    return "Null implementation: NetworkEditorPythonAPI::saveNetwork()";
  }
}

std::string NetworkEditorPythonAPI::currentNetworkFile()
{
  Guard g(pythonLock_.get());

  if (impl_)
    return impl_->currentNetworkFile();
  else
  {
    return "Null implementation: NetworkEditorPythonAPI::currentNetworkFile()";
  }
}

std::string NetworkEditorPythonAPI::loadNetwork(const std::string& filename)
{
  Guard g(pythonLock_.get());

  if (impl_ && impl_->isModuleContext())
    return "In module context--function not available";

  if (impl_)
    return impl_->loadNetwork(filename);
  else
  {
    return "Null implementation: NetworkEditorPythonAPI::loadNetwork()";
  }
}

std::string NetworkEditorPythonAPI::importNetwork(const std::string& filename)
{
  Guard g(pythonLock_.get());

  if (impl_ && impl_->isModuleContext())
    return "In module context--function not available";

  if (impl_)
    return impl_->importNetwork(filename);
  else
  {
    return "Null implementation: NetworkEditorPythonAPI::importNetwork()";
  }
}

std::string NetworkEditorPythonAPI::runScript(const std::string& filename)
{
  Guard g(pythonLock_.get());

  if (impl_ && impl_->isModuleContext())
    return "In module context--function not available";

  if (impl_)
    return impl_->runScript(filename);
  else
  {
    return "Null implementation: NetworkEditorPythonAPI::runScript()";
  }
}

std::string NetworkEditorPythonAPI::quit(bool force)
{
  Guard g(pythonLock_.get());

  if (impl_ && impl_->isModuleContext())
    return "In module context--function not available";

  if (impl_)
    return impl_->quit(force);
  else
  {
    return "Null implementation: NetworkEditorPythonAPI::quit()";
  }
}

boost::python::object NetworkEditorPythonAPI::scirun_get_module_state(const std::string& moduleId, const std::string& stateVariable)
{
  Guard g(pythonLock_.get());
  auto module = impl_->findModule(moduleId);
  if (module)
    return module->getattr(stateVariable, false);
  return boost::python::object();
}

std::string NetworkEditorPythonAPI::scirun_set_module_state(const std::string& moduleId, const std::string& stateVariable, const boost::python::object& value)
{
  Guard g(pythonLock_.get());
  auto module = impl_->findModule(moduleId);
  if (module)
  {
    if (stateVariable == "__UI__")
    {
      boost::python::extract<bool> e(value);
      if (e.check())
      {
        e() ? module->showUI() : module->hideUI();
        return "UI adjusted";
      }
    }
    module->setattr(stateVariable, value, false);
    return "Value set";
  }
  return "Module or value not found";
}

boost::python::object NetworkEditorPythonAPI::scirun_get_module_transient_state(const std::string& moduleId, const std::string& stateVariable)
{
  Guard g(pythonLock_.get());
  auto module = impl_->findModule(moduleId);
  if (module)
    return module->getattr(stateVariable, true);
  return boost::python::object();
}

std::string NetworkEditorPythonAPI::scirun_set_module_transient_state(const std::string& moduleId, const std::string& stateVariable, const boost::python::object& value)
{
  //Guard g(pythonLock_.get());
  auto module = impl_->findModule(moduleId);
  if (module)
  {
    module->setattr(stateVariable, value, true);
    return "Transient value set";
  }
  return "Module or value not found";
}

std::string NetworkEditorPythonAPI::scirun_dump_module_state(const std::string& moduleId)
{
  Guard g(pythonLock_.get());
  auto module = impl_->findModule(moduleId);
  if (module)
  {
    return module->stateToString();
  }
  return "Module not found";
}

/// @todo: bizarre reason for this return type and casting. but it works.
boost::shared_ptr<PyPort> SCIRun::operator>>(const PyPort& from, const PyPort& to)
{
  Guard g(NetworkEditorPythonAPI::getLock().get());
  from.connect(to);
  auto ptr = const_cast<PyPort&>(to).shared_from_this();
  return boost::ref(ptr);
}

std::string SimplePythonAPI::scirun_add_module(const std::string& name)
{
  auto mod = NetworkEditorPythonAPI::addModule(name);
  return mod ? mod->id() : "<Null module--function not available or module not defined>";
}

std::string SimplePythonAPI::scirun_quit()
{
  return NetworkEditorPythonAPI::quit(false);
}

std::string SimplePythonAPI::scirun_force_quit()
{
  return NetworkEditorPythonAPI::quit(true);
}

std::string NetworkEditorPythonAPI::scirun_get_module_input_type(const std::string& moduleId, int portIndex)
{
  Guard g(pythonLock_.get());

  auto module = impl_->findModule(moduleId);
  if (module)
  {
    auto port = module->input()->getitem(portIndex);
    if (port)
    {
      return "INPUT FROM " + moduleId + " port " + boost::lexical_cast<std::string>(portIndex) + " NAME = " + port->name()
        + "  Typename: " + port->dataTypeName();
    }
    return "Port not available";
  }
  return "Module not found";
}

//TODO: refactor copy/paste
boost::shared_ptr<PyDatatype> NetworkEditorPythonAPI::scirun_get_module_input_object_index(const std::string& moduleId, int portIndex)
{
  Guard g(pythonLock_.get()/*, "NetworkEditorPythonAPI::scirun_get_module_input"*/);

  auto module = impl_->findModule(moduleId);
  if (module)
  {
    auto port = module->input()->getitem(portIndex);
    if (port)
    {
      return port->data();
    }
  }
  return nullptr;
}

boost::shared_ptr<PyDatatype> NetworkEditorPythonAPI::scirun_get_module_input_object(const std::string& moduleId, const std::string& portName)
{
  Guard g(pythonLock_.get()/*, "NetworkEditorPythonAPI::scirun_get_module_input"*/);

  auto module = impl_->findModule(moduleId);
  if (module)
  {
    auto port = module->input()->getattr(portName);
    if (port)
    {
      return port->data();
    }
  }
  return nullptr;
}

boost::python::object NetworkEditorPythonAPI::scirun_get_module_input_value_index(const std::string& moduleId, int portIndex)
{
  auto pyData = scirun_get_module_input_object_index(moduleId, portIndex);
  Guard g(pythonLock_.get()/*, "NetworkEditorPythonAPI::scirun_get_module_input_copy"*/);
  if (pyData)
    return pyData->value();
  return {};
}

boost::python::object NetworkEditorPythonAPI::scirun_get_module_input_value(const std::string& moduleId, const std::string& portName)
{
  auto pyData = scirun_get_module_input_object(moduleId, portName);
  Guard g(pythonLock_.get()/*, "NetworkEditorPythonAPI::scirun_get_module_input_copy"*/);
  if (pyData)
    return pyData->value();
  return {};
}

boost::python::object SimplePythonAPI::scirun_module_ids()
{
  auto mods = NetworkEditorPythonAPI::modules();
  std::vector<std::string> ids;
  std::transform(mods.begin(), mods.end(), std::back_inserter(ids), [](const PyModulePtr& pm) { return pm->id(); });
  return Core::Python::toPythonList(ids);
}

NetworkEditorPythonAPI::PythonModuleContextApiDisabler::PythonModuleContextApiDisabler()
{
  if (impl_)
    impl_->setModuleContext(true);
}

NetworkEditorPythonAPI::PythonModuleContextApiDisabler::~PythonModuleContextApiDisabler()
{
  if (impl_)
    impl_->setModuleContext(false);
}
