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


/// @todo Documentation Dataflow/Engine/Controller/PythonImpl.h

#ifndef ENGINE_NETWORK_PYTHONIMPL_H
#define ENGINE_NETWORK_PYTHONIMPL_H

#ifdef BUILD_WITH_PYTHON

#include <boost/noncopyable.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Command/CommandFactory.h>
#include <Dataflow/Engine/Python/NetworkEditorPythonInterface.h>
#include <Dataflow/Engine/Controller/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  class NetworkEditorController;
  class PythonImplImpl;

  class SCISHARE PythonImpl : public NetworkEditorPythonInterface, boost::noncopyable
  {
  public:
    PythonImpl(NetworkEditorController& nec, Core::Commands::GlobalCommandFactoryHandle cmdFactory);
    ~PythonImpl();
    SharedPointer<PyModule> addModule(const std::string& name) override;
    std::string removeModule(const std::string& id) override;
    std::vector<SharedPointer<PyModule>> moduleList() const override;
    SharedPointer<PyModule> findModule(const std::string& id) const override;
    std::string executeAll() override;
    std::string connect(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex) override;
    std::string disconnect(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex) override;
    std::string saveNetwork(const std::string& filename) override;
    std::string loadNetwork(const std::string& filename) override;
    std::string currentNetworkFile() const override;
    std::string importNetwork(const std::string& filename) override;
    std::string runScript(const std::string& filename) override;
    std::string quit(bool force) override;
    std::string setConnectionStatus(const std::string& moduleIdFrom, int fromIndex, const std::string& moduleIdTo, int toIndex, bool enable) override;
    void setUnlockFunc(boost::function<void()> unlock) override;
    void setModuleContext(bool inModule) override { inModule_ = inModule; }
    bool isModuleContext() const override { return inModule_; }
  private:
    void pythonModuleAddedSlot(const std::string&, Networks::ModuleHandle, ModuleCounter);
    void pythonModuleRemovedSlot(const Networks::ModuleId&);
    void executionFromPythonStart();
    void executionFromPythonFinish(int);
    std::map<std::string, SharedPointer<PyModule>> modules_;
    NetworkEditorController& nec_;
    Core::Commands::GlobalCommandFactoryHandle cmdFactory_;
    boost::function<void()> unlock_;
    std::vector<boost::signals2::connection> connections_;
    bool inModule_ = false;
  };

}}}

#endif
#endif
