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

boost::any SimpleMapModuleState::getValue(const std::string& parameterName) const
{
  StateMap::const_iterator i = stateMap_.find(parameterName);
  return i != stateMap_.end() ? i->second : boost::any();
}

void SimpleMapModuleState::setValue(const std::string& parameterName, boost::any value)
{
  stateMap_[parameterName] = value;

  sig_();
}

boost::signals::connection SimpleMapModuleState::connect_state_changed(state_changed_sig_t::slot_function_type subscriber)
{
  return sig_.connect(subscriber);
}

std::vector<std::string> SimpleMapModuleState::getKeys() const
{
  std::vector<std::string> keys;
  BOOST_FOREACH(const StateMap::value_type& p, stateMap_)
    keys.push_back(p.first);
  return keys;
}

ModuleStateInterface* SimpleMapModuleStateFactory::make_state(const std::string& name) const
{
  return new SimpleMapModuleState;
}
