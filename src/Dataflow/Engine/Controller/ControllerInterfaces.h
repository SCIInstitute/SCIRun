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


/// @todo Documentation Dataflow/Engine/Controller/ControllerInterfaces.h

#ifndef ENGINE_NETWORK_CONTROLLERINTERFACES_H
#define ENGINE_NETWORK_CONTROLLERINTERFACES_H

#include <Dataflow/Network/NetworkFwd.h>
#include <boost/atomic.hpp>
#include <boost/signals2.hpp>
#include <Dataflow/Engine/Controller/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  template <class Memento>
  class SCISHARE NetworkIOInterface
  {
  public:
    virtual ~NetworkIOInterface() {}
    virtual Memento saveNetwork() const = 0;
    virtual void loadNetwork(const Memento& xml) = 0;
    virtual void clear() = 0;
  };

  typedef SharedPointer<NetworkIOInterface<Networks::NetworkFileHandle>> NetworkIOHandle;

  template <class Memento>
  class ProvenanceItem;

  typedef SharedPointer<Dataflow::Engine::ProvenanceItem<Networks::NetworkFileHandle> > ProvenanceItemHandle;

  template <class Memento>
  class ProvenanceManager;

  typedef SharedPointer<Dataflow::Engine::ProvenanceManager<Networks::NetworkFileHandle> > ProvenanceManagerHandle;

  struct SCISHARE ModuleCounter
  {
    ModuleCounter() : count(new boost::atomic<int>(0)) {}
    ModuleCounter(const ModuleCounter& rhs) : count(rhs.count)
    {
    }
    void increment() const
    {
      count->fetch_add(1);
    }
    mutable SharedPointer<boost::atomic<int>> count;
  };

  typedef boost::signals2::signal<void (const std::string&, Networks::ModuleHandle, ModuleCounter)> ModuleAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ModuleId&)> ModuleRemovedSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionDescription&)> ConnectionAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionDescription&)> InvalidConnectionSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionId&)> ConnectionRemovedSignalType;
  typedef boost::signals2::signal<void (const Networks::ConnectionId&, bool)> ConnectionStatusChangedSignalType;
  typedef boost::signals2::signal<void (const Networks::ModuleId&, const Networks::PortId&)> PortAddedSignalType;
  typedef boost::signals2::signal<void (const Networks::ModuleId&, const Networks::PortId&)> PortRemovedSignalType;
  typedef boost::signals2::signal<void (int)> NetworkDoneLoadingSignalType;

  class DynamicPortManager;

  struct SCISHARE DisableDynamicPortSwitch
  {
    explicit DisableDynamicPortSwitch(SharedPointer<DynamicPortManager> dpm);
    ~DisableDynamicPortSwitch();
  private:
    bool first_;
    SharedPointer<DynamicPortManager> dpm_;
  };

}
}
}

#endif
