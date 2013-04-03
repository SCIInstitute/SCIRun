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
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Dataflow/Engine/Controller/PythonImpl.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

namespace
{
  class PyModuleImpl : public PyModule
  {
  public:
    explicit PyModuleImpl(ModuleHandle mod) : module_(mod) {}

    virtual std::string id() const
    {
      if (module_)
        return module_->get_id();
      return "<Null module>";
    }

    virtual void showUI()
    {
      if (module_)
        module_->setUiVisible(true);
    }

    virtual void hideUI()
    {
      if (module_)
        module_->setUiVisible(false);
    }

    virtual void reset() 
    {
      module_.reset();
    }

    virtual boost::python::object getattr(const std::string& name)
    {
      //std::cout << "i'm in the correct method, calling getattr with " << name << std::endl;
      if (module_)
      {
        auto state = module_->get_state();
        AlgorithmParameterName apn(name);
        if (!state->containsKey(apn))
          return boost::python::object();

        auto v = state->getValue(apn);

        //TODO: extract
        if ( const int* p = boost::get<int>( &v.value_ ) )
          return boost::python::object(*p);
        else if ( const std::string* p = boost::get<std::string>( &v.value_ ) )
          return boost::python::object(*p);
        else if ( const double* p = boost::get<double>( &v.value_ ) )
          return boost::python::object(*p);
        else if ( const bool* p = boost::get<bool>( &v.value_ ) )
          return boost::python::object(*p);

        return boost::python::object();
      }
      return boost::python::object();
    }

    virtual void setattr(const std::string& name, boost::python::object object)
    {
      if (module_)
      {
        auto state = module_->get_state();
        AlgorithmParameterName apn(name);

        //if (!state->containsKey(apn))
        //{
        //  std::cout << "No state variable by name: " << name << std::endl;
        //  return;
        //}

        state->setValue(apn, convert(object));
      }

    }
    
    virtual std::vector<std::string> stateVars() const
    {
      if (module_)
      {
        std::vector<std::string> keyStrings;
        auto keys = module_->get_state()->getKeys();
        std::transform(keys.begin(), keys.end(), std::back_inserter(keyStrings), [](const AlgorithmParameterName& n) { return n.name_; });
        return keyStrings;
      }
      return std::vector<std::string>();
    }
  private:
    ModuleHandle module_;

    AlgorithmParameter::Value convert(boost::python::object object) const
    {
      AlgorithmParameter::Value value;

      //TODO: barf
      {
        boost::python::extract<int> e(object);
        if (e.check())
        {
          value = e();
          return value;
        }
      }
      {
        boost::python::extract<double> e(object);
        if (e.check())
        {
          value = e();
          return value;
        }
      }
      {
        boost::python::extract<std::string> e(object);
        if (e.check())
        {
          value = e();
          return value;
        }
      }
      {
        boost::python::extract<bool> e(object);
        if (e.check())
        {
          value = e();
          return value;
        }
      }
      return value;
    }
  };
}

PythonImpl::PythonImpl(NetworkEditorController& nec) : nec_(nec) {}

boost::shared_ptr<PyModule> PythonImpl::addModule(const std::string& name)
{
  auto m = nec_.addModule(name);
  if (m)
    std::cout << "Module added: " + m->get_id().id_ << std::endl;
  else
    std::cout << "Module add failed, no such module type" << std::endl;
  return boost::make_shared<PyModuleImpl>(m);
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
  try
  {
    //TODO: duplicated code from SCIRunMainWindow. Obviously belongs in a separate class.
    NetworkFileHandle file = nec_.saveNetwork();
    XMLSerializer::save_xml(*file, filename, "networkFile");
    return filename + " saved.";
  }
  catch (...)
  {
    return "Save failed.";
  }
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