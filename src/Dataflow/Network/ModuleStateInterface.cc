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

#include <Dataflow/Network/ModuleStateInterface.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Dataflow::Networks;

ModuleStateInterface::~ModuleStateInterface()
{
}

ModuleStateInterfaceFactory::~ModuleStateInterfaceFactory()
{
}

StateChangeObserver::StateChangeObserver() : stateChanged_(true)
{
}

StateChangeObserver::~StateChangeObserver()
{
}

void StateChangeObserver::initStateObserver(ModuleStateInterface* state)
{
  if (state)
  {
    //LOG_DEBUG("StateChangeObserver::initStateObserver(), connecting to state" << std::endl);
    conn_ = state->connect_state_changed(boost::bind(&StateChangeObserver::stateChanged, this));
  }
}

void StateChangeObserver::stateChanged()
{
  //LOG_DEBUG("StateChangeObserver::stateChanged()" << std::endl);
  stateChanged_ = true;
}

void StateChangeObserver::resetStateChanged()
{
  //LOG_DEBUG("StateChangeObserver::resetStateChanged()" << std::endl);
  stateChanged_ = false;
}

bool StateChangeObserver::newStatePresent() const
{
  return stateChanged_;
}

static const ModuleStateInterface::Name metadataKey("metadata");

MetadataMap::MetadataMap(ModuleStateHandle& state) : state_(state)
{
  StringMap stringMap;
  state_->setTransientValue(metadataKey, stringMap, false);
}

std::string MetadataMap::getMetadata(const std::string& key) const
{
  auto map = transient_value_cast<StringMap>(state_->getTransientValue(metadataKey));
  auto iter = map.find(key);
  return iter != map.end() ? iter->second : ("[key not found : " + key + "]");
}

void MetadataMap::setMetadata(const std::string& key, const std::string& value)
{
  auto map = transient_value_cast<StringMap>(state_->getTransientValue(metadataKey));
  map[key] = value;
  state_->setTransientValue(metadataKey, map, false);
}

StringMap MetadataMap::getFullMap() const
{
  return transient_value_cast<StringMap>(state_->getTransientValue(metadataKey));
}
