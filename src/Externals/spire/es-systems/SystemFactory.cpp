#include <stdexcept>

#include "SystemFactory.hpp"

namespace spire {

bool SystemFactory::hasSystem(const char* name)
{
  auto it = mMap.find(name);
  return (it != mMap.end());
}

std::shared_ptr<spire::BaseSystem> SystemFactory::newSystemFromName(const char* name)
{
  auto it = mMap.find(name);
  if (it != mMap.end())
  {
    return it->second();
  }
  else
  {
    std::cerr << "es-systems: Unable to find system from name. Name: " << name << std::endl;
    throw std::runtime_error("es-systems: Unable to find system from name.");
    return nullptr;
  }
}

} // namespace spire 

