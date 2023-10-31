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
    explicit SimpleMapModuleState(const std::string& name = "<dflt>");
    SimpleMapModuleState(SimpleMapModuleState&& rhs);
    SimpleMapModuleState(const SimpleMapModuleState& rhs);
    SimpleMapModuleState& operator=(const SimpleMapModuleState& rhs);
    const Value getValue(const Name& name) const override;
    void setValue(const Name& name, const SCIRun::Core::Algorithms::AlgorithmParameter::Value& value) override;
    bool containsKey(const Name& name) const override;
    Keys getKeys() const override;
    SCIRun::Dataflow::Networks::ModuleStateHandle clone() const override;
    boost::signals2::connection connectStateChanged(state_changed_sig_t::slot_function_type subscriber) override;
    boost::signals2::connection connectSpecificStateChanged(const Name& stateKeyToObserve, state_changed_sig_t::slot_function_type subscriber) override;
    boost::signals2::connection connectProvenanceStateChanged(provenance_state_changed_sig_t::slot_function_type subscriber) override;

    TransientValueOption getTransientValue(const Name& name) const override;
    void setTransientValue(const Name& name, const TransientValue& value, bool fireSignal) override;
    void fireTransientStateChangeSignal() override;
    void disconnectAll() override;

  protected:
    typedef std::map<Name, Value> StateMap;
    StateMap stateMap_;
    typedef std::map<std::string, TransientValue> TransientStateMap;
    TransientStateMap transientStateMap_;
    state_changed_sig_t stateChangedSignal_;
    provenance_state_changed_sig_t provenanceStateChangedSignal_;
    std::map<Name, state_changed_sig_t> specificStateChangeSignalMap_;
    std::string name_;
    std::vector<boost::signals2::connection> generalStateConnections_, specificStateConnections_, provenanceStateConnections_;
  private:
    void print() const;
  };

  class SCISHARE SimpleMapModuleStateFactory : public SCIRun::Dataflow::Networks::ModuleStateInterfaceFactory
  {
  public:
    SCIRun::Dataflow::Networks::ModuleStateInterface* make_state(const std::string& name) const override;
  };

}}}

#endif
