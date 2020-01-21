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


/// @todo Documentation Dataflow/State/SimpleMapModuleState.cc

#include <Dataflow/State/SimpleMapModuleState.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Logging/Log.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <boost/lexical_cast.hpp>

using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;

SimpleMapModuleState::SimpleMapModuleState(const std::string& name) : name_(name)
{
  //std::cout << "SMMS ctor " << name_ << std::endl;
}

SimpleMapModuleState::SimpleMapModuleState(SimpleMapModuleState&& rhs)
  : stateMap_(std::move(rhs.stateMap_)),
  transientStateMap_(std::move(rhs.transientStateMap_)),
  name_(std::move(rhs.name_))
{
  //std::cout << "SMMS move ctor " << name_ << std::endl;
  stateChangedSignal_.swap(rhs.stateChangedSignal_);
}

SimpleMapModuleState::SimpleMapModuleState(const SimpleMapModuleState& rhs)
  : stateMap_(rhs.stateMap_),
  transientStateMap_(rhs.transientStateMap_), /// @todo: I think this is wrong, transient shouldn't be copied
  name_(rhs.name_)
{
  //std::cout << "SMMS copy ctor " << name_ << std::endl;
}

SimpleMapModuleState& SimpleMapModuleState::operator=(const SimpleMapModuleState& rhs)
{
  if (&rhs != this)
  {
    stateMap_ = rhs.stateMap_;
    transientStateMap_ = rhs.transientStateMap_;  /// @todo: I think this is wrong, transient shouldn't be copied
    name_ = rhs.name_;
    //std::cout << "SMMS copy assign " << name_<< std::endl;
    /// @todo??
    //stateChangedSignal_.disconnect_all_slots();
  }
  return *this;
}

ModuleStateHandle SimpleMapModuleState::clone() const
{
  //std::cout << "SMMS clone " << name_ << std::endl;
  return boost::make_shared<SimpleMapModuleState>(*this);
}

const ModuleStateInterface::Value SimpleMapModuleState::getValue(const Name& parameterName) const
{
  StateMap::const_iterator i = stateMap_.find(parameterName);
  if (i != stateMap_.end())
    return i->second;
  BOOST_THROW_EXCEPTION(AlgorithmParameterNotFound() << Core::ErrorMessage("Module has no state value with name " + parameterName.name_));
}

bool SimpleMapModuleState::containsKey(const Name& name) const
{
  return stateMap_.find(name) != stateMap_.end();
}

void SimpleMapModuleState::setValue(const Name& parameterName, const SCIRun::Core::Algorithms::AlgorithmParameter::Value& value)
{
  auto oldLocation = stateMap_.find(parameterName);
  bool newValue = oldLocation == stateMap_.end() || !(oldLocation->second.value() == value);

  stateMap_[parameterName] = AlgorithmParameter(parameterName, value);

  if (newValue)
  {
    LOG_DEBUG("----signaling from state map: ({}, {}), num_slots = {}", parameterName.name_,
      SCIRun::Core::to_string(value), stateChangedSignal_.num_slots());
    stateChangedSignal_();
    auto specSig = specificStateChangeSignalMap_.find(parameterName);
    if (specSig != specificStateChangeSignalMap_.end())
      specSig->second();
  }
}

boost::signals2::connection SimpleMapModuleState::connectStateChanged(state_changed_sig_t::slot_function_type subscriber)
{
  auto conn = stateChangedSignal_.connect(subscriber);
  LOG_TRACE("SimpleMapModuleState::connectStateChanged, num_slots = {}", stateChangedSignal_.num_slots());
  return conn;
}

boost::signals2::connection SimpleMapModuleState::connectSpecificStateChanged(const Name& stateKeyToObserve, state_changed_sig_t::slot_function_type subscriber)
{
  return specificStateChangeSignalMap_[stateKeyToObserve].connect(subscriber);
}

ModuleStateInterface::Keys SimpleMapModuleState::getKeys() const
{
  Keys keys;
  for (const auto& p : stateMap_)
    keys.push_back(p.first);
  return keys;
}

void SimpleMapModuleState::print() const
{
  std::cout << "Printing transient map: " << this << " name: " << name_ << std::endl;
  for (auto q = transientStateMap_.begin(); q != transientStateMap_.end(); ++q)
  {
    std::cout << "\t" << q->first << " : " << "any"  << std::endl;
  }
  std::cout << "Done" << std::endl;
}

SimpleMapModuleState::TransientValueOption SimpleMapModuleState::getTransientValue(const Name& name) const
{
  //print();
  auto i = transientStateMap_.find(name.name());
  return i != transientStateMap_.end() && !i->second.empty() ? boost::make_optional(i->second) : TransientValueOption();
}

void SimpleMapModuleState::setTransientValue(const Name& name, const TransientValue& value, bool fireSignal)
{
  transientStateMap_[name.name()] = value;
  //print();

  if (fireSignal)
  {
    fireTransientStateChangeSignal();
    auto specSig = specificStateChangeSignalMap_.find(name);
    if (specSig != specificStateChangeSignalMap_.end())
      specSig->second();
  }
}

void SimpleMapModuleState::fireTransientStateChangeSignal()
{
  stateChangedSignal_();
}

ModuleStateInterface* SimpleMapModuleStateFactory::make_state(const std::string& name) const
{
  return new SimpleMapModuleState(name);
}
