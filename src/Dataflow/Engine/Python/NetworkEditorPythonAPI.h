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

#ifndef ENGINE_PYTHON_NETWORKEDITORPYTHONAPI_H
#define ENGINE_PYTHON_NETWORKEDITORPYTHONAPI_H

#include <vector>
#include <boost/python.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Thread/Mutex.h>
#include <atomic>
#include <Dataflow/Engine/Python/share.h>

namespace SCIRun {

  class NetworkEditorPythonInterface;
  class PyModule;
  class PyDatatype;

  class SCISHARE NetworkEditorPythonAPI
  {
  public:
    static boost::shared_ptr<PyModule> addModule(const std::string& name);
    static std::vector<boost::shared_ptr<PyModule>> modules();
    static std::string removeModule(const std::string& id);
    static std::string connect(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex);
    static std::string disconnect(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex);
    static boost::python::object scirun_get_module_state(const std::string& moduleId, const std::string& stateVariable);
    static std::string scirun_set_module_state(const std::string& moduleId, const std::string& stateVariable, const boost::python::object& value);
    static std::string scirun_dump_module_state(const std::string& moduleId);
    static std::string scirun_get_module_input_type(const std::string& moduleId, int portIndex);
    //static std::string scirun_get_module_output_type(const std::string& moduleId, int portIndex);

    //TODO: these don't work on Mac
    static boost::shared_ptr<PyDatatype> scirun_get_module_input_object_index(const std::string& moduleId, int portIndex);
    static boost::shared_ptr<PyDatatype> scirun_get_module_input_object(const std::string& moduleId, const std::string& portName);

    //these work on all platforms
    static boost::python::object scirun_get_module_input_value_index(const std::string& moduleId, int portIndex);
    static boost::python::object scirun_get_module_input_value(const std::string& moduleId, const std::string& portName);

    static std::string executeAll();
    static std::string saveNetwork(const std::string& filename);
    static std::string loadNetwork(const std::string& filename);
    static std::string importNetwork(const std::string& filename);
    
    static std::string quit(bool force);

    static void setImpl(boost::shared_ptr<NetworkEditorPythonInterface> impl);
    /// @todo: smelly!
    static void setExecutionContext(Dataflow::Networks::ExecutableLookup* lookup);
    static Core::Thread::Mutex& getLock() { return pythonLock_; }
  private:
    NetworkEditorPythonAPI() = delete;
    static boost::shared_ptr<NetworkEditorPythonInterface> impl_;
    static Dataflow::Networks::ExecutableLookup* lookup_;
    static std::map<std::string, boost::shared_ptr<PyModule>> modules_;
    static void unlock();
    static Core::Thread::Mutex pythonLock_;
    static std::atomic<bool> executeLockedFromPython_;
  };

  class SCISHARE SimplePythonAPI
  {
  public:
    static std::string scirun_add_module(const std::string& name);

    static std::string scirun_quit();
    static std::string scirun_force_quit();
  private:
    SimplePythonAPI() = delete;
  };

}

#endif
