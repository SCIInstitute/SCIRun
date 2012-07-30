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

#ifndef ENGINE_STATE_NULLMODULESTATE_H
#define ENGINE_STATE_NULLMODULESTATE_H

#include <Core/Dataflow/Network/ModuleStateInterface.h>
#include <Core/Dataflow/Network/Share.h>

namespace SCIRun {
namespace Engine {
namespace State {
  
  class SCISHARE NullModuleState : public SCIRun::Domain::Networks::ModuleStateInterface
  {
  public:
    virtual void setValue(const Name&, const SCIRun::Algorithms::AlgorithmParameter::Value&);
    virtual const Value getValue(const Name&) const;
    virtual Keys getKeys() const;
    virtual const TransientValue getTransientValue(const std::string& name) const;
    virtual void setTransientValue(const std::string& name, const TransientValue& value);
    virtual boost::signals::connection connect_state_changed(state_changed_sig_t::slot_function_type subscriber);
  };

}}}

#endif
