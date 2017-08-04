#ifndef SPIRE_SYSTEM_SYSTEMFACTORY_HPP
#define SPIRE_SYSTEM_SYSTEMFACTORY_HPP

#include <memory>
#include <stdexcept>
#include <entity-system/ESCoreBase.hpp>

namespace spire {

/// System factory.
class SystemFactory
{
  typedef std::shared_ptr<spire::BaseSystem> (*ClassFactoryFunPtr)();
public:

  template <typename T>
  void registerSystem(const char* name)
  {
    ClassFactoryFunPtr fun = &createSystem<T>;
    auto ret = mMap.insert(std::make_pair(name, fun));
    if (!std::get<1>(ret))
    {
      std::cerr << "es-systems: Duplicate system name: " << name << std::endl;
      throw std::runtime_error("Duplicate system name.");
    }
  }

  /// new's a new system based purely on name.
  /// Warns and returns a nullptr if \p name is not found.
  std::shared_ptr<spire::BaseSystem> newSystemFromName(const char* name);

  /// True if the system with the given name exists in our map.
  bool hasSystem(const char* name);

  /// Clear registered systems.
  void clearSystems() {mMap.clear();}

private:

  template<typename T>
  static std::shared_ptr<spire::BaseSystem> createSystem()
  {
    return std::shared_ptr<spire::BaseSystem>(new T);
  }

  std::map<std::string, ClassFactoryFunPtr> mMap;
};

} // namespace spire 

#endif 
