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


/// @todo Documentation Dataflow/Engine/Controller/PythonImpl.cc

#ifdef BUILD_WITH_PYTHON

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
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Matlab/matlabfile.h>
#include <Core/Utils/CurrentFileName.h>

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <Core/Python/PythonDatatypeConverter.h>
#include <Core/Python/PythonInterpreter.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Python;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;


namespace
{
  class PyDatatypeString : public PyDatatype
  {
  public:
    explicit PyDatatypeString(StringHandle underlying) : underlying_(underlying), str_(convertStringToPython(underlying))
    {
    }

    virtual std::string type() const override
    {
      return underlying_->dynamic_type_name();
    }

    virtual boost::python::object value() const override
    {
      return str_;
    }

  private:
    StringHandle underlying_;
    boost::python::object str_;
  };

  class PyDatatypeDenseMatrix : public PyDatatype
  {
  public:
    explicit PyDatatypeDenseMatrix(DenseMatrixHandle underlying) : underlying_(underlying), pyMat_(convertMatrixToPython(underlying))
    {
    }

    virtual std::string type() const override
    {
      return underlying_->dynamic_type_name();
    }

    virtual boost::python::object value() const override
    {
      return pyMat_;
    }

  private:
    DenseMatrixHandle underlying_;
    boost::python::list pyMat_;
  };

  class PyDatatypeSparseRowMatrix : public PyDatatype
  {
  public:
    explicit PyDatatypeSparseRowMatrix(SparseRowMatrixHandle underlying) : underlying_(underlying), pyMat_(convertMatrixToPython(underlying))
    {
    }

    virtual std::string type() const override
    {
      return underlying_->dynamic_type_name();
    }

    virtual boost::python::object value() const override
    {
      return pyMat_;
    }

  private:
    SparseRowMatrixHandle underlying_;
    boost::python::dict pyMat_;
  };

  class PyDatatypeField : public PyDatatype
  {
  public:
    explicit PyDatatypeField(FieldHandle underlying) : underlying_(underlying), matlabStructure_(convertFieldToPython(underlying))
    {
    }

    virtual std::string type() const override
    {
      return underlying_->dynamic_type_name();
    }

    virtual boost::python::object value() const override
    {
      return matlabStructure_;
    }

  private:
    FieldHandle underlying_;
    boost::python::dict matlabStructure_;
  };

  class PyDatatypeFactory
  {
  public:
    static boost::shared_ptr<PyDatatype> createWrapper(DatatypeHandle data)
    {
      {
        auto str = boost::dynamic_pointer_cast<String>(data);
        if (str)
          return boost::make_shared<PyDatatypeString>(str);
      }
      {
        auto dense = boost::dynamic_pointer_cast<DenseMatrix>(data);
        if (dense)
          return boost::make_shared<PyDatatypeDenseMatrix>(dense);
      }
      {
        auto sparse = boost::dynamic_pointer_cast<SparseRowMatrix>(data);
        if (sparse)
          return boost::make_shared<PyDatatypeSparseRowMatrix>(sparse);
      }
      {
        auto field = boost::dynamic_pointer_cast<Field>(data);
        if (field)
          return boost::make_shared<PyDatatypeField>(field);
      }
      return nullptr;
    }
  };

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

    virtual std::string id() const override
    {
      return port_ ? port_->id().toString() : "<Null>";
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

    virtual std::string dataTypeName() const override
    {
      auto output = boost::dynamic_pointer_cast<OutputPortInterface>(port_);
      if (output)
        return "Output port data not available yet!";

      auto data = getDataImpl();
      if (!data)
        return "[No data]";
      if (!*data)
        return "[Null data]";
      return (*data)->dynamic_type_name();
    }

    virtual boost::shared_ptr<PyDatatype> data() const override
    {
      auto dataOpt = getDataImpl();
      if (dataOpt && *dataOpt)
      {
        return PyDatatypeFactory::createWrapper(*dataOpt);
      }
      return nullptr;
    }

    void reset()
    {
      port_.reset();
    }
  private:
    DatatypeHandleOption getDataImpl() const
    {
      auto input = boost::dynamic_pointer_cast<InputPortInterface>(port_);
      if (input)
      {
        return input->getData();
      }
      return boost::none;
    }
    boost::shared_ptr<PortDescriptionInterface> port_;
    NetworkEditorController& nec_;
  };

  /// @todo: need to test with dynamic ports
  class PyPortsImpl : public PyPorts
  {
  public:
    PyPortsImpl(ModuleHandle mod, bool input, NetworkEditorController& nec) : mod_(mod), nec_(nec), modId_(mod->id())
    {
      //wish:
      //boost::push_back(ports_,
      //  (input ? mod->inputPorts() : mod->outputPorts())
      //  | boost::adaptors::transformed([&](boost::shared_ptr<PortDescriptionInterface> p) { return boost::make_shared<PyPortImpl>(p, nec_); })
      //  );
      if (input)
      {
        setupInputs();

        if (mod->hasDynamicPorts())
        {
          connections_.push_back(boost::make_shared<boost::signals2::scoped_connection>(nec_.connectPortAdded([this](const ModuleId& mid, const PortId& pid) { portAddedSlot(mid, pid); })));
          connections_.push_back(boost::make_shared<boost::signals2::scoped_connection>(nec_.connectPortRemoved([this](const ModuleId& mid, const PortId& pid) { portRemovedSlot(mid, pid); })));
        }
      }
      else
      {
        setupOutputs();
      }
    }

    virtual boost::shared_ptr<PyPort> getattr(const std::string& name) override
    {
      auto port = std::find_if(ports_.begin(), ports_.end(), [&](boost::shared_ptr<PyPortImpl> p) { return name == p->name(); });
      if (port != ports_.end())
        return *port;

      port = std::find_if(ports_.begin(), ports_.end(), [&](boost::shared_ptr<PyPortImpl> p) { return name == p->id(); });
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
    void portAddedSlot(const ModuleId& mid, const PortId& pid)
    {
      setupInputs();
    }

    void portRemovedSlot(const ModuleId& mid, const PortId& pid)
    {
      setupInputs();
    }

    void setupInputs()
    {
      ports_.clear();
      for (const auto& p : mod_->inputPorts())
        ports_.push_back(boost::make_shared<PyPortImpl>(p, nec_));
    }

    void setupOutputs()
    {
      ports_.clear();
      for (const auto& p : mod_->outputPorts())
        ports_.push_back(boost::make_shared<PyPortImpl>(p, nec_));
    }

    std::vector<boost::shared_ptr<PyPortImpl>> ports_;
    ModuleHandle mod_;
    NetworkEditorController& nec_;
    ModuleId modId_;
    std::vector<boost::shared_ptr<boost::signals2::scoped_connection>> connections_;
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
      creationTime_ = boost::posix_time::second_clock::local_time();
    }

    virtual std::string id() const override
    {
      if (module_)
        return module_->id();
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

    virtual boost::python::object getattr(const std::string& name, bool transient) override
    {
      if (module_)
      {
        if (!transient)
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
        else
        {
          auto state = module_->get_state();
          AlgorithmParameterName apn(name);

          auto v = state->getTransientValue(apn);

          //TODO: extract and use for state get/set
          /// @todo: extract
          if (transient_value_check<int>(v))
            return boost::python::object(transient_value_cast<int>(v));
          if (transient_value_check<std::string>(v))
            return boost::python::object(transient_value_cast<std::string>(v));
          if (transient_value_check<double>(v))
            return boost::python::object(transient_value_cast<double>(v));
          if (transient_value_check<bool>(v))
            return boost::python::object(transient_value_cast<bool>(v));
          if (transient_value_check<Variable>(v))
            return boost::python::object(convertVariableToPythonObject(transient_value_cast<Variable>(v)));
          if (transient_value_check<boost::python::object>(v))
            return transient_value_cast<boost::python::object>(v);

          return boost::python::object();
        }
      }
      return boost::python::object();
    }

    virtual void setattr(const std::string& name, boost::python::object object, bool transient) override
    {
      if (module_)
      {
        auto state = module_->get_state();
        AlgorithmParameterName apn(name);
        if (!transient)
        {
          if (!state->containsKey(apn))
          {
            throw std::invalid_argument("Module state key " + name + " not defined.");
          }
          state->setValue(apn, convertPythonObjectToVariable(object).value());
        }
        else
        {
          state->setTransientValue(apn, convertPythonObjectToVariable(object), false);
        }
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
      return {};
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

    virtual boost::posix_time::ptime creationTime() const override
    {
      return creationTime_;
    }

  private:
    ModuleHandle module_;
    NetworkEditorController& nec_;
    boost::shared_ptr<PyPortsImpl> input_, output_;
    boost::posix_time::ptime creationTime_;
  };
}

namespace SCIRun {
  namespace Dataflow {
    namespace Engine {
      class PythonImplImpl
      {
      public:
        std::map<std::string, std::map<int, std::map<std::string, std::map<int, std::string>>>> connectionIdLookup_;
      };
    }
  }
}

PythonImpl::PythonImpl(NetworkEditorController& nec, GlobalCommandFactoryHandle cmdFactory) : impl_(new PythonImplImpl), nec_(nec), cmdFactory_(cmdFactory)
{
  connections_.push_back(nec_.connectNetworkExecutionFinished([this](int) { executionFromPythonFinish(0); }));
  connections_.push_back(nec_.connectModuleAdded([this](const std::string& id, ModuleHandle m, ModuleCounter mc) { pythonModuleAddedSlot(id, m, mc); }));
  connections_.push_back(nec_.connectModuleRemoved([this](const ModuleId& id) { pythonModuleRemovedSlot(id); }));
}

PythonImpl::~PythonImpl()
{
  for (const auto& c : connections_)
    c.disconnect();
}

void PythonImpl::setUnlockFunc(boost::function<void()> unlock)
{
  unlock_ = unlock;
}

void PythonImpl::executionFromPythonStart()
{
  //std::cout << "Python impl exec start" << std::endl;
}

void PythonImpl::executionFromPythonFinish(int)
{
  if (unlock_)
  {
    unlock_();
  }
}

boost::shared_ptr<PyModule> PythonImpl::addModule(const std::string& name)
{
  auto m = nec_.addModule(name);
  if (m)
    std::cout << "Module added: " + m->id().id_ << std::endl;
  else
    std::cout << "Module add failed, no such module type" << std::endl;

  return modules_[m->id().id_];
}

void PythonImpl::pythonModuleAddedSlot(const std::string& modId, ModuleHandle m, ModuleCounter)
{
  auto pyM = boost::make_shared<PyModuleImpl>(m, nec_);
  modules_[pyM->id()] = pyM;
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

void PythonImpl::pythonModuleRemovedSlot(const ModuleId& mid)
{
  modules_.erase(mid);
}

std::vector<boost::shared_ptr<PyModule>> PythonImpl::moduleList() const
{
  std::vector<boost::shared_ptr<PyModule>> modules;
  boost::copy(modules_ | boost::adaptors::map_values, std::back_inserter(modules));
  std::sort(modules.begin(), modules.end(), [](const boost::shared_ptr<PyModule> lhs, const boost::shared_ptr<PyModule> rhs) { return lhs->creationTime() < rhs->creationTime(); });
  return modules;
}

boost::shared_ptr<PyModule> PythonImpl::findModule(const std::string& id) const
{
  auto modIter = modules_.find(id);
  return modIter != modules_.end() ? modIter->second : nullptr;
}

std::string PythonImpl::executeAll(const ExecutableLookup* lookup)
{
  //cmdFactory_->create(GlobalCommands::DisableViewScenes)->execute();

  nec_.executeAll(lookup);
  return "Execution started."; //TODO: attach log for execution ended event.
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

std::string PythonImpl::currentNetworkFile() const
{
  return SCIRun::Core::getCurrentFileName();
}

std::string PythonImpl::importNetwork(const std::string& filename)
{
  auto import = cmdFactory_->create(GlobalCommands::ImportNetworkFile);
  import->set(Variables::Filename, filename);
  return import->execute() ? (filename + " imported") : "Import failed";
  //TODO: provide more informative python return value string
}

std::string PythonImpl::runScript(const std::string& filename)
{
  PythonInterpreter::Instance().run_script("exec(open('" + filename + "').read())");
  return filename + " executed.";
}

std::string PythonImpl::quit(bool force)
{
  auto quitCmd(cmdFactory_->create(force ? GlobalCommands::QuitCommand : GlobalCommands::SetupQuitAfterExecute));
  quitCmd->set(Name("RunningPython"), true);
  quitCmd->execute();
  return "Quit after execute enabled.";
}

#endif
