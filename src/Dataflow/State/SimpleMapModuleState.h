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
/// @todo Documentation Dataflow/State/SimpleModuleState.h
#ifndef DATAFLOW_STATE_SIMPLEMAPMODULESTATE_H
#define DATAFLOW_STATE_SIMPLEMAPMODULESTATE_H

#include <map>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/State/share.h>

namespace SCIRun {
namespace Dataflow {
namespace State {
  
  class SCISHARE SimpleMapModuleState : public SCIRun::Dataflow::Networks::ModuleStateInterface
  {
  public:
    SimpleMapModuleState();
    SimpleMapModuleState(SimpleMapModuleState&& rhs);
    SimpleMapModuleState(const SimpleMapModuleState& rhs);
    SimpleMapModuleState& operator=(const SimpleMapModuleState& rhs);
    virtual const Value getValue(const Name& name) const;
    virtual void setValue(const Name& name, const SCIRun::Core::Algorithms::AlgorithmParameter::Value& value);
    virtual bool containsKey(const Name& name) const;
    virtual Keys getKeys() const;
    virtual SCIRun::Dataflow::Networks::ModuleStateHandle clone() const;
    virtual boost::signals2::connection connect_state_changed(state_changed_sig_t::slot_function_type subscriber);

    virtual TransientValueOption getTransientValue(const std::string& name) const;
    virtual void setTransientValue(const std::string& name, const TransientValue& value, bool fireSignal);
    virtual void fireTransientStateChangeSignal();

  protected:
    typedef std::map<Name, Value> StateMap;
    StateMap stateMap_;
    typedef std::map<std::string, TransientValue> TransientStateMap;
    TransientStateMap transientStateMap_;
    state_changed_sig_t stateChangedSignal_;
  };

  class SCISHARE SimpleMapModuleStateFactory : public SCIRun::Dataflow::Networks::ModuleStateInterfaceFactory
  {
  public:
    virtual SCIRun::Dataflow::Networks::ModuleStateInterface* make_state(const std::string& name) const;
  };

}}}

#endif
