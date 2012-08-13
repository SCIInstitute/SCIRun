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

#ifndef CORE_DATAFLOW_NETWORK_MODULE_STATE_INTERFACE_H
#define CORE_DATAFLOW_NETWORK_MODULE_STATE_INTERFACE_H 

#include <string>
#include <iostream>
#include <boost/signal.hpp>
#include <boost/any.hpp>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Dataflow/Network/NetworkFwd.h>
#include <Core/Dataflow/Network/Share.h>

namespace SCIRun {
namespace Domain {
namespace Networks {

  class SCISHARE ModuleStateInterface
  {
  public:
    virtual ~ModuleStateInterface();
    
    typedef std::vector<SCIRun::Algorithms::AlgorithmParameterName> Keys;
    typedef SCIRun::Algorithms::AlgorithmParameterName Name;
    typedef SCIRun::Algorithms::AlgorithmParameter Value;

    //serialized state
    virtual const Value getValue(const Name& name) const = 0;
    virtual void setValue(const Name& name, const SCIRun::Algorithms::AlgorithmParameter::Value& value) = 0;
    virtual Keys getKeys() const = 0;

    //non-serialized state: algorithm output needing to be pushed, for instance--TODO: make classes instead of raw string/any
    typedef boost::any TransientValue;
    virtual const TransientValue getTransientValue(const std::string& name) const = 0;
    virtual void setTransientValue(const std::string& name, const TransientValue& value) = 0;

    typedef boost::signal<void()> state_changed_sig_t;

    virtual boost::signals::connection connect_state_changed(state_changed_sig_t::slot_function_type subscriber) = 0;
  };

  class SCISHARE ModuleStateInterfaceFactory
  {
  public:
    virtual ~ModuleStateInterfaceFactory();
    virtual ModuleStateInterface* make_state(const std::string& name) const = 0;
  };

  //TODO split
  template <class T>
  T any_cast_or_default(const boost::any& x)
  {
    try
    {
      return boost::any_cast<T>(x);
    }
    catch (boost::bad_any_cast&)
    {
      std::cout << "Attempted any_cast failed, returning default value." << std::endl;
      return T();
    }
  }

}}}

#endif
