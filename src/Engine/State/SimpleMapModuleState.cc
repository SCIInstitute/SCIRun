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

#include <Engine/State/SimpleMapModuleState.h>
#include <boost/foreach.hpp>

using namespace SCIRun::Domain::State;
using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Algorithms;

SimpleMapModuleState::SimpleMapModuleState()
{
}

SimpleMapModuleState::SimpleMapModuleState(const SimpleMapModuleState& rhs)
  : stateMap_(rhs.stateMap_), transientStateMap_(rhs.transientStateMap_)
{
}

SimpleMapModuleState& SimpleMapModuleState::operator=(const SimpleMapModuleState& rhs)
{
  if (&rhs != this)
  {
    stateMap_ = rhs.stateMap_;
    transientStateMap_ = rhs.transientStateMap_;
    //TODO??
    //stateChangedSignal_.disconnect_all_slots();
  }
  return *this;
}

const ModuleStateInterface::Value SimpleMapModuleState::getValue(const Name& parameterName) const
{
  StateMap::const_iterator i = stateMap_.find(parameterName);
  return i != stateMap_.end() ? i->second : Value(AlgorithmParameterName(""), -1);
}

void SimpleMapModuleState::setValue(const Name& parameterName, const SCIRun::Algorithms::AlgorithmParameter::Value& value)
{
  stateMap_[parameterName] = AlgorithmParameter(parameterName, value);

  stateChangedSignal_();
}

boost::signals::connection SimpleMapModuleState::connect_state_changed(state_changed_sig_t::slot_function_type subscriber)
{
  return stateChangedSignal_.connect(subscriber);
}

ModuleStateInterface::Keys SimpleMapModuleState::getKeys() const
{
  Keys keys;
  BOOST_FOREACH(const StateMap::value_type& p, stateMap_)
    keys.push_back(p.first);
  return keys;
}

const SimpleMapModuleState::TransientValue SimpleMapModuleState::getTransientValue(const std::string& name) const
{
  TransientStateMap::const_iterator i = transientStateMap_.find(name);
  return i != transientStateMap_.end() ? i->second : TransientValue();
}

void SimpleMapModuleState::setTransientValue(const std::string& name, const TransientValue& value)
{
  transientStateMap_[name] = value;

  stateChangedSignal_();  //TODO: ???
}

ModuleStateInterface* SimpleMapModuleStateFactory::make_state(const std::string& name) const
{
  return new SimpleMapModuleState;
}
