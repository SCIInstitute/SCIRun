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
/// @todo Documentation Dataflow/Engine/Controller/PythonImpl.cc

#ifdef BUILD_WITH_PYTHON

#include <boost/range/adaptor/transformed.hpp>
#include <boost/python/to_python_converter.hpp>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/PortInterface.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Dataflow/Engine/Controller/PythonImpl.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

namespace
{
  class PyPortImpl : public PyPort
  {
  public:
    PyPortImpl(boost::shared_ptr<PortDescriptionInterface> port, NetworkEditorController& nec) : port_(port), nec_(nec)
    {
    }
    virtual std::string name() const override
    {
      return port_ ? port_->get_portname() : "<Null>";
    }

    virtual std::string type() const override
    {
      return port_ ? port_->get_typename() : "<Null>";
    }

    virtual bool isInput() const override
    {
      return port_ ? port_->isInput() : false;
    }

    virtual void connect(const PyPort& other) const override
    {
      auto otherPort = dynamic_cast<const PyPortImpl*>(&other);
      if (port_ && otherPort)
        nec_.requestConnection(port_.get(), otherPort->port_.get());
    }

    void reset()
    {
      port_.reset();
    }
  private:
    boost::shared_ptr<PortDescriptionInterface> port_;
    NetworkEditorController& nec_;
  };

  /// @todo: need to test with dynamic ports
  class PyPortsImpl : public PyPorts
  {
  public:
    PyPortsImpl(ModuleHandle mod, bool input, NetworkEditorController& nec) : nec_(nec), modId_(mod->get_id())
    {
      //wish:
      //boost::push_back(ports_,
      //  (input ? mod->inputPorts() : mod->outputPorts())
      //  | boost::adaptors::transformed([&](boost::shared_ptr<PortDescriptionInterface> p) { return boost::make_shared<PyPortImpl>(p, nec_); })
      //  );
      if (input)
      {
        for (const auto& p : mod->inputPorts())
          ports_.push_back(boost::make_shared<PyPortImpl>(p, nec_));
      }
      else
      {
        for (const auto& p : mod->outputPorts())
          ports_.push_back(boost::make_shared<PyPortImpl>(p, nec_));
      }
    }

    virtual boost::shared_ptr<PyPort> getattr(const std::string& name) override
    {
      auto port = std::find_if(ports_.begin(), ports_.end(), [&](boost::shared_ptr<PyPortImpl> p) { return name == p->name(); });
      if (port != ports_.end())
        return *port;

      std::cerr << "Could not find port with name " << name << " on module " << modId_.id_ << std::endl;
      PyErr_SetObject(PyExc_KeyError, boost::python::object(name).ptr());
      throw boost::python::error_already_set();
    }

    virtual boost::shared_ptr<PyPort> getitem(int index) override
    {
      if (index < 0)
        index += size();
      if (index < 0 || index >= size())
      {
        PyErr_SetObject(PyExc_KeyError, boost::python::object(index).ptr());
        throw boost::python::error_already_set();
      }
      return ports_[index];
    }

    virtual size_t size() const override
    {
      return ports_.size();
    }

    void reset()
    {
      std::for_each(ports_.begin(), ports_.end(), [](boost::shared_ptr<PyPortImpl> p) { p->reset(); p.reset(); });
      ports_.clear();
    }
  private:
    std::vector<boost::shared_ptr<PyPortImpl>> ports_;
    NetworkEditorController& nec_;
    ModuleId modId_;
  };

  class PyModuleImpl : public PyModule
  {
  public:
    PyModuleImpl(ModuleHandle mod, NetworkEditorController& nec) : module_(mod), nec_(nec)
    {
      if (module_)
      {
        input_ = boost::make_shared<PyPortsImpl>(module_, true, nec_);
        output_ = boost::make_shared<PyPortsImpl>(module_, false, nec_);
      }
    }

    virtual std::string id() const override
    {
      if (module_)
        return module_->get_id();
      return "<Null module>";
    }

    virtual void showUI() override
    {
      if (module_)
        module_->setUiVisible(true);
    }

    virtual void hideUI() override
    {
      if (module_)
        module_->setUiVisible(false);
    }

    virtual void reset() override
    {
      module_.reset();
      input_->reset();
      output_->reset();
      input_.reset();
      output_.reset();
    }

    virtual boost::python::object getattr(const std::string& name) override
    {
      if (module_)
      {
        auto state = module_->get_state();
        AlgorithmParameterName apn(name);
        if (!state->containsKey(apn))
        {
          throw std::invalid_argument("Module state key " + name + " not defined.");
        }

        auto v = state->getValue(apn);

//TODO: extract and use for state get/set
        /// @todo: extract
        if ( const int* p = boost::get<int>( &v.value() ) )
          return boost::python::object(*p);
        else if ( const std::string* p = boost::get<std::string>( &v.value() ) )
          return boost::python::object(*p);
        else if ( const double* p = boost::get<double>( &v.value() ) )
          return boost::python::object(*p);
        else if ( const bool* p = boost::get<bool>( &v.value() ) )
          return boost::python::object(*p);

        return boost::python::object();
      }
      return boost::python::object();
    }

    virtual void setattr(const std::string& name, boost::python::object object) override
    {
      if (module_)
      {
        auto state = module_->get_state();
        AlgorithmParameterName apn(name);
        if (!state->containsKey(apn))
        {
          throw std::invalid_argument("Module state key " + name + " not defined.");
        }
        state->setValue(apn, convert(object));
      }
    }

    virtual std::vector<std::string> stateVars() const override
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

    virtual std::string stateToString() const override
    {
      if (module_)
      {
        std::ostringstream ostr;
        auto state = module_->get_state();
        for (const auto& key : state->getKeys())
        {
          ostr << state->getValue(key) << std::endl;
        }
        return ostr.str();
      }
      return "[null module]";
    }

    virtual boost::shared_ptr<PyPorts> output() override
    {
      return output_;
    }

    virtual boost::shared_ptr<PyPorts> input() override
    {
      return input_;
    }

  private:
    ModuleHandle module_;
    NetworkEditorController& nec_;
    boost::shared_ptr<PyPortsImpl> input_, output_;

//TODO: extract and use for state get/set
    AlgorithmParameter::Value convert(boost::python::object object) const
    {
      AlgorithmParameter::Value value;

      /// @todo: yucky
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

namespace SCIRun {
  namespace Dataflow {
    namespace Engine {
      class PythonImplImpl
      {
      public:
        std::map<std::string, std::map<int, std::map<std::string, std::map<int, std::string>>>> connectionIdLookup_; //seems silly
      };
    }
  }
}

PythonImpl::PythonImpl(NetworkEditorController& nec, GlobalCommandFactoryHandle cmdFactory) : impl_(new PythonImplImpl), nec_(nec), cmdFactory_(cmdFactory), executionMutex_(nullptr)
{
  nec_.connectNetworkExecutionFinished([this](int) { executionFromPythonFinish(0); });
}

void PythonImpl::setLock(Mutex* mutex)
{
  executionMutex_ = mutex;
}

void PythonImpl::executionFromPythonStart()
{
  //std::cout << "Python impl exec start" << std::endl;
}

void PythonImpl::executionFromPythonFinish(int)
{
  if (executionMutex_)
    executionMutex_->unlock();
}

boost::shared_ptr<PyModule> PythonImpl::addModule(const std::string& name)
{
  auto m = nec_.addModule(name);
  if (m)
    std::cout << "Module added: " + m->get_id().id_ << std::endl;
  else
    std::cout << "Module add failed, no such module type" << std::endl;
  return boost::make_shared<PyModuleImpl>(m, nec_);
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

std::string PythonImpl::executeAll(const ExecutableLookup* lookup)
{
  nec_.executeAll(lookup);
  return "Execution finished.";
}

std::string PythonImpl::connect(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex)
{
  auto network = nec_.getNetwork();
  auto modFrom = network->lookupModule(ModuleId(moduleIdFrom));
  auto outputPort = modFrom->outputPorts().at(fromIndex);
  auto modTo = network->lookupModule(ModuleId(moduleIdTo));
  auto inputPort = modTo->inputPorts().at(toIndex);
  auto id = nec_.requestConnection(outputPort.get(), inputPort.get());
  if (id)
  {
    impl_->connectionIdLookup_[moduleIdFrom][fromIndex][moduleIdTo][toIndex] = id->id_;
  }

  return "PythonImpl::connect success";
}

std::string PythonImpl::disconnect(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex)
{
  //TODO: doesn't work at all since there is no GUI connection to this network change event. Issue is #...
  auto id = impl_->connectionIdLookup_[moduleIdFrom][fromIndex][moduleIdTo][toIndex];
  if (!id.empty())
  {
    nec_.removeConnection(id);
    return "PythonImpl::disconnect IS NOT IMPLEMENTED";
  }
  else
  {
    return "PythonImpl::disconnect: connection not found";
  }
}

std::string PythonImpl::saveNetwork(const std::string& filename)
{
  auto save = cmdFactory_->create(GlobalCommands::SaveNetworkFile);
  save->set(Variables::Filename, filename);
  return save->execute() ? (filename + " saved") : "Save failed";
  //TODO: provide more informative python return value string
}

std::string PythonImpl::loadNetwork(const std::string& filename)
{
  auto load = cmdFactory_->create(GlobalCommands::LoadNetworkFile);
  load->set(Variables::Filename, filename);
  return load->execute() ? (filename + " loaded") : "Load failed";
  //TODO: provide more informative python return value string
}

std::string PythonImpl::quit(bool force)
{
  if (force)
    cmdFactory_->create(GlobalCommands::QuitCommand)->execute();
  else
    cmdFactory_->create(GlobalCommands::SetupQuitAfterExecute)->execute();
  return "Quit after execute enabled.";
}

#endif
