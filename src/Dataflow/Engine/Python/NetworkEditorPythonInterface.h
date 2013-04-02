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

#ifndef ENGINE_PYTHON_NETWORKEDITORPYTHONINTERFACE_H
#define ENGINE_PYTHON_NETWORKEDITORPYTHONINTERFACE_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Python/Share.h>

namespace SCIRun 
{

  class SCISHARE PyModule
  {
  public:
    virtual ~PyModule() {}
    virtual std::string id() const = 0;
    virtual void showUI() = 0;
    virtual void hideUI() = 0;
  };

  class SCISHARE NetworkEditorPythonInterface
  {
  public:
    virtual ~NetworkEditorPythonInterface() {}
    virtual boost::shared_ptr<PyModule> addModule(const std::string& name) = 0;
    virtual std::string removeModule(const std::string& id) = 0;
    virtual std::string executeAll(const Dataflow::Networks::ExecutableLookup& lookup) = 0;
    virtual std::string connect(const std::string& moduleId1, int port1, const std::string& moduleId2, int port2) = 0;
    virtual std::string disconnect(const std::string& moduleId1, int port1, const std::string& moduleId2, int port2) = 0;
    virtual std::string saveNetwork(const std::string& filename) = 0;
    virtual std::string loadNetwork(const std::string& filename) = 0;
    virtual std::string quit(bool force) = 0;
  };

}

#endif