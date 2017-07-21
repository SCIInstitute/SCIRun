#include <stdexcept>

#include "SystemFactory.hpp"

namespace CPM_ES_SYSTEMS_NS {

bool SystemFactory::hasSystem(const char* name)
{
  auto it = mMap.find(name);
  return (it != mMap.end());
}

std::shared_ptr<CPM_ES_NS::BaseSystem> SystemFactory::newSystemFromName(const char* name)
{
  auto it = mMap.find(name);
  if (it != mMap.end())
  {
    return it->second();
  }
  else
  {
    std::cerr << "cpm-es-systems: Unable to find system from name. Name: " << name << std::endl;
    throw std::runtime_error("cpm-es-systems: Unable to find system from name.");
    return nullptr;
  }
}

} // namespace CPM_ES_SYSTEMS_NS 

