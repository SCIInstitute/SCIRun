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


/// @todo Documentation Dataflow/Network/NullModuleState.cc

#include <Dataflow/Network/NullModuleState.h>

using namespace SCIRun::Engine::State;
using namespace SCIRun::Dataflow::Networks;

void NullModuleState::setValue(const Name&, const SCIRun::Core::Algorithms::AlgorithmParameter::Value&)
{
}

const NullModuleState::Value NullModuleState::getValue(const Name&) const
{
  return Value();
}

NullModuleState::Keys NullModuleState::getKeys() const
{
  return Keys();
}

bool NullModuleState::containsKey(const Name&) const
{
  return false;
}

ModuleStateHandle NullModuleState::clone() const
{
  return boost::make_shared<NullModuleState>();
}

boost::signals2::connection NullModuleState::connectStateChanged(state_changed_sig_t::slot_function_type)
{
  return {};
}

boost::signals2::connection NullModuleState::connectSpecificStateChanged(const Name&, state_changed_sig_t::slot_function_type)
{
  return {};
}

NullModuleState::TransientValueOption NullModuleState::getTransientValue(const Name& name) const
{
  return TransientValueOption();
}

void NullModuleState::setTransientValue(const Name& name, const TransientValue& value, bool b)
{
}
