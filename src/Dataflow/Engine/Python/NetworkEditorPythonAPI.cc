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
#include <Dataflow/Engine/Python/SCIRunPythonModule.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Networks;

boost::shared_ptr<NetworkEditorPythonInterface> NetworkEditorPythonAPI::impl_;
ExecutableLookup* NetworkEditorPythonAPI::lookup_ = 0;
std::vector<boost::shared_ptr<PyModule>> NetworkEditorPythonAPI::modules_;


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

    boost::python::to_python_converter< std::vector< boost::shared_ptr<PyModule> >, 
      StdVectorToListConverter< boost::shared_ptr<PyModule> >, true >();
    boost::python::to_python_converter< std::vector< std::string >, 
      StdVectorToListConverter< std::string >, true >();
  }
}

void NetworkEditorPythonAPI::setExecutionContext(ExecutableLookup* lookup)
{
  lookup_ = lookup;
}

boost::shared_ptr<PyModule> NetworkEditorPythonAPI::addModule(const std::string& name)
{
  if (impl_)
  {
    auto m = impl_->addModule(name);
    modules_.push_back(m);
    return m;
  }
  else
  {
    std::cout << "Null implementation: NetworkEditorPythonAPI::addModule()" << std::endl;
    return boost::shared_ptr<PyModule>();
  }
}

std::string NetworkEditorPythonAPI::removeModule(const std::string& id)
{
  if (impl_)
  {
    modules_.erase(std::remove_if(modules_.begin(), modules_.end(), [&](boost::shared_ptr<PyModule> m) -> bool { bool same = m->id() == id; if (same) m->reset(); return same; }), modules_.end());
    return impl_->removeModule(id);
  }
  else
  {
    return "Null implementation: NetworkEditorPythonAPI::removeModule()";
  }
}

std::vector<boost::shared_ptr<PyModule>> NetworkEditorPythonAPI::modules()
{
  return modules_;
}

std::string NetworkEditorPythonAPI::executeAll()
{
  if (impl_)
    return impl_->executeAll(lookup_);
  else
  {
    return "Null implementation or execution context: NetworkEditorPythonAPI::executeAll()"; 
  }
}

std::string NetworkEditorPythonAPI::saveNetwork(const std::string& filename)
{
  if (impl_)
    return impl_->saveNetwork(filename);
  else
  {
    return "Null implementation: NetworkEditorPythonAPI::saveNetwork()";
  }
}

std::string NetworkEditorPythonAPI::loadNetwork(const std::string& filename)
{
  if (impl_)
    return impl_->loadNetwork(filename);
  else
  {
    return "Null implementation: NetworkEditorPythonAPI::loadNetwork()";
  }
}

std::string NetworkEditorPythonAPI::quit(bool force)
{
  if (impl_)
    return impl_->quit(force);
  else
  {
    return "Null implementation: NetworkEditorPythonAPI::quit()";
  }
}

/// @todo: bizarre reason for this return type and casting. but it works.
boost::shared_ptr<PyPort> SCIRun::operator>>(const PyPort& from, const PyPort& to)
{
  from.connect(to);
  auto ptr = const_cast<PyPort&>(to).shared_from_this();
  return boost::ref(ptr);
}

std::string SimplePythonAPI::scirun_addModule(const std::string& name)
{
  return NetworkEditorPythonAPI::addModule(name)->id();
}

std::string SimplePythonAPI::scirun_quit(bool force)
{
  return NetworkEditorPythonAPI::quit(force);
}