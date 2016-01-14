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
/// @todo Documentation Dataflow/Engine/Python/NetworkEditorPythonInterface.cc

#ifndef ENGINE_PYTHON_NETWORKEDITORPYTHONINTERFACE_H
#define ENGINE_PYTHON_NETWORKEDITORPYTHONINTERFACE_H

#include <vector>
#include <boost/python.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Python/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Thread
    {
      class Mutex;
    }
  }

  class SCISHARE PyModule
  {
  public:
    virtual ~PyModule() {}
    virtual std::string id() const = 0;
    virtual void showUI() = 0;
    virtual void hideUI() = 0;
    virtual void reset() = 0;

    //state
    virtual boost::python::object getattr(const std::string& name) = 0;
    virtual void setattr(const std::string& name, boost::python::object object) = 0;
    virtual std::vector<std::string> stateVars() const = 0;
    virtual std::string stateToString() const = 0;

    //ports
    virtual boost::shared_ptr<class PyPorts> output() = 0;
    virtual boost::shared_ptr<class PyPorts> input() = 0;
  };

  class SCISHARE PyDatatype
  {
  public:
    virtual ~PyDatatype() {}
    virtual std::string type() const = 0;
    virtual boost::python::object value() const = 0;
  };

  class SCISHARE PyPort : public boost::enable_shared_from_this<PyPort>
  {
  public:
    virtual ~PyPort() {}
    virtual std::string name() const = 0;
    virtual std::string type() const = 0;
    virtual bool isInput() const = 0;
    virtual void connect(const PyPort& other) const = 0;
    virtual std::string dataTypeName() const = 0; //TODO: precursor to getting actual data off of port
    virtual boost::shared_ptr<PyDatatype> data() const = 0;
  };

  class SCISHARE PyConnection
  {
  public:
    virtual ~PyConnection() {}
    virtual std::string id() const = 0;
  };

  SCISHARE boost::shared_ptr<PyPort> operator>>(const PyPort& from, const PyPort& to);

  class SCISHARE PyPorts
  {
  public:
    virtual ~PyPorts() {}
    //by name
    virtual boost::shared_ptr<PyPort> getattr(const std::string& name) = 0;
    //by index
    virtual boost::shared_ptr<PyPort> getitem(int index) = 0;

    virtual size_t size() const = 0;
  };

  /// @todo idea: write addmodule.ShowMesh in python
  class SCISHARE AddModule
  {
  public:
    virtual ~AddModule() {}
    virtual boost::shared_ptr<PyModule> getattr(const std::string& name) = 0;
  };

  class SCISHARE NetworkEditorPythonInterface
  {
  public:
    virtual ~NetworkEditorPythonInterface() {}
    virtual boost::shared_ptr<PyModule> addModule(const std::string& name) = 0;
    virtual std::string removeModule(const std::string& id) = 0;
    virtual std::string connect(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex) = 0;
    virtual std::string disconnect(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex) = 0;
    virtual std::string executeAll(const Dataflow::Networks::ExecutableLookup* lookup) = 0;
    virtual std::string saveNetwork(const std::string& filename) = 0;
    virtual std::string loadNetwork(const std::string& filename) = 0;
    virtual std::string importNetwork(const std::string& filename) = 0;
    virtual std::string quit(bool force) = 0;
    virtual void setUnlockFunc(boost::function<void()> unlock) = 0;
  };

}

#endif
