#ifndef IAUNS_SYSTEM_SYSTEMFACTORY_HPP
#define IAUNS_SYSTEM_SYSTEMFACTORY_HPP

#include <memory>
#include <stdexcept>
#include <entity-system/ESCoreBase.hpp>

namespace CPM_ES_SYSTEMS_NS {

/// System factory.
class SystemFactory
{
  typedef std::shared_ptr<CPM_ES_NS::BaseSystem> (*ClassFactoryFunPtr)();
public:

  template <typename T>
  void registerSystem(const char* name)
  {
    ClassFactoryFunPtr fun = &createSystem<T>;
    auto ret = mMap.insert(std::make_pair(name, fun));
    if (std::get<1>(ret) == false)
    {
      std::cerr << "cpm-es-systems: Duplicate system name: " << name << std::endl;
      throw std::runtime_error("Duplicate system name.");
    }
  }

  /// new's a new system based purely on name.
  /// Warns and returns a nullptr if \p name is not found.
  std::shared_ptr<CPM_ES_NS::BaseSystem> newSystemFromName(const char* name);

  /// True if the system with the given name exists in our map.
  bool hasSystem(const char* name);

  /// Clear registered systems.
  void clearSystems() {mMap.clear();}

private:

  template<typename T>
  static std::shared_ptr<CPM_ES_NS::BaseSystem> createSystem()
  {
    return std::shared_ptr<CPM_ES_NS::BaseSystem>(new T);
  }

  std::map<std::string, ClassFactoryFunPtr> mMap;
};

} // namespace CPM_ES_SYSTEMS_NS 

#endif 
