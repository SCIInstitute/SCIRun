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

/// @todo Documentation Dataflow/Network/ModuleStateInterface.h

#ifndef DATAFLOW_NETWORK_MODULE_STATE_INTERFACE_H
#define DATAFLOW_NETWORK_MODULE_STATE_INTERFACE_H

#include <string>
#include <iostream>
#include <boost/signals2/signal.hpp>
#include <boost/optional.hpp>
#include <boost/any.hpp>
#include <boost/atomic.hpp>
#include <Core/Algorithms/Base/Name.h>
#include <Core/Algorithms/Base/Variable.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE ModuleStateInterface
  {
  public:
    virtual ~ModuleStateInterface();
    
    typedef SCIRun::Core::Algorithms::AlgorithmParameterName Name;
    typedef SCIRun::Core::Algorithms::AlgorithmParameter Value;
    typedef std::vector<Name> Keys;

    //serialized state
    virtual const Value getValue(const Name& name) const = 0;
    virtual void setValue(const Name& name, const Value::Value& value) = 0;
    virtual bool containsKey(const Name& name) const = 0;
    virtual Keys getKeys() const = 0;
    virtual ModuleStateHandle clone() const = 0;

    //non-serialized state: algorithm output needing to be pushed, for instance--TODO: make classes instead of raw string/any
    typedef boost::any TransientValue;
    typedef boost::optional<TransientValue> TransientValueOption;
    virtual TransientValueOption getTransientValue(const Name& name) const = 0;
    TransientValueOption getTransientValue(const std::string& name) const { return getTransientValue(Name(name)); }
    virtual void setTransientValue(const Name& name, const TransientValue& value, bool fireSignal) = 0;
    void setTransientValue(const Name& name, const TransientValue& value) { setTransientValue(name, value, true); }
    void setTransientValue(const std::string& name, const TransientValue& value) { setTransientValue(Name(name), value); }
    virtual void fireTransientStateChangeSignal() = 0;

    typedef boost::signals2::signal<void()> state_changed_sig_t;

    virtual boost::signals2::connection connect_state_changed(state_changed_sig_t::slot_function_type subscriber) = 0;
  };

  class SCISHARE ModuleStateInterfaceFactory
  {
  public:
    virtual ~ModuleStateInterfaceFactory();
    virtual ModuleStateInterface* make_state(const std::string& name) const = 0;
  };

  /// @todo split
  template <class T>
  T any_cast_or_default_(const boost::any& x)
  {
    try
    {
      return boost::any_cast<T>(x);
    }
    catch (boost::bad_any_cast&)
    {
      /// @todo: use real logger here
      //std::cout << "Attempted any_cast failed, returning default value." << std::endl;
      return T();
    }
  }

  template <class T>
  T transient_value_cast(const ModuleStateInterface::TransientValueOption& x)
  {
    return x ? any_cast_or_default_<T>(*x) : T();
  }

  class SCISHARE StateChangeObserver
  {
  public:
    StateChangeObserver();
    virtual ~StateChangeObserver();
    void initStateObserver(ModuleStateInterface* state);
    void stateChanged();
    void resetStateChanged();
    bool newStatePresent() const;
  private:
    boost::atomic<bool> stateChanged_;
    boost::signals2::connection conn_;
  };

  typedef std::map<std::string, std::string> StringMap;

  // Keep it simple
  class SCISHARE MetadataMap
  {
  public:
    explicit MetadataMap(ModuleStateHandle& state);
    MetadataMap(const MetadataMap&) = delete;
    MetadataMap& operator=(const MetadataMap&) = delete;

    std::string getMetadata(const std::string& key) const;
    void setMetadata(const std::string& key, const std::string& value);
    StringMap getFullMap() const;
  private:
    ModuleStateHandle& state_;
  };

}}}

#endif
