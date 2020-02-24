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


/// @todo Documentation Dataflow/Engine/Controller/DynamicPortManager.h

#ifndef ENGINE_NETWORK_DYNAMICPORTMANAGER_H
#define ENGINE_NETWORK_DYNAMICPORTMANAGER_H

#include <boost/signals2.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Engine/Controller/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  class SCISHARE DynamicPortManager : boost::noncopyable
  {
  public:
    DynamicPortManager(ConnectionAddedSignalType& addedSignal, ConnectionRemovedSignalType& removeSignal, const NetworkEditorController* controller);
    void connectionAddedNeedToCloneAPort(const SCIRun::Dataflow::Networks::ConnectionDescription&);
    void connectionRemovedNeedToRemoveAPort(const SCIRun::Dataflow::Networks::ConnectionId&);

    boost::signals2::connection connectPortAdded(const PortAddedSignalType::slot_type& subscriber);
    boost::signals2::connection connectPortRemoved(const PortRemovedSignalType::slot_type& subscriber);

    bool isDisabled() const { return !enabled_; }
    void enable() { enabled_ = true; }
    void disable() { enabled_ = false; }
  private:
    const NetworkEditorController* controller_;
    PortAddedSignalType portAdded_;
    PortRemovedSignalType portRemoved_;
    bool enabled_;
  };
}
}
}

#endif
