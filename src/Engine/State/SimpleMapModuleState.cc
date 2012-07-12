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

//TODO DAN

#include <Engine/State/SimpleMapModuleState.h>
#include <boost/scope_exit.hpp>

using namespace SCIRun::Domain::State;
using namespace SCIRun::Domain::Networks;

boost::any SimpleMapModuleState::getValue(const std::string& parameterName) const
{
  StateMap::const_iterator i = stateMap_.find(parameterName);
  if (i == stateMap_.end())
  {
    std::cout << "~~~value not found for key: " << parameterName << std::endl;
  }
  else
  {
    std::cout << "~~~State map returning value of type " << i->second.type().name() << " for key " << parameterName << std::endl;
  }
  return i != stateMap_.end() ? i->second : boost::any();
}

void SimpleMapModuleState::setValue(const std::string& parameterName, boost::any value)
{
  stateMap_[parameterName] = value;

  std::cout << "STATE: emitted state change signal" << std::endl;
  sig_();
}

boost::signals::connection SimpleMapModuleState::connect_state_changed(state_changed_sig_t::slot_function_type subscriber)
{
  return sig_.connect(subscriber);
}

ModuleStateInterface* SimpleMapModuleStateFactory::make_state(const std::string& name) const
{
  std::cout << "STATE FACTORY: returning new SimpleMapState" << std::endl;
  return new SimpleMapModuleState;
}
