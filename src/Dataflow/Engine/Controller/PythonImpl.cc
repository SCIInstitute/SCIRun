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
#include <Dataflow/Network/Connection.h>
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

namespace py = boost::python;

namespace
{
  class PyDatatypeString : public PyDatatype
  {
  public:
    explicit PyDatatypeString(StringHandle underlying) : underlying_(underlying), str_(convertStringToPython(underlying))
    {
    }

    std::string type() const override
    {
      return underlying_->dynamic_type_name();
    }

    py::object value() const override
    {
      return str_;
    }

  private:
    StringHandle underlying_;
    py::object str_;
  };

  class PyDatatypeDenseMatrix : public PyDatatype
  {
  public:
    explicit PyDatatypeDenseMatrix(DenseMatrixHandle underlying) : underlying_(underlying), pyMat_(convertMatrixToPython(underlying))
    {
    }

    std::string type() const override
    {
      return underlying_->dynamic_type_name();
    }

    py::object value() const override
    {
      return pyMat_;
    }

  private:
    DenseMatrixHandle underlying_;
    py::list pyMat_;
  };

  class PyDatatypeSparseRowMatrix : public PyDatatype
  {
  public:
    explicit PyDatatypeSparseRowMatrix(SparseRowMatrixHandle underlying) : underlying_(underlying), pyMat_(convertMatrixToPython(underlying))
    {
    }

    std::string type() const override
    {
      return underlying_->dynamic_type_name();
    }

    py::object value() const override
    {
      return pyMat_;
    }

  private:
    SparseRowMatrixHandle underlying_;
    py::dict pyMat_;
  };

  class PyDatatypeField : public PyDatatype
  {
  public:
    explicit PyDatatypeField(FieldHandle underlying) : underlying_(underlying), matlabStructure_(convertFieldToPython(underlying))
    {
    }

    std::string type() const override
    {
      return underlying_->dynamic_type_name();
    }

    py::object value() const override
    {
      return matlabStructure_;
    }

  private:
    FieldHandle underlying_;
    py::dict matlabStructure_;
  };

  class PyDatatypeFactory
  {
  public:
    static SharedPointer<PyDatatype> createWrapper(DatatypeHandle data)
    {
      {
        auto str = std::dynamic_pointer_cast<String>(data);
        if (str)
          return makeShared<PyDatatypeString>(str);
      }
      {
        auto dense = std::dynamic_pointer_cast<DenseMatrix>(data);
        if (dense)
          return makeShared<PyDatatypeDenseMatrix>(dense);
      }
      {
        auto sparse = std::dynamic_pointer_cast<SparseRowMatrix>(data);
        if (sparse)
          return makeShared<PyDatatypeSparseRowMatrix>(sparse);
      }
      {
        auto field = std::dynamic_pointer_cast<Field>(data);
        if (field)
          return makeShared<PyDatatypeField>(field);
      }
      return nullptr;
    }
  };

  class PyPortImpl : public PyPort
  {
  public:
    PyPortImpl(SharedPointer<PortDescriptionInterface> port, NetworkEditorController& nec) : port_(port), nec_(nec)
    {
    }
    std::string name() const override
    {
      return port_ ? port_->get_portname() : "<Null>";
    }

    std::string id() const override
    {
      return port_ ? port_->externalId().toString() : "<Null>";
    }

    std::string type() const override
    {
      return port_ ? port_->get_typename() : "<Null>";
    }

    bool isInput() const override
    {
      return port_ ? port_->isInput() : false;
    }

    void connect(const PyPort& other) const override
    {
      auto otherPort = dynamic_cast<const PyPortImpl*>(&other);
      if (port_ && otherPort)
        nec_.requestConnection(port_.get(), otherPort->port_.get());
    }

    std::string dataTypeName() const override
    {
      auto output = std::dynamic_pointer_cast<OutputPortInterface>(port_);
      if (output)
        return "Output port data not available yet!";

      auto data = getDataImpl();
      if (!data)
        return "[No data]";
      if (!*data)
        return "[Null data]";
      return (*data)->dynamic_type_name();
    }

    SharedPointer<PyDatatype> data() const override
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
      auto input = std::dynamic_pointer_cast<InputPortInterface>(port_);
      if (input)
      {
        return input->getData();
      }
      auto output = std::dynamic_pointer_cast<OutputPortInterface>(port_);
      if (output)
      {
        return output->peekData();
      }
      return {};
    }
    SharedPointer<PortDescriptionInterface> port_;
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
      //  | boost::adaptors::transformed([&](SharedPointer<PortDescriptionInterface> p) { return makeShared<PyPortImpl>(p, nec_); })
      //  );
      if (input)
      {
        setupInputs();

        if (mod->hasDynamicPorts())
        {
          connections_.push_back(makeShared<boost::signals2::scoped_connection>(nec_.connectPortAdded([this](const ModuleId& mid, const PortId& pid) { portAddedSlot(mid, pid); })));
          connections_.push_back(makeShared<boost::signals2::scoped_connection>(nec_.connectPortRemoved([this](const ModuleId& mid, const PortId& pid) { portRemovedSlot(mid, pid); })));
        }
      }
      else
      {
        setupOutputs();
      }
    }

    SharedPointer<PyPort> getattr(const std::string& name) override
    {
      auto port = std::find_if(ports_.begin(), ports_.end(), [&](SharedPointer<PyPortImpl> p) { return name == p->name(); });
      if (port != ports_.end())
        return *port;

      port = std::find_if(ports_.begin(), ports_.end(), [&](SharedPointer<PyPortImpl> p) { return name == p->id(); });
      if (port != ports_.end())
        return *port;

      std::cerr << "Could not find port with name " << name << " on module " << modId_.id_ << std::endl;
      PyErr_SetObject(PyExc_KeyError, py::object(name).ptr());
      throw py::error_already_set();
    }

    SharedPointer<PyPort> getitem(int index) override
    {
      if (index < 0)
        index += size();
      if (index < 0 || index >= size())
      {
        PyErr_SetObject(PyExc_KeyError, py::object(index).ptr());
        throw py::error_already_set();
      }
      return ports_[index];
    }

    size_t size() const override
    {
      return ports_.size();
    }

    void reset()
    {
      std::for_each(ports_.begin(), ports_.end(), [](SharedPointer<PyPortImpl> p) { p->reset(); p.reset(); });
      ports_.clear();
    }
  private:
    void portAddedSlot(const ModuleId&, const PortId&)
    {
      setupInputs();
    }

    void portRemovedSlot(const ModuleId&, const PortId&)
    {
      setupInputs();
    }

    void setupInputs()
    {
      ports_.clear();
      for (const auto& p : mod_->inputPorts())
        ports_.push_back(makeShared<PyPortImpl>(p, nec_));
    }

    void setupOutputs()
    {
      ports_.clear();
      for (const auto& p : mod_->outputPorts())
        ports_.push_back(makeShared<PyPortImpl>(p, nec_));
    }

    std::vector<SharedPointer<PyPortImpl>> ports_;
    ModuleHandle mod_;
    NetworkEditorController& nec_;
    ModuleId modId_;
    std::vector<SharedPointer<boost::signals2::scoped_connection>> connections_;
  };

  class PyModuleImpl : public PyModule
  {
  public:
    PyModuleImpl(ModuleHandle mod, NetworkEditorController& nec) : module_(mod), nec_(nec)
    {
      if (module_)
      {
        input_ = makeShared<PyPortsImpl>(module_, true, nec_);
        output_ = makeShared<PyPortsImpl>(module_, false, nec_);
      }
      creationTime_ = boost::posix_time::second_clock::local_time();
    }

    std::string id() const override
    {
      if (module_)
        return module_->id();
      return "<Null module>";
    }

    void showUI() override
    {
      if (module_)
        module_->setUiVisible(true);
    }

    void hideUI() override
    {
      if (module_)
        module_->setUiVisible(false);
    }

    void reset() override
    {
      module_.reset();
      input_->reset();
      output_->reset();
      input_.reset();
      output_.reset();
    }

    py::object getattr(const std::string& name, bool transient) override
    {
      if (module_)
      {
        auto state = module_->get_state();
        AlgorithmParameterName apn(name);
        if (transient)
          return convertTransientVariableToPythonObject(state->getTransientValue(apn));
        else
          if (state->containsKey(apn))
            return convertVariableToPythonObject(state->getValue(apn));
          else
            throw std::invalid_argument("Module state key " + name + " not defined.");
      }
      else
        return py::object();
    }

    void setattr(const std::string& name, const py::object& object, bool transient) override
    {
      if (module_)
      {
        auto state = module_->get_state();
        AlgorithmParameterName apn(name);
        if (transient)
          state->setTransientValue(apn, convertPythonObjectToVariable(object), false);
        else
        {
          if (state->containsKey(apn))
            state->setValue(apn, convertPythonObjectToVariableWithTypeInference(
                              object, state->getValue(apn)).value());
          else
            throw std::invalid_argument("Module state key " + name + " not defined.");
        }
      }
    }

    std::vector<std::string> stateVars() const override
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

    std::string stateToString() const override
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

    SharedPointer<PyPorts> output() override
    {
      return output_;
    }

    SharedPointer<PyPorts> input() override
    {
      return input_;
    }

    boost::posix_time::ptime creationTime() const override
    {
      return creationTime_;
    }

    void setEnabled(bool enable) override
    {
      module_->setExecutionDisabled(!enable);
    }

  private:
    ModuleHandle module_;
    NetworkEditorController& nec_;
    SharedPointer<PyPortsImpl> input_, output_;
    boost::posix_time::ptime creationTime_;
  };
}

PythonImpl::PythonImpl(NetworkEditorController& nec, GlobalCommandFactoryHandle cmdFactory) :
  nec_(nec), cmdFactory_(cmdFactory)
{
  connections_.push_back(nec_.connectStaticNetworkExecutionFinished([this](int) { executionFromPythonFinish(0); }));
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

SharedPointer<PyModule> PythonImpl::addModule(const std::string& name)
{
  auto m = nec_.addModule(name);
  if (m)
    std::cout << "Module added: " + m->id().id_ << std::endl;
  else
    std::cout << "Module add failed, no such module type" << std::endl;

  return modules_[m->id().id_];
}

void PythonImpl::pythonModuleAddedSlot(const std::string&, ModuleHandle m, ModuleCounter)
{
  auto pyM = makeShared<PyModuleImpl>(m, nec_);
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

std::vector<SharedPointer<PyModule>> PythonImpl::moduleList() const
{
  std::vector<SharedPointer<PyModule>> modules;
  boost::copy(modules_ | boost::adaptors::map_values, std::back_inserter(modules));
  std::sort(modules.begin(), modules.end(), [](const SharedPointer<PyModule> lhs, const SharedPointer<PyModule> rhs) { return lhs->creationTime() < rhs->creationTime(); });
  return modules;
}

SharedPointer<PyModule> PythonImpl::findModule(const std::string& id) const
{
  auto modIter = modules_.find(id);
  return modIter != modules_.end() ? modIter->second : nullptr;
}

std::string PythonImpl::executeAll()
{
  //cmdFactory_->create(GlobalCommands::DisableViewScenes)->execute();

  nec_.executeAll();
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
  return "PythonImpl::connect success: " + id->id_;
}

std::string PythonImpl::disconnect(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex)
{
  //TODO: doesn't work at all since there is no GUI connection to this network change event. Issue is #...
  auto conn = nec_.getNetwork()->lookupConnection(moduleIdFrom, fromIndex, moduleIdTo, toIndex);
  if (conn)
  {
    nec_.removeConnection(conn->id());
    return "PythonImpl::disconnect is not connected to GUI";
  }
  else
  {
    return "PythonImpl::disconnect: connection not found";
  }
}

std::string PythonImpl::setConnectionStatus(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex, bool enable)
{
  return nec_.setConnectionStatus(moduleIdFrom, fromIndex, moduleIdTo, toIndex, enable);
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
